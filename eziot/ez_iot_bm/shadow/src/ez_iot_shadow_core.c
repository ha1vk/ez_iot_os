#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <limits.h>
#include "ezos.h"
#include "ezlist.h"
#include "cJSON.h"
#include "ezlog.h"
#include "misc.h"
#include "ez_iot_shadow.h"
#include "ez_iot_shadow_core.h"
#include "ez_iot_shadow_def.h"
#include "ez_iot_shadow_protocol.h"
#include "ez_iot_core_lowlvl.h"

typedef enum
{
    shadow_core_status_report = 0, ///< 上报所有状态
    shadow_core_status_sync,       ///< 期望状态同步
    shadow_core_status_chg,        ///< 状态变更上报
    shadow_core_status_max,        ///< 状态值上线
} shadow_core_status_e;

typedef enum
{
    shadow_prot_ver_min = 0,
    shadow_prot_ver_1,
    shadow_prot_ver_2,
    shadow_prot_ver_3,
    shadow_prot_ver_max,
} shadow_prot_ver_e;

typedef ez_int32_t (*shadow_cloud2dev_cb)(const ez_shadow_value_t *pValue, ez_shadow_business2dev_param_t *pstParam);

typedef ez_int32_t (*shadow_devcloud_cb)(ez_shadow_value_t *pValue, ez_shadow_business2cloud_param_t *pstParam);

typedef struct
{
    ez_node_t node;    ///< 双向链表的节点
    ez_char_t key[32]; /// Shadow服务的Key

    shadow_cloud2dev_cb cloud2dev;
    shadow_devcloud_cb dev2cloud;

    ez_uint32_t stat_ver;         ///< 状态版本号
    ez_bool_t need_report;        ///< 是否需要上报标志
    ez_uint32_t msg_seq;          ///< 上报时序
    ez_void_t *json_value;        ///< 缓存的value
    ezos_timespec_t report_timer; ///< 上报定时器
} node_key_t;

typedef struct
{
    ez_node_t node;              ///< 双向链表的节点
    shadow_prot_ver_e prot_ver;  ///< 协议版本号
    ez_int32_t domain_int;       ///< 领域id(int)
    ez_char_t domain_string[32]; ///< 领域id(string)
    ez_list_t key_lst;           ///< 信令路由表
} node_domain_t;

typedef struct
{
    ez_node_t node;             ///< 双向链表的节点
    ez_int32_t index;           ///< 资源id
    ez_bool_t need_sync;        ///< 是否需要同步
    ezos_timespec_t sync_timer; ///< 同步定时器
    ez_uint16_t retry_count;    ///< 同步请求重试计数
    ez_list_t domain_lst;       ///< 领域列表
} node_index_t;

typedef struct
{
    ez_node_t node;      ///< 双向链表的节点
    ez_char_t type[32];  ///< 资源类型
    ez_list_t index_lst; ///< 资源列表
} node_index_set_t;

typedef struct
{
    ez_node_t node;       ///< 双向链表的节点
    ez_char_t dev_sn[48]; ///< 设备序列号
    ez_list_t type_lst;   ///< 资源类型列表
} node_dev_t;

/* shadow线程句柄 */
static ez_thread_t g_thread_handle = NULL;

/* 线程运行标志位 */
static ez_bool_t g_running = ez_false;

/* 设备是否在线 */
static ez_bool_t g_is_online = ez_false;

/* 信号量句柄 */
static ez_sem_t *g_sem = NULL;

static ez_char_t *g_p2c_buf = NULL;

static ez_shadow_notice g_notice_pfunc = NULL;

/* shadow 状态*/
static shadow_core_status_e g_shadow_status = shadow_core_status_report;

static ez_bool_t g_need_reset = ez_true;
static ez_bool_t g_need_report = ez_false;
static ez_bool_t g_need_sync = ez_false;
//< shadow module manager class
static ez_list_t g_shaodw_modules;

static node_dev_t *shadow_module_find_dev(ez_list_t *lst, ez_char_t *sn);
static node_index_set_t *shadow_module_find_indexs(ez_list_t *lst, ez_char_t *res_type);
static node_index_t *shadow_module_find_index(ez_list_t *lst, ez_int16_t index);
static node_domain_t *shadow_module_find_domain(ez_list_t *lst, ez_char_t *domain_id);
static node_key_t *shadow_module_find_key(ez_list_t *lst, ez_char_t *key);

static node_dev_t *shadow_module_add_dev(ez_list_t *lst, ez_char_t *sn);
static node_index_set_t *shadow_module_add_index_set(ez_list_t *lst, ez_char_t *res_type);
static node_index_t *shadow_module_add_index(ez_list_t *lst, ez_int16_t index);
static node_domain_t *shadow_module_add_domain(ez_list_t *lst, ez_char_t *domain_id, ez_uint16_t props_num, ez_void_t *props, shadow_prot_ver_e ver);

static ez_void_t *shadow_module_get_next(ez_list_t *lst, ez_node_t *node);

/* shadow 线程函数 */
static ez_void_t shadow_core_yeild(ez_void_t *pparam);

/* 主流程函数 */
static ez_int32_t shadow_proc_do();

/* 强制上报所有状态 */
static ez_int32_t shadow_proc_report();

/* 发起期望状态同步 */
static ez_int32_t shadow_proc_sync();

/* 状态变更上报 */
static ez_int32_t shadow_proc_chg();

/* 检测模块是否被重置 */
static ez_void_t check_status_update();

/* 重置所有状态 */
static ez_void_t shadow_status_reset();

static ez_void_t shadow_status2sync();

static ez_void_t shadow_reply2report(ez_uint32_t seq, ez_int32_t code);

static ez_void_t shadow_reply2query(ez_uchar_t *devsn, ez_char_t *res_type, ez_int32_t index, ez_char_t *payload);

static ez_int32_t shadow_set2dev(ez_uchar_t *devsn, ez_char_t *res_type, ez_int32_t index, ez_char_t *payload);

static ez_void_t shadow_status_sync_disable(ez_uchar_t *devsn, ez_char_t *res_type, ez_int32_t index);

/**
 * @brief 执行上报
 * 
 * @param devsn 
 * @param res_type 
 * @param index 
 * @param domain 
 * @param key 
 * @return ez_int32_t -1上报失败，0不需要上报，1表示正在上报中
 */
static ez_int32_t do_report(ez_uchar_t *devsn, ez_char_t *res_type, ez_int32_t index, ez_char_t *domain, node_key_t *node_key);

static ez_int32_t do_set(ez_uchar_t *devsn, ez_char_t *res_type, ez_int32_t index, ez_char_t *domain, ez_char_t *key, cJSON *value);

static cJSON *tlv2json(ez_shadow_value_t *ptlv);

static ez_int32_t json2tlv(cJSON *pvalue, ez_shadow_value_t *ptlv);

/**
 * @brief 判断json number是int或double
 * 
 * @param a pvalue->valueint
 * @param b pvalue->valuedouble
 * @return ez_true double
 * @return ez_false int
 */
static ez_bool_t is_double(int a, double b);

/**
 * @brief destroy the objs obtained from json2tlv
 * 
 * @param ptlv 
 */
