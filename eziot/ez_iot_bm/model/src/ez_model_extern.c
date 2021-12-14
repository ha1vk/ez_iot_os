/*******************************************************************************
 * Copyright ? 2017-2021 Ezviz Inc.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at
 *   http://www.eclipse.org/org/documents/edl-v10.php.
* Contributors:
 *    shenhongyin - initial API and implementation and/or initial documentation
 *******************************************************************************/
#include <float.h>
#include <limits.h>
#include <math.h>
#include <ezos.h>
#include "ezlog.h"
#include "ezlist.h"
#include "ez_iot_core_def.h"
#include "ez_iot_core_lowlvl.h"

#include "ez_model_bus.h"
#include "ez_model_comm.h"
#include "ez_iot_model_def.h"
#include "ez_model_extern.h"
#include "ez_model_user.h"

typedef struct
{
    ez_node_t node; ///< 双向链表的节点
    ez_domain_reg_t *domain_info;
} ez_domain_node_t;

static ez_int32_t json2tlv(cJSON *pvalue, ez_model_msg_t *ptlv);

/**
 * @brief 判断json number是int或double
 * 
 * @param a pvalue->valueint
 * @param b pvalue->valuedouble
 * @return true double
 * @return false int
 */
static ez_bool_t is_double(int a, double b);

/**
 * @brief destroy the objs obtained from json2tlv
 * 
 * @param ptlv 
 */
static void tlv_destroy(ez_model_msg_t *ptlv);

static ez_list_t g_model_domains;

static ez_mutex_t g_model_domain_lock_h = NULL;
static ez_mutex_t g_model_cb_lock_h     = NULL;

ez_model_default_cb_t g_ez_model_data_router = {NULL,};

/*这里将SDK内部使用的领域单独计数*/
int ez_get_list_size()
{
    int list_size = 0;
    ezos_mutex_lock(g_model_domain_lock_h);
    list_size = ezlist_get_size(&g_model_domains);
    ezos_mutex_unlock(g_model_domain_lock_h);

    return list_size;
}

int ez_set_data_route_cb(ez_model_default_cb_t *ez_data_router)
{
    if (NULL == ez_data_router)
    {
        return EZ_CODE_INVALID_PARAM;
    }
    ezos_mutex_lock(g_model_cb_lock_h);
    g_ez_model_data_router = *ez_data_router;
    ezos_mutex_unlock(g_model_cb_lock_h);

    return EZ_CODE_SUCESS;
}

static ez_domain_node_t *ez_model_find_domain_info(ez_list_t *list, const char *domain)
{
    ez_domain_node_t *pnode = NULL;
    if (NULL == list || 0 == ezlist_get_size(list) || NULL == domain)
    {
        ezlog_e(TAG_MOD, "ez_model_find_domain,list size  is 0 or domain is null\n");
        return NULL;
    }
    LIST_FOR_EACH(ez_domain_node_t, pnode, list)
    {
        if (0 == ezos_strcmp(pnode->domain_info->domain, domain))
        {
            return pnode;
        }
         ezlog_i(TAG_MOD, "node find domain\n");
    }
    return NULL;
}

static int ez_model_add_domain_info(ez_list_t *list, const ez_domain_reg_t *domain_reg)
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
    pnode = (ez_domain_node_t *)ezos_malloc(sizeof(ez_domain_node_t));
    if (NULL == pnode)
    {
        ezlog_e(TAG_MOD, "ez_domain_node_t malloc err\n");
        return EZ_CODE_MODEL_MALLOC_ERR;
    }
    ezos_memset(pnode, 0, sizeof(ez_domain_node_t));

    pnode->domain_info = (ez_domain_reg_t *)ezos_malloc(sizeof(ez_domain_reg_t));
    if (NULL == pnode->domain_info)
    {
        ezlog_e(TAG_MOD, "domain_info malloc err\n");
        ezos_free(pnode);
        pnode = NULL;
        return EZ_CODE_MODEL_MALLOC_ERR;
    }
    ezos_memset(pnode->domain_info, 0, sizeof(ez_domain_reg_t));
    ezos_memcpy(pnode->domain_info, domain_reg, sizeof(ez_domain_reg_t));

    ezlist_add_last(list, &pnode->node);

    ezlog_v(TAG_MOD, "ez_model_add_domain_info success size:%d\n", ezlist_get_size(&g_model_domains));
    return EZ_CODE_SUCESS;
}

int ez_reg_domain(const ez_domain_reg_t *domain_reg)
{
    ez_model_errcode_e ret = EZ_CODE_SUCESS;
    if (NULL == domain_reg->das_reply_router || NULL == domain_reg->das_req_router || 0 == ezos_strlen(domain_reg->domain))
    {
        return EZ_CODE_INVALID_PARAM;
    }
    ezos_mutex_lock(g_model_domain_lock_h);
    ret = ez_model_add_domain_info(&g_model_domains, domain_reg);
    ezos_mutex_unlock(g_model_domain_lock_h);
    if (EZ_CODE_SUCESS == ret)
    {
        ezlog_v(TAG_MOD, "Domain reg success  size:%d\n", ezlist_get_size(&g_model_domains));
    }
    return ret;
}

int ez_dereg_domain(const char *domain)
{
    ez_domain_node_t *pnode = NULL;

    ezos_mutex_lock(g_model_domain_lock_h);
    pnode = ez_model_find_domain_info(&g_model_domains, domain);
    if (pnode)
    {
        if (pnode->domain_info)
        {
            ezos_free(pnode->domain_info);
            pnode->domain_info = NULL;
        }
        ezlist_delete(&g_model_domains, &pnode->node);
        ezos_free(pnode);
        pnode = NULL;
    }
    ezos_mutex_unlock(g_model_domain_lock_h);

    ezlog_v(TAG_MOD, "Domain dereg success  size:%d\n", ezlist_get_size(&g_model_domains));
    return EZ_CODE_SUCESS;
}

/** 
 *  \brief		根据domain找到领域上下文
 *  \method		ez_get_reg_domain
 *  \param[in] 	domain	领域
 *  \return		成功返回领域上下文指针 失败返回NULL
 */
ez_domain_reg_t *ez_get_reg_domain(const char *domain)
{
    ez_domain_node_t *pnode_domain = NULL;
    if (NULL == domain)
    {
        ezlog_e(TAG_MOD, "ez_get_reg_domain input err\n");
        return NULL;
    }
    ezos_mutex_lock(g_model_domain_lock_h);
    pnode_domain = ez_model_find_domain_info(&g_model_domains, domain);
    ezos_mutex_unlock(g_model_domain_lock_h);
    if (NULL == pnode_domain)
    {
        ezlog_e(TAG_MOD, "ez_model no find_domain_info n");
        return NULL;
    }
    return pnode_domain->domain_info;
}

static void ez_parse_das_reply(void *pjsroot, ez_err_info_t *err_info, ez_model_msg_t *data)
{
    cJSON *pjsstatus = NULL;
    cJSON *pjscode = NULL;
    cJSON *pjserr = NULL;
    cJSON *pjsdata = NULL;
    cJSON *pjsvalue = NULL;
    if (NULL == pjsroot || NULL == err_info || NULL == data)
    {
        ezlog_e(TAG_MOD, "ez_parse_status input invalid\n");
        return;
    }

    pjsstatus = cJSON_GetObjectItem((cJSON *)pjsroot, "status");
    if (pjsstatus)
    {
        err_info->status = pjsstatus->valueint;
    }
    pjscode = cJSON_GetObjectItem(pjsroot, "code");
    if (pjscode && cJSON_String == pjscode->type)
    {
        ezos_strncpy(err_info->err_code, pjscode->valuestring, EZ_ERR_CODE_LEN - 1);
    }

    pjserr = cJSON_GetObjectItem(pjsroot, "errorMsg");
    if (pjserr && cJSON_String == pjserr->type)
    {
        ezos_strncpy(err_info->err_msg, pjserr->valuestring, EZ_ERR_MSG_LEN - 1);
    }
    pjsdata = cJSON_GetObjectItem(pjsroot, "data");
    if (pjsdata)
    {
        if (NULL != (pjsvalue = cJSON_GetObjectItem(pjsdata, "Value")))
        {
            if (0 != json2tlv(pjsvalue, data))
            {
                ezlog_v(TAG_MOD, "json2tlv err\n");
            }
        }
        else
        {
            if (0 != json2tlv(pjsdata, data))
            {
                ezlog_v(TAG_MOD, "json2tlv err\n");
            }
        }
    }
    ezlog_v(TAG_MOD, "code:%s, status:%d, err_msg:%s\n", err_info->err_code, err_info->status, err_info->err_msg);
}

static void ez_attribute_data_route(ez_basic_info_t *basic_info, char *msg_type, void *buf, unsigned int msg_seq, ez_domain_reg_t *domain_info)
{
    char *ptr_str = NULL;
    cJSON *pjsRoot = NULL;
    cJSON *pjsValue = NULL;
    cJSON *pjsData = NULL;
    ez_model_msg_t tlv;

    ez_err_info_t err_info;
    ezos_memset(&err_info, 0, sizeof(ez_err_info_t));
    ezos_memset(&tlv, 0, sizeof(ez_model_msg_t));

    if (NULL == basic_info || NULL == buf || NULL == domain_info)
    {
        ezlog_e(TAG_MOD, "recv attribute msg, buf is null\n");
        return;
    }
    ezlog_i(TAG_MOD, "recv attribute msg:domain_id:%s,identifier:%s \n", basic_info->domain, basic_info->identifier);
    do
    {
        pjsRoot = cJSON_Parse((const char *)buf);
        if (NULL == pjsRoot)
        {
            ezlog_e(TAG_MOD, "attribute_data_route JSON_Parse err \n");
            break;
        }
        ptr_str = ezos_strstr(msg_type, "reply");
        if (ptr_str)
        {
            ez_parse_das_reply((void *)pjsRoot, &err_info, &tlv);
            if (domain_info->das_reply_router)
            {
                domain_info->das_reply_router(basic_info, msg_type, &err_info, &tlv, msg_seq);
                ezlog_v(TAG_MOD, " das_reply_router success:%s, seq:%d \n", basic_info->domain, msg_seq);
                break;
            }
        }
        if (0 == ezos_strcmp(msg_type, "set"))
        {
            pjsData = cJSON_GetObjectItem(pjsRoot, "data");
            if (NULL == pjsData)
            {
                ezlog_e(TAG_MOD, "attribute get item data err \n");
                break;
            }
            pjsValue = cJSON_GetObjectItem(pjsData, "Value");
            if (NULL == pjsValue)
            {
                ezlog_e(TAG_MOD, "attribute get item Value err \n");
                break;
            }

            if (0 != json2tlv(pjsValue, &tlv))
            {
                ezlog_e(TAG_MOD, "attribute json print err \n");
                break;
            }
        }
        else
        {
            if (0 != json2tlv(pjsValue, &tlv))
            {
                ezlog_e(TAG_MOD, "attribute json print err \n");
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
        cJSON_Delete(pjsRoot);
    }
}

static void ez_service_data_route(ez_basic_info_t *basic_info, char *msg_type, void *buf, unsigned int msg_seq, ez_domain_reg_t *domain_info)
{
    char *ptr_str = NULL;
    cJSON *pjsRoot = NULL;
    cJSON *pjsData = NULL;
    ez_model_msg_t tlv;
    ez_err_info_t err_info;

    if (NULL == basic_info || NULL == buf || NULL == msg_type || NULL == domain_info)
    {
        ezlog_e(TAG_MOD, "recv service msg, buf is null\n");
        return;
    }

    ezos_memset(&err_info, 0, sizeof(ez_err_info_t));
    ezos_memset(&tlv, 0, sizeof(ez_model_msg_t));

    ezlog_v(TAG_MOD, "recv service msg:resource_id:%s,resource_type:%s, msg_type:%s \n", basic_info->domain, basic_info->identifier, msg_type);
    do
    {
        pjsRoot = cJSON_Parse((const char *)buf);
        if (NULL == pjsRoot)
        {
            ezlog_e(TAG_MOD, "service recv JSON_Parse err \n");
            break;
        }
        ptr_str = ezos_strstr(msg_type, "reply");
        if (ptr_str)
        {
            ezlog_v(TAG_MOD, "recv service reply:domain_id:%s,identifier:%s \n", basic_info->domain, basic_info->identifier);
            ez_parse_das_reply((void *)pjsRoot, &err_info, &tlv);
            if (domain_info->das_reply_router)
            {
                domain_info->das_reply_router(basic_info, msg_type, &err_info, &tlv, msg_seq);
                ezlog_v(TAG_MOD, "das_reply_router:domain_id:%s, seq:%d \n", basic_info->domain, msg_seq);
            }
            break;
        }
        pjsData = cJSON_GetObjectItem(pjsRoot, "data");
        if (NULL == pjsData)
        {
            ezlog_e(TAG_MOD, "service recv get item <data> err \n");
            break;
        }

        if (0 != json2tlv(pjsData, &tlv))
        {
            ezlog_e(TAG_MOD, "attribute json print err \n");
            break;
        }

        if (domain_info->das_req_router)
        {
            domain_info->das_req_router(basic_info, msg_type, &tlv, msg_seq);
        }

    } while (0);

    ezlog_v(TAG_MOD, "recv service msg:resource_id:%s,resource_type:%s, msg_type:%s \n", basic_info->domain, basic_info->identifier, msg_type);

    tlv_destroy(&tlv);

    if (pjsRoot)
    {
        cJSON_Delete(pjsRoot);
    }
}

void ez_model_data_route(ez_kernel_submsg_v3_t *ptr_submsg)
{
    char *str_ext_msg = NULL;
    int domain_len = 0;
    ez_basic_info_t basic_info;
    ez_domain_reg_t *domain_info = NULL;
    do
    {
        if (NULL == ptr_submsg->buf)
        {
            ezlog_e(TAG_MOD, "model_data_route msg input buf is NULL \n");
            break;
        }

        ezos_memset(&basic_info, 0, sizeof(ez_basic_info_t));

        ezlog_v(TAG_MOD, "model_data_route recv msg:%s \n", (char *)ptr_submsg->buf);

        ezlog_d(TAG_MOD, "msg_type:%s ,ext_msg:%s ,seq:%d\n", ptr_submsg->msg_type, ptr_submsg->ext_msg, ptr_submsg->msg_seq);
        if (0 == ezos_strcmp(ptr_submsg->method, "event"))
        {
            basic_info.type = ez_event;
        }
        else if (0 == ezos_strcmp(ptr_submsg->method, "attribute"))
        {
            basic_info.type = ez_attribute;
        }
        else if (0 == ezos_strcmp(ptr_submsg->method, "service"))
        {
            basic_info.type = ez_service;
        }

        ezlog_d(TAG_MOD, "resource_id:%s ,resource_type:%s\n", ptr_submsg->resource_id, ptr_submsg->resource_type);

        ezos_strncpy(basic_info.resource_type, ptr_submsg->resource_type, sizeof(basic_info.resource_type) - 1);
        ezos_strncpy(basic_info.resource_id, ptr_submsg->resource_id, sizeof(basic_info.resource_id) - 1);
        ezos_strncpy(basic_info.subserial, ptr_submsg->sub_serial, sizeof(basic_info.subserial) - 1);

        str_ext_msg = ezos_strstr(ptr_submsg->ext_msg, "/");
        if (NULL == str_ext_msg)
        {
            ezlog_e(TAG_MOD, "ext_msg format error\n");
            return;
        }
        ezos_strncpy(basic_info.identifier, str_ext_msg + 1, sizeof(basic_info.identifier) - 1);

        domain_len = ezos_strlen(ptr_submsg->ext_msg) - ezos_strlen(str_ext_msg);
        if (domain_len > sizeof(basic_info.identifier) - 1)
        {
            ezlog_e(TAG_MOD, "identifier len is over ranage:%d\n", domain_len);
            return;
        }
        ezos_strncpy(basic_info.domain, ptr_submsg->ext_msg, domain_len);
        ezlog_v(TAG_MOD, "type:%d,domain:%s, identifier:%s\n", basic_info.type, basic_info.domain, basic_info.identifier);

        domain_info = ez_get_reg_domain(basic_info.domain);
        if (NULL == domain_info)
        {
            ezlog_w(TAG_MOD, "domain not register:%s \n", basic_info.domain);
            ezos_mutex_lock(g_model_cb_lock_h);
            if (g_ez_model_data_router.ez_model_default_cb_t)
            {
                g_ez_model_data_router.ez_model_default_cb_t(&basic_info, ptr_submsg->msg_type, ptr_submsg->buf, ptr_submsg->buf_len, ptr_submsg->msg_seq);
            }
            ezos_mutex_unlock(g_model_cb_lock_h);
            break;
        }
        ezlog_i(TAG_MOD, "data router:identifier:%s\n", basic_info.identifier);
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

static void ez_model_event_route(ez_kernel_event_t* ptr_event)
{
	ezlog_i(TAG_OTA,"model event router,type: %d\n", ptr_event->event_type);
    switch(ptr_event->event_type)
    {
        case SDK_KERNEL_EVENT_ONLINE:
        case SDK_KERNEL_EVENT_RECONNECT:
            {
                ezlog_d(TAG_OTA,"model login\n");
            }
            break;
        case SDK_KERNEL_EVENT_BREAK:
            {
                ezlog_d(TAG_OTA,"model offline\n");
            }
            break;
        case SDK_KERNEL_EVENT_PUBLISH_ACK:
            {
                ez_kernel_publish_ack_t *err_ctx = (ez_kernel_publish_ack_t *)ptr_event->event_context;
                ezlog_d(TAG_OTA,"ez model_run_time err: code:%d, seq:%d \n", err_ctx->last_error, err_ctx->msg_seq);
            }
            break;
        default:
            break;
    }
}


int ez_model_extern_init()
{
    int ret = EZ_CODE_SUCESS;
    ez_err_t ezRv = EZ_CORE_ERR_SUCC;

    ez_kernel_extend_v3_t extern_info;
    ezos_memset(&extern_info, 0, sizeof(ez_kernel_extend_v3_t));
    do
    {
        extern_info.ez_kernel_data_route = ez_model_data_route;
        extern_info.ez_kernel_event_route = ez_model_event_route;
        ezos_strncpy(extern_info.module, "model", ezdev_sdk_module_name_len - 1);
        if (EZ_CORE_ERR_SUCC != (ezRv = ez_kernel_extend_load_v3(&extern_info)))
        {
            ezlog_e(TAG_MOD, "model load err:%0x\n", ezRv);
            ret = EZ_CODE_MODEL_REG_FAIL;
        }

        ezlist_init(&g_model_domains);

        g_model_domain_lock_h = ezos_mutex_create();
        if(NULL == g_model_domain_lock_h)
        {
            ezlog_e(TAG_MOD, "domain_lock create faild\n");
            ret = EZ_CODE_MODEL_MALLOC_ERR;
            break;
        }
        g_model_cb_lock_h = ezos_mutex_create();
        if(NULL == g_model_cb_lock_h)
        {
            ezlog_e(TAG_MOD, "cb_lock create faild\n");
            ret = EZ_CODE_MODEL_MALLOC_ERR;
            break;
        }

    }while(0);
    
    if(EZ_CODE_SUCESS!=ret)
    {
        if(NULL!=g_model_domain_lock_h)
        {
            ezos_mutex_destroy(g_model_domain_lock_h);
        }
    }

    return ret;
}

void ez_model_extern_deinit()
{
    if(NULL!=g_model_domain_lock_h)
    {
        ezos_mutex_destroy(g_model_domain_lock_h);
    }

    if(NULL!=g_model_cb_lock_h)
    {
       ezos_mutex_destroy(g_model_cb_lock_h); 
    }

}

static ez_int32_t json2tlv(cJSON *pvalue, ez_model_msg_t *ptlv)
{
    ez_int32_t rv = 0;

    if (!pvalue || !ptlv)
        return -1;

    ezos_memset(ptlv, 0, sizeof(ez_model_msg_t));
    switch (pvalue->type)
    {
    case cJSON_False:
    case cJSON_True:
        ptlv->value_bool = (ez_bool_t)pvalue->valueint;
        ptlv->length = sizeof(pvalue->valueint);
        ptlv->type = model_data_type_bool;
        break;
    case cJSON_Number:
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
    case cJSON_NULL:
        ptlv->value = NULL;
        ptlv->length = 0;
        ptlv->type = model_data_type_null;
        break;
    case cJSON_String:
        ptlv->value = pvalue->valuestring;
        ptlv->length = ezos_strlen(pvalue->valuestring);
        ptlv->type = model_data_type_string;
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
        ptlv->type = model_data_type_array;
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
        ptlv->type = model_data_type_object;
        break;
    }
    default:
        rv = -1;
        break;
    }

    return rv;
}

static void tlv_destroy(ez_model_msg_t *ptlv)
{
    if (!ptlv)
    {
        return;
    }

    if (NULL != ptlv->value && (model_data_type_object == ptlv->type || model_data_type_array == ptlv->type))
    {
        ezos_free(ptlv->value);
        ptlv->value = NULL;
    }

    ezos_memset(ptlv, 0, sizeof(ez_model_msg_t));
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
