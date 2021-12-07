#include "ez_iot_tsl.h"
#include "ez_iot_core_def.h"
#include "ez_iot_core_lowlvl.h"
#include "ez_iot_shadow.h"
#include "tsl_def.h"
#include "tsl_adapter.h"
#include "tsl_legality.h"
#include "tsl_profile.h"
#include "tsl_storage.h"
#include "ezos.h"
#include "uuid.h"
#include "cJSON.h"
#include "webclient.h"
#include "ezlist.h"
#include "misc.h"
#include "mbedtls/md5.h"
#include "mbedtls/base64.h"

typedef struct
{
    ez_char_t url[260];
    ez_char_t md5[33];
    ez_int32_t expire;
} download_info_t;

typedef enum
{
    status_need_update, ///< 需要更新描述文件
    status_query,       ///< 正在查询升级包信息
    status_loading,     ///< 正在下载
} dev_stauts_e;

typedef struct
{
    ez_node_t node;
    ez_char_t dev_sn[48];
    ez_char_t dev_type[24];
    ez_char_t dev_fw_ver[36];
    dev_stauts_e status;
    ezos_timespec_t timer;
    download_info_t download_info;
} query_info_t;

/* tsl message process */
static ez_void_t iot_core_event_route(ez_kernel_event_t *ptr_event);
static ez_void_t tsl_data_route_cb(ez_kernel_submsg_v3_t *psub_msg);
static ez_char_t *assemble_rsp_code_msg(int code, ez_tsl_value_t *value_out);
static ez_tsl_data_type_e json_type_transform_dev(int type);
static ez_void_t strip_msg_wrap(ez_void_t *buf, ez_tsl_value_t *tsl_data);

/* profile download */
typedef ez_void_t (*profile_query_rsp)(const ez_char_t *rsp_msg);
extern ez_err_t ez_iot_base_lowlvl_profile_query(const ez_char_t *req_msg, profile_query_rsp func_rsp);
static ez_void_t tsl_adapter_profile_query_rsp(const ez_char_t *rsp_msg);
static ez_void_t ez_profile_get_thread(void *param);
static ez_int32_t profile_downloading(query_info_t *query_info, ez_char_t **buf, ez_int32_t *length);
static ez_bool_t tsl_adapter_profile_download(ez_tsl_devinfo_t *dev_info);
static ez_bool_t tsl_adapter_dl_task_yeild();
static ez_bool_t tsl_adapter_query_job_add(ez_tsl_devinfo_t *dev_info);
static ez_void_t tsl_adapter_query_job_update(ez_char_t *dev_sn, dev_stauts_e status, download_info_t *download_info);

/* shadow adapter */
static void tsl_adapter_shadow_inst(ez_char_t *dev_sn);
static void tsl_adapter_shadow_uninst(ez_char_t *dev_sn);
static ez_err_t business2cloud_imp(ez_shadow_value_t *pvalue, ez_shadow_business2cloud_param_t *ppram);
static ez_err_t business2dev_imp(const ez_shadow_value_t *pvalue, ez_shadow_business2dev_param_t *ppram);

/* private variables */
static ez_tsl_callbacks_t g_tsl_things_cbs = {0};
static ez_thread_t g_download_thread = NULL;
static ez_bool_t g_download_thread_running = ez_false;
static ez_list_t g_download_list;
static ez_mutex_t g_adapter_mutex = NULL;
static ez_bool_t g_is_online = ez_false;