static ez_void_t tlv_destroy(ez_shadow_value_t *ptlv);

static ez_void_t *g_hlock = NULL;
static ez_bool_t shadow_module_init();
static ez_void_t shadow_module_lock();
static ez_void_t shadow_module_unlock();
static ez_void_t shadow_module_deinit();

ez_bool_t shadow_core_start(ez_shadow_notice pfunc)
{
    if (NULL == g_sem && NULL == (g_sem = ezos_sem_create(0, 1)))
    {
        return ez_false;
    }

    if (!shadow_module_init())
    {
        return ez_false;
    }

    g_running = ez_true;
    g_notice_pfunc = pfunc;
    ezlist_init(&g_shaodw_modules);

    if (ezos_thread_create(&g_thread_handle, "ez_shadow_core", shadow_core_yeild, g_sem,
                           CONFIG_EZIOT_SHADOW_STACK_SIZE, CONFIG_EZIOT_SHADOW_TASK_PRIORITY))
    {
        ezlog_e(TAG_SHD, "shadow thread create error");
        ezos_sem_destroy(g_sem);
        g_running = ez_false;
        return ez_false;
    }

    g_p2c_buf = ezos_malloc(SHADOW_PUSH_BUF_MAX);
    if (NULL == g_p2c_buf)
    {
        g_running = ez_false;
        return ez_false;
    }
    ezos_memset(g_p2c_buf, 0, SHADOW_PUSH_BUF_MAX);

    return ez_true;
}

ez_void_t shadow_core_stop()
{
    do
    {
        if (ez_false == g_running)
            break;

        g_running = ez_false;
        ezos_sem_post(g_sem);
        ezos_thread_destroy(g_thread_handle);
        ezos_sem_destroy(g_sem);
        shadow_module_deinit();

        if (NULL != g_p2c_buf)
        {
            ezos_free(g_p2c_buf);
        }

        g_sem = NULL;
        g_thread_handle = NULL;
    } while (0);
}

static ez_void_t shadow_core_yeild(ez_void_t *pparam)
{
    ez_void_t *hsem = pparam;
    ez_int32_t proc_result = -1;

    while (g_running)
    {
        if (!g_is_online)
        {
            /* 设备不在线或者未绑定，则不进行上报 */
            ezlog_v(TAG_SHD, "!online");
            ezos_delay_ms(1000);
            continue;
        }

        /* 如果上次处理完成才会等待信号量 */
        if (0 == proc_result)
        {
            if (-1 == ezos_sem_wait(hsem, CONFIG_EZIOT_SHADOW_FORCE_FULL_SYNC_INTERVAL * 1000))
            {
                /* 长时间状态没变化，强制同步 */
                shadow_status_reset();
            }

            ezlog_d(TAG_SHD, "get semaphore");
        }

        proc_result = shadow_proc_do();
        if (0 != proc_result)
        {
            ezlog_v(TAG_SHD, "proc_result:%d", proc_result);
            ezos_delay_ms(1000);
        }
    };
}

static ez_int32_t shadow_proc_do()
{
    ez_int32_t ret = -1;
    check_status_update();

    ezlog_v(TAG_SHD, "shadow_proc_do, status:%d", g_shadow_status);

    switch (g_shadow_status)
    {
    case shadow_core_status_report:
        ret = shadow_proc_report();
        break;
    case shadow_core_status_sync:
        ret = shadow_proc_sync();
        break;
    case shadow_core_status_chg:
        ret = shadow_proc_chg();
        break;
    default:
        break;
    }

    if (0 == ret)
    {
        shadow_module_lock();
        if (g_shadow_status < shadow_core_status_max - 1)
        {
            ret = 2;
            ezlog_w(TAG_SHD, "status chg++, %d", g_shadow_status++);
        }
        shadow_module_unlock();
    }

    return ret;
}

ez_void_t shadow_core_event_occured(shadow_event_type_e event_type)
{
    ezlog_w(TAG_SHD, "set semaphore, t:%d", event_type);

    switch (event_type)
    {
    case SHADOW_EVENT_TYPE_RESET:
        shadow_module_lock();
        g_need_reset = ez_true;
        g_is_online = ez_true;
        shadow_module_unlock();
        break;
    case SHADOW_EVENT_TYPE_ONLINE:
        shadow_module_lock();
        g_need_report = ez_true;
        g_is_online = ez_true;
        shadow_module_unlock();
        break;
    case SHADOW_EVENT_TYPE_ADD:
        shadow_module_lock();
        g_need_report = ez_true;
        shadow_module_unlock();
        break;
    case SHADOW_EVENT_TYPE_OFFLINE:
        shadow_module_lock();
        g_need_sync = ez_true;
        g_is_online = ez_false;
        shadow_module_unlock();
        break;
    case SHADOW_EVENT_TYPE_REPORT:
        //< 防止同步期间服务没响应，设备发生状态变更不能上报的问题
        if (shadow_core_status_sync == g_shadow_status)
        {
            shadow_module_lock();
            g_need_report = ez_true;
            shadow_module_unlock();
        }
    case SHADOW_EVENT_TYPE_RECV:
        /* do nothing */
        break;
    default:
        break;
    }

    if (!g_running || NULL == g_sem)
        return;

    ezos_sem_post(g_sem);
}

static ez_void_t check_status_update()
{
    ez_int32_t status_chg = -1;

    if (g_need_reset)
    {
        g_notice_pfunc(EZ_EVENT_FULL_REPORT, NULL, 0);
        shadow_status_reset();
        status_chg = shadow_core_status_report;
    }
    else if (g_need_sync)
    {
        shadow_status2sync();
        status_chg = shadow_core_status_report;
    }
    else if (g_need_report)
    {
        status_chg = shadow_core_status_report;
    }

    if (-1 == status_chg)
    {
        return;
    }

    shadow_module_lock();
    g_need_reset = ez_false;
    g_need_sync = ez_false;
    g_need_report = ez_false;
    ezlog_w(TAG_SHD, "status chg, %d to %d", g_shadow_status, status_chg);
    g_shadow_status = (shadow_core_status_e)status_chg;
    shadow_module_unlock();
}

