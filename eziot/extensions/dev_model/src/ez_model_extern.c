#include "base_typedef.h"
#include "stdio.h"
#include <float.h>
#include <limits.h>
#include <math.h>
#include <stdbool.h>

#include "hal_thread.h"
#include "double_linked_list.h"
#include "ez_sdk_log.h"
#include "ezdev_sdk_kernel.h"
#include "ezdev_sdk_kernel_struct.h"
#include "pthread.h"

#include "ez_model_bus.h"
#include "ez_model_comm.h"
#include "ez_model_def.h"
#include "ez_model_extern.h"
#include "ez_model_user.h"

typedef struct
{
    node_t node; ///< 双向链表的节点
    ez_domain_reg *domain_info;
} ez_domain_node_t;

static int32_t json2tlv(bscJSON *pvalue, ez_model_msg *ptlv);

/**
 * @brief 判断json number是int或double
 * 
 * @param a pvalue->valueint
 * @param b pvalue->valuedouble
 * @return true double
 * @return false int
 */
static bool is_double(int a, double b);

/**
 * @brief destroy the objs obtained from json2tlv
 * 
 * @param ptlv 
 */
static void tlv_destroy(ez_model_msg *ptlv);

static list_t g_model_domains;

static void* g_model_domain_lock_h = NULL;
static void* g_model_cb_lock_h     = NULL;

ez_model_default_cb g_ez_model_data_router = {NULL,};

/*这里将SDK内部使用的领域单独计数*/
int ez_get_list_size()
{
    int list_size = 0;
    hal_thread_mutex_lock(&g_model_domain_lock_h);
    list_size = list_get_size(&g_model_domains);
    hal_thread_mutex_unlock(&g_model_domain_lock_h);

    return list_size;
}

int ez_set_data_route_cb(ez_model_default_cb *ez_data_router)
{
    if (NULL == ez_data_router)
    {
        return EZ_CODE_INVALID_PARAM;
    }
    hal_thread_mutex_lock(&g_model_cb_lock_h);
    g_ez_model_data_router = *ez_data_router;
    hal_thread_mutex_unlock(&g_model_cb_lock_h);

    return EZ_CODE_SUCESS;
}

static ez_domain_node_t *ez_model_find_domain_info(list_t *list, const char *domain)
{
    ez_domain_node_t *pnode = NULL;

    if (NULL == list || 0 == list_get_size(list) || NULL == domain)
    {
        ez_log_e(TAG_MOD, "ez_model_find_domain,list size  is 0 or domain is null\n");
        return NULL;
    }

    LIST_FOR_EACH(list, ez_domain_node_t, pnode)
    {
        if (0 == strcmp(pnode->domain_info->domain, domain))
        {
            return pnode;
        }
    }
    return NULL;
}

static int ez_model_add_domain_info(list_t *list, const ez_domain_reg *domain_reg)
{
    ez_domain_node_t *pnode = NULL;
    if (NULL == list || NULL == domain_reg || NULL == domain_reg->das_reply_router || NULL == domain_reg->das_req_router)
    {
        return EZ_CODE_INVALID_PARAM;
    }
    pnode = ez_model_find_domain_info(list, domain_reg->domain);
    if (NULL != pnode)
    {
        return EZ_CODE_MODEL_DOMAIN_REGED;
    }
    pnode = (ez_domain_node_t *)malloc(sizeof(ez_domain_node_t));
    if (NULL == pnode)
    {
        ez_log_e(TAG_MOD, "ez_domain_node_t malloc err\n");
        return EZ_CODE_MODEL_MALLOC_ERR;
    }
    memset(pnode, 0, sizeof(ez_domain_node_t));

    pnode->domain_info = (ez_domain_reg *)malloc(sizeof(ez_domain_reg));
    if (NULL == pnode->domain_info)
    {
        ez_log_e(TAG_MOD, "domain_info malloc err\n");
        free(pnode);
        pnode = NULL;
        return EZ_CODE_MODEL_MALLOC_ERR;
    }
    memset(pnode->domain_info, 0, sizeof(ez_domain_reg));
    memcpy(pnode->domain_info, domain_reg, sizeof(ez_domain_reg));

    list_add_from_tail(list, &pnode->node);

    ez_log_v(TAG_MOD, "ez_model_add_domain_info success size:%d\n", list_get_size(&g_model_domains));
    return EZ_CODE_SUCESS;
}