static ez_int32_t tsl_action_process(ez_kernel_submsg_v3_t *submsg)
{
    ez_int32_t rv = -1;
    ez_char_t *rsp_buf = NULL;
    ez_char_t domain[32] = {0};
    ez_char_t identifier[32] = {0};

    ez_tsl_rsc_t rsc_info = {.res_type = submsg->resource_type, .local_index = submsg->resource_id};
    ez_tsl_key_t key_info = {.domain = domain, .key = identifier};
    ez_tsl_value_t value_in = {0};
    value_in.type = EZ_TSL_DATA_TYPE_NULL;
    ez_tsl_value_t value_out = {0};
    value_out.type = EZ_TSL_DATA_TYPE_NULL;
    ez_kernel_pubmsg_v3_t pubmsg = {0};

    ezlog_i(TAG_TSL, "seq in: %d", submsg->msg_seq);
    ezlog_d(TAG_TSL, "recv msg: %s", submsg->buf);

    // example "domain/identifier"
    ez_int32_t num = ezos_sscanf(submsg->ext_msg, "%32[^/]/%32", domain, identifier);
    if (2 != num)
    {
        ezlog_e(TAG_TSL, "tsl key is illegal., msg:%s", submsg->ext_msg);
        goto done;
    }

    strip_msg_wrap(submsg->buf, &value_in);
    rv = g_tsl_things_cbs.action2dev(submsg->sub_serial, &rsc_info, &key_info, &value_in, &value_out);
    CHECK_RV_DONE(rv);

    rsp_buf = assemble_rsp_code_msg(rv, &value_out);
    CHECK_COND_DONE(NULL == rsp_buf, EZ_TSL_ERR_MEMORY);

    pubmsg.msg_response = 1;
    pubmsg.msg_qos = QOS_T1;
    pubmsg.msg_seq = submsg->msg_seq;
    pubmsg.msg_body = rsp_buf;
    pubmsg.msg_body_len = ezos_strlen(rsp_buf);

    ezos_strncpy(pubmsg.resource_type, rsc_info.res_type, sizeof(pubmsg.resource_type) - 1);
    ezos_strncpy(pubmsg.resource_id, rsc_info.local_index, sizeof(pubmsg.resource_id) - 1);
    ezos_strncpy(pubmsg.module, TSL_MODULE_NAME, sizeof(pubmsg.module) - 1);
    ezos_strncpy(pubmsg.method, TSL_SERVICE_METHOD_NAME, sizeof(pubmsg.module) - 1);
    ezos_strncpy(pubmsg.msg_type, TSL_MSG_TYPE_OPERATE_REPLY, sizeof(pubmsg.msg_type) - 1);
    ezos_strncpy(pubmsg.ext_msg, submsg->ext_msg, sizeof(pubmsg.ext_msg) - 1);
    ezos_strncpy(pubmsg.sub_serial, submsg->sub_serial, sizeof(pubmsg.sub_serial) - 1);

    rv = ez_kernel_send_v3(&pubmsg);
    CHECK_COND_DONE(ez_kernel_send_v3(&pubmsg), EZ_TSL_ERR_GENERAL);

done:
    SAFE_FREE(rsp_buf);

    if (EZ_TSL_DATA_TYPE_STRING == value_in.type ||
        EZ_TSL_DATA_TYPE_ARRAY == value_in.type ||
        EZ_TSL_DATA_TYPE_OBJECT == value_in.type)
    {
        SAFE_FREE(value_in.value);
    }

    if (EZ_TSL_DATA_TYPE_STRING == value_out.type ||
        EZ_TSL_DATA_TYPE_ARRAY == value_out.type ||
        EZ_TSL_DATA_TYPE_OBJECT == value_out.type)
    {
        SAFE_FREE(value_out.value);
    }

    return rv;
}

static ez_err_t business2dev_imp(const ez_shadow_value_t *pvalue, ez_shadow_business2dev_param_t *ppram)
{
    ez_err_t rv = -1;
    ez_tsl_value_t tsl_value = {0};
    ez_char_t local_index[8] = {0};
    ezos_snprintf(local_index, sizeof(local_index), "%d", ppram->pres.local_index);

    ez_tsl_rsc_t rsc_info = {.res_type = ppram->pres.res_type, .local_index = local_index};
    ez_tsl_key_t key_info = {.domain = (ez_char_t *)ppram->pdomain, .key = (ez_char_t *)ppram->pkey};
    tsl_value.type = pvalue->type;
    tsl_value.size = pvalue->length;
    tsl_value.value_double = pvalue->value_double;

    rv = g_tsl_things_cbs.property2dev(ppram->pres.dev_serial, &rsc_info, &key_info, &tsl_value);
    if (0 != rv)
    {
        ezlog_e(TAG_TSL, "dev process failed.");
    }

    return rv;
}