static ez_int32_t shadow_proc_report()
{
    ez_int32_t rv = 0;
    ez_int32_t total_remaining = 0;

    ez_node_t *node_dev = NULL;
    ez_node_t *node_index_set = NULL;
    ez_node_t *node_index = NULL;
    ez_node_t *node_domain = NULL;
    ez_node_t *node_key = NULL;

    node_dev_t *pnode_dev = NULL;
    node_index_set_t *pnode_index_set = NULL;
    node_index_t *pnode_index = NULL;
    node_domain_t *pnode_domain = NULL;
    node_key_t *pnode_key = NULL;

    ezlog_w(TAG_SHD, "func report");

    shadow_module_lock();

    while (NULL != (node_dev = shadow_module_get_next(&g_shaodw_modules, node_dev)))
    {
        pnode_dev = (node_dev_t *)node_dev;
        node_index_set = NULL;

        ezlog_v(TAG_SHD, "found dev:%s", pnode_dev->dev_sn);
        while (NULL != (node_index_set = shadow_module_get_next(&pnode_dev->type_lst, node_index_set)))
        {
            pnode_index_set = (node_index_set_t *)node_index_set;
            node_index = NULL;

            ezlog_v(TAG_SHD, "found index set:%s", pnode_index_set->type);
            while (NULL != (node_index = shadow_module_get_next(&pnode_index_set->index_lst, node_index)))
            {
                pnode_index = (node_index_t *)node_index;
                node_domain = NULL;

                ezlog_v(TAG_SHD, "found index:%d", pnode_index->index);
                while (NULL != (node_domain = shadow_module_get_next(&pnode_index->domain_lst, node_domain)))
                {
                    pnode_domain = (node_domain_t *)node_domain;
                    node_key = NULL;

                    ezlog_v(TAG_SHD, "found domain:%s", pnode_domain->domain_string);
                    while (NULL != (node_key = shadow_module_get_next(&pnode_domain->key_lst, node_key)))
                    {
                        pnode_key = (node_key_t *)node_key;
                        ezlog_v(TAG_SHD, "found key:%s", pnode_key->key);

                        ez_int32_t result = do_report(pnode_dev->dev_sn, pnode_index_set->type, pnode_index->index, pnode_domain->domain_string, pnode_key);
                        if (-1 == result)
                        {
                            rv = -1;
                            goto done;
                        }
                        else if (1 == result)
                        {
                            total_remaining++;
                        }
                    }
                }
            }
        }
    }

    if (total_remaining)
    {
        ezlog_v(TAG_SHD, "total_remaining:%d", total_remaining);
        rv = 1;
    }

done:
    shadow_module_unlock();

    if (0 != rv)
    {
        ezos_delay_ms(1000);
    }

    return rv;
}

static ez_int32_t shadow_proc_sync()
{
    ez_int32_t rv = 0;
    ez_int32_t total_remaining = 0;

    ez_node_t *node_dev = NULL;
    ez_node_t *node_index_set = NULL;
    ez_node_t *node_index = NULL;

    node_dev_t *pnode_dev = NULL;
    node_index_set_t *pnode_index_set = NULL;
    node_index_t *pnode_index = NULL;

    ezlog_w(TAG_SHD, "func sync");

    shadow_module_lock();

    while (NULL != (node_dev = shadow_module_get_next(&g_shaodw_modules, node_dev)))
    {
        pnode_dev = (node_dev_t *)node_dev;
        node_index_set = NULL;

        ezlog_d(TAG_SHD, "found dev:%s", pnode_dev->dev_sn);
        while (NULL != (node_index_set = shadow_module_get_next(&pnode_dev->type_lst, node_index_set)))
        {
            pnode_index_set = (node_index_set_t *)node_index_set;
            node_index = NULL;

            ezlog_d(TAG_SHD, "found index set:%s", pnode_index_set->type);
            while (NULL != (node_index = shadow_module_get_next(&pnode_index_set->index_lst, node_index)))
            {
                pnode_index = (node_index_t *)node_index;

                if (0 == pnode_index->need_sync)
                {
                    continue;
                }

                if (0 != pnode_index->sync_timer.tv_sec)
                {
                    if (!time_isexpired(&pnode_index->sync_timer))
                    {
                        ezlog_d(TAG_SHD, "wait4sync rsp");
                        total_remaining++;
                        continue;
                    }

                    ezlog_e(TAG_SHD, "wait4sync rsp timeout");
                    ezos_bzero(&pnode_index->sync_timer, sizeof(pnode_index->sync_timer));
                }

                ///< 如果同步超过最大次数，则不再进行
                if (++pnode_index->retry_count > CONFIG_EZIOT_SHADOW_FULL_SYNC_RETRY_MAX)
                {
                    pnode_index->retry_count = 0;
                    pnode_index->need_sync = 0;
                    continue;
                }

                if (shadow_protocol_query_desired(pnode_dev->dev_sn, pnode_index_set->type, pnode_index->index))
                {
                    ezlog_e(TAG_SHD, "query req");
                    rv = -1;
                    break;
                }

                total_remaining++;
                time_countdown(&pnode_index->sync_timer, CONFIG_EZIOT_SHADOW_FULL_SYNC_RETRY_INTERVAL);
            }
        }
    }

    if (total_remaining)
    {
        ezlog_v(TAG_SHD, "total_remaining:%d", total_remaining);
        rv = 1;
    }

    shadow_module_unlock();

    if (0 != rv)
    {
        ezos_delay_ms(1000);
    }

    return rv;
}

static ez_int32_t shadow_proc_chg()
{
    return shadow_proc_report();
}

static ez_void_t shadow_status_reset()
{
    ez_node_t *node_dev = NULL;
    ez_node_t *node_index_set = NULL;
    ez_node_t *node_index = NULL;
    ez_node_t *node_domain = NULL;
    ez_node_t *node_key = NULL;

    node_dev_t *pnode_dev = NULL;
    node_index_set_t *pnode_index_set = NULL;
    node_index_t *pnode_index = NULL;
    node_domain_t *pnode_domain = NULL;
    node_key_t *pnode_key = NULL;

    shadow_module_lock();

    while (NULL != (node_dev = shadow_module_get_next(&g_shaodw_modules, node_dev)))
    {
        pnode_dev = (node_dev_t *)node_dev;
        node_index_set = NULL;

        ezlog_v(TAG_SHD, "found dev:%s", pnode_dev->dev_sn);
        while (NULL != (node_index_set = shadow_module_get_next(&pnode_dev->type_lst, node_index_set)))
        {
            pnode_index_set = (node_index_set_t *)node_index_set;
            node_index = NULL;

            ezlog_v(TAG_SHD, "found index set:%s", pnode_index_set->type);
            while (NULL != (node_index = shadow_module_get_next(&pnode_index_set->index_lst, node_index)))
            {
                pnode_index = (node_index_t *)node_index;
                node_domain = NULL;
                pnode_index->need_sync = 1;
                ezos_bzero(&pnode_index->sync_timer, sizeof(pnode_index->sync_timer));

                ezlog_v(TAG_SHD, "found index:%d", pnode_index->index);
                while (NULL != (node_domain = shadow_module_get_next(&pnode_index->domain_lst, node_domain)))
                {
                    pnode_domain = (node_domain_t *)node_domain;
                    node_key = NULL;

                    ezlog_v(TAG_SHD, "found domain:%s", pnode_domain->domain_string);
                    while (NULL != (node_key = shadow_module_get_next(&pnode_domain->key_lst, node_key)))
                    {
                        pnode_key = (node_key_t *)node_key;
                        ezlog_v(TAG_SHD, "found key:%s", pnode_key->key);
                        pnode_key->need_report = 1;
                        ezos_bzero(&pnode_index->sync_timer, sizeof(pnode_index->sync_timer));
                    }
                }
            }
        }
    }

    shadow_module_unlock();
}

