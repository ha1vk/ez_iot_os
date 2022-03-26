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
 *******************************************************************************/

#include <ezos.h>
#include <ezlog.h>
#include "sdk_kernel_def.h"
#include "das_transport.h"
#include "mkernel_internal_error.h"
#include "dev_protocol_def.h"
#include "ezdev_sdk_kernel_queue.h"
#include "cJSON.h"
#include "MQTTClient.h"
#include "MQTTPorting.h"
#include "aes_support.h"
#include "ezdev_sdk_kernel_extend.h"
#include "ezdev_sdk_kernel_risk_control.h"
#include "ezdev_sdk_kernel_event.h"
#include "access_domain_bus.h"
#include "utils.h"
#include "net_module.h"

#define MSG_TYPE_REQ 1
#define MSG_TYPE_RSP 2

EXTERN_QUEUE_FUN(submsg)
EXTERN_QUEUE_FUN(pubmsg_exchange)
EXTERN_QUEUE_FUN(submsg_v3)
EXTERN_QUEUE_FUN(pubmsg_exchange_v3)

EXTERN_QUEUE_BASE_FUN
AES_SUPPORT_INTERFACE
EZDEV_SDK_KERNEL_EXTEND_INTERFACE
ACCESS_DOMAIN_BUS_INTERFACE
EZDEV_SDK_KERNEL_RISK_CONTROL_INTERFACE
EZDEV_SDK_KERNEL_EVENT_INTERFACE

extern ezdev_sdk_kernel g_ezdev_sdk_kernel;

MQTTClient g_DasClient;
Network g_DasNetWork;
unsigned char g_sendbuf[CONFIG_EZIOT_CORE_MESSAGE_SIZE_MAX];
unsigned char g_readbuf[CONFIG_EZIOT_CORE_MESSAGE_SIZE_MAX];
EZDEV_SDK_UINT32 g_das_transport_seq; ///<	与DAS通信数据包seq
static EZDEV_SDK_BOOL g_is_first_session = EZDEV_SDK_TRUE;

static mkernel_internal_error das_subscribe_revc_topic(ezdev_sdk_kernel *sdk_kernel, EZDEV_SDK_INT8 open);
static mkernel_internal_error das_message_send(ezdev_sdk_kernel *sdk_kernel);
static mkernel_internal_error das_send_pubmsg(ezdev_sdk_kernel *sdk_kernel, ezdev_sdk_kernel_pubmsg *pubmsg);

static void MQTTNetInit(Network *net_work);
static mkernel_internal_error MQTTNetConnect(Network *net_work, char *ip, int port);
static int MQTTNet_read(Network *n, unsigned char *buffer, int len, int timeout_ms);
static int MQTTNet_write(Network *n, unsigned char *buffer, int len, int timeout_ms);
static void MQTTNetDisconnect(Network *net_work);
static void MQTTNetFini(Network *net_work);

static mkernel_internal_error serialize_payload_common_v3(EZDEV_SDK_UINT32 msg_seq, unsigned char **output_buf, EZDEV_SDK_UINT16 *output_length)
{
    mkernel_internal_error sdk_error = mkernel_internal_succ;
    char *payload_common_jsonstring = NULL;
    cJSON *pJsonRoot = NULL;
    do
    {
        pJsonRoot = cJSON_CreateObject();
        if (NULL == pJsonRoot)
        {
            sdk_error = mkernel_internal_malloc_error;
            break;
        }
        cJSON_AddNumberToObject(pJsonRoot, "Seq", msg_seq);
        payload_common_jsonstring = cJSON_PrintUnformatted(pJsonRoot);
        if (NULL == payload_common_jsonstring)
        {
            sdk_error = mkernel_internal_json_format_error;
            break;
        }
        *output_buf = (unsigned char *)payload_common_jsonstring;
        *output_length = ezos_strlen(payload_common_jsonstring);
    } while (0);

    if (NULL != pJsonRoot)
    {
        cJSON_Delete(pJsonRoot);
        pJsonRoot = NULL;
    }
    return sdk_error;
}

static mkernel_internal_error serialize_payload_common(EZDEV_SDK_INT8 msg_type, const char *cmd_version, EZDEV_SDK_UINT32 msg_seq, unsigned char **output_buf, EZDEV_SDK_UINT16 *output_length)
{
    mkernel_internal_error sdk_error = mkernel_internal_succ;
    char *payload_common_jsonstring = NULL;
    cJSON *pJsonRoot = NULL;
    do
    {
        pJsonRoot = cJSON_CreateObject();
        if (NULL == pJsonRoot)
        {
            sdk_error = mkernel_internal_malloc_error;
            break;
        }
        cJSON_AddStringToObject(pJsonRoot, "CmdVer", cmd_version);
        cJSON_AddNumberToObject(pJsonRoot, "Seq", msg_seq);
        cJSON_AddNumberToObject(pJsonRoot, "MsgType", msg_type);

        payload_common_jsonstring = cJSON_PrintUnformatted(pJsonRoot);
        if (NULL == payload_common_jsonstring)
        {
            sdk_error = mkernel_internal_json_format_error;
            break;
        }
        *output_buf = (unsigned char *)payload_common_jsonstring;
        *output_length = ezos_strlen(payload_common_jsonstring);
    } while (0);

    if (NULL != pJsonRoot)
    {
        cJSON_Delete(pJsonRoot);
        pJsonRoot = NULL;
    }
    return sdk_error;
}

static void serialize_short(unsigned char buf[2], EZDEV_SDK_UINT16 src_short)
{
    buf[0] = (unsigned char)(src_short / 256);
    buf[1] = (unsigned char)(src_short % 256);
}

static EZDEV_SDK_UINT16 deserialize_short(unsigned char buf[2])
{
    EZDEV_SDK_UINT16 src_short = buf[0] * 256 + buf[1];
    return src_short;
}

static mkernel_internal_error serialize_lightreginfo(ezdev_sdk_kernel *sdk_kernel, unsigned char **output_buf, EZDEV_SDK_UINT32 *output_length)
{
    char *lightreg_jsstr = NULL;
    EZDEV_SDK_UINT32 jsonstring_len = 0;
    EZDEV_SDK_UINT32 jsonstring_len_padding = 0;
    unsigned char *input_buf = NULL;
    unsigned char *enc_output_buf = NULL;
    mkernel_internal_error sdk_error = mkernel_internal_succ;
    cJSON *pJsonRoot = NULL;
    char dev_id[ezdev_sdk_devid_len + 1] = {0};

    do
    {
        pJsonRoot = cJSON_CreateObject();
        if (NULL == pJsonRoot)
        {
            sdk_error = mkernel_internal_malloc_error;
            break;
        }
        ezos_memcpy(dev_id, sdk_kernel->dev_id, ezdev_sdk_devid_len);
        dev_id[ezdev_sdk_devid_len] = '\0';
        cJSON_AddStringToObject(pJsonRoot, "SubSerial", sdk_kernel->dev_info.dev_subserial);
        cJSON_AddStringToObject(pJsonRoot, "DeviceID", dev_id);
        lightreg_jsstr = cJSON_PrintUnformatted(pJsonRoot);
        if (lightreg_jsstr == NULL)
        {
            sdk_error = mkernel_internal_json_format_error;
            break;
        }
        jsonstring_len = ezos_strlen(lightreg_jsstr);
        jsonstring_len_padding = calculate_padding_len(jsonstring_len);
        input_buf = (unsigned char *)ezos_malloc(jsonstring_len_padding);
        enc_output_buf = (unsigned char *)ezos_malloc(jsonstring_len_padding);
        if (enc_output_buf == NULL)
        {
            sdk_error = mkernel_internal_malloc_error;
            break;
        }

        if (input_buf == NULL)
        {
            ezos_free(enc_output_buf);
            enc_output_buf = NULL;
            sdk_error = mkernel_internal_malloc_error;
            break;
        }
        ezos_memset(enc_output_buf, 0, jsonstring_len_padding);
        ezos_memset(input_buf, 0, jsonstring_len_padding);
        ezos_memcpy(input_buf, lightreg_jsstr, jsonstring_len);
        sdk_error = aes_cbc_128_enc_padding(sdk_kernel->session_key, input_buf, jsonstring_len, jsonstring_len_padding, enc_output_buf, output_length);
        if (sdk_error != mkernel_internal_succ)
        {
            ezos_free(enc_output_buf);
            enc_output_buf = NULL;
            break;
        }
        *output_buf = enc_output_buf;
    } while (0);

    if (pJsonRoot != NULL)
    {
        cJSON_Delete(pJsonRoot);
    }
    if (lightreg_jsstr != NULL)
    {
        ezos_free(lightreg_jsstr);
        lightreg_jsstr = NULL;
    }
    if (input_buf != NULL)
    {
        ezos_free(input_buf);
        input_buf = NULL;
    }

    return sdk_error;
}

static mkernel_internal_error serialize_devinfo(ezdev_sdk_kernel *sdk_kernel, unsigned char **output_buf, EZDEV_SDK_UINT32 *output_length)
{
    char *devinfo_jsonstring = NULL;
    EZDEV_SDK_UINT32 devinfo_jsonstring_len = 0;
    EZDEV_SDK_UINT32 devinfo_jsonstring_len_padding = 0;
    unsigned char *input_buf = NULL;

    unsigned char *enc_output_buf = NULL;

    mkernel_internal_error sdk_error = mkernel_internal_succ;
    cJSON *pJsonRoot = NULL;
    char dev_id[ezdev_sdk_devid_len + 1] = {0};
    do
    {
        pJsonRoot = cJSON_CreateObject();
        if (NULL == pJsonRoot)
        {
            sdk_error = mkernel_internal_malloc_error;
            break;
        }
        ezos_memcpy(dev_id, sdk_kernel->dev_id, ezdev_sdk_devid_len);
        dev_id[ezdev_sdk_devid_len] = '\0';
        cJSON_AddStringToObject(pJsonRoot, "DevSerial", sdk_kernel->dev_info.dev_serial);
        cJSON_AddStringToObject(pJsonRoot, "SubSerial", sdk_kernel->dev_info.dev_subserial);
        cJSON_AddStringToObject(pJsonRoot, "FirmwareVersion", sdk_kernel->dev_info.dev_firmwareversion);
        cJSON_AddStringToObject(pJsonRoot, "DevType", sdk_kernel->dev_info.dev_type);
        cJSON_AddStringToObject(pJsonRoot, "DevTypeDisplay", sdk_kernel->dev_info.dev_typedisplay);
        cJSON_AddStringToObject(pJsonRoot, "MAC", sdk_kernel->dev_info.dev_mac);
        cJSON_AddNumberToObject(pJsonRoot, "Status", sdk_kernel->dev_info.dev_status);
        cJSON_AddStringToObject(pJsonRoot, "FirmwareIdentificationCode", sdk_kernel->dev_info.dev_firmwareidentificationcode);
        cJSON_AddStringToObject(pJsonRoot, "LbsDomain", sdk_kernel->server_info.server_name);
        cJSON_AddNumberToObject(pJsonRoot, "RegMode", sdk_kernel->reg_mode);
        cJSON_AddStringToObject(pJsonRoot, "SDKMainVersion", sdk_kernel->szMainVersion);
        cJSON_AddStringToObject(pJsonRoot, "DeviceID", dev_id);

        sdk_error = extend_serialize_sdk_version(pJsonRoot);
        if (sdk_error != mkernel_internal_succ)
        {
            break;
        }
        devinfo_jsonstring = cJSON_PrintUnformatted(pJsonRoot);
        if (devinfo_jsonstring == NULL)
        {
            sdk_error = mkernel_internal_json_format_error;
            break;
        }
        devinfo_jsonstring_len = ezos_strlen(devinfo_jsonstring);
        devinfo_jsonstring_len_padding = calculate_padding_len(devinfo_jsonstring_len);
        input_buf = (unsigned char *)ezos_malloc(devinfo_jsonstring_len_padding);
        enc_output_buf = (unsigned char *)ezos_malloc(devinfo_jsonstring_len_padding);
        if (enc_output_buf == NULL)
        {
            sdk_error = mkernel_internal_malloc_error;
            break;
        }

        if (input_buf == NULL)
        {
            ezos_free(enc_output_buf);
            enc_output_buf = NULL;
            sdk_error = mkernel_internal_malloc_error;
            break;
        }

        ezos_memset(enc_output_buf, 0, devinfo_jsonstring_len_padding);
        ezos_memset(input_buf, 0, devinfo_jsonstring_len_padding);
        ezos_memcpy(input_buf, devinfo_jsonstring, devinfo_jsonstring_len);

        sdk_error = aes_cbc_128_enc_padding(sdk_kernel->session_key,
                                            (unsigned char *)input_buf, devinfo_jsonstring_len, devinfo_jsonstring_len_padding,
                                            enc_output_buf, output_length);
        if (sdk_error != mkernel_internal_succ)
        {
            ezos_free(enc_output_buf);
            enc_output_buf = NULL;
            break;
        }
        *output_buf = enc_output_buf;
    } while (0);

    if (pJsonRoot != NULL)
    {
        cJSON_Delete(pJsonRoot);
        pJsonRoot = NULL;
    }
    if (devinfo_jsonstring != NULL)
    {
        ezos_free(devinfo_jsonstring);
        devinfo_jsonstring = NULL;
    }
    if (input_buf != NULL)
    {
        ezos_free(input_buf);
        input_buf = NULL;
    }

    return sdk_error;
}

static mkernel_internal_error deserialize_common_v3(unsigned char *common_buf, EZDEV_SDK_UINT16 common_buf_len, ezdev_sdk_kernel_submsg_v3 *ptr_submsg)
{
    /**
	* \brief  解析通用协议体
	*/
    mkernel_internal_error sdk_error = mkernel_internal_succ;
    cJSON *json_item = NULL;
    cJSON *json_seq_item = NULL;

    do
    {
        json_item = cJSON_Parse((const char *)common_buf);
        if (json_item == NULL)
        {
            ezlog_d(TAG_CORE, "deserialize_common Parse json error");
            sdk_error = mkernel_internal_json_parse_error;
            break;
        }
        json_seq_item = cJSON_GetObjectItem(json_item, "Seq");

        if (json_seq_item == NULL)
        {
            ezlog_d(TAG_CORE, "deserialize_common Parse seq miss feild error");
            sdk_error = mkernel_internal_json_parse_error;
            break;
        }

        if (json_seq_item->type != cJSON_Number)
        {
            ezlog_d(TAG_CORE, "deserialize_common Parse BusiVer or CmdVer type error");
            sdk_error = mkernel_internal_json_parse_error;
            break;
        }
        ptr_submsg->msg_seq = json_seq_item->valueint;

    } while (0);

    if (json_item != NULL)
    {
        cJSON_Delete(json_item);
        json_item = NULL;
    }

    return sdk_error;
}

static mkernel_internal_error deserialize_common(unsigned char *common_buf, EZDEV_SDK_UINT16 common_buf_len, ezdev_sdk_kernel_submsg *ptr_submsg)
{
    /**
	* \brief  解析通用协议体
	*/
    mkernel_internal_error sdk_error = mkernel_internal_succ;
    cJSON *json_item = NULL;
    cJSON *json_seq_item = NULL;
    cJSON *json_cmdver_item = NULL;

    do
    {
        json_item = cJSON_Parse((const char *)common_buf);
        if (json_item == NULL)
        {
            ezlog_d(TAG_CORE, "deserialize_common Parse json error");
            sdk_error = mkernel_internal_json_parse_error;
            break;
        }
        json_seq_item = cJSON_GetObjectItem(json_item, "Seq");
        json_cmdver_item = cJSON_GetObjectItem(json_item, "CmdVer");
        if (json_cmdver_item == NULL || json_seq_item == NULL)
        {
            ezlog_d(TAG_CORE, "deserialize_common Parse BusiVer or CmdVer miss feild error");
            sdk_error = mkernel_internal_json_parse_error;
            break;
        }

        if (json_seq_item->type != cJSON_Number ||
            json_cmdver_item->type != cJSON_String || NULL == json_cmdver_item->valuestring)
        {
            ezlog_d(TAG_CORE, "deserialize_common Parse BusiVer or CmdVer type error");
            sdk_error = mkernel_internal_json_parse_error;
            break;
        }

        if (ezos_strlen(json_cmdver_item->valuestring) >= version_max_len)
        {
            ezos_strncpy(ptr_submsg->command_ver, json_cmdver_item->valuestring, version_max_len - 1);
        }
        else
        {
            ezos_strncpy(ptr_submsg->command_ver, json_cmdver_item->valuestring, ezos_strlen(json_cmdver_item->valuestring));
        }

        ptr_submsg->msg_seq = json_seq_item->valueint;

    } while (0);

    if (json_item != NULL)
    {
        cJSON_Delete(json_item);
        json_item = NULL;
    }
    return sdk_error;
}

static void handle_sub_msg_v3(ezdev_sdk_kernel_submsg_v3 *ptr_submsg)
{
    EZDEV_SDK_BOOL is_delete = EZDEV_SDK_TRUE;
    mkernel_internal_error kernel_error = mkernel_internal_succ;
    kernel_error = push_queue_submsg_v3(ptr_submsg);
    if (kernel_error != mkernel_internal_succ)
    {
        ezlog_d(TAG_CORE, "push_queue_submsg v3 error,module:%s, seq:%d", ptr_submsg->module, ptr_submsg->msg_seq);
    }
    else
    {
        is_delete = EZDEV_SDK_FALSE;
    }

    if (is_delete)
    {
        if (ptr_submsg->buf != NULL)
        {
            ezos_free(ptr_submsg->buf);
            ptr_submsg->buf = NULL;
        }

        ezos_free(ptr_submsg);
        ptr_submsg = NULL;
    }
}

static void handle_sub_msg(ezdev_sdk_kernel_submsg *ptr_submsg)
{
    EZDEV_SDK_BOOL is_delete = EZDEV_SDK_TRUE;
    mkernel_internal_error kernel_error = mkernel_internal_succ;

    if (ptr_submsg->msg_domain_id == DAS_CMD_DOMAIN)
    {
        kernel_error = access_domain_bus_handle(ptr_submsg);
    }
    else if (ptr_submsg->msg_domain_id == DAS_CMD_COMMON_FUN || ptr_submsg->msg_command_id == DAS_CMD_PU2CENPLTUPGRADERSP)
    {
        is_delete = EZDEV_SDK_FALSE;
    }
    else
    {
        kernel_error = push_queue_submsg(ptr_submsg);
        if (kernel_error != mkernel_internal_succ)
        {
            ezlog_d(TAG_CORE, "handle_sub_msg push_queue_submsg error,demain:%d, cmd:%d", ptr_submsg->msg_domain_id, ptr_submsg->msg_command_id);
        }
        else
        {
            is_delete = EZDEV_SDK_FALSE;
        }
    }
    if (is_delete)
    {
        if (ptr_submsg->buf != NULL)
        {
            ezos_free(ptr_submsg->buf);
            ptr_submsg->buf = NULL;
        }

        ezos_free(ptr_submsg);
        ptr_submsg = NULL;
    }
}

static mkernel_internal_error ezdev_parse_topic(ezdev_sdk_kernel_submsg_v3 *ptr_submsg, char *topic, char *find_str)
{
    char *ptemp1 = NULL;
    char *ptemp2 = NULL;
    int temp_len = 0;
    int find_str_len = 0;
    mkernel_internal_error sdk_error = mkernel_internal_succ;
    if (NULL == ptr_submsg || NULL == topic || NULL == find_str)
    {
        return mkernel_internal_input_param_invalid;
    }
    do
    {
        find_str_len = ezos_strlen(find_str);
        ptemp1 = ezos_strstr(topic, find_str);
        if (!ptemp1)
        {
            ezlog_e(TAG_CORE, "ezdev_parse_topic, not find:%s", find_str);
            sdk_error = mkernel_internal_platform_appoint_error;
            break;
        }
        ptemp2 = ezos_strrchr(ptemp1, '/');
        if (!ptemp2)
        {
            sdk_error = mkernel_internal_platform_appoint_error;
            break;
        }
        temp_len = ptemp2 - ptemp1 - find_str_len;
        if (temp_len <= 0 || temp_len > sizeof(ptr_submsg->method))
        {
            ezlog_e(TAG_CORE, "ezdev_parse_topic temp_len, calculate err:%d", temp_len);
            sdk_error = mkernel_internal_platform_appoint_error;
            break;
        }
        ezos_strncpy(ptr_submsg->msg_type, ptemp2 + 1, sizeof(ptr_submsg->msg_type) - 1);
        ezos_strncpy(ptr_submsg->method, ptemp1 + find_str_len, temp_len);

    } while (0);

    return sdk_error;
}

static void das_message_receive_v3(MessageData *msg_data)
{
    /**
	* \brief  topic /{领域编号}/{指令编号}
	*/
    mkernel_internal_error sdk_error = mkernel_internal_succ;
    ezdev_sdk_kernel_submsg_v3 *ptr_submsg = NULL;
    EZDEV_SDK_INT32 division_num = 0;
    unsigned char *output_buf = NULL;
    EZDEV_SDK_UINT32 output_buf_len = 0;
    EZDEV_SDK_UINT16 common_len = 0;
    unsigned char common_len_buf[2];
    char find_str[32] = {0};
    char msg_topic[512];
    char dev_serial[ezdev_sdk_devserial_maxlen];

    ezos_memset(common_len_buf, 0, 2);
    ezos_memset(msg_topic, 0, 512);
    ezos_memset(dev_serial, 0, ezdev_sdk_devserial_maxlen);
    do
    {
        ptr_submsg = (ezdev_sdk_kernel_submsg_v3 *)ezos_malloc(sizeof(ezdev_sdk_kernel_submsg_v3));
        if (ptr_submsg == NULL)
        {
            ezlog_e(TAG_CORE, "das_message_receive_v3 mallc submsg error ");
            sdk_error = mkernel_internal_malloc_error;
            break;
        }
        ezos_memset(ptr_submsg, 0, sizeof(ezdev_sdk_kernel_submsg_v3));

        if (msg_data->topicName->lenstring.len >= 512)
        {
            ezlog_e(TAG_CORE, "  recv a das v3 msg which size is too len:%d", msg_data->topicName->lenstring.len);
            sdk_error = mkernel_internal_platform_appoint_error;
            break;
        }
        ezos_strncpy(msg_topic, msg_data->topicName->lenstring.data, msg_data->topicName->lenstring.len);

        division_num = sscanf(msg_topic, "/iot/%72[^/]/%72[^/]/%64[^-]-%64[^/]/%16[^/]/", dev_serial, (char *)&ptr_submsg->sub_serial, (char *)&ptr_submsg->resource_id, (char *)&ptr_submsg->resource_type,
                              (char *)&ptr_submsg->module);
        if (division_num != 5)
        {
            ezlog_e(TAG_CORE, " decode common topic err :%s", msg_topic);
            sdk_error = mkernel_internal_platform_appoint_error;
            break;
        }

        snprintf(find_str, 32, "%s/", ptr_submsg->module);
        if (0 == ezos_strcmp(ptr_submsg->module, "model"))
        {
            division_num = sscanf(msg_topic, "/iot/%72[^/]/%72[^/]/%64[^-]-%64[^/]/model/%64[^/]/%32[^/]/%s", dev_serial, (char *)&ptr_submsg->sub_serial, (char *)&ptr_submsg->resource_id, (char *)&ptr_submsg->resource_type,
                                  (char *)&ptr_submsg->method, (char *)&ptr_submsg->msg_type, (char *)&ptr_submsg->ext_msg);
            if (division_num != 7)
            {
                ezlog_e(TAG_CORE, "decode model topic error :%s", msg_topic);
                sdk_error = mkernel_internal_platform_appoint_error;
                break;
            }
        }
        else
        {
            sdk_error = ezdev_parse_topic(ptr_submsg, msg_topic, find_str);
            if (mkernel_internal_succ != sdk_error)
            {
                break;
            }
        }
        //将整个报文解密
        output_buf = (unsigned char *)ezos_malloc(msg_data->message->payloadlen);
        if (NULL == output_buf)
        {
            ezlog_d(TAG_CORE, "receive_v3 malloc err,module:%s", ptr_submsg->module);
            sdk_error = mkernel_internal_malloc_error;
            break;
        }
        ezos_memset(output_buf, 0, msg_data->message->payloadlen);

        sdk_error = aes_cbc_128_dec_padding(get_ezdev_sdk_kernel()->session_key, (unsigned char *)msg_data->message->payload, msg_data->message->payloadlen, output_buf, &output_buf_len);
        if (sdk_error != mkernel_internal_succ)
        {
            ezlog_d(TAG_CORE, "receive_v3 aes_128_dec_padding err,module:%s, seq:%d", ptr_submsg->module, ptr_submsg->msg_seq);
            break;
        }

        ezos_memcpy(common_len_buf, output_buf, 2);
        common_len = deserialize_short(common_len_buf);
        if (common_len >= output_buf_len)
        {
            ezlog_e(TAG_CORE, "v3  common len decode err, module:%s, seq:%d", ptr_submsg->module, ptr_submsg->msg_seq);
            sdk_error = mkernel_internal_rev_invalid_packet;
            break;
        }
        sdk_error = deserialize_common_v3((unsigned char *)output_buf + 2, common_len, ptr_submsg);
        if (sdk_error != mkernel_internal_succ)
        {
            ezlog_d(TAG_CORE, "v3 deserialize_common err,module:%s, seq:%d", ptr_submsg->module, ptr_submsg->msg_seq);
            break;
        }
        if (output_buf_len - 2 - common_len > 0)
        {
            ezos_memmove(output_buf, output_buf + 2 + common_len, output_buf_len - 2 - common_len);
            ezos_memset(output_buf + output_buf_len - 2 - common_len, 0, 2 + common_len);
            ptr_submsg->buf = output_buf;
            ptr_submsg->buf_len = output_buf_len - 2 - common_len;
        }
        else
        {
            /**
			* \brief   数据为空
			*/
            //数据为空包的时候不能直接返回空指针,这里malloc一个字节的空间
            ptr_submsg->buf = ezos_malloc(sizeof(char));
            if (NULL == ptr_submsg->buf)
            {
                ezlog_e(TAG_CORE, "parse das msg context is empty,malloc err");
                sdk_error = mkernel_internal_malloc_error;
                break;
            }
            ezos_memset(ptr_submsg->buf, 0, sizeof(char));
            ptr_submsg->buf_len = sizeof(char);
            ezlog_d(TAG_CORE, "Recv v3 empty context  module:%s, seq:%d", ptr_submsg->module, ptr_submsg->msg_seq);

            ezos_free(output_buf);
            output_buf = NULL;
        }

        ezlog_d(TAG_CORE, "receive topic:%s, seq:%d", msg_topic, ptr_submsg->msg_seq);
        ezlog_v(TAG_CORE, "receive payload:%s", ptr_submsg->buf);
    } while (0);

    /**
	* \brief   成功返回
	*/
    if (sdk_error == mkernel_internal_succ)
    {
        handle_sub_msg_v3(ptr_submsg);
        return;
    }

    if (ptr_submsg != NULL)
    {
        ezos_free(ptr_submsg);
        ptr_submsg = NULL;
    }
    if (output_buf != NULL)
    {
        ezos_free(output_buf);
        output_buf = NULL;
    }
}

void das_message_receive_ex(MessageData *msg_data)
{
    mkernel_internal_error sdk_error = mkernel_internal_succ;
    ezdev_sdk_kernel_submsg *ptr_submsg = NULL;
    EZDEV_SDK_INT32 division_num = 0;
    EZDEV_SDK_UINT16 common_len = 0;
    unsigned char *output_buf = NULL;
    EZDEV_SDK_UINT32 output_buf_len = 0;
    char msg_topic[128];
    unsigned char common_len_buf[2];
    char dev_serial[ezdev_sdk_devserial_maxlen];
    ezdev_sdk_kernel_domain_info *kernel_domain = NULL;

    ezos_memset(msg_topic, 0, 128);
    ezos_memset(common_len_buf, 0, 2);
    ezos_memset(dev_serial, 0, ezdev_sdk_devserial_maxlen);

    if (msg_data == NULL)
    {
        goto fail;
    }

    ptr_submsg = (ezdev_sdk_kernel_submsg *)ezos_malloc(sizeof(ezdev_sdk_kernel_submsg));
    if (ptr_submsg == NULL)
    {
        ezlog_e(TAG_CORE, "das_message_receive_ex mallc submsg error ");
        sdk_error = mkernel_internal_malloc_error;
        goto fail;
    }
    ezos_memset(ptr_submsg, 0, sizeof(ezdev_sdk_kernel_submsg));
    if (msg_data->topicName->lenstring.len >= 128)
    {
        ezlog_e(TAG_CORE, "das_message_receive_ex recv a msg which size is too len:%d", msg_data->topicName->lenstring.len);
        sdk_error = mkernel_internal_msg_len_overrange;
        goto fail;
    }

    ezos_strncpy(msg_topic, msg_data->topicName->lenstring.data, msg_data->topicName->lenstring.len);

    division_num = sscanf(msg_topic, "/%16[^/]/%d/%d", dev_serial, &ptr_submsg->msg_domain_id, &ptr_submsg->msg_command_id);
    if (division_num != 3)
    {
        ezlog_e(TAG_CORE, "das_message_receive_ex decode topicName error :%s", msg_topic);
        sdk_error = mkernel_internal_msg_len_overrange;
        goto fail;
    }
    output_buf = (unsigned char *)ezos_malloc(msg_data->message->payloadlen);
    if (NULL == output_buf)
    {
        ezlog_e(TAG_CORE, "das_message_receive_ex malloc error,domain:%d, cmd:%d, lenstring len:%d", ptr_submsg->msg_domain_id, ptr_submsg->msg_command_id, msg_data->topicName->lenstring.len);
        sdk_error = mkernel_internal_malloc_error;
        goto fail;
    }
    ezos_memset(output_buf, 0, msg_data->message->payloadlen);

    sdk_error = aes_cbc_128_dec_padding(get_ezdev_sdk_kernel()->session_key, (unsigned char *)msg_data->message->payload, msg_data->message->payloadlen, output_buf, &output_buf_len);
    if (sdk_error != mkernel_internal_succ)
    {
        ezlog_d(TAG_CORE, "das_message_receive_ex aes_cbc_128_dec_padding error,domain:%d, cmd:%d", ptr_submsg->msg_domain_id, ptr_submsg->msg_command_id);
        goto fail;
    }

    ezos_memcpy(common_len_buf, output_buf, 2);
    common_len = deserialize_short(common_len_buf);
    if (common_len >= output_buf_len)
    {
        ezlog_d(TAG_CORE, "das_message_receive_ex decode error common len, domain:%d, cmd:%d", ptr_submsg->msg_domain_id, ptr_submsg->msg_command_id);
        sdk_error = mkernel_internal_msg_len_overrange;
        goto fail;
    }
    sdk_error = deserialize_common((unsigned char *)output_buf + 2, common_len, ptr_submsg);
    if (sdk_error != mkernel_internal_succ)
    {
        ezlog_d(TAG_CORE, "das_message_receive_ex deserialize_common error,domain:%d, cmd:%d", ptr_submsg->msg_domain_id, ptr_submsg->msg_command_id);
        goto fail;
    }
    if (output_buf_len - 2 - common_len > 0)
    {
        ezos_memmove(output_buf, output_buf + 2 + common_len, output_buf_len - 2 - common_len);
        ezos_memset(output_buf + output_buf_len - 2 - common_len, 0, 2 + common_len);
        ptr_submsg->buf = output_buf;
        ptr_submsg->buf_len = output_buf_len - 2 - common_len;
    }
    else
    {
        ptr_submsg->buf = NULL;
        ptr_submsg->buf_len = 0;

        ezos_free(output_buf);
        output_buf = NULL;
    }
    ezlog_d(TAG_CORE, "das_message_receive_ex msg_topic:%s, seq:%d", msg_topic, ptr_submsg->msg_seq);

    if (sdk_error == mkernel_internal_succ)
    {
        kernel_domain = extend_get(ptr_submsg->msg_domain_id);

        if (kernel_domain)
        {
            kernel_domain->kernel_extend.ezdev_sdk_kernel_extend_data_route(ptr_submsg, kernel_domain->kernel_extend.pUser);
        }
        else
        {
            ezlog_e(TAG_CORE, "das_message_receive_ex find domain error %d", ptr_submsg->msg_domain_id);
        }
    }
fail:
    ezlog_e(TAG_CORE, "das_message_receive_ex end");

    if (ptr_submsg != NULL)
    {
        ezos_free(ptr_submsg);
        ptr_submsg = NULL;
    }
    if (output_buf != NULL)
    {
        ezos_free(output_buf);
        output_buf = NULL;
    }
}

static void das_message_receive(MessageData *msg_data)
{
    /**
	* \brief  topic /{领域编号}/{指令编号}
	*/
    mkernel_internal_error sdk_error = mkernel_internal_succ;
    ezdev_sdk_kernel_submsg *ptr_submsg = NULL;
    EZDEV_SDK_INT32 division_num = 0;
    EZDEV_SDK_UINT16 common_len = 0;
    unsigned char *output_buf = NULL;
    EZDEV_SDK_UINT32 output_buf_len = 0;
    char msg_topic[128];
    unsigned char common_len_buf[2];
    char dev_serial[ezdev_sdk_devserial_maxlen];

    ezos_memset(msg_topic, 0, 128);
    ezos_memset(common_len_buf, 0, 2);
    ezos_memset(dev_serial, 0, ezdev_sdk_devserial_maxlen);
    do
    {
        ptr_submsg = (ezdev_sdk_kernel_submsg *)ezos_malloc(sizeof(ezdev_sdk_kernel_submsg));
        if (ptr_submsg == NULL)
        {
            ezlog_d(TAG_CORE, "das_message_receive mallc submsg error ");
            sdk_error = mkernel_internal_malloc_error;
            break;
        }
        ezos_memset(ptr_submsg, 0, sizeof(ezdev_sdk_kernel_submsg));

        if (msg_data->topicName->lenstring.len >= 128)
        {
            ezlog_e(TAG_CORE, "das_message_receive recv a msg which size is too len:%d", msg_data->topicName->lenstring.len);
            sdk_error = mkernel_internal_platform_appoint_error;
            break;
        }

        ezos_strncpy(msg_topic, msg_data->topicName->lenstring.data, msg_data->topicName->lenstring.len);
        division_num = sscanf(msg_topic, "/%72[^/]/%d/%d", dev_serial, &ptr_submsg->msg_domain_id, &ptr_submsg->msg_command_id);
        if (division_num != 3)
        {
            ezlog_e(TAG_CORE, "das_message_receive decode topicName error :%s", msg_topic);
            sdk_error = mkernel_internal_platform_appoint_error;
            break;
        }

        /**
		 * \brief   将整个报文解密
		 */
        output_buf = (unsigned char *)ezos_malloc(msg_data->message->payloadlen);
        if (NULL == output_buf)
        {
            ezlog_d(TAG_CORE, "das_message_receive mallc error,domain:%d, cmd:%d, len:%d", ptr_submsg->msg_domain_id, ptr_submsg->msg_command_id, msg_data->topicName->lenstring.len);
            sdk_error = mkernel_internal_malloc_error;
            break;
        }
        ezos_memset(output_buf, 0, msg_data->message->payloadlen);

        sdk_error = aes_cbc_128_dec_padding(get_ezdev_sdk_kernel()->session_key, (unsigned char *)msg_data->message->payload, msg_data->message->payloadlen, output_buf, &output_buf_len);
        if (sdk_error != mkernel_internal_succ)
        {
            ezlog_d(TAG_CORE, "das_message_receive aes_cbc_128_dec_padding error,domain:%d, cmd:%d", ptr_submsg->msg_domain_id, ptr_submsg->msg_command_id);
            break;
        }
        ezos_memcpy(common_len_buf, output_buf, 2);
        common_len = deserialize_short(common_len_buf);
        if (common_len >= output_buf_len)
        {
            ezlog_e(TAG_CORE, "das_message_receive decode error common len, domain:%d, cmd:%d", ptr_submsg->msg_domain_id, ptr_submsg->msg_command_id);
            sdk_error = mkernel_internal_rev_invalid_packet;
            break;
        }
        sdk_error = deserialize_common((unsigned char *)output_buf + 2, common_len, ptr_submsg);
        if (sdk_error != mkernel_internal_succ)
        {
            ezlog_d(TAG_CORE, "das_message_receive deserialize_common error,domain:%d, cmd:%d", ptr_submsg->msg_domain_id, ptr_submsg->msg_command_id);
            break;
        }
        if (output_buf_len - 2 - common_len > 0)
        {
            ezos_memmove(output_buf, output_buf + 2 + common_len, output_buf_len - 2 - common_len);
            ezos_memset(output_buf + output_buf_len - 2 - common_len, 0, 2 + common_len);
            ptr_submsg->buf = output_buf;
            ptr_submsg->buf_len = output_buf_len - 2 - common_len;
        }
        else
        {
            //数据为空包的时候不能直接返回空指针,这里malloc一个字节的空间
            ptr_submsg->buf = ezos_malloc(sizeof(char));
            if (NULL == ptr_submsg->buf)
            {
                ezlog_e(TAG_CORE, "parse das msg context is empty,malloc err");
                sdk_error = mkernel_internal_malloc_error;
                break;
            }
            ezos_memset(ptr_submsg->buf, 0, sizeof(char));
            ptr_submsg->buf_len = sizeof(char);
            ezlog_d(TAG_CORE, "Recv v2 empty context, domain_id:%d, cmd_id:%d, seq:%d", ptr_submsg->msg_domain_id, ptr_submsg->msg_command_id, ptr_submsg->msg_seq);

            ezos_free(output_buf);
            output_buf = NULL;
        }
        ezlog_d(TAG_CORE, "das_message_receive msg_topic:%s, seq:%d", msg_topic, ptr_submsg->msg_seq);
    } while (0);

    /**
	* \brief   成功返回
	*/
    if (sdk_error == mkernel_internal_succ)
    {
        handle_sub_msg(ptr_submsg);
        return;
    }

    if (ptr_submsg != NULL)
    {
        ezos_free(ptr_submsg);
        ptr_submsg = NULL;
    }
    if (output_buf != NULL)
    {
        ezos_free(output_buf);
        output_buf = NULL;
    }
}

static mkernel_internal_error das_send_pubmsg_v3(ezdev_sdk_kernel *sdk_kernel, ezdev_sdk_kernel_pubmsg_v3 *pubmsg)
{
    MQTTMessage mqtt_msg;
    mkernel_internal_error sdk_error = mkernel_internal_succ;
    EZDEV_SDK_INT32 mqtt_result_code = 0;
    unsigned char *common_output_buf = NULL;
    EZDEV_SDK_UINT32 enc_output_buf_len = 0;
    unsigned char *payload_buf = NULL;
    unsigned char *payload_buf_enc = NULL;
    EZDEV_SDK_UINT32 payload_buf_len = 0;
    EZDEV_SDK_UINT16 common_output_buf_len = 0;
    unsigned char common_len_serialize_buf[2];
    char dev_serial[ezdev_sdk_devserial_maxlen] = {0};
    char dev_subserial[ezdev_sdk_devserial_maxlen] = {0};
    char publish_topic[512] = {0};
    do
    {
        ezos_memset(common_len_serialize_buf, 0, 2);
        ezos_strncpy(dev_serial, sdk_kernel->dev_info.dev_subserial, ezdev_sdk_devserial_maxlen - 1);
        if (ezos_strlen(pubmsg->sub_serial) > 0)
        {
            ezos_strncpy(dev_subserial, pubmsg->sub_serial, ezdev_sdk_devserial_maxlen - 1);
        }
        else
        {
            ezos_strncpy(dev_subserial, "global", ezos_strlen("global") + 1);
        }
        if (0 == ezos_strcmp(pubmsg->module, "model"))
        {
            snprintf(publish_topic, 512, "/iot/%s/%s/%s-%s/model/%s/%s/%s", dev_serial, dev_subserial, pubmsg->resource_id,
                     pubmsg->resource_type, pubmsg->method, pubmsg->msg_type, pubmsg->ext_msg);
        }
        else
        {
            snprintf(publish_topic, 512, "/iot/%s/%s/%s-%s/%s/%s/%s", dev_serial, dev_subserial, pubmsg->resource_id,
                     pubmsg->resource_type, pubmsg->module, pubmsg->method, pubmsg->msg_type);
        }

        ezlog_d(TAG_CORE, "publish topic:%s, seq:%d", publish_topic, pubmsg->msg_seq);
        ezlog_v(TAG_CORE, "payload:%s", pubmsg->msg_body);

        ezos_memset(&mqtt_msg, 0, sizeof(mqtt_msg));
        mqtt_msg.qos = pubmsg->msg_qos;
        mqtt_msg.retained = 1; //平台不关心
        mqtt_msg.dup = 0;
        sdk_error = serialize_payload_common_v3(pubmsg->msg_seq, &common_output_buf, &common_output_buf_len);
        if (sdk_error != mkernel_internal_succ)
        {
            break;
        }
        payload_buf_len = calculate_padding_len(common_output_buf_len + 2 + pubmsg->msg_body_len);
        payload_buf = (unsigned char *)ezos_malloc(payload_buf_len);
        payload_buf_enc = (unsigned char *)ezos_malloc(payload_buf_len);
        if (payload_buf == NULL || payload_buf_enc == NULL)
        {
            ezlog_e(TAG_CORE, "malloc payload len error");
            sdk_error = mkernel_internal_malloc_error;
            break;
        }
        ezos_memset(payload_buf, 0, payload_buf_len);
        ezos_memset(payload_buf_enc, 0, payload_buf_len);
        serialize_short(common_len_serialize_buf, common_output_buf_len);
        ezos_memcpy(payload_buf, common_len_serialize_buf, 2);
        ezos_memcpy(payload_buf + 2, common_output_buf, common_output_buf_len);
        ezos_memcpy(payload_buf + 2 + common_output_buf_len, pubmsg->msg_body, pubmsg->msg_body_len);

        sdk_error = aes_cbc_128_enc_padding(sdk_kernel->session_key, payload_buf, common_output_buf_len + 2 + pubmsg->msg_body_len, payload_buf_len, payload_buf_enc, &enc_output_buf_len);
        if (sdk_error != mkernel_internal_succ)
        {
            break;
        }
        mqtt_msg.payload = (void *)payload_buf_enc;
        mqtt_msg.payloadlen = enc_output_buf_len;
        mqtt_result_code = MQTTPublish(&g_DasClient, publish_topic, &mqtt_msg);

        if (mqtt_result_code == MQTTPACKET_BUFFER_TOO_SHORT)
        {
            ezlog_e(TAG_CORE, "mqtt buffer too short");
            sdk_error = mkernel_internal_call_mqtt_buffer_too_short;
            break;
        }

        if (mqtt_result_code != 0)
        {
            ezlog_e(TAG_CORE, "mqtt publish error");
            sdk_error = mkernel_internal_call_mqtt_pub_error;
            break;
        }

    } while (0);

    if (common_output_buf)
    {
        ezos_free(common_output_buf);
        common_output_buf = NULL;
    }
    if (payload_buf)
    {
        ezos_free(payload_buf);
        payload_buf = NULL;
    }
    if (payload_buf_enc)
    {
        ezos_free(payload_buf_enc);
        payload_buf_enc = NULL;
    }
    return sdk_error;
}

static mkernel_internal_error das_send_pubmsg(ezdev_sdk_kernel *sdk_kernel, ezdev_sdk_kernel_pubmsg *pubmsg)
{
    MQTTMessage mqtt_msg;
    mkernel_internal_error sdk_error = mkernel_internal_succ;
    EZDEV_SDK_INT32 mqtt_result_code = 0;
    unsigned char *common_output_buf = NULL;
    EZDEV_SDK_UINT32 enc_output_buf_len = 0;
    unsigned char *payload_buf = NULL;
    unsigned char *payload_buf_enc = NULL;
    EZDEV_SDK_UINT32 payload_buf_len = 0;
    EZDEV_SDK_UINT16 common_output_buf_len = 0;
    EZDEV_SDK_INT8 msg_type = 0;

    unsigned char common_len_serialize_buf[2];
    char publish_topic[128];
    ezos_memset(common_len_serialize_buf, 0, 2);
    ezos_memset(publish_topic, 0, 128);
    snprintf(publish_topic, 128, "/%d/%d", pubmsg->msg_domain_id, pubmsg->msg_command_id);

    ezos_memset(&mqtt_msg, 0, sizeof(mqtt_msg));
    mqtt_msg.qos = pubmsg->msg_qos;
    mqtt_msg.retained = 1; //平台不关心
    mqtt_msg.dup = 0;      //0 非重试  1 重试

    if (pubmsg->msg_response == 0)
    {
        msg_type = MSG_TYPE_REQ;
    }
    else
    {
        msg_type = MSG_TYPE_RSP;
    }
    do
    {
        sdk_error = serialize_payload_common(msg_type, pubmsg->command_ver, pubmsg->msg_seq, &common_output_buf, &common_output_buf_len);
        if (sdk_error != mkernel_internal_succ)
        {
            break;
        }
        payload_buf_len = calculate_padding_len(common_output_buf_len + 2 + pubmsg->msg_body_len);
        // = common_output_buf_len + 2 + pubmsg->msg_body_len;
        payload_buf = (unsigned char *)ezos_malloc(payload_buf_len);
        payload_buf_enc = (unsigned char *)ezos_malloc(payload_buf_len);
        if (payload_buf == NULL || payload_buf_enc == NULL)
        {
            ezlog_e(TAG_CORE, "malloc payload error");
            sdk_error = mkernel_internal_malloc_error;
            break;
        }
        ezos_memset(payload_buf, 0, payload_buf_len);
        serialize_short(common_len_serialize_buf, common_output_buf_len);
        ezos_memcpy(payload_buf, common_len_serialize_buf, 2);
        ezos_memcpy(payload_buf + 2, common_output_buf, common_output_buf_len);
        ezos_memcpy(payload_buf + 2 + common_output_buf_len, pubmsg->msg_body, pubmsg->msg_body_len);

        sdk_error = aes_cbc_128_enc_padding(sdk_kernel->session_key,
                                            payload_buf, common_output_buf_len + 2 + pubmsg->msg_body_len, payload_buf_len,
                                            payload_buf_enc, &enc_output_buf_len);
        if (sdk_error != mkernel_internal_succ)
        {
            break;
        }

        mqtt_msg.payload = (void *)payload_buf_enc;
        mqtt_msg.payloadlen = enc_output_buf_len;

        mqtt_result_code = MQTTPublish(&g_DasClient, publish_topic, &mqtt_msg);
        //ezdev_sdk_kernel_log_info(mqtt_result_code, mqtt_result_code, "MQTTPublish seq:%d", pubmsg->msg_seq);
        if (mqtt_result_code == MQTTPACKET_BUFFER_TOO_SHORT)
        {
            ezlog_e(TAG_CORE, "mqtt buffer too short");
            sdk_error = mkernel_internal_call_mqtt_buffer_too_short;
            break;
        }

        if (mqtt_result_code != 0)
        {
            ezlog_e(TAG_CORE, "mqtt publish error");
            sdk_error = mkernel_internal_call_mqtt_pub_error;
            break;
        }

    } while (0);

    if (NULL != common_output_buf)
    {
        ezos_free(common_output_buf);
        common_output_buf = NULL;
    }
    if (NULL != payload_buf)
    {
        ezos_free(payload_buf);
        payload_buf = NULL;
    }
    if (NULL != payload_buf_enc)
    {
        ezos_free(payload_buf_enc);
        payload_buf_enc = NULL;
    }
    return sdk_error;
}

static mkernel_internal_error send_message_to_das_v3(ezdev_sdk_kernel *sdk_kernel)
{
    ezdev_sdk_kernel_pubmsg_exchange_v3 *ptr_pubmsg_exchange = NULL;
    mkernel_internal_error sdk_error = mkernel_internal_succ;
    ez_kernel_publish_ack_t ack_context = {0};

    do
    {
        if (mkernel_internal_succ != (sdk_error = pop_queue_pubmsg_exchange_v3(&ptr_pubmsg_exchange)) ||
            NULL == ptr_pubmsg_exchange)
        {
            break;
        }

        sdk_error = das_send_pubmsg_v3(sdk_kernel, &ptr_pubmsg_exchange->msg_conntext_v3);

        if (mkernel_internal_call_mqtt_pub_error == sdk_error)
        {
            if (ptr_pubmsg_exchange->max_send_count-- > 1)
            {
                ezlog_e(TAG_CORE, "v3 msg send failed,das need reconnect, count--:%d", ptr_pubmsg_exchange->max_send_count);
                push_queue_head_pubmsg_exchange_v3(ptr_pubmsg_exchange);
                return mkernel_internal_das_need_reconnect;
            }
        }

        ack_context.last_error = mkiE2ezE(sdk_error);
        ack_context.msg_seq = ptr_pubmsg_exchange->msg_conntext_v3.msg_seq;
        ezos_strncpy(ack_context.module_name, ptr_pubmsg_exchange->msg_conntext_v3.module, sizeof(ack_context.module_name) - 1);
        ezos_strncpy(ack_context.msg_type, ptr_pubmsg_exchange->msg_conntext_v3.msg_type, sizeof(ack_context.msg_type) - 1);
        broadcast_user_event(SDK_KERNEL_EVENT_PUBLISH_ACK, (void *)&ack_context, sizeof(ack_context));

        if (ptr_pubmsg_exchange->msg_conntext_v3.msg_body)
            ezos_free(ptr_pubmsg_exchange->msg_conntext_v3.msg_body);

        ezos_free(ptr_pubmsg_exchange);
    } while (0);

    return sdk_error;
}

static mkernel_internal_error send_message_to_das_v2(ezdev_sdk_kernel *sdk_kernel)
{
    char cRiskResult = 0;
    ezdev_sdk_kernel_pubmsg_exchange *ptr_pubmsg_exchange = NULL;
    mkernel_internal_error sdk_error = mkernel_internal_succ;

    do
    {
        if (mkernel_internal_succ != (sdk_error = pop_queue_pubmsg_exchange(&ptr_pubmsg_exchange)) ||
            NULL == ptr_pubmsg_exchange)
        {
            break;
        }

        if (0 == (cRiskResult = check_cmd_risk_control(sdk_kernel, ptr_pubmsg_exchange->msg_conntext.msg_domain_id, ptr_pubmsg_exchange->msg_conntext.msg_command_id)))
        {
            sdk_error = das_send_pubmsg(sdk_kernel, &ptr_pubmsg_exchange->msg_conntext);
        }

        if (1 == cRiskResult)
            sdk_error = mkernel_internal_extend_no_find;
        else if (2 == cRiskResult)
            sdk_error = mkernel_internal_force_domain_risk;
        else if (3 == cRiskResult)
            sdk_error = mkernel_internal_force_cmd_risk;

        ezlog_i(TAG_CORE, "pub msg result, domain:%d ,cmd:%d, len:%d, seq:%d, qos:%d",
                ptr_pubmsg_exchange->msg_conntext.msg_domain_id, ptr_pubmsg_exchange->msg_conntext.msg_command_id, ptr_pubmsg_exchange->msg_conntext.msg_body_len, ptr_pubmsg_exchange->msg_conntext.msg_seq, ptr_pubmsg_exchange->msg_conntext.msg_qos);

        if (mkernel_internal_call_mqtt_pub_error == sdk_error)
        {
            //发布失败，重连设备，并缓存指令
            if (ptr_pubmsg_exchange->max_send_count-- > 1)
            {
                ezlog_i(TAG_CORE, "ptr_pubmsg_exchange send failed,das need reconnect, count--:%d", ptr_pubmsg_exchange->max_send_count);
                push_queue_head_pubmsg_exchange(ptr_pubmsg_exchange);
                return mkernel_internal_das_need_reconnect;
            }
        }

        if (ptr_pubmsg_exchange->msg_conntext.msg_body)
            ezos_free(ptr_pubmsg_exchange->msg_conntext.msg_body);

        ezos_free(ptr_pubmsg_exchange);
    } while (0);

    return sdk_error;
}

static mkernel_internal_error das_message_send(ezdev_sdk_kernel *sdk_kernel)
{
    mkernel_internal_error sdk_error = mkernel_internal_succ;
    do
    {
        sdk_error = send_message_to_das_v2(sdk_kernel);
        if (mkernel_internal_succ != sdk_error && mkernel_internal_queue_empty != sdk_error)
        {
            break;
        }
        sdk_error = send_message_to_das_v3(sdk_kernel);

    } while (0);

    return sdk_error;
}

static mkernel_internal_error das_mqttlogin2das(ezdev_sdk_kernel *sdk_kernel, EZDEV_SDK_BOOL light_reg)
{
    mkernel_internal_error sdk_error = mkernel_internal_succ;
    EZDEV_SDK_INT32 mqtt_result_code = 0;
    MQTTPacket_connectData connectData = MQTTPacket_connectData_initializer;
    char sub_topic[128];
    unsigned char *will_message = NULL;
    EZDEV_SDK_UINT32 will_message_len = 0;

    ezos_memset(g_sendbuf, 0, CONFIG_EZIOT_CORE_MESSAGE_SIZE_MAX);
    ezos_memset(g_readbuf, 0, CONFIG_EZIOT_CORE_MESSAGE_SIZE_MAX);

    do
    {
        sdk_error = MQTTNetConnect(&g_DasNetWork, sdk_kernel->redirect_das_info.das_address, sdk_kernel->redirect_das_info.das_port);
        if (sdk_error != mkernel_internal_succ)
        {
            ezlog_d(TAG_CORE, "das_mqtt_reg2das NetworkConnect :%s :%d, error:%d",
                    sdk_kernel->redirect_das_info.das_address, sdk_kernel->redirect_das_info.das_port, sdk_error);
            break;
        }
        connectData.MQTTVersion = 4;
        if (light_reg)
        {
            connectData.cleansession = 0;
            connectData.willFlag = 1;
        }
        else
        {
            /**
			* \brief	0：表示设备断线重连
			1：表示设备重新上线
			*/
            connectData.cleansession = 1;
            /**
			* \brief	0:断线重连上线
			1:设备重新上线，包含遗嘱消息（设备信息）
			*/
            connectData.willFlag = 1;
        }
        /**
		* \brief		QoS0： 00	QoS1： 01	QoS2： 10	使用QoS1
		*/
        connectData.will.qos = 1;
        connectData.will.retained = 1;

        ezos_memset(&connectData.username, 0, sizeof(connectData.username));
        ezos_memset(&connectData.password, 0, sizeof(connectData.password));
        connectData.keepAliveInterval = sdk_kernel->das_keepalive_interval;
        if (ezos_strcmp("", (const char *)sdk_kernel->dev_id) == 0)
        {
            ezlog_e(TAG_CORE, "das_mqttlogin2das,dev_id is empty!!!");
            sdk_error = mkernel_internal_input_param_invalid;
            break;
        }
        connectData.clientID.lenstring.data = "";
        connectData.clientID.lenstring.len = 0;
        connectData.username.cstring = sdk_kernel->dev_info.dev_subserial;
        connectData.password.cstring = "test";
        ezos_memset(sub_topic, 0, 128);

        if (g_is_first_session)
            snprintf(sub_topic, 128, "/Basic/pu2cenplt/%s/firstconnect", sdk_kernel->dev_info.dev_subserial);
        else
            snprintf(sub_topic, 128, "/Basic/pu2cenplt/%s/breakconnect", sdk_kernel->dev_info.dev_subserial);

        connectData.will.topicName.lenstring.data = sub_topic;
        connectData.will.topicName.lenstring.len = 128;

        if (!light_reg)
        {
            sdk_error = serialize_devinfo(sdk_kernel, &will_message, &will_message_len);
            if (sdk_error != mkernel_internal_succ)
            {
                ezlog_e(TAG_CORE, "mqtt serialize_devinfo error");
                break;
            }
            connectData.will.message.lenstring.data = (char *)will_message;
            connectData.will.message.lenstring.len = will_message_len;
        }
        else
        {
            sdk_error = serialize_lightreginfo(sdk_kernel, &will_message, &will_message_len);
            if (sdk_error != mkernel_internal_succ)
            {
                ezlog_e(TAG_CORE, "serialize_lightreginfo error");
                break;
            }
            connectData.will.message.lenstring.data = (char *)will_message;
            connectData.will.message.lenstring.len = will_message_len;
        }

        if (0 != (mqtt_result_code = MQTTConnect(&g_DasClient, &connectData)))
        {
            if (FAILURE == mqtt_result_code)
                sdk_error = mkernel_internal_call_mqtt_connect;
            else
            {
                sdk_error = mkernel_internal_mqtt_error_begin + mqtt_result_code;
            }

            break;
        }

        g_is_first_session = EZDEV_SDK_FALSE;
    } while (0);

    if (will_message != NULL)
    {
        ezos_free(will_message);
        will_message = NULL;
    }

    if (sdk_error != mkernel_internal_succ)
    {
        MQTTNetDisconnect(&g_DasNetWork);
        MQTTNetFini(&g_DasNetWork);
    }

    ezlog_d(TAG_CORE, "mqtt connect, ip:%s, port:%d, code:%d", sdk_kernel->redirect_das_info.das_address, sdk_kernel->redirect_das_info.das_port, sdk_error);
    return sdk_error;
}

static mkernel_internal_error das_mqtt_logout2das()
{
    EZDEV_SDK_INT32 mqtt_code = 0;
    mqtt_code = MQTTDisconnect(&g_DasClient);
    if (mqtt_code != 0)
    {
        ezlog_w(TAG_CORE, "das_mqtt_logout2das error:%d", mqtt_code);
    }
    MQTTNetDisconnect(&g_DasNetWork);
    MQTTNetFini(&g_DasNetWork);
    return mkernel_internal_succ;
}

void das_object_init(ezdev_sdk_kernel *sdk_kernel)
{
    EZDEV_SDK_UNUSED(sdk_kernel)
    MQTTNetInit(&g_DasNetWork);
    ezos_memset(g_sendbuf, 0, CONFIG_EZIOT_CORE_MESSAGE_SIZE_MAX);
    ezos_memset(g_readbuf, 0, CONFIG_EZIOT_CORE_MESSAGE_SIZE_MAX);

    MQTTClientInit(&g_DasClient, &g_DasNetWork, 6 * 1000, g_sendbuf, CONFIG_EZIOT_CORE_MESSAGE_SIZE_MAX, g_readbuf, CONFIG_EZIOT_CORE_MESSAGE_SIZE_MAX);

    init_queue(CONFIG_EZIOT_CORE_MESSAGE_NUMBER_MAX, CONFIG_EZIOT_CORE_MESSAGE_NUMBER_MAX, CONFIG_EZIOT_CORE_MESSAGE_NUMBER_MAX * 4);
}

void das_object_fini(ezdev_sdk_kernel *sdk_kernel)
{
    EZDEV_SDK_UNUSED(sdk_kernel)
    ezos_memset(g_sendbuf, 0, CONFIG_EZIOT_CORE_MESSAGE_SIZE_MAX);
    ezos_memset(g_readbuf, 0, CONFIG_EZIOT_CORE_MESSAGE_SIZE_MAX);

    MQTTNetFini(&g_DasNetWork);
    fini_queue();

    g_das_transport_seq = 0;
}

mkernel_internal_error das_reg(ezdev_sdk_kernel *sdk_kernel)
{
    mkernel_internal_error sdk_error = mkernel_internal_succ;
    sdk_error = das_mqttlogin2das(sdk_kernel, EZDEV_SDK_FALSE);
    if (sdk_error != mkernel_internal_succ)
    {
        return sdk_error;
    }

    sdk_error = das_subscribe_revc_topic(sdk_kernel, 1);
    if (sdk_error != mkernel_internal_succ)
    {
        das_mqtt_logout2das();
        ezlog_w(TAG_CORE, "das_reg subscribe error");
    }

    ezlog_d(TAG_CORE, "das_reg result:%d", sdk_error);
    return sdk_error;
}

mkernel_internal_error das_light_reg(ezdev_sdk_kernel *sdk_kernel)
{
    mkernel_internal_error sdk_error = mkernel_internal_succ;
    das_mqtt_logout2das();

    sdk_error = das_mqttlogin2das(sdk_kernel, EZDEV_SDK_TRUE);

    ezlog_d(TAG_CORE, "das_light_reg result:%d", sdk_error);

    return sdk_error;
}

mkernel_internal_error das_light_reg_v2(ezdev_sdk_kernel *sdk_kernel)
{
    mkernel_internal_error sdk_error = mkernel_internal_succ;

    sdk_error = das_mqttlogin2das(sdk_kernel, EZDEV_SDK_TRUE);

    if (sdk_error != mkernel_internal_succ)
    {
        return sdk_error;
    }

    sdk_error = das_subscribe_revc_topic(sdk_kernel, 1);
    if (sdk_error != mkernel_internal_succ)
    {
        das_mqtt_logout2das();
        ezlog_w(TAG_CORE, "das_light_reg_v2 subscribe error");
    }

    ezlog_d(TAG_CORE, "das_light_reg result:%d", sdk_error);

    return sdk_error;
}

mkernel_internal_error das_unreg(ezdev_sdk_kernel *sdk_kernel)
{
    mkernel_internal_error sdk_error = mkernel_internal_succ;
    sdk_error = das_subscribe_revc_topic(sdk_kernel, 0);
    if (sdk_error != mkernel_internal_succ)
    {
        ezlog_w(TAG_CORE, "das_unreg unsubscribe error");
    }
    das_mqtt_logout2das();
    ezlog_i(TAG_CORE, "das_unreg complete");
    return mkernel_internal_succ;
}

/** 
*  \brief		非阻塞发布消息
*  \method		das_send_pubmsg_async
*  \param[in] 	ezdev_sdk_kernel * sdk_kernel
*  \param[in] 	const ezdev_sdk_kernel_pubmsg * msg
*  \return 		mkernel_internal_error
*/
mkernel_internal_error das_send_pubmsg_async(ezdev_sdk_kernel *sdk_kernel, ezdev_sdk_kernel_pubmsg_exchange *msg_exchange)
{
    /**
	* \brief   将消息放到发布队列中去
	*/
    mkernel_internal_error sdk_error = push_queue_pubmsg_exchange(msg_exchange);
    EZDEV_SDK_UNUSED(sdk_kernel);
    return sdk_error;
}

/** 
*  \brief		非阻塞发布消息
*  \method		das_send_pubmsg_async_v3
*  \param[in] 	ezdev_sdk_kernel * sdk_kernel
*  \param[in] 	const ezdev_sdk_kernel_pubmsg_v3 * msg
*  \return 		mkernel_internal_error
*/
mkernel_internal_error das_send_pubmsg_async_v3(ezdev_sdk_kernel *sdk_kernel, ezdev_sdk_kernel_pubmsg_exchange_v3 *msg_exchange)
{
    /**
	* \brief   将消息放到发布队列中去
	*/
    mkernel_internal_error sdk_error = push_queue_pubmsg_exchange_v3(msg_exchange);
    EZDEV_SDK_UNUSED(sdk_kernel);
    return sdk_error;
}

mkernel_internal_error das_change_keep_alive_interval(ezdev_sdk_kernel *sdk_kernel, EZDEV_SDK_UINT16 interval)
{
    mkernel_internal_error sdk_error = mkernel_internal_succ;
    EZDEV_SDK_UNUSED(sdk_kernel)
    ezlog_i(TAG_CORE, "das_change_keep_alive interval:%d", interval);
    sdk_kernel->das_keepalive_interval = interval;
    g_DasClient.keepAliveInterval = interval;
    TimerCountdown(&g_DasClient.ping_timer, interval);

    return sdk_error;
}

static mkernel_internal_error das_subscribe_revc_topic(ezdev_sdk_kernel *sdk_kernel, EZDEV_SDK_INT8 open)
{
    /**
	* \brief   订阅topic 来接收数据 /设备序列号/#
	*/
    EZDEV_SDK_INT32 mqtt_result_code = 0;
    char subscribe_topic[ezdev_sdk_recv_topic_len];
    ezos_memset(subscribe_topic, 0, ezdev_sdk_recv_topic_len);
    if (open)
    {
        if (sdk_v3_reged == sdk_kernel->v3_reg_status)
        {
            snprintf(subscribe_topic, ezdev_sdk_recv_topic_len, "/iot/%s/#", sdk_kernel->dev_info.dev_subserial);
            ezlog_d(TAG_CORE, "mqtt subscribe:%s ", subscribe_topic);
            mqtt_result_code = MQTTSubscribe(&g_DasClient, subscribe_topic, QOS1, das_message_receive_v3);
            if (mqtt_result_code != 0)
            {
                ezlog_i(TAG_CORE, "mqtt subscribe:%s error, code:%d", subscribe_topic, mqtt_result_code);
                return mkernel_internal_call_mqtt_sub_error;
            }
        }

        ezos_memset(subscribe_topic, 0, ezdev_sdk_recv_topic_len);
        snprintf(subscribe_topic, ezdev_sdk_recv_topic_len, "/%s/#", sdk_kernel->dev_info.dev_subserial);
        mqtt_result_code = MQTTSubscribe(&g_DasClient, subscribe_topic, QOS1, das_message_receive);
        if (mqtt_result_code != 0)
        {
            ezlog_i(TAG_CORE, "mqtt subscribe:%s error, code:%d", subscribe_topic, mqtt_result_code);
            return mkernel_internal_call_mqtt_sub_error;
        }
        ezlog_d(TAG_CORE, "mqtt subscribe old topic:%s", subscribe_topic);
    }
    else
    {
        if (sdk_v3_reged == sdk_kernel->v3_reg_status)
        {
            snprintf(subscribe_topic, ezdev_sdk_recv_topic_len, "/iot/%s/#", sdk_kernel->dev_info.dev_subserial);
            ezlog_d(TAG_CORE, "mqtt subscribe:%s", subscribe_topic);
            mqtt_result_code = MQTTUnsubscribe(&g_DasClient, subscribe_topic);
            if (mqtt_result_code != 0)
            {
                ezlog_v(TAG_CORE, "mqtt unsubscribe:%s error, code:%d", subscribe_topic, mqtt_result_code);
            }
        }
        ezos_memset(subscribe_topic, 0, ezdev_sdk_recv_topic_len);
        snprintf(subscribe_topic, ezdev_sdk_recv_topic_len, "/%s/#", sdk_kernel->dev_info.dev_subserial);
        mqtt_result_code = MQTTUnsubscribe(&g_DasClient, subscribe_topic);
        if (mqtt_result_code != 0)
        {
            ezlog_v(TAG_CORE, "mqtt unsubscribe:%s error, code:%d", subscribe_topic, mqtt_result_code);
        }
    }
    return mkernel_internal_succ;
}

mkernel_internal_error das_yield(ezdev_sdk_kernel *sdk_kernel)
{
    mkernel_internal_error sdk_error = mkernel_internal_succ;
    int mqtt_result = MQTTYield(&g_DasClient, 10);

    do
    {
        if (mqtt_result < 0)
        {
            sdk_error = mkernel_internal_das_need_reconnect;
            ezlog_e(TAG_CORE, "socket error need rereg");
            break;
        }

        /**
		* \brief   更新超时时间
		*/
        if (g_DasClient.keepAliveInterval != sdk_kernel->das_keepalive_interval)
        {
            g_DasClient.keepAliveInterval = sdk_kernel->das_keepalive_interval;
        }

        /**
		* \brief   判断心跳是否超时
		*/
        if (!ezcore_time_isexpired_bydiff(&g_DasClient.connect_timer.end_time, g_DasClient.keepAliveInterval * 2000))
        {
            //信令发送失败已经在内部上抛

            if (mkernel_internal_das_need_reconnect != (sdk_error = das_message_send(sdk_kernel)))
                sdk_error = mkernel_internal_succ;
        }
        else
        {
            sdk_error = mkernel_internal_das_need_reconnect;
            ezlog_e(TAG_CORE, "heart timeout need rereg");
        }
    } while (0);

    return sdk_error;
}

int ezdev_sdk_kernel_get_das_socket(ezdev_sdk_kernel *sdk_kernel)
{
    return g_DasNetWork.socket_fd;
}

static int MQTTNet_write(Network *n, unsigned char *buffer, int len, int timeout_ms)
{
    int real_write_buf_size = 0;
    mkernel_internal_error sdk_error = mkernel_internal_succ;
    sdk_error = net_write(n->socket_fd, buffer, len, timeout_ms, &real_write_buf_size);
    if (sdk_error == mkernel_internal_succ)
    {
        ezlog_v(TAG_CORE, "mqtt call net_work_write succ, want:%d, send len:%d", len, real_write_buf_size);
        return real_write_buf_size;
    }
    else
    {
        ezlog_e(TAG_CORE, "mqtt call net_work_write error:%d", sdk_error);
        return -1;
    }
}

static int MQTTNet_read(Network *n, unsigned char *buffer, int len, int timeout_ms)
{
    mkernel_internal_error sdk_error = mkernel_internal_succ;

    sdk_error = net_read(n->socket_fd, buffer, len, timeout_ms);
    if (sdk_error == mkernel_internal_succ)
    {
        return len;
    }

    return 0;
}

static void MQTTNetInit(Network *network)
{
    network->socket_fd = -1;
    network->mqttread = MQTTNet_read;
    network->mqttwrite = MQTTNet_write;
}

static mkernel_internal_error MQTTNetConnect(Network *network, char *ip, int port)
{
    mkernel_internal_error error_code = mkernel_internal_succ;
    char szRealIp[ezdev_sdk_ip_max_len] = {0};
    do
    {
        network->socket_fd = net_create(NULL);
        if (network->socket_fd < 0)
        {
            error_code = mkernel_internal_create_sock_error;
            break;
        }

        error_code = net_connect(network->socket_fd, ip, port, 5 * 1000, szRealIp);
        if (mkernel_internal_succ != error_code && mkernel_internal_net_gethostbyname_error != error_code)
        {
            error_code = mkernel_internal_net_connect_error;
            break;
        }

    } while (0);

    if (error_code == mkernel_internal_succ)
    {
        return error_code;
    }

    if (network->socket_fd >= 0)
    {
        net_disconnect(network->socket_fd);
        network->socket_fd = -1;
    }

    return error_code;
}

static void MQTTNetDisconnect(Network *network)
{
    if (network->socket_fd != -1)
    {
        net_disconnect(network->socket_fd);
        network->socket_fd = -1;
    }
}

static void MQTTNetFini(Network *network)
{
    if (network->socket_fd == -1)
    {
        return;
    }
    network->socket_fd = -1;
}