static ez_err_t business2cloud_imp(ez_shadow_value_t *pvalue, ez_shadow_business2cloud_param_t *ppram)
{
    ez_err_t rv = -1;
    ez_tsl_value_t *tsl_value = (ez_tsl_value_t *)pvalue;
    ez_char_t local_index[8] = {0};
    ezos_snprintf(local_index, sizeof(local_index), "%d", ppram->pres.local_index);

    ez_tsl_rsc_t rsc_info = {.res_type = ppram->pres.res_type, .local_index = local_index};
    ez_tsl_key_t key_info = {.domain = (ez_char_t *)ppram->pdomain, .key = (ez_char_t *)ppram->pkey};

    ezlog_d(TAG_TSL, "busi to cloud. dev_sn: %s", ppram->pres.dev_serial);
    rv = g_tsl_things_cbs.property2cloud(ppram->pres.dev_serial, &rsc_info, &key_info, tsl_value);
    if (0 != rv)
    {
        ezlog_e(TAG_TSL, "property2cloud failed.");
    }

    return rv;
}

static void tsl_adapter_shadow_inst(ez_char_t *dev_sn)
{
    const tsl_capacity_t *capacity = tsl_profile_get_lock(dev_sn);
    if (NULL == capacity)
    {
        ezlog_e(TAG_TSL, "shadow inst, capacity not found");
        return;
    }

    ez_int32_t rsc_num = capacity->rsc_num;
    ezlog_i(TAG_TSL, "resource num: %d", rsc_num);

    for (size_t i = 0; i < capacity->rsc_num; i++)
    {
        ez_int32_t index_num = capacity->resource[i].index_num;
        ezlog_i(TAG_TSL, "index num: %d", index_num);

        for (size_t j = 0; j < index_num; j++)
        {
            ez_int32_t domain_num = capacity->resource[i].domain_num;
            ezlog_i(TAG_TSL, "domain num: %d", domain_num);
            for (size_t k = 0; k < domain_num; k++)
            {
                ez_int32_t prop_num = capacity->resource[i].domain[k].prop_num;
                ezlog_i(TAG_TSL, "prop num: %d", prop_num);
                for (size_t l = 0; l < prop_num; l++)
                {
                    tsl_domain_prop *prop = capacity->resource[i].domain[k].prop + l;

                    ez_shadow_business_t shadow_busi = {0};
                    ezos_memset(&shadow_busi, 0, sizeof(ez_shadow_business_t));
                    ezos_strncpy((char *)shadow_busi.key, prop->identifier, sizeof(shadow_busi.key) - 1);

                    if (prop->access & ACCESS_READ)
                    {
                        shadow_busi.business2cloud = business2cloud_imp;
                    }

                    if (prop->access & ACCESS_WRITE)
                    {
                        shadow_busi.business2dev = business2dev_imp;
                    }

                    ezos_strncpy((char *)shadow_busi.key, prop->identifier, sizeof(shadow_busi.key) - 1);

                    ez_shadow_res_t shadow_res = {0};
                    ez_shadow_module_t shadow_module = {0};
                    ezos_strncpy((char *)shadow_res.dev_serial, (char *)dev_sn, sizeof(shadow_res.dev_serial) - 1);
                    ezos_strncpy((char *)shadow_res.res_type, (char *)capacity->resource[i].rsc_category, sizeof(shadow_res.res_type) - 1);
                    shadow_res.local_index = ezos_atoi(capacity->resource[i].index + MAX_LOCAL_INDEX_LENGTH * j);

                    shadow_module.num = 1;
                    shadow_module.business = &shadow_busi;
                    ez_int32_t ret = ez_iot_shadow_reg(&shadow_res, capacity->resource[i].domain[k].identifier, &shadow_module);
                    if (0 != ret)
                    {
                        ezlog_e(TAG_TSL, "shadow register failed.");
                        continue;
                    }
                }
            }
        }
    }

    tsl_profile_get_unlock();
}

static void tsl_adapter_shadow_uninst(ez_char_t *dev_sn)
{
    ez_iot_shadow_unreg(dev_sn);
}