static ez_void_t shadow_status2sync()
{
    ez_node_t *node_dev = NULL;
    ez_node_t *node_index_set = NULL;
    ez_node_t *node_index = NULL;

    node_dev_t *pnode_dev = NULL;
    node_index_set_t *pnode_index_set = NULL;
    node_index_t *pnode_index = NULL;

    shadow_module_lock();

    while (NULL != (node_dev = shadow_module_get_next(&g_shaodw_modules, node_dev)))
    {
        pnode_dev = (node_dev_t *)node_dev;
        node_index_set = NULL;

        ezlog_d(TAG_SHD, "found dev:%s", pnode_dev->dev_sn);
        while (NULL != (node_index_set = shadow_module_get_next(&pnode_dev->type_lst, node_index_set)))
        {
            pnode_index_set = (node_index_set_t *)node_index_set;
            node_index = NULL;

            ezlog_d(TAG_SHD, "found index set:%s", pnode_index_set->type);
            while (NULL != (node_index = shadow_module_get_next(&pnode_index_set->index_lst, node_index)))
            {
                pnode_index = (node_index_t *)node_index;
                pnode_index->need_sync = 1;
                ezos_bzero(&pnode_index->sync_timer, sizeof(pnode_index->sync_timer));
                pnode_index->retry_count = 0;
            }
        }
    }

    shadow_module_unlock();
}

static ez_int32_t do_report(ez_uchar_t *devsn, ez_char_t *res_type, ez_int32_t index, ez_char_t *domain, node_key_t *node_key)
{
    ez_int32_t rv = 0;
    ez_shadow_value_t tlv = {0};
    cJSON *json_value = NULL;
    ez_shadow_business2cloud_param_t stGenParam = {0};
    stGenParam.pdomain = domain;
    stGenParam.pkey = node_key->key;

    ezos_strncpy(stGenParam.pres.res_type, res_type, sizeof(stGenParam.pres.res_type) - 1);
    stGenParam.pres.local_index = index;

    if (0 == ezos_strcmp(SHADOW_DEFAULT_NAME, devsn))
    {
        ezos_strncpy(stGenParam.pres.dev_serial, ez_kernel_getdevinfo_bykey("dev_subserial"), sizeof(stGenParam.pres.dev_serial) - 1);
    }
    else
    {
        ezos_strncpy(stGenParam.pres.dev_serial, devsn, sizeof(stGenParam.pres.dev_serial) - 1);
    }

    do
    {
        if (!node_key->need_report)
        {
            ezlog_v(TAG_SHD, "no need report");
            break;
        }

        if (0 != node_key->report_timer.tv_sec &&
            !time_isexpired(&node_key->report_timer))
        {
            rv = 1;
            break;
        }

        if (node_key->json_value)
        {
            /* 通过ez_iot_shadow_push_value_v3接口上报 */
            json_value = cJSON_Duplicate(node_key->json_value, 1);
        }
        else
        {
            if (NULL == node_key->dev2cloud)
            {
                ezlog_d(TAG_SHD, "no need report2");
                node_key->need_report = 0;
                break;
            }

            tlv.value = g_p2c_buf;
            tlv.length = SHADOW_PUSH_BUF_MAX;
            if (NULL == tlv.value)
            {
                ezlog_e(TAG_SHD, "do_report malloc");
                rv = -1;
                break;
            }

            ezos_memset(tlv.value, 0, SHADOW_PUSH_BUF_MAX);
            if (0 != node_key->dev2cloud(&tlv, &stGenParam))
            {
                ezlog_e(TAG_SHD, "dev2cloud cb err");
                node_key->need_report = 0;
                break;
            }

            if (NULL == (json_value = tlv2json(&tlv)))
            {
                ezlog_e(TAG_SHD, "tlv2json");
                rv = -1;
                break;
            }
        }

        ez_uint32_t seq = 0;
        if (shadow_protocol_report(devsn, res_type, index, domain, node_key->key, json_value, node_key->stat_ver, &seq))
        {
            ezlog_e(TAG_SHD, "report v3%d", rv);
            rv = -1;
            break;
        }

        ezlog_v(TAG_SHD, "send seq:%d", seq);
        node_key->msg_seq = seq;
        time_countdown(&node_key->report_timer, CONFIG_EZIOT_SHADOW_FULL_SYNC_RETRY_INTERVAL);
    } while (0);

    return rv;
}

ez_err_t shadow_core_module_addv3(ez_char_t *sn, ez_char_t *res_type, ez_int16_t index, ez_char_t *domain_id, ez_uint16_t props_num, ez_void_t *props)
{
    ez_err_t rv = EZ_SHD_ERR_MEMORY;
    node_dev_t *pnode_dev;
    node_index_set_t *pnode_indexs;
    node_index_t *pnode_index;
    ez_char_t *dev_sn = sn;

    if (0 == ezos_strcmp(ez_kernel_getdevinfo_bykey("dev_subserial"), sn))
    {
        dev_sn = SHADOW_DEFAULT_NAME;
    }

    ezlog_v(TAG_SHD, "addv3, sn:%s", dev_sn);
    ezlog_v(TAG_SHD, "addv3, rtype:%s", res_type);
    ezlog_v(TAG_SHD, "addv3, index:%d", index);
    ezlog_v(TAG_SHD, "addv3, domain:%s", domain_id);
    ezlog_v(TAG_SHD, "addv3: num:%d", props_num);

    shadow_module_lock();

    do
    {
        pnode_dev = shadow_module_find_dev(&g_shaodw_modules, dev_sn);
        if (NULL == pnode_dev && NULL == (pnode_dev = shadow_module_add_dev(&g_shaodw_modules, dev_sn)))
        {
            ezlog_e(TAG_SHD, "add dev err");
            break;
        }

        pnode_indexs = shadow_module_find_indexs(&pnode_dev->type_lst, res_type);
        if (NULL == pnode_indexs && NULL == (pnode_indexs = shadow_module_add_index_set(&pnode_dev->type_lst, res_type)))
        {
            ezlog_e(TAG_SHD, "add index set err");
            break;
        }

        pnode_index = shadow_module_find_index(&pnode_indexs->index_lst, index);
        if (NULL == pnode_index && NULL == (pnode_index = shadow_module_add_index(&pnode_indexs->index_lst, index)))
        {
            ezlog_e(TAG_SHD, "add index err");
            break;
        }

        if (NULL == shadow_module_add_domain(&pnode_index->domain_lst, domain_id, props_num, props, shadow_prot_ver_3))
        {
            ezlog_e(TAG_SHD, "add domain err");
            break;
        }

        rv = EZ_SHD_ERR_SUCC;
    } while (0);

    ezlog_d(TAG_SHD, "shadow_core_module_addv3, dev count:%d", ezlist_get_size(&g_shaodw_modules));

    shadow_module_unlock();

    return rv;
}