int ez_reg_domain(const ez_domain_reg *domain_reg)
{
    EZ_ERR_CODE_E ret = EZ_CODE_SUCESS;
    if (NULL == domain_reg->das_reply_router || NULL == domain_reg->das_req_router || 0 == strlen(domain_reg->domain))
    {
        return EZ_CODE_INVALID_PARAM;
    }
    hal_thread_mutex_lock(&g_model_domain_lock_h);
    ret = ez_model_add_domain_info(&g_model_domains, domain_reg);
    hal_thread_mutex_unlock(&g_model_domain_lock_h);
    if (EZ_CODE_SUCESS == ret)
    {
        ez_log_v(TAG_MOD, "Domain reg success  size:%d\n", list_get_size(&g_model_domains));
    }
    return ret;
}

int ez_dereg_domain(const char *domain)
{
    ez_domain_node_t *pnode = NULL;

    hal_thread_mutex_lock(&g_model_domain_lock_h);
    pnode = ez_model_find_domain_info(&g_model_domains, domain);
    if (pnode)
    {
        if (pnode->domain_info)
        {
            free(pnode->domain_info);
            pnode->domain_info = NULL;
        }
        list_delete_node(&g_model_domains, &pnode->node);
        free(pnode);
        pnode = NULL;
    }
    hal_thread_mutex_unlock(&g_model_domain_lock_h);

    ez_log_v(TAG_MOD, "Domain dereg success  size:%d\n", list_get_size(&g_model_domains));
    return EZ_CODE_SUCESS;
}

/** 
 *  \brief		根据domain找到领域上下文
 *  \method		ez_get_reg_domain
 *  \param[in] 	domain	领域
 *  \return		成功返回领域上下文指针 失败返回NULL
 */
ez_domain_reg *ez_get_reg_domain(const char *domain)
{
    ez_domain_node_t *pnode_domain = NULL;
    if (NULL == domain)
    {
        ez_log_e(TAG_MOD, "ez_get_reg_domain input err\n");
        return NULL;
    }
    hal_thread_mutex_lock(&g_model_domain_lock_h);
    pnode_domain = ez_model_find_domain_info(&g_model_domains, domain);
    hal_thread_mutex_unlock(&g_model_domain_lock_h);
    if (NULL == pnode_domain)
    {
        return NULL;
    }
    return pnode_domain->domain_info;
}

static void ez_parse_das_reply(void *pjsroot, ez_err_info *err_info, ez_model_msg *data)
{
    bscJSON *pjsstatus = NULL;
    bscJSON *pjscode = NULL;
    bscJSON *pjserr = NULL;
    bscJSON *pjsdata = NULL;
    bscJSON *pjsvalue = NULL;
    if (NULL == pjsroot || NULL == err_info || NULL == data)
    {
        ez_log_e(TAG_MOD, "ez_parse_status input invalid\n");
        return;
    }

    pjsstatus = bscJSON_GetObjectItem((bscJSON *)pjsroot, "status");
    if (pjsstatus)
    {
        err_info->status = pjsstatus->valueint;
    }
    pjscode = bscJSON_GetObjectItem(pjsroot, "code");
    if (pjscode && bscJSON_String == pjscode->type)
    {
        strncpy(err_info->err_code, pjscode->valuestring, EZ_ERR_CODE_LEN - 1);
    }

    pjserr = bscJSON_GetObjectItem(pjsroot, "errorMsg");
    if (pjserr && bscJSON_String == pjserr->type)
    {
        strncpy(err_info->err_msg, pjserr->valuestring, EZ_ERR_MSG_LEN - 1);
    }
    pjsdata = bscJSON_GetObjectItem(pjsroot, "data");
    if (pjsdata)
    {
        if (NULL != (pjsvalue = bscJSON_GetObjectItem(pjsdata, "Value")))
        {
            if (0 != json2tlv(pjsvalue, data))
            {
                ez_log_v(TAG_MOD, "json2tlv err\n");
            }
        }
        else
        {
            if (0 != json2tlv(pjsdata, data))
            {
                ez_log_v(TAG_MOD, "json2tlv err\n");
            }
        }
    }
    ez_log_v(TAG_MOD, "code:%s, status:%d, err_msg:%s\n", err_info->err_code, err_info->status, err_info->err_msg);
}

static void ez_attribute_data_route(ez_basic_info *basic_info, char *msg_type, void *buf, unsigned int msg_seq, ez_domain_reg *domain_info)
{
    char *ptr_str = NULL;
    bscJSON *pjsRoot = NULL;
    bscJSON *pjsValue = NULL;
    bscJSON *pjsData = NULL;
    ez_model_msg tlv;

    ez_err_info err_info;
    memset(&err_info, 0, sizeof(ez_err_info));
    memset(&tlv, 0, sizeof(ez_model_msg));

    if (NULL == basic_info || NULL == buf || NULL == domain_info)
    {
        ez_log_e(TAG_MOD, "recv attribute msg, buf is null\n");
        return;
    }
    ez_log_v(TAG_MOD, "recv attribute msg:domain_id:%s,identifier:%s \n", basic_info->domain, basic_info->identifier);
    do
    {
        pjsRoot = bscJSON_Parse((const char *)buf);
        if (NULL == pjsRoot)
        {
            ez_log_e(TAG_MOD, "attribute_data_route JSON_Parse err \n");
            break;
        }
        ptr_str = strstr(msg_type, "reply");
        if (ptr_str)
        {
            ez_parse_das_reply((void *)pjsRoot, &err_info, &tlv);
            if (domain_info->das_reply_router)
            {
                domain_info->das_reply_router(basic_info, msg_type, &err_info, &tlv, msg_seq);
                ez_log_v(TAG_MOD, " das_reply_router success:%s, seq:%d \n", basic_info->domain, msg_seq);
                break;
            }
        }
        if (0 == strcmp(msg_type, "set"))
        {
            pjsData = bscJSON_GetObjectItem(pjsRoot, "data");
            if (NULL == pjsData)
            {
                ez_log_e(TAG_MOD, "attribute get item data err \n");
                break;
            }
            pjsValue = bscJSON_GetObjectItem(pjsData, "Value");
            if (NULL == pjsValue)
            {
                ez_log_e(TAG_MOD, "attribute get item Value err \n");
                break;
            }

            if (0 != json2tlv(pjsValue, &tlv))
            {
                ez_log_e(TAG_MOD, "attribute json print err \n");
                break;
            }
        }
        else
        {
            if (0 != json2tlv(pjsValue, &tlv))
            {
                ez_log_e(TAG_MOD, "attribute json print err \n");
                break;
            }
        }

        if (domain_info->das_req_router)
        {
            domain_info->das_req_router(basic_info, msg_type, &tlv, msg_seq);
        }

    } while (0);

    tlv_destroy(&tlv);

    if (pjsRoot)
    {
        bscJSON_Delete(pjsRoot);
    }
}

static void ez_service_data_route(ez_basic_info *basic_info, char *msg_type, void *buf, unsigned int msg_seq, ez_domain_reg *domain_info)
{
    char *ptr_str = NULL;
    bscJSON *pjsRoot = NULL;
    bscJSON *pjsData = NULL;
    ez_model_msg tlv;
    ez_err_info err_info;

    if (NULL == basic_info || NULL == buf || NULL == msg_type || NULL == domain_info)
    {
        ez_log_e(TAG_MOD, "recv service msg, buf is null\n");
        return;
    }

    memset(&err_info, 0, sizeof(ez_err_info));
    memset(&tlv, 0, sizeof(ez_model_msg));

    ez_log_v(TAG_MOD, "recv service msg:resource_id:%s,resource_type:%s, msg_type:%s \n", basic_info->domain, basic_info->identifier, msg_type);
    do
    {
        pjsRoot = bscJSON_Parse((const char *)buf);
        if (NULL == pjsRoot)
        {
            ez_log_e(TAG_MOD, "service recv JSON_Parse err \n");
            break;
        }
        ptr_str = strstr(msg_type, "reply");
        if (ptr_str)
        {
            ez_log_v(TAG_MOD, "recv service reply:domain_id:%s,identifier:%s \n", basic_info->domain, basic_info->identifier);
            ez_parse_das_reply((void *)pjsRoot, &err_info, &tlv);
            if (domain_info->das_reply_router)
            {
                domain_info->das_reply_router(basic_info, msg_type, &err_info, &tlv, msg_seq);
                ez_log_v(TAG_MOD, "das_reply_router:domain_id:%s, seq:%d \n", basic_info->domain, msg_seq);
            }
            break;
        }
        pjsData = bscJSON_GetObjectItem(pjsRoot, "data");
        if (NULL == pjsData)
        {
            ez_log_e(TAG_MOD, "service recv get item <data> err \n");
            break;
        }

        if (0 != json2tlv(pjsData, &tlv))
        {
            ez_log_e(TAG_MOD, "attribute json print err \n");
            break;
        }

        if (domain_info->das_req_router)
        {
            domain_info->das_req_router(basic_info, msg_type, &tlv, msg_seq);
        }

    } while (0);

    ez_log_v(TAG_MOD, "recv service msg:resource_id:%s,resource_type:%s, msg_type:%s \n", basic_info->domain, basic_info->identifier, msg_type);

    tlv_destroy(&tlv);

    if (pjsRoot)
    {
        bscJSON_Delete(pjsRoot);
    }
}

void ez_model_data_route(ezdev_sdk_kernel_submsg_v3 *ptr_submsg)
{
    char *str_ext_msg = NULL;
    int domain_len = 0;
    ez_basic_info basic_info;
    ez_domain_reg *domain_info = NULL;
    do
    {
        if (NULL == ptr_submsg->buf)
        {
            ez_log_e(TAG_MOD, "model_data_route msg input buf is NULL \n");
            break;
        }

        memset(&basic_info, 0, sizeof(ez_basic_info));

        ez_log_v(TAG_MOD, "model_data_route recv msg:%s \n", (char *)ptr_submsg->buf);

        ez_log_d(TAG_MOD, "msg_type:%s ,ext_msg:%s ,seq:%d\n", ptr_submsg->msg_type, ptr_submsg->ext_msg, ptr_submsg->msg_seq);
        if (0 == strcmp(ptr_submsg->method, "event"))
        {
            basic_info.type = ez_event;
        }
        else if (0 == strcmp(ptr_submsg->method, "attribute"))
        {
            basic_info.type = ez_attribute;
        }
        else if (0 == strcmp(ptr_submsg->method, "service"))
        {
            basic_info.type = ez_service;
        }

        ez_log_d(TAG_MOD, "resource_id:%s ,resource_type:%s\n", ptr_submsg->resource_id, ptr_submsg->resource_type);

        strncpy(basic_info.resource_type, ptr_submsg->resource_type, sizeof(basic_info.resource_type) - 1);
        strncpy(basic_info.resource_id, ptr_submsg->resource_id, sizeof(basic_info.resource_id) - 1);
        strncpy(basic_info.subserial, ptr_submsg->sub_serial, sizeof(basic_info.subserial) - 1);

        str_ext_msg = strstr(ptr_submsg->ext_msg, "/");
        if (NULL == str_ext_msg)
        {
            ez_log_e(TAG_MOD, "ext_msg format error\n");
            return;
        }
        strncpy(basic_info.identifier, str_ext_msg + 1, sizeof(basic_info.identifier) - 1);

        domain_len = strlen(ptr_submsg->ext_msg) - strlen(str_ext_msg);
        if (domain_len > sizeof(basic_info.identifier) - 1)
        {
            ez_log_e(TAG_MOD, "identifier len is over ranage:%d\n", domain_len);
            return;
        }
        strncpy(basic_info.domain, ptr_submsg->ext_msg, domain_len);
        ez_log_v(TAG_MOD, "type:%d,domain:%s, identifier:%s\n", basic_info.type, basic_info.domain, basic_info.identifier);

        domain_info = ez_get_reg_domain(basic_info.domain);
        if (NULL == domain_info)
        {
            ez_log_w(TAG_MOD, "domain not register:%s \n", basic_info.domain);
            hal_thread_mutex_lock(&g_model_cb_lock_h);
            if (g_ez_model_data_router.ez_model_default_cb)
            {
                g_ez_model_data_router.ez_model_default_cb(&basic_info, ptr_submsg->msg_type, ptr_submsg->buf, ptr_submsg->buf_len, ptr_submsg->msg_seq);
            }
            hal_thread_mutex_unlock(&g_model_cb_lock_h);
            break;
        }
        switch (basic_info.type)
        {
        case ez_event:
            {
            }
            break;
        case ez_attribute:
            {
                ez_attribute_data_route(&basic_info, ptr_submsg->msg_type, ptr_submsg->buf, ptr_submsg->msg_seq, domain_info);
            }
            break;
        case ez_service:
            {
                ez_service_data_route(&basic_info, ptr_submsg->msg_type, ptr_submsg->buf, ptr_submsg->msg_seq, domain_info);
            }
            break;
        default:
            break;
        }

    } while (0);
}

void ez_model_event_route(ezdev_sdk_kernel_event *ptr_event)
{
    ez_log_v(TAG_MOD, "model rec event type: %d \n", ptr_event->event_type);

    switch (ptr_event->event_type)
    {
    case sdk_kernel_event_online:
    case sdk_kernel_event_fast_reg_online:
    case sdk_kernel_event_reconnect_success:
    case sdk_kernel_event_switchover:
    {
        ez_log_d(TAG_MOD, " model device online\n");
    }
    break;
    case sdk_kernel_event_break:
    {
        ez_log_d(TAG_MOD, " model device offline\n");
    }
    break;
    case sdk_kernel_event_invaild_authcode:
    {
        ez_log_d(TAG_MOD, " model device invaild_authcode\n");
    }
    break;
    case sdk_kernel_event_runtime_err:
    {
    }
    case sdk_kernel_event_heartbeat_interval_changed:
    {
    }
    break;
    default:
        break;
    }
    return;
}

int ez_model_extern_init()
{
    int ret = EZ_CODE_SUCESS;
    ezdev_sdk_kernel_error ezRv = ezdev_sdk_kernel_succ;

    ezdev_sdk_kernel_extend_v3 extern_info;
    memset(&extern_info, 0, sizeof(ezdev_sdk_kernel_extend_v3));
    do
    {
        extern_info.ezdev_sdk_kernel_data_route = ez_model_data_route;
        extern_info.ezdev_sdk_kernel_event_route = ez_model_event_route;
        strncpy(extern_info.module, "model", ezdev_sdk_module_name_len - 1);
        if (ezdev_sdk_kernel_succ != (ezRv = ezdev_sdk_kernel_extend_load_v3(&extern_info)))
        {
            ez_log_e(TAG_MOD, "model load err:%0x\n", ezRv);
            ret = EZ_CODE_MODEL_REG_FAIL;
        }

        list_init(&g_model_domains);

        g_model_domain_lock_h = hal_thread_mutex_create();
        if(NULL == g_model_domain_lock_h)
        {
            ez_log_e(TAG_MOD, "domain_lock create faild\n");
            ret = EZ_CODE_MODEL_MALLOC_ERR;
            break;
        }
        g_model_cb_lock_h = hal_thread_mutex_create();
        {
            ez_log_e(TAG_MOD, "cb_lock create faild\n");
            ret = EZ_CODE_MODEL_MALLOC_ERR;
            break;
        }

    }while(0);
    
    if(EZ_CODE_SUCESS!=ret)
    {
        if(NULL!=g_model_domain_lock_h)
        {
            hal_thread_mutex_destroy(g_model_domain_lock_h);
        }
    }

    return ret;
}

void ez_model_extern_deinit()
{
    if(NULL!=g_model_domain_lock_h)
    {
        hal_thread_mutex_destroy(g_model_domain_lock_h);
    }

    if(NULL!=g_model_cb_lock_h)
    {
       hal_thread_mutex_destroy(g_model_cb_lock_h); 
    }

}

static int32_t json2tlv(bscJSON *pvalue, ez_model_msg *ptlv)
{
    int32_t rv = 0;

    if (!pvalue || !ptlv)
        return -1;

    memset(ptlv, 0, sizeof(ez_model_msg));
    switch (pvalue->type)
    {
    case bscJSON_False:
    case bscJSON_True:
        ptlv->value_bool = (bool)pvalue->valueint;
        ptlv->length = sizeof(pvalue->valueint);
        ptlv->type = model_data_type_bool;
        break;
    case bscJSON_Number:
    {
        if (!is_double(pvalue->valueint, pvalue->valuedouble))
        {
            ptlv->value_int = pvalue->valueint;
            ptlv->length = sizeof(pvalue->valueint);
            ptlv->type = model_data_type_int;
        }
        else
        {
            ptlv->value_double = pvalue->valuedouble;
            ptlv->length = sizeof(pvalue->valuedouble);
            ptlv->type = model_data_type_double;
        }
    }
    break;
    case bscJSON_NULL:
        ptlv->value = NULL;
        ptlv->length = 0;
        ptlv->type = model_data_type_null;
        break;
    case bscJSON_String:
        ptlv->value = pvalue->valuestring;
        ptlv->length = strlen(pvalue->valuestring);
        ptlv->type = model_data_type_string;
        break;
    case bscJSON_Array:
    {
        ptlv->value = bscJSON_PrintUnformatted(pvalue);
        if (NULL == ptlv->value)
        {
            rv = -1;
            break;
        }

        ptlv->length = strlen(ptlv->value);
        ptlv->type = model_data_type_array;
        break;
    }
    case bscJSON_Object:
    {
        ptlv->value = bscJSON_PrintUnformatted(pvalue);
        if (NULL == ptlv->value)
        {
            rv = -1;
            break;
        }

        ptlv->length = strlen(ptlv->value);
        ptlv->type = model_data_type_object;
        break;
    }
    default:
        rv = -1;
        break;
    }

    return rv;
}

static void tlv_destroy(ez_model_msg *ptlv)
{
    if (!ptlv)
    {
        return;
    }

    if (NULL != ptlv->value && (model_data_type_object == ptlv->type || model_data_type_array == ptlv->type))
    {
        free(ptlv->value);
        ptlv->value = NULL;
    }

    memset(ptlv, 0, sizeof(ez_model_msg));
}

static bool is_double(int i, double d)
{
    bool rv = true;

    if (fabs(((double)i) - d) <= DBL_EPSILON && d <= INT_MAX && d >= INT_MIN)
    {
        rv = false;
    }

    return rv;
}