static ez_int32_t profile_downloading(query_info_t *query_info, ez_char_t **buf, ez_int32_t *length)
{
    ez_int32_t rv = -1;

    ez_char_t md5_hex_up[16 * 2 + 1] = {0};
    ez_char_t md5_hex[16 * 2 + 1] = {0};
    ez_int32_t already_len = 0;
    ez_char_t md5_output[16] = {0};
    mbedtls_md5_context md5_ctx = {0};

    struct webclient_session *session = NULL;
    mbedtls_md5_init(&md5_ctx);
    mbedtls_md5_starts(&md5_ctx);

    session = webclient_session_create(512);
    ez_int32_t rsp_status = 0;
    do
    {
        ezlog_w(TAG_TSL, "url: %s", query_info->download_info.url);
        rsp_status = webclient_get(session, query_info->download_info.url);
        if (200 != rsp_status)
        {
            ezlog_e(TAG_TSL, "webclient get request failed. http_code: %d", rsp_status);
            break;
        }

        if (0 >= session->content_length)
        {
            ezlog_e(TAG_TSL, "content length illegal: %d", session->content_length);
            break;
        }

        *buf = (char *)ezos_malloc(session->content_length + 1);
        if (NULL == *buf)
        {
            ezlog_e(TAG_TSL, "memory not enough.");
            break;
        }
        ezos_memset(*buf, 0, session->content_length + 1);

        ez_int32_t read_len = 0;
        do
        {
            read_len = webclient_read(session, *buf + already_len, session->content_length - already_len);
            if (0 >= read_len)
            {
                break;
            }
            if (already_len == session->content_length)
            {
                break;
            }
            already_len += read_len;

        } while (ez_true);

        mbedtls_md5_update(&md5_ctx, (*buf), session->content_length);
        mbedtls_md5_finish(&md5_ctx, md5_output);
        bin2hexstr(md5_output, sizeof(md5_output), 1, md5_hex_up);
        bin2hexstr(md5_output, sizeof(md5_output), 0, md5_hex);

        if (0 != ezos_strcmp((char *)md5_hex, query_info->download_info.md5) && 0 != ezos_strcmp((char *)md5_hex_up, query_info->download_info.md5))
        {
            ezlog_e(TAG_TSL, "check_sum mismatch:%s, profile md5:%s", md5_hex, query_info->download_info.md5);
            break;
        }
        *length = session->content_length;
        rv = 0;
    } while (0);

    webclient_close(session);

    if (0 != rv)
    {
        ezos_free(*buf);
    }

    return rv;
}

/**
 * @brief 查询下载功能
 *
 * @param
 */
static void ez_profile_get_thread(void *param)
{
    query_info_t *query_info = NULL;
    ez_bool_t need_schema = ez_true;
    char *thread_name = (char *)param;

#ifdef EZIOT_TSL_LEGALITY_CHECK_STRONG
    need_schema = ez_true;
#endif

    do
    {
        if (!g_download_thread_running)
        {
            break;
        }

        if (!g_is_online)
        {
            ezos_delay_ms(500);
            continue;
        }

        if (0 == ezlist_get_size(&g_download_list))
        {
            /* 下载完成，任务退出 */
            break;
        }

        ezos_mutex_unlock(g_adapter_mutex);

        LIST_FOR_EACH(query_info_t, query_info, &g_download_list)
        {
            if (status_loading == query_info->status)
            {
                ezlog_d(TAG_TSL, "profile thread get url");

                /* 已拿到下载链接，开始下载 */
                ez_char_t *buf = NULL;
                ez_int32_t length = 0;
                if (0 != profile_downloading(query_info, &buf, &length))
                {
                    continue;
                }

                ezlog_w(TAG_TSL, "profile dl succ, sn:%s", query_info->dev_sn);

                if (!tsl_profile_reg(query_info->dev_sn, query_info->dev_type,
                                     query_info->dev_fw_ver, buf))
                {
                    continue;
                }

                ezlog_w(TAG_TSL, "profile reg succ, sn:%s", query_info->dev_sn);
                tsl_adapter_shadow_inst(query_info->dev_sn);

                if (!tsl_storage_save(query_info->dev_sn, query_info->dev_type,
                                      query_info->dev_fw_ver, buf, length))
                {
                    ezos_free(buf);
                    continue;
                }

                ezlog_w(TAG_TSL, "profile save succ, sn:%s", query_info->dev_sn);

                ezlist_delete(&g_download_list, &query_info->node);
                ezos_free(query_info);
                ezos_free(buf);
                query_info = NULL;
            }
            else if (status_query == query_info->status && !time_isexpired(&query_info->timer))
            {
                /* 正在查询下载链接 */
                ezlog_v(TAG_TSL, "profile thread wait url");
            }
            else
            {
                /* 查询下载链接 */
                cJSON *json_query = cJSON_CreateObject();
                if (NULL == json_query)
                {
                    continue;
                }

                cJSON_AddStringToObject(json_query, "devSerial", query_info->dev_sn);
                cJSON_AddStringToObject(json_query, "pid", query_info->dev_type);
                cJSON_AddStringToObject(json_query, "version", query_info->dev_fw_ver);
                cJSON_AddStringToObject(json_query, "profileVersion", "3.0");
                cJSON_AddBoolToObject(json_query, "requireSchema", need_schema);
                char *json_str = cJSON_PrintUnformatted(json_query);
                if (NULL == json_str)
                {
                    cJSON_Delete(json_query);
                    continue;
                }

                if (0 == ez_iot_base_lowlvl_profile_query(json_str, tsl_adapter_profile_query_rsp))
                {
                    tsl_adapter_query_job_update(query_info->dev_sn, status_query, NULL);
                }
                else
                {
                    tsl_adapter_query_job_update(query_info->dev_sn, status_need_update, NULL);
                }

                ezos_free(json_str);
                cJSON_Delete(json_query);
            }
        }

        ezos_mutex_unlock(g_adapter_mutex);
        ezos_delay_ms(100);

    } while (1);

    ezos_mutex_lock(g_adapter_mutex);
    g_download_thread_running = ez_false;
    ezos_mutex_unlock(g_adapter_mutex);

    ezlog_w(TAG_TSL, "thread %s exit", thread_name);

    return;
}