ez_err_t shadow_core_module_clear(ez_char_t *sn)
{
    ez_node_t *node_dev = NULL;
    ez_node_t *node_index_set = NULL;
    ez_node_t *node_index = NULL;
    ez_node_t *node_domain = NULL;
    ez_node_t *node_key = NULL;

    node_dev_t *pnode_dev = NULL;
    node_index_set_t *pnode_index_set = NULL;
    node_index_t *pnode_index = NULL;
    node_domain_t *pnode_domain = NULL;
    node_key_t *pnode_key = NULL;
    ez_char_t *dev_sn = sn;

    if (0 == ezos_strcmp(ez_kernel_getdevinfo_bykey("dev_subserial"), sn))
    {
        dev_sn = SHADOW_DEFAULT_NAME;
    }

    shadow_module_lock();

    pnode_dev = shadow_module_find_dev(&g_shaodw_modules, dev_sn);
    if (!pnode_dev)
    {
        goto done;
    }

    while (NULL != (node_index_set = shadow_module_get_next(&pnode_dev->type_lst, node_index_set)))
    {
        pnode_index_set = (node_index_set_t *)node_index_set;
        node_index = NULL;

        while (NULL != (node_index = shadow_module_get_next(&pnode_index_set->index_lst, node_index)))
        {
            pnode_index = (node_index_t *)node_index;
            node_domain = NULL;

            while (NULL != (node_domain = shadow_module_get_next(&pnode_index->domain_lst, node_domain)))
            {
                pnode_domain = (node_domain_t *)node_domain;
                node_key = NULL;

                while (NULL != (node_key = shadow_module_get_next(&pnode_domain->key_lst, node_key)))
                {
                    pnode_key = (node_key_t *)node_key;
                    if (pnode_key->json_value)
                    {
                        cJSON_Delete((cJSON *)pnode_key->json_value);
                        pnode_key->json_value = NULL;
                    }

                    ezlist_delete(&pnode_domain->key_lst, node_key);
                    ezos_free(node_key);
                    node_key = NULL;
                }

                ezlist_delete(&pnode_index->domain_lst, node_domain);
                ezos_free(node_domain);
                node_domain = NULL;
            }

            ezlist_delete(&pnode_index_set->index_lst, node_index);
            ezos_free(node_index);
            node_index = NULL;
        }

        ezlist_delete(&pnode_dev->type_lst, node_index_set);
        ezos_free(node_index_set);
        node_index_set = NULL;
    }

    ezlist_delete(&g_shaodw_modules, &pnode_dev->node);
    ezos_free(pnode_dev);

done:
    shadow_module_unlock();

    return EZ_SHD_ERR_SUCC;
}

ez_err_t shadow_core_propertie_changed(ez_char_t *sn, ez_char_t *res_type, ez_int16_t index, ez_char_t *domain_id, ez_char_t *pkey, ez_void_t *value)
{
    ez_err_t rv = EZ_SHD_ERR_SUCC;
    node_dev_t *pnode_dev;
    node_index_set_t *pnode_indexs;
    node_index_t *pnode_index;
    node_domain_t *pnode_domain;
    node_key_t *pnode_key;
    ez_char_t *dev_sn = sn;

    if (0 == ezos_strcmp(ez_kernel_getdevinfo_bykey("dev_subserial"), sn))
    {
        dev_sn = SHADOW_DEFAULT_NAME;
    }

    shadow_module_lock();

    do
    {
        pnode_dev = shadow_module_find_dev(&g_shaodw_modules, dev_sn);
        if (NULL == pnode_dev)
        {
            ezlog_e(TAG_SHD, "dev not found!, sn:%s", dev_sn);
            break;
        }

        pnode_indexs = shadow_module_find_indexs(&pnode_dev->type_lst, res_type);
        if (NULL == pnode_indexs)
        {
            ezlog_e(TAG_SHD, "indexs not found!, t:%s", res_type);
            break;
        }

        pnode_index = shadow_module_find_index(&pnode_indexs->index_lst, index);
        if (NULL == pnode_index)
        {
            ezlog_e(TAG_SHD, "index not found!, i:%d", index);
            break;
        }
        pnode_domain = shadow_module_find_domain(&pnode_index->domain_lst, domain_id);
        if (NULL == pnode_domain)
        {
            ezlog_e(TAG_SHD, "domain not found!, t:%s", domain_id);
            break;
        }
        pnode_key = shadow_module_find_key(&pnode_domain->key_lst, pkey);
        if (NULL == pnode_key)
        {
            ezlog_e(TAG_SHD, "pkey not found!, t:%s", pkey);
            break;
        }

        ezos_bzero(&pnode_index->sync_timer, sizeof(pnode_index->sync_timer));
        pnode_key->stat_ver++;
        pnode_key->need_report = 1;
        pnode_key->msg_seq = 0;
        if (pnode_key->json_value)
        {
            cJSON_Delete((cJSON *)pnode_key->json_value);
            pnode_key->json_value = NULL;
        }

        if (NULL != value)
        {
            pnode_key->json_value = tlv2json((ez_shadow_value_t *)value);
        }

        rv = 0;
    } while (0);
    shadow_module_unlock();

    return rv;
}

ez_int32_t shadow_core_cloud_data_in(ez_void_t *shadow_res, ez_uint32_t seq, ez_char_t *business_type, ez_char_t *payload)
{
    ez_int32_t rv = -1;
    ez_shadow_res_t *pshadow_res = (ez_shadow_res_t *)shadow_res;

    ezlog_d(TAG_SHD, "data in:%s, seq:%d", business_type, seq);

    do
    {
        if (0 == ezos_strcmp("set", business_type))
        {
            rv = shadow_set2dev(pshadow_res->dev_serial, pshadow_res->res_type, pshadow_res->local_index, payload);
            shadow_protocol_set_reply(pshadow_res->dev_serial, pshadow_res->res_type, pshadow_res->local_index, rv, seq);
        }
        else if (0 == ezos_strcmp("query_reply", business_type))
        {
            shadow_reply2query(pshadow_res->dev_serial, pshadow_res->res_type, pshadow_res->local_index, payload);
        }
        else if (0 == ezos_strcmp("report", business_type))
        {
            shadow_reply2report(seq, *(ez_int32_t *)payload);
        }
        else
        {
            break;
        }
    } while (0);

    shadow_core_event_occured(SHADOW_EVENT_TYPE_RECV);
    return rv;
}

static ez_bool_t shadow_module_init()
{
    g_hlock = ezos_mutex_create();
    if (NULL == g_hlock)
    {
        return ez_false;
    }

    return ez_true;
}

static ez_void_t shadow_module_lock()
{
    if (NULL == g_hlock)
    {
        return;
    }

    ezos_mutex_lock(g_hlock);
}

static ez_void_t shadow_module_unlock()
{
    if (NULL == g_hlock)
    {
        return;
    }

    ezos_mutex_unlock(g_hlock);
}

static ez_void_t shadow_module_deinit()
{
    if (NULL == g_hlock)
    {
        return;
    }

    ezos_mutex_destroy(g_hlock);
    g_hlock = NULL;
}

static node_dev_t *shadow_module_find_dev(ez_list_t *lst, ez_char_t *sn)
{
    node_dev_t *pnode = NULL;

    if (NULL == lst || 0 == ezlist_get_size(lst))
    {
        return NULL;
    }

    LIST_FOR_EACH(node_dev_t, pnode, lst)
    {
        if (0 != ezos_strcmp(pnode->dev_sn, sn))
        {
            continue;
        }
        else
        {
            break;
        }
    }

    return pnode;
}

