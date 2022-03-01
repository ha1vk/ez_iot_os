/*******************************************************************************
 * Copyright © 2017-2021 Ezviz Inc.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 * 
 * Contributors:
 * XuRongjun (xurongjun@ezvizlife.com)
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-11-25     xurongjun    first version 
 *******************************************************************************/

#include "ez_iot_base_protocol.h"
#include "ez_iot_base.h"
#include "ez_iot_base_def.h"
#include "ez_iot_core_lowlvl.h"
#include "ez_iot_base_ctx.h"
#include "ezxml.h"
#include "cJSON.h"

#define MSG_TYPE_REQ 0
#define MSG_TYPE_RSP 1
#define CIVIL_RESULT_GENERAL_NO_ERROR 0x00000000     ///< 无错误
#define CIVIL_RESULT_GENERAL_PARSE_FAILED 0x00000003 ///< 报文解析失败
#define CIVIL_RESULT_GENERAL_SYSTEM_ERROR 0x00000004 ///< 系统内部错误

typedef ez_void_t (*profile_query_rsp)(const ez_char_t *rsp_msg);

static ezxml_t xml_make_rsp(ez_int32_t result);
static cJSON *json_make_rsp(ez_int32_t result);
static ez_err_t dev2cloud_msg_send(ez_char_t *buf, ez_int32_t domain_id, ez_int32_t cmd_id,
                                   ez_char_t *cmd_version, ez_uint8_t msg_response, ez_uint32_t msg_seq);

static profile_query_rsp g_profile_rsp_func = NULL;

void cloud2dev_set_ntpinfo_req(void *buf, ez_int32_t len)
{
    cJSON *pRoot = cJSON_Parse(buf);
    cJSON *NTP = NULL;
    cJSON *json = NULL;
    ez_base_ntp_info_t ntp_info = {0};

    do
    {
        if (NULL == pRoot)
        {
            break;
        }

        NTP = cJSON_GetObjectItem(pRoot, "NTP");
        if (NULL == NTP)
        {
            break;
        }

        json = cJSON_GetObjectItem(NTP, "Addr");
        if (NULL != json)
        {
            ezos_strncpy(ntp_info.host, json->valuestring, sizeof(ntp_info.host) - 1);
        }

        json = cJSON_GetObjectItem(NTP, "Port");
        if (NULL != json)
        {
            ntp_info.port = json->valueint;
        }

        json = cJSON_GetObjectItem(NTP, "Interval");
        if (NULL != json)
        {
            ntp_info.interval = json->valueint / 60;
        }

        json = cJSON_GetObjectItem(NTP, "Timezone");
        if (NULL != json)
        {
            ezos_strncpy(ntp_info.timezone, json->valuestring, sizeof(ntp_info.timezone) - 1);
        }

        base_notice_get()(EZ_EVENT_NTP_INFO, &ntp_info, sizeof(ntp_info));

    } while (0);

    if (NULL != pRoot)
    {
        cJSON_Delete(pRoot);
    }
}

ez_err_t cloud2dev_xml_req_bushandle(ez_void_t *buf, ez_int32_t len, ez_int32_t rsp_cmd,
                                     ez_uint32_t seq, handle_proc_fun proc_func)
{
    ezxml_t rspxml = NULL;

    ez_char_t *result_str = NULL;
    ez_int32_t result = proc_func(buf, len);

    if (NULL != (rspxml = xml_make_rsp(result)) &&
        NULL != (result_str = ezxml_toxml(rspxml)))
    {
        result = dev2cloud_msg_send(result_str, BASE_DOMAIN_ID, rsp_cmd, BASE_DOMAIN_VER, MSG_TYPE_RSP, seq);
    }

    if (result_str)
        ezos_free(result_str);

    if (rspxml)
        ezxml_free(rspxml);

    return result;
}

ez_err_t cloud2dev_json_req_bushandle(ez_void_t *buf, ez_int32_t len, ez_int32_t rsp_cmd,
                                      ez_uint32_t seq, handle_proc_fun proc_func)
{
    cJSON *rspjson = NULL;

    ez_char_t *result_str = NULL;
    ez_int32_t result = proc_func(buf, len);

    if (NULL != (rspjson = json_make_rsp(result)) &&
        NULL != (result_str = cJSON_PrintUnformatted(rspjson)))
    {
        result = dev2cloud_msg_send(result_str, BASE_DOMAIN_ID, rsp_cmd, BASE_DOMAIN_VER, MSG_TYPE_RSP, seq);
    }

    if (result_str)
        ezos_free(result_str);

    if (rspjson)
        cJSON_Delete(rspjson);

    return result;
}

ez_err_t base_protocol_bind_status_notice_req(ez_void_t *buf, ez_int32_t len)
{
    ez_err_t rv = CIVIL_RESULT_GENERAL_NO_ERROR;
    ezxml_t req_xml = NULL;
    ezxml_t userid_xml = NULL;
    ez_char_t *name = NULL;

    req_xml = ezxml_parse_str(buf, len);
    CHECK_COND_DONE(!req_xml, CIVIL_RESULT_GENERAL_PARSE_FAILED);

    name = ezxml_name(req_xml);
    CHECK_COND_DONE(!name, CIVIL_RESULT_GENERAL_PARSE_FAILED);
    CHECK_COND_DONE(ezos_strcmp(name, "Request"), CIVIL_RESULT_GENERAL_PARSE_FAILED);

    userid_xml = ezxml_child(req_xml, "UserId");
    CHECK_COND_DONE(!userid_xml, CIVIL_RESULT_GENERAL_PARSE_FAILED);

    if (0 == ezos_strlen(ezxml_txt(userid_xml)))
    {
        base_notice_get()(EZ_EVENT_UNBINDING, NULL, 0);
    }
    else
    {
        ez_bind_info_t bind_info = {0};
        ezos_strncpy(bind_info.user_id, ezxml_txt(userid_xml), sizeof(bind_info.user_id) - 1);
        base_notice_get()(EZ_EVENT_BINDING, &bind_info, sizeof(bind_info));
    }

done:

    if (req_xml)
        ezxml_free(req_xml);

    return rv;
}

ez_err_t base_protocol_bind_status_query_req()
{
    ez_err_t rv = EZ_BASE_ERR_SUCC;
    ezxml_t xml_root = ezxml_new("Request");
    ezxml_t xml_devserial = NULL;
    ezxml_t xml_auth = NULL;
    ez_char_t *req_str = NULL;

    CHECK_COND_DONE(!xml_root, EZ_BASE_ERR_MEMORY);

    xml_devserial = ezxml_add_child(xml_root, "DevSerial", 0);
    xml_auth = ezxml_add_child(xml_root, "Authorization", 0);

    CHECK_COND_DONE(!xml_devserial, EZ_BASE_ERR_MEMORY);
    CHECK_COND_DONE(!xml_auth, EZ_BASE_ERR_MEMORY);

    ezxml_set_txt(xml_devserial, ez_kernel_getdevinfo_bykey("dev_subserial"));
    req_str = ezxml_toxml(xml_root);
    CHECK_COND_DONE(!req_str, EZ_BASE_ERR_MEMORY);

    rv = dev2cloud_msg_send(req_str, BASE_DOMAIN_ID, kPu2CenPltGetUserListReq, BASE_DOMAIN_VER, MSG_TYPE_REQ, 0);
    CHECK_COND_DONE(!req_str, EZ_BASE_ERR_GENERAL);

done:
    if (xml_root)
        ezxml_free(xml_root);

    if (req_str)
        ezos_free(req_str);

    return rv;
}

ez_err_t base_protocol_bind_status_query_rsp(ez_void_t *buf, ez_int32_t len)
{
    ez_err_t rv = EZ_BASE_ERR_SUCC;
    ezxml_t root_xml = NULL;
    ezxml_t result_xml = NULL;
    ezxml_t user_xml = NULL;
    ez_char_t *name = NULL;

    root_xml = ezxml_parse_str(buf, len);
    CHECK_COND_DONE(!root_xml, EZ_BASE_ERR_MEMORY);

    name = ezxml_name(root_xml);
    CHECK_COND_DONE(!name, EZ_BASE_ERR_GENERAL);
    CHECK_COND_DONE(ezos_strcmp(name, "Response"), EZ_BASE_ERR_GENERAL);

    result_xml = ezxml_child(root_xml, "Result");
    CHECK_COND_DONE(!result_xml, EZ_BASE_ERR_GENERAL);

    if (0x101c02 == ezos_atoi(ezxml_txt(result_xml)))
    {
        base_notice_get()(EZ_EVENT_UNBINDING, NULL, 0);
        goto done;
    }

    user_xml = ezxml_child(root_xml, "User");
    if (!user_xml)
    {
        base_notice_get()(EZ_EVENT_UNBINDING, NULL, 0);
    }
    else
    {
        ez_bind_info_t bind_info = {0};
        ezos_strncpy(bind_info.user_id, ezxml_attr(user_xml, "Id"), sizeof(bind_info.user_id) - 1);
        base_notice_get()(EZ_EVENT_BINDING, &bind_info, sizeof(bind_info));
    }

done:
    if (root_xml)
        ezxml_free(root_xml);

    return rv;
}

ez_err_t base_protocol_near_bind_req(ez_char_t *token)
{
    ez_err_t rv = EZ_BASE_ERR_SUCC;
    cJSON *root = NULL;
    ez_char_t *json_str = NULL;

    root = cJSON_CreateObject();
    CHECK_COND_DONE(!root, EZ_BASE_ERR_MEMORY);

    cJSON_AddStringToObject(root, "token", (const ez_char_t *)token);
    json_str = cJSON_PrintUnformatted(root);
    CHECK_COND_DONE(!json_str, EZ_BASE_ERR_MEMORY);

    rv = dev2cloud_msg_send(json_str, BASE_DOMAIN_ID, kPu2CenPltBindUserWithTokenReq, BASE_DOMAIN_VER, MSG_TYPE_REQ, 0);
    CHECK_COND_DONE(rv, EZ_BASE_ERR_GENERAL);

done:
    if (json_str)
        ezos_free(json_str);

    if (root)
        cJSON_Delete(root);

    return rv;
}

ez_err_t base_protocol_bind_challenge_req(ez_void_t *buf, ez_int32_t len)
{
    ez_err_t rv = CIVIL_RESULT_GENERAL_NO_ERROR;
    cJSON *root = NULL;
    cJSON *token = NULL;
    cJSON *bindPeriod = NULL;
    ez_bind_challenge_t challenge_info = {0};

    root = cJSON_Parse(buf);
    CHECK_COND_DONE(!root, CIVIL_RESULT_GENERAL_PARSE_FAILED);

    token = cJSON_GetObjectItem(root, "token");
    CHECK_COND_DONE(!token, CIVIL_RESULT_GENERAL_PARSE_FAILED);
    CHECK_COND_DONE(cJSON_Number != token->type, CIVIL_RESULT_GENERAL_PARSE_FAILED);

    bindPeriod = cJSON_GetObjectItem(root, "bindPeriod");
    CHECK_COND_DONE(!bindPeriod, CIVIL_RESULT_GENERAL_PARSE_FAILED);
    CHECK_COND_DONE(cJSON_Number != bindPeriod->type, CIVIL_RESULT_GENERAL_PARSE_FAILED);

    challenge_info.challenge_code = token->valueint;
    challenge_info.validity_period = bindPeriod->valueint;

    rv = base_notice_get()(EZ_EVENT_BINDING_CHALLENGE, &challenge_info, sizeof(challenge_info));
    CHECK_COND_DONE(rv, CIVIL_RESULT_GENERAL_SYSTEM_ERROR);

done:

    if (NULL != root)
    {
        cJSON_Delete(root);
    }

    return rv;
}

ez_err_t base_protocol_bind_response_req(ez_int32_t response_code)
{
    ez_err_t rv = EZ_BASE_ERR_SUCC;
    cJSON *root = NULL;
    ez_char_t *json_str = NULL;

    root = cJSON_CreateObject();
    CHECK_COND_DONE(!root, EZ_BASE_ERR_MEMORY);

    cJSON_AddNumberToObject(root, "token", response_code);
    json_str = cJSON_PrintUnformatted(root);
    CHECK_COND_DONE(!json_str, EZ_BASE_ERR_MEMORY);

    rv = dev2cloud_msg_send(json_str, BASE_DOMAIN_ID, kPu2CenPltReportBindUserTouchWithTokenReq, BASE_DOMAIN_VER, MSG_TYPE_REQ, 0);
    CHECK_COND_DONE(rv, EZ_BASE_ERR_GENERAL);

done:
    if (NULL != json_str)
        ezos_free(json_str);

    if (NULL != root)
        cJSON_Delete(root);

    return rv;
}

ez_err_t base_protocol_query_profile_req(const ez_char_t *req_msg, ez_void_t *func_rsp)
{
    ez_err_t rv = EZ_BASE_ERR_SUCC;

    g_profile_rsp_func = (profile_query_rsp)func_rsp;
    rv = dev2cloud_msg_send((ez_char_t *)req_msg, BASE_DOMAIN_ID, Pu2CenPltQueryFeatureProfileReq,
                            BASE_DOMAIN_VER, MSG_TYPE_REQ, 0);
    CHECK_COND_DONE(rv, EZ_BASE_ERR_GENERAL);

done:
    return rv;
}

ez_err_t base_protocol_query_profile_rsp(ez_void_t *buf, ez_int32_t len)
{
    g_profile_rsp_func(buf);

    return EZ_BASE_ERR_SUCC;
}

static ezxml_t xml_make_rsp(ez_int32_t result)
{
    ezxml_t xml_root = ezxml_new("Response");
    ezxml_t xml_result = NULL;
    ez_char_t buf[32] = {0};

    if (NULL == xml_root)
        return NULL;

    sprintf(buf, "%d", (int)result);
    xml_result = ezxml_add_child(xml_root, "Result", 0);
    if (NULL == xml_result)
    {
        ezxml_free(xml_root);
        return NULL;
    }

    ezxml_set_txt(xml_result, buf);

    return xml_root;
}

static cJSON *json_make_rsp(ez_int32_t result)
{
    cJSON *root = cJSON_CreateObject();
    if (NULL == root)
    {
        return NULL;
    }

    cJSON_AddNumberToObject(root, "result", result);

    return root;
}

static ez_err_t dev2cloud_msg_send(ez_char_t *buf, ez_int32_t domain_id, ez_int32_t cmd_id,
                                   ez_char_t *cmd_version, ez_uint8_t msg_response, ez_uint32_t msg_seq)
{
    ez_kernel_pubmsg_t pubmsg = {0};

    pubmsg.msg_response = msg_response;
    pubmsg.msg_seq = msg_seq;
    pubmsg.msg_body = buf;
    pubmsg.msg_body_len = ezos_strlen(buf);

    pubmsg.msg_domain_id = domain_id;
    pubmsg.msg_command_id = cmd_id;

    ezos_strncpy(pubmsg.command_ver, cmd_version, sizeof(pubmsg.command_ver) - 1);

    return ez_kernel_send(&pubmsg);
}