ez_err_t tsl_adapter_init(ez_tsl_callbacks_t *things_cbs)
{
    ez_err_t rv = EZ_TSL_ERR_SUCC;
    ez_kernel_extend_v3_t extend_info = {0};

    ezlist_init(&g_download_list);
    ezos_memcpy(&g_tsl_things_cbs, things_cbs, sizeof(ez_tsl_callbacks_t));

    extend_info.ez_kernel_event_route = iot_core_event_route;
    extend_info.ez_kernel_data_route = tsl_data_route_cb;
    ezos_strncpy(extend_info.module, TSL_MODULE_NAME, sizeof(extend_info.module) - 1);

    rv = ez_kernel_extend_load_v3(&extend_info);
    CHECK_COND_DONE(EZ_CORE_ERR_NOT_INIT == rv, EZ_TSL_ERR_NOT_READY);
    CHECK_COND_DONE(EZ_CORE_ERR_MEMORY == rv, EZ_TSL_ERR_MEMORY);
    CHECK_COND_DONE(EZ_CORE_ERR_SUCC != rv, EZ_TSL_ERR_GENERAL);

    CHECK_COND_DONE(tsl_profile_init(), EZ_TSL_ERR_MEMORY);

done:
    return rv;
}

ez_err_t tsl_adapter_add(ez_tsl_devinfo_t *dev_info, ez_char_t *profile)
{
    ez_err_t rv = EZ_TSL_ERR_SUCC;
    ez_char_t *buf = NULL;
    ez_uint32_t len = 0;

    /* 已经注册，返回成功 */
    if (tsl_profile_check(dev_info->dev_subserial))
    {
        goto done;
    }

    /* 有同型号版本号设备注册过，增加引用计数 */
    if (tsl_profile_ref_add(dev_info->dev_subserial,
                            dev_info->dev_type, dev_info->dev_firmwareversion))
    {
        tsl_adapter_shadow_inst(dev_info->dev_subserial);
        goto done;
    }

    if (profile)
    {
        /* 从参数传入 profile */
        ezlog_d(TAG_TSL, "try param");
        rv = tsl_profile_reg(dev_info->dev_subserial, dev_info->dev_type,
                             dev_info->dev_firmwareversion, profile);

        CHECK_COND_DONE(rv, EZ_TSL_ERR_PROFILE_LOADING);
        tsl_adapter_shadow_inst(dev_info->dev_subserial);
    }
    else
    {
        /* 使用已经下载的profile */
        ezlog_d(TAG_TSL, "try flash");
        rv = tsl_storage_load(dev_info->dev_subserial, &buf, &len);
        if (EZ_KV_ERR_SUCC == rv)
        {
            rv = tsl_profile_reg(dev_info->dev_subserial, dev_info->dev_type,
                                 dev_info->dev_firmwareversion, profile);

            CHECK_COND_DONE(rv, EZ_TSL_ERR_MEMORY);
            tsl_adapter_shadow_inst(dev_info->dev_subserial);
        }
        else if (EZ_KV_ERR_NAME == rv)
        {
            ezlog_w(TAG_TSL, "try download");
            rv = tsl_adapter_profile_download(dev_info);
            CHECK_COND_DONE(rv, EZ_TSL_ERR_MEMORY);
        }
        else
        {
            CHECK_COND_DONE(rv, EZ_TSL_ERR_MEMORY);
        }
    }

done:
    SAFE_FREE(buf);

    return rv;
}

ez_err_t tsl_adapter_del(ez_char_t *dev_sn)
{
    if (!tsl_profile_check(dev_sn))
    {
        return EZ_TSL_ERR_SUCC;
    }

    tsl_profile_ref_del(dev_sn);
    tsl_adapter_shadow_uninst(dev_sn);

    return EZ_TSL_ERR_SUCC;
}

void tsl_adapter_deinit()
{
    ezlog_w(TAG_TSL, "tsl adapter deinit.");

    g_download_thread_running = ez_false;
    ezos_thread_destroy(g_download_thread);
    ezos_mutex_destroy(g_adapter_mutex);
    ezlist_clear(&g_download_list);
    tsl_profile_deinit();

    g_adapter_mutex = NULL;
    g_download_thread = NULL;
}

static ez_bool_t tsl_adapter_profile_download(ez_tsl_devinfo_t *dev_info)
{
    if (!tsl_adapter_query_job_add(dev_info))
    {
        return ez_false;
    }

    if (!tsl_adapter_dl_task_yeild())
    {
        return ez_false;
    }

    return ez_true;
}

static ez_bool_t tsl_adapter_query_job_add(ez_tsl_devinfo_t *dev_info)
{
    query_info_t *query_info = (query_info_t *)ezos_malloc(sizeof(query_info_t));
    if (!query_info)
    {
        return ez_false;
    }

    ezos_bzero(query_info, sizeof(query_info_t));
    ezos_strncpy(query_info->dev_fw_ver, dev_info->dev_firmwareversion, sizeof(query_info->dev_fw_ver) - 1);
    ezos_strncpy(query_info->dev_type, dev_info->dev_type, sizeof(query_info->dev_type) - 1);
    ezos_strncpy(query_info->dev_sn, dev_info->dev_subserial, sizeof(query_info->dev_sn) - 1);

    ezos_mutex_lock(g_adapter_mutex);
    ezlist_add_last(&g_download_list, &query_info->node);
    ezos_mutex_unlock(g_adapter_mutex);

    return ez_true;
}

static void tsl_adapter_query_job_update(ez_char_t *dev_sn, dev_stauts_e status, download_info_t *download_info)
{
    query_info_t *query_info = NULL;

    ezos_mutex_lock(g_adapter_mutex);

    LIST_FOR_EACH(query_info_t, query_info, &g_download_list)
    {
        if (0 == ezos_strcmp(dev_sn, query_info->dev_sn))
        {
            break;
        }
    }

    if (NULL == query_info)
    {
        ezos_mutex_unlock(g_adapter_mutex);
        return;
    }

    switch (query_info->status)
    {
    case status_need_update:
    {
        time_countdown(&query_info->timer, 30 * 1000);
    }
    break;
    case status_query:
    {
        time_countdown(&query_info->timer, 30 * 1000);
        query_info->status = status;
    }
    break;
    case status_loading:
    {
        query_info->status = status;
    }
    break;
    default:
        break;
    }

    ezos_mutex_unlock(g_adapter_mutex);
}

ez_bool_t tsl_adapter_status_check(ez_char_t *dev_sn)
{
    query_info_t *query_info = NULL;
    ez_bool_t rv = ez_false;

    ezos_mutex_lock(g_adapter_mutex);

    LIST_FOR_EACH(query_info_t, query_info, &g_download_list)
    {
        if (0 == ezos_strcmp(dev_sn, query_info->dev_sn))
        {
            rv = ez_true;
            break;
        }
    }

    ezos_mutex_unlock(g_adapter_mutex);

    return rv;
}

static ez_bool_t tsl_adapter_dl_task_yeild()
{
    ez_bool_t rv = ez_false;
    const ez_char_t *dl_thread_name = "ez_tsl_get";

    do
    {
        ezos_mutex_lock(g_adapter_mutex);
        if (g_download_thread_running)
        {
            ezos_mutex_unlock(g_adapter_mutex);
            rv = ez_true;
            break;
        }
        ezos_mutex_unlock(g_adapter_mutex);

        ezos_thread_destroy(g_download_thread);

        if (0 != ezos_thread_create(&g_download_thread, dl_thread_name,
                                    ez_profile_get_thread, (void *)dl_thread_name,
                                    CONFIG_EZIOT_TSL_DOWNLOAD_STACK_SIZE,
                                    CONFIG_EZIOT_TSL_DOWNLOAD_TASK_PRIORITY))
        {
            ezlog_e(TAG_TSL, "create profile get thread failed.");
            break;
        }

        rv = ez_true;
    } while (0);

    return rv;
}

static ez_void_t iot_core_event_route(ez_kernel_event_t *ptr_event)
{
    if (!ptr_event)
    {
        return;
    }

    switch (ptr_event->event_type)
    {
    case SDK_KERNEL_EVENT_ONLINE:
    case SDK_KERNEL_EVENT_SWITCHOVER:
        g_is_online = ez_true;
        break;
    case SDK_KERNEL_EVENT_BREAK:
        g_is_online = ez_false;
        break;
    default:
        break;
    }
}

static void tsl_data_route_cb(ez_kernel_submsg_v3_t *psub_msg)
{
    if (0 == ezos_strcmp(psub_msg->method, TSL_SERVICE_METHOD_NAME))
    {
        tsl_action_process(psub_msg);
    }
    else if (0 == ezos_strcmp(psub_msg->method, TSL_ATTRIBUTE_METHOD_NAME))
    {
        ezlog_e(TAG_TSL, "attr not support.");
    }
}

ez_void_t tsl_adapter_set_profile_url(char *dev_sn, char *url, char *md5, int expire)
{
    download_info_t download_info = {0};

    ezos_snprintf(download_info.url, sizeof(download_info.url) - 1, "http://%s", url);
    download_info.expire = expire;
    ezos_strncpy(download_info.md5, md5, sizeof(download_info.md5) - 1);

    tsl_adapter_query_job_update(dev_sn, status_loading, &download_info);
}

static ez_void_t tsl_adapter_profile_query_rsp(const ez_char_t *rsp_msg)
{
    ez_err_t rv = EZ_TSL_ERR_SUCC;
    (ez_void_t) rv;

    cJSON *root = NULL;
    cJSON *url = NULL;
    cJSON *expire = NULL;
    cJSON *md5 = NULL;
    cJSON *dev_sn = NULL;

    root = cJSON_Parse(rsp_msg);
    CHECK_COND_DONE(!root, EZ_TSL_ERR_MEMORY);

    url = cJSON_GetObjectItem(root, "url");
    CHECK_COND_DONE(!url, EZ_TSL_ERR_MEMORY);
    CHECK_COND_DONE(cJSON_String != url->type, EZ_TSL_ERR_GENERAL);

    expire = cJSON_GetObjectItem(root, "expire");
    CHECK_COND_DONE(!expire, EZ_TSL_ERR_MEMORY);
    CHECK_COND_DONE(cJSON_Number != expire->type, EZ_TSL_ERR_MEMORY);

    md5 = cJSON_GetObjectItem(root, "md5");
    CHECK_COND_DONE(!md5, EZ_TSL_ERR_MEMORY);
    CHECK_COND_DONE(cJSON_String != md5->type, EZ_TSL_ERR_MEMORY);

    dev_sn = cJSON_GetObjectItem(root, "devSerial");
    CHECK_COND_DONE(!dev_sn, EZ_TSL_ERR_MEMORY);
    CHECK_COND_DONE(cJSON_String != dev_sn->type, EZ_TSL_ERR_MEMORY);

    tsl_adapter_set_profile_url(dev_sn->valuestring, url->valuestring, md5->valuestring, expire->valueint);

done:
    cJSON_Delete(root);
}