static node_index_set_t *shadow_module_find_indexs(ez_list_t *lst, ez_char_t *res_type)
{
    node_index_set_t *pnode = NULL;

    if (NULL == lst || 0 == ezlist_get_size(lst))
    {
        return NULL;
    }

    LIST_FOR_EACH(node_index_set_t, pnode, lst)
    {
        if (0 != ezos_strcmp(pnode->type, res_type))
        {
            continue;
        }
        else
        {
            break;
        }
    }

    return pnode;
}

static node_index_t *shadow_module_find_index(ez_list_t *lst, ez_int16_t index)
{
    node_index_t *pnode = NULL;

    if (NULL == lst || 0 == ezlist_get_size(lst))
    {
        return NULL;
    }

    LIST_FOR_EACH(node_index_t, pnode, lst)
    {
        if (index != pnode->index)
        {
            continue;
        }
        else
        {
            break;
        }
    }

    return pnode;
}

static node_domain_t *shadow_module_find_domain(ez_list_t *lst, ez_char_t *domain_id)
{
    node_domain_t *pnode = NULL;

    if (NULL == lst || 0 == ezlist_get_size(lst))
    {
        return NULL;
    }

    LIST_FOR_EACH(node_domain_t, pnode, lst)
    {
        if (0 != ezos_strcmp(pnode->domain_string, domain_id))
        {
            continue;
        }
        else
        {
            break;
        }
    }

    return pnode;
}

static node_key_t *shadow_module_find_key(ez_list_t *lst, ez_char_t *key)
{
    node_key_t *pnode = NULL;

    if (NULL == lst || 0 == ezlist_get_size(lst))
    {
        return NULL;
    }

    LIST_FOR_EACH(node_key_t, pnode, lst)
    {
        if (0 != ezos_strcmp(pnode->key, key))
        {
            continue;
        }
        else
        {
            break;
        }
    }

    return pnode;
}

static node_dev_t *shadow_module_add_dev(ez_list_t *lst, ez_char_t *sn)
{
    node_dev_t *pnode = NULL;

    if (NULL == lst)
    {
        return NULL;
    }

    pnode = shadow_module_find_dev(lst, sn);
    if (NULL != pnode)
    {
        return pnode;
    }

    pnode = (node_dev_t *)malloc(sizeof(node_dev_t));
    if (NULL == pnode)
    {
        return NULL;
    }

    ezos_memset(pnode, 0, sizeof(node_dev_t));
    ezos_strncpy(pnode->dev_sn, sn, sizeof(pnode->dev_sn) - 1);
    ezlist_add_last(lst, &pnode->node);

    return pnode;
}

static node_index_set_t *shadow_module_add_index_set(ez_list_t *lst, ez_char_t *res_type)
{
    node_index_set_t *pnode = NULL;

    if (NULL == lst)
    {
        return NULL;
    }

    pnode = shadow_module_find_indexs(lst, res_type);
    if (NULL != pnode)
    {
        return pnode;
    }

    pnode = (node_index_set_t *)malloc(sizeof(node_index_set_t));
    if (NULL == pnode)
    {
        return NULL;
    }

    ezos_memset(pnode, 0, sizeof(node_index_set_t));
    if (0 == ezos_strlen(res_type))
    {
        ezos_strncpy(pnode->type, SHADOW_DEFAULT_NAME, sizeof(pnode->type) - 1);
    }
    else
    {
        ezos_strncpy(pnode->type, res_type, sizeof(pnode->type) - 1);
    }

    ezlist_add_last(lst, &pnode->node);

    return pnode;
}

static node_index_t *shadow_module_add_index(ez_list_t *lst, ez_int16_t index)
{
    node_index_t *pnode = NULL;

    if (NULL == lst)
    {
        return NULL;
    }

    pnode = shadow_module_find_index(lst, index);
    if (NULL != pnode)
    {
        return pnode;
    }

    pnode = (node_index_t *)malloc(sizeof(node_index_t));
    if (NULL == pnode)
    {
        return NULL;
    }

    ezos_memset(pnode, 0, sizeof(node_index_t));
    pnode->index = index;
    pnode->need_sync = 1;
    pnode->retry_count = 0;
    ezlist_add_last(lst, &pnode->node);

    return pnode;
}

static node_domain_t *shadow_module_add_domain(ez_list_t *lst, ez_char_t *domain_id, ez_uint16_t props_num, ez_void_t *props, shadow_prot_ver_e ver)
{
    node_domain_t *rv = NULL;
    node_domain_t *pnode_domain = NULL;
    ez_shadow_business_t *pservices = (ez_shadow_business_t *)props;
    size_t i;

    if (NULL == lst || NULL == domain_id || 0 == props_num || NULL == props)
    {
        return NULL;
    }

    do
    {
        pnode_domain = shadow_module_find_domain(lst, domain_id);
        if (NULL == pnode_domain)
        {
            pnode_domain = (node_domain_t *)malloc(sizeof(node_domain_t));
            if (NULL == pnode_domain)
            {
                break;
            }

            ezos_memset(pnode_domain, 0, sizeof(node_domain_t));
            ezos_strncpy(pnode_domain->domain_string, domain_id, sizeof(pnode_domain->domain_string) - 1);
            pnode_domain->prot_ver = ver;
            ezlist_add_last(lst, &pnode_domain->node);
        }

        for (i = 0; i < props_num; i++)
        {
            node_key_t *pnode_key = shadow_module_find_key(&pnode_domain->key_lst, pservices[i].key);
            if (pnode_key)
            {
                if (pnode_key->cloud2dev != pservices[i].business2dev)
                {
                    pnode_key->cloud2dev = pservices[i].business2dev;
                }

                if (pnode_key->dev2cloud != pservices[i].business2cloud)
                {
                    pnode_key->dev2cloud = pservices[i].business2cloud;
                }

                pnode_key->need_report = 1;
                continue;
            }
            else
            {
                pnode_key = (node_key_t *)malloc(sizeof(node_key_t));
                ezlog_v(TAG_SHD, "addv3: key:%s", pservices[i].key);
                ezos_memset(pnode_key, 0, sizeof(node_key_t));
                ezos_strncpy(pnode_key->key, pservices[i].key, sizeof(pnode_key->key) - 1);
                pnode_key->cloud2dev = pservices[i].business2dev;
                pnode_key->dev2cloud = pservices[i].business2cloud;
                pnode_key->stat_ver = 0;
                pnode_key->msg_seq = 0;
                pnode_key->need_report = 1;

                ezlist_add_last(&pnode_domain->key_lst, &pnode_key->node);
            }
        }

        rv = pnode_domain;
    } while (0);

    return rv;
}

static ez_void_t *shadow_module_get_next(ez_list_t *lst, ez_node_t *node)
{
    if (NULL == lst)
    {
        return NULL;
    }

    if (NULL == node)
    {
        return ezlist_get_first(lst);
    }

    return ezlist_get_next(node);
}

