#include "ez_iot_shadow.h"
#include "ez_iot_shadow_protocol.h"
#include "ez_iot_shadow_def.h"
#include "ez_iot_core_lowlvl.h"
#include "ez_iot_core_def.h"
#include "cJSON.h"
#include "ezlog.h"

#define SHADOW_PROTOCOL_SET_REPLY "{\"method\":\"set_reply\",\"payload\":{\"code\":%d},\"timestamp\":%lu}"
#define SHADOW_PROTOCOL_QUERY "{\"method\":\"query\",\"windowsize\":%d}"

ez_err_t shadow_protocol_query_desired(ez_char_t *devsn, ez_char_t *res_type, ez_int32_t index)
{
    ez_err_t rv = EZ_SHD_ERR_SUCC;
    char buf[128] = {0};
    snprintf(buf, sizeof(buf), SHADOW_PROTOCOL_QUERY, SHADOW_PUSH_BUF_MAX);

    ez_kernel_pubmsg_v3_t pubmsg = {0};
    pubmsg.msg_qos = QOS_T1;
    pubmsg.msg_body = buf;
    pubmsg.msg_body_len = ezos_strlen(buf);

    ezos_strncpy(pubmsg.sub_serial, devsn, sizeof(pubmsg.sub_serial) - 1);
    ezos_strncpy(pubmsg.resource_type, res_type, sizeof(pubmsg.resource_type) - 1);
    ezos_strncpy(pubmsg.module, SHADOW_MODULE_NAME, sizeof(pubmsg.module) - 1);
    ezos_strncpy(pubmsg.method, SHADOW_METHOD_NAME, sizeof(pubmsg.module) - 1);
    ezos_strncpy(pubmsg.msg_type, SHADOW_MSG_TYPE_QUERY, sizeof(pubmsg.msg_type) - 1);
    ezos_sprintf(pubmsg.resource_id, "%d", index);

    CHECK_COND_DONE(ez_kernel_send_v3(&pubmsg), EZ_SHD_ERR_GENERAL);

done:

    return rv;
}

ez_err_t shadow_protocol_report(ez_char_t *devsn, ez_char_t *res_type, ez_uint16_t index, ez_char_t *domain, ez_char_t *key,
                                ez_void_t *json_value, ez_uint32_t ver, ez_uint32_t *seq)
{
    ez_err_t rv = EZ_SHD_ERR_SUCC;
    cJSON *pstJsRoot = cJSON_CreateObject();
    cJSON *pstJsState = cJSON_CreateObject();
    cJSON *pstReported = cJSON_CreateObject();
    ez_kernel_pubmsg_v3_t pubmsg = {0};

    do
    {
        if (!pstJsRoot || !pstJsState || !pstReported)
        {
            rv = EZ_SHD_ERR_MEMORY;
            cJSON_Delete((cJSON *)json_value);
            break;
        }

        cJSON_AddStringToObject(pstReported, "domain", domain);
        cJSON_AddStringToObject(pstReported, "identifier", key);
        cJSON_AddItemToObject(pstReported, "value", (cJSON *)json_value);
        cJSON_AddItemToObject(pstJsState, "reported", pstReported);
        cJSON_AddNumberToObject(pstJsState, "version", ver);
        cJSON_AddStringToObject(pstJsRoot, "method", "report");
        cJSON_AddNumberToObject(pstJsRoot, "spv", 3);
        cJSON_AddItemToObject(pstJsRoot, "state", pstJsState);
        pubmsg.msg_body = cJSON_PrintUnformatted(pstJsRoot);
        if (NULL == pubmsg.msg_body)
        {
            rv = EZ_SHD_ERR_MEMORY;
            break;
        }

        pubmsg.msg_qos = QOS_T1;
        pubmsg.msg_body_len = ezos_strlen(pubmsg.msg_body);

        ezos_strncpy(pubmsg.sub_serial, devsn, sizeof(pubmsg.sub_serial) - 1);
        ezos_strncpy(pubmsg.resource_type, res_type, sizeof(pubmsg.resource_type) - 1);
        ezos_strncpy(pubmsg.module, SHADOW_MODULE_NAME, sizeof(pubmsg.module) - 1);
        ezos_strncpy(pubmsg.method, SHADOW_METHOD_NAME, sizeof(pubmsg.module) - 1);
        ezos_strncpy(pubmsg.msg_type, SHADOW_MSG_TYPE_REPORT, sizeof(pubmsg.msg_type) - 1);
        ezos_sprintf(pubmsg.resource_id, "%d", index);

        if (0 != ez_kernel_send_v3(&pubmsg))
        {
            rv = EZ_SHD_ERR_GENERAL;
            ezlog_e(TAG_SHD, "kernel send v3:%d", rv);
            break;
        }

        *seq = pubmsg.msg_seq;
    } while (0);

    if (pstJsRoot)
    {
        cJSON_Delete(pstJsRoot);
    }

    if (pubmsg.msg_body)
    {
        ezos_free(pubmsg.msg_body);
    }

    return rv;
}

ez_err_t shadow_protocol_set_reply(ez_char_t *devsn, ez_char_t *res_type, ez_int32_t index, ez_int32_t code, ez_int32_t seq)
{
    ez_err_t rv = EZ_SHD_ERR_SUCC;
    char protocal_reply[256] = {0};
    sprintf(protocal_reply, SHADOW_PROTOCOL_SET_REPLY, code, ezos_time(NULL));

    ez_kernel_pubmsg_v3_t pubmsg = {0};
    pubmsg.msg_response = 1;
    pubmsg.msg_qos = QOS_T0;
    pubmsg.msg_seq = seq;
    pubmsg.msg_body_len = ezos_strlen(protocal_reply);
    pubmsg.msg_body = protocal_reply;

    ezos_strncpy(pubmsg.sub_serial, devsn, sizeof(pubmsg.sub_serial) - 1);
    ezos_strncpy(pubmsg.resource_type, res_type, sizeof(pubmsg.resource_type) - 1);
    ezos_strncpy(pubmsg.module, SHADOW_MODULE_NAME, sizeof(pubmsg.module) - 1);
    ezos_strncpy(pubmsg.method, SHADOW_METHOD_NAME, sizeof(pubmsg.module) - 1);
    ezos_strncpy(pubmsg.msg_type, SHADOW_MSG_TYPE_SET_REPLY, sizeof(pubmsg.msg_type) - 1);
    ezos_sprintf(pubmsg.resource_id, "%d", index);

    if (0 != ez_kernel_send_v3(&pubmsg))
    {
        rv = EZ_SHD_ERR_GENERAL;
    }

    return rv;
}