static ez_char_t *assemble_rsp_code_msg(int code, ez_tsl_value_t *value_out)
{
    cJSON *js_root = NULL;
    ez_char_t *rv = NULL;

    do
    {
        js_root = cJSON_CreateObject();
        if (NULL == js_root)
        {
            ezlog_e(TAG_TSL, "json create object failed.");
            break;
        }

        cJSON_AddNumberToObject(js_root, "code", code);

        if (NULL != value_out)
        {
            cJSON_AddRawToObject(js_root, "data", (char *)value_out->value);
        }

        rv = cJSON_PrintUnformatted(js_root);
        if (NULL == rv)
        {
            ezlog_e(TAG_TSL, "json print error.");
            break;
        }
    } while (ez_false);

    cJSON_Delete(js_root);

    return rv;
}

static ez_tsl_data_type_e json_type_transform_dev(int type)
{
    ez_tsl_data_type_e ret_type = EZ_TSL_DATA_TYPE_MAX;
    switch (type)
    {
    case cJSON_False:
    case cJSON_True:
        ret_type = EZ_TSL_DATA_TYPE_BOOL;
        break;
    case cJSON_NULL:
        break;
    case cJSON_Number:
        ret_type = EZ_TSL_DATA_TYPE_INT;
        break;
    case cJSON_String:
        ret_type = EZ_TSL_DATA_TYPE_STRING;
        break;

    case cJSON_Array:
        ret_type = EZ_TSL_DATA_TYPE_ARRAY;
        break;

    case cJSON_Object:
        ret_type = EZ_TSL_DATA_TYPE_OBJECT;
        break;
    default:
        break;
    }

    return ret_type;
}

static ez_void_t strip_msg_wrap(ez_void_t *buf, ez_tsl_value_t *tsl_data)
{
    cJSON *js_msg = NULL;
    cJSON *js_data = NULL;

    do
    {
        js_msg = cJSON_Parse((char *)buf);
        if (NULL == js_msg)
        {
            ezlog_e(TAG_TSL, "msg parse: %s", buf);
            break;
        }

        js_data = cJSON_GetObjectItem(js_msg, "data");
        if (NULL == js_data)
        {
            ezlog_e(TAG_TSL, "msg format error: %s", (char *)buf);
            break;
        }

        tsl_data->type = json_type_transform_dev(js_data->type);

        switch (tsl_data->type)
        {
        case EZ_TSL_DATA_TYPE_BOOL:
            tsl_data->size = sizeof(ez_bool_t);
            tsl_data->value_bool = (ez_bool_t)js_data->valueint;
            break;
        case EZ_TSL_DATA_TYPE_INT:
            tsl_data->value_int = js_data->valueint;
            tsl_data->size = sizeof(js_data->valueint);
            break;
        case EZ_TSL_DATA_TYPE_DOUBLE:
            tsl_data->size = sizeof(js_data->valuedouble);
            tsl_data->value_double = js_data->valuedouble;
            break;
        case EZ_TSL_DATA_TYPE_STRING:
            tsl_data->value = (char *)ezos_malloc(ezos_strlen(js_data->valuestring) + 1);
            if (NULL == tsl_data->value)
            {
                break;
            }

            tsl_data->size = ezos_strlen(js_data->valuestring);
            ezos_strcpy(tsl_data->value, js_data->valuestring);
            break;
        case EZ_TSL_DATA_TYPE_ARRAY:
        case EZ_TSL_DATA_TYPE_OBJECT:
            tsl_data->value = cJSON_PrintUnformatted(js_data);
            if (NULL == tsl_data->value)
            {
                break;
            }

            tsl_data->size = ezos_strlen(tsl_data->value);
            break;

        default:
            break;
        }
    } while (0);

    cJSON_Delete(js_msg);
}