static cJSON *tlv2json(ez_shadow_value_t *ptlv)
{
    cJSON *json_value = NULL;
    if (!ptlv || 0 == ptlv->length)
        return NULL;

    ezlog_d(TAG_SHD, "tlv type:%d", ptlv->type);
    ezlog_d(TAG_SHD, "tlv size:%d", ptlv->length);
    ezlog_d(TAG_SHD, "tlv value:%d", ptlv->value_int);

    switch (ptlv->type)
    {
    case SHD_DATA_TYPE_BOOL:
        json_value = cJSON_CreateBool(ptlv->value_int);
        break;
    case SHD_DATA_TYPE_INT:
        json_value = cJSON_CreateNumber(ptlv->value_int);
        break;
    case SHD_DATA_TYPE_DOUBLE:
        json_value = cJSON_CreateNumber(ptlv->value_double);
        break;
    case SHD_DATA_TYPE_STRING:
        json_value = cJSON_CreateString(ptlv->value);
        break;
    case SHD_DATA_TYPE_ARRAY:
    case SHD_DATA_TYPE_OBJECT:
        json_value = cJSON_Parse(ptlv->value);
        break;
    default:
        break;
    }

    return json_value;
}

static ez_void_t shadow_reply2report(ez_uint32_t seq, ez_int32_t code)
{
    ez_node_t *node_dev = NULL;
    ez_node_t *node_index_set = NULL;
    ez_node_t *node_index = NULL;
    ez_node_t *node_domain = NULL;
    ez_node_t *node_key = NULL;

    node_dev_t *pnode_dev = NULL;
    node_index_set_t *pnode_index_set = NULL;
    node_index_t *pnode_index = NULL;
    node_domain_t *pnode_domain = NULL;
    node_key_t *pnode_key = NULL;

    shadow_module_lock();

    while (NULL != (node_dev = shadow_module_get_next(&g_shaodw_modules, node_dev)))
    {
        pnode_dev = (node_dev_t *)node_dev;
        node_index_set = NULL;

        while (NULL != (node_index_set = shadow_module_get_next(&pnode_dev->type_lst, node_index_set)))
        {
            pnode_index_set = (node_index_set_t *)node_index_set;
            node_index = NULL;

            while (NULL != (node_index = shadow_module_get_next(&pnode_index_set->index_lst, node_index)))
            {
                pnode_index = (node_index_t *)node_index;
                node_domain = NULL;

                while (NULL != (node_domain = shadow_module_get_next(&pnode_index->domain_lst, node_domain)))
                {
                    pnode_domain = (node_domain_t *)node_domain;
                    node_key = NULL;

                    while (NULL != (node_key = shadow_module_get_next(&pnode_domain->key_lst, node_key)))
                    {
                        pnode_key = (node_key_t *)node_key;

                        ezlog_v(TAG_SHD, "local seq:%d, ack seq:%d, code:%d", pnode_key->msg_seq, seq, code);
                        if (pnode_key->msg_seq == seq)
                        {
                            ezlog_d(TAG_SHD, "seq match, code:%d", code);
                            if (0 == code)
                            {
                                pnode_key->need_report = 0;
                                cJSON_Delete((cJSON *)pnode_key->json_value);
                                pnode_key->json_value = NULL;
                            }

                            pnode_key->msg_seq = 0;
                            ezos_bzero(&pnode_index->sync_timer, sizeof(pnode_index->sync_timer));

                            goto done;
                        }
                    }
                }
            }
        }
    }

done:
    shadow_module_unlock();
}

static ez_int32_t shadow_set2dev(ez_uchar_t *devsn, ez_char_t *res_type, ez_int32_t index, ez_char_t *payload)
{
    cJSON *json_root = NULL;
    cJSON *json_state = NULL;
    cJSON *json_desired = NULL;
    cJSON *json_domain = NULL;
    cJSON *json_identifier = NULL;
    cJSON *json_value = NULL;

    ez_int32_t rv = -1;

    do
    {
        if (NULL == (json_root = cJSON_Parse(payload)))
        {
            break;
        }

        if (NULL == (json_state = cJSON_GetObjectItem(json_root, "state")))
        {
            break;
        }

        if (NULL == (json_desired = cJSON_GetObjectItem(json_state, "desired")))
        {
            break;
        }

        if (NULL == (json_domain = cJSON_GetObjectItem(json_desired, "domain")))
        {
            break;
        }

        if (NULL == (json_identifier = cJSON_GetObjectItem(json_desired, "identifier")))
        {
            break;
        }

        if (NULL == (json_value = cJSON_GetObjectItem(json_desired, "value")))
        {
            break;
        }

        rv = do_set(devsn, res_type, index, json_domain->valuestring, json_identifier->valuestring, json_value);
    } while (0);

    if (0 != rv)
    {
        ezlog_e(TAG_SHD, "reply invalid!");
        ezlog_d(TAG_SHD, "reply:%s", payload);
    }

    if (json_root)
    {
        cJSON_Delete(json_root);
    }

    return rv;
}

static ez_void_t shadow_status_sync_disable(ez_uchar_t *devsn, ez_char_t *res_type, ez_int32_t index)
{
    node_dev_t *pnode_dev;
    node_index_set_t *pnode_indexs;
    node_index_t *pnode_index;

    shadow_module_lock();

    do
    {
        if (NULL == (pnode_dev = shadow_module_find_dev(&g_shaodw_modules, devsn)))
        {
            ezlog_e(TAG_SHD, "dev not found!");
            break;
        }

        if (NULL == (pnode_indexs = shadow_module_find_indexs(&pnode_dev->type_lst, res_type)))
        {
            ezlog_e(TAG_SHD, "indexs not found!");
            break;
        }

        if (NULL == (pnode_index = shadow_module_find_index(&pnode_indexs->index_lst, index)))
        {
            ezlog_e(TAG_SHD, "index not found!");
            break;
        }

        pnode_index->need_sync = 0;
        ezos_bzero(&pnode_index->sync_timer, sizeof(pnode_index->sync_timer));
        pnode_index->retry_count = 0;

    } while (0);

    shadow_module_unlock();
}

static ez_void_t shadow_reply2query(ez_uchar_t *devsn, ez_char_t *res_type, ez_int32_t index, ez_char_t *payload)
{
    cJSON *json_root = NULL;
    cJSON *json_code = NULL;
    cJSON *json_state = NULL;
    cJSON *json_desired = NULL;
    cJSON *json_domain_array = NULL;
    cJSON *json_domain = NULL;
    cJSON *json_identifier = NULL;
    cJSON *json_value = NULL;

    ez_int32_t rv = 0;

    do
    {
        if (NULL == (json_root = cJSON_Parse(payload)))
        {
            rv = -1;
            break;
        }

        if (NULL == (json_code = cJSON_GetObjectItem(json_root, "code")))
        {
            rv = -1;
            break;
        }

        if (cJSON_Number != json_code->type || 0 != json_code->valueint)
        {
            //服务内部出错，不再重试
            ezlog_e(TAG_SHD, "query_reply code:%d", json_code->valueint);
            rv = -2;
            break;
        }

        if (NULL == (json_state = cJSON_GetObjectItem(json_root, "state")))
        {
            rv = -1;
            break;
        }

        if (NULL == (json_desired = cJSON_GetObjectItem(json_state, "desired")) || cJSON_Array != json_desired->type)
        {
            //没有期望状态
            break;
        }

        if (0 == cJSON_GetArraySize(json_desired))
        {
            break;
        }

        for (size_t i = 0; i < cJSON_GetArraySize(json_desired); i++)
        {
            if (NULL == (json_domain_array = cJSON_GetArrayItem(json_desired, i)))
            {
                continue;
            }

            if (NULL == (json_domain = cJSON_GetObjectItem(json_domain_array, "domain")))
            {
                rv = -1;
                continue;
            }

            if (NULL == (json_identifier = cJSON_GetObjectItem(json_domain_array, "identifier")))
            {
                rv = -1;
                continue;
            }

            if (NULL == (json_value = cJSON_GetObjectItem(json_domain_array, "value")))
            {
                rv = -1;
                continue;
            }

            rv |= do_set(devsn, res_type, index, json_domain->valuestring, json_identifier->valuestring, json_value);
        }
    } while (0);

    if (0 != rv)
    {
        ezlog_e(TAG_SHD, "query_reply invalid! rv=%d", rv);
        ezlog_d(TAG_SHD, "query_reply:%s", payload);
    }

    shadow_status_sync_disable(devsn, res_type, index);

    if (json_root)
    {
        cJSON_Delete(json_root);
    }
}

static ez_int32_t do_set(ez_uchar_t *devsn, ez_char_t *res_type, ez_int32_t index, ez_char_t *domain, ez_char_t *key, cJSON *value)
{
    ezlog_v(TAG_SHD, "do_set");

    ez_int32_t rv = -1;
    node_dev_t *pnode_dev;
    node_index_set_t *pnode_indexs;
    node_index_t *pnode_index;
    node_domain_t *pnode_domain = NULL;
    node_key_t *pnode_key = NULL;
    ez_shadow_value_t tlv = {0};
    shadow_cloud2dev_cb cloud2dev_cb = NULL;

    ez_shadow_business2dev_param_t pstParseSync = {0};
    pstParseSync.pdomain = domain;
    pstParseSync.pkey = key;

    ezos_strncpy(pstParseSync.pres.res_type, res_type, sizeof(pstParseSync.pres.res_type) - 1);
    pstParseSync.pres.local_index = index;

    if (0 == ezos_strcmp(SHADOW_DEFAULT_NAME, devsn))
    {
        ezos_strncpy(pstParseSync.pres.dev_serial, ez_kernel_getdevinfo_bykey("dev_subserial"), sizeof(pstParseSync.pres.dev_serial) - 1);
    }
    else
    {
        ezos_strncpy(pstParseSync.pres.dev_serial, devsn, sizeof(pstParseSync.pres.dev_serial) - 1);
    }

    shadow_module_lock();

    do
    {
        if (NULL == (pnode_dev = shadow_module_find_dev(&g_shaodw_modules, devsn)))
        {
            ezlog_e(TAG_SHD, "dev not found!");
            break;
        }

        if (NULL == (pnode_indexs = shadow_module_find_indexs(&pnode_dev->type_lst, res_type)))
        {
            ezlog_e(TAG_SHD, "indexs not found!");
            break;
        }

        if (NULL == (pnode_index = shadow_module_find_index(&pnode_indexs->index_lst, index)))
        {
            ezlog_e(TAG_SHD, "index not found!");
            break;
        }

        if (NULL == (pnode_domain = shadow_module_find_domain(&pnode_index->domain_lst, domain)))
        {
            ezlog_e(TAG_SHD, "domain not found!");
            break;
        }

        if (NULL == (pnode_key = shadow_module_find_key(&pnode_domain->key_lst, key)))
        {
            ezlog_e(TAG_SHD, "key not found!");
            break;
        }

        if (pnode_key->cloud2dev == NULL)
        {
            rv = -1;
            ezlog_e(TAG_SHD, "cloud2dev is null");
            break;
        }

        if (0 != (rv = json2tlv((cJSON *)value, &tlv)))
        {
            ezlog_e(TAG_SHD, "json2tlv");
            break;
        }

        cloud2dev_cb = pnode_key->cloud2dev;

        rv = 0;
    } while (0);

    shadow_module_unlock();

    if (0 == rv && 0 != (rv = cloud2dev_cb(&tlv, &pstParseSync)))
    {
        ezlog_e(TAG_SHD, "cloud2dev err, key=%s", key);
    }

    tlv_destroy(&tlv);

    return rv;
}

static ez_int32_t json2tlv(cJSON *pvalue, ez_shadow_value_t *ptlv)
{
    ez_int32_t rv = 0;

    if (!pvalue || !ptlv)
        return -1;

    ezos_memset(ptlv, 0, sizeof(ez_shadow_value_t));
    switch (pvalue->type)
    {
    case cJSON_False:
    case cJSON_True:
        ptlv->value_bool = (ez_bool_t)pvalue->valueint;
        ptlv->length = sizeof(pvalue->valueint);
        ptlv->type = SHD_DATA_TYPE_BOOL;
        break;
    case cJSON_Number:
    {
        if (!is_double(pvalue->valueint, pvalue->valuedouble))
        {
            ptlv->value_int = pvalue->valueint;
            ptlv->length = sizeof(pvalue->valueint);
            ptlv->type = SHD_DATA_TYPE_INT;
        }
        else
        {
            ptlv->value_double = pvalue->valuedouble;
            ptlv->length = sizeof(pvalue->valuedouble);
            ptlv->type = SHD_DATA_TYPE_DOUBLE;
        }
    }
    break;
    case cJSON_String:
        ptlv->value = pvalue->valuestring;
        ptlv->length = ezos_strlen(pvalue->valuestring);
        ptlv->type = SHD_DATA_TYPE_STRING;
        break;
    case cJSON_Array:
    {
        ptlv->value = cJSON_PrintUnformatted(pvalue);
        if (NULL == ptlv->value)
        {
            rv = -1;
            break;
        }

        ptlv->length = ezos_strlen(ptlv->value);
        ptlv->type = SHD_DATA_TYPE_ARRAY;
        break;
    }
    case cJSON_Object:
    {
        ptlv->value = cJSON_PrintUnformatted(pvalue);
        if (NULL == ptlv->value)
        {
            rv = -1;
            break;
        }

        ptlv->length = ezos_strlen(ptlv->value);
        ptlv->type = SHD_DATA_TYPE_OBJECT;
        break;
    }
    default:
        rv = -1;
        break;
    }

    return rv;
}

static ez_void_t tlv_destroy(ez_shadow_value_t *ptlv)
{
    if (!ptlv)
    {
        return;
    }

    if (NULL != ptlv->value &&
        (SHD_DATA_TYPE_ARRAY == ptlv->type || SHD_DATA_TYPE_OBJECT == ptlv->type))
    {
        ezos_free(ptlv->value);
    }

    ezos_memset(ptlv, 0, sizeof(ez_shadow_value_t));
}

static ez_bool_t is_double(int i, double d)
{
    ez_bool_t rv = ez_true;

    if (fabs(((double)i) - d) <= DBL_EPSILON && d <= INT_MAX && d >= INT_MIN)
    {
        rv = ez_false;
    }

    return rv;
}
