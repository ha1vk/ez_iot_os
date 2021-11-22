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
#include "access_domain_bus.h"
#include "dev_protocol_def.h"
#include "mkernel_internal_error.h"
#include "ezdev_sdk_kernel_risk_control.h"
#include "ezdev_sdk_kernel_xml_parser.h"
#include "ezdev_sdk_kernel_struct.h"
#include "ezdev_sdk_kernel_event.h"
#include "das_transport.h"
#include "utils.h"
#include "cJSON.h"

EZDEV_SDK_KERNEL_RISK_CONTROL_INTERFACE
EZDEV_SDK_KERNEL_XML_PARSER_INTERFACE
EZDEV_SDK_KERNEL_EVENT_INTERFACE
DAS_TRANSPORT_INTERFACE

extern ezdev_sdk_kernel g_ezdev_sdk_kernel;

static mkernel_internal_error bus_handle_domainconfig_keepalive(ezdev_sdk_kernel *sdk_kernel, cJSON *json_item)
{
    cJSON *json_interval = NULL;
    if (NULL == json_item)
    {
        return mkernel_internal_json_parse_error;
    }
    json_interval = cJSON_GetObjectItem(json_item, "Interval");
    if (NULL == json_interval)
    {
        return mkernel_internal_get_error_json;
    }

    if (json_interval->type != cJSON_Number)
    {
        return mkernel_internal_get_error_json;
    }

    sdk_kernel->das_keepalive_interval = json_interval->valueint;

    return mkernel_internal_succ;
}

static mkernel_internal_error bus_handle_domainconfig_ntp(ezdev_sdk_kernel *sdk_kernel, cJSON *json_item)
{
    return mkernel_internal_succ;
}

static mkernel_internal_error bus_handle_domainconfig(ezdev_sdk_kernel *sdk_kernel, const ezdev_sdk_kernel_submsg *ptr_submsg)
{
    cJSON *json_keepalive = NULL;
    cJSON *json_ntp = NULL;
    cJSON *json_root = NULL;
    if (NULL == ptr_submsg->buf)
    {
        return mkernel_internal_rev_invalid_packet;
    }
    json_root = cJSON_Parse((const char *)ptr_submsg->buf);
    if (json_root == NULL)
    {
        return mkernel_internal_json_parse_error;
    }
    json_keepalive = cJSON_GetObjectItem(json_root, "KeepAlive");
    json_ntp = cJSON_GetObjectItem(json_root, "NTP");
    if (json_keepalive != NULL)
    {
        bus_handle_domainconfig_keepalive(sdk_kernel, json_keepalive);
    }
    if (json_ntp != NULL)
    {
        bus_handle_domainconfig_ntp(sdk_kernel, json_keepalive);
    }
    cJSON_Delete(json_root);
    return mkernel_internal_succ;
}

static mkernel_internal_error bus_handle_dev_redirect(ezdev_sdk_kernel *sdk_kernel, const ezdev_sdk_kernel_submsg *ptr_submsg)
{
    char domain_name[ezdev_sdk_name_len] = {0};
    mkernel_internal_error sdk_error = mkernel_internal_succ;
    do
    {
        sdk_error = cenplt2pusetlbsdomainnamebydasreq_xml_parser(ptr_submsg->buf, ptr_submsg->buf_len, domain_name);
        if (sdk_error != mkernel_internal_succ)
        {
            break;
        }

        if (ezos_strcmp(sdk_kernel->server_info.server_name, domain_name) == 0)
        {
            ezlog_d(TAG_CORE, "no need redirect cur:%s, want:%s", sdk_kernel->server_info.server_name, domain_name);
            break;
        }

        ezlog_w(TAG_CORE, "need redirect cur:%s, want:%s", sdk_kernel->server_info.server_name, domain_name);

        ezos_strncpy(sdk_kernel->server_info.server_name, domain_name, ezdev_sdk_name_len);
        ezos_memset(sdk_kernel->server_info.server_ip, 0, ezdev_sdk_name_len);
        ezos_memset(sdk_kernel->master_key, 0, ezdev_sdk_masterkey_len);

        sdk_kernel->entr_state = sdk_entrance_switchover;
        sdk_kernel->cnt_state = sdk_cnt_unredirect;

    } while (0);

    return sdk_error;
}

static mkernel_internal_error bus_handle_dev_risk(ezdev_sdk_kernel *sdk_kernel, const ezdev_sdk_kernel_submsg *ptr_submsg)
{
    int indexa = 0;
    int indexb = 0;
    int domain_size = 0;
    int domain_cmd_size = 0;
    EZDEV_SDK_UINT32 domain_id = 0;
    cJSON *json_risk_array = NULL;
    cJSON *json_domain = NULL;
    cJSON *json_domain_cmd = NULL;
    cJSON *json_root = NULL;

    if (NULL == ptr_submsg->buf)
    {
        return mkernel_internal_rev_invalid_packet;
    }

    json_root = cJSON_Parse((const char *)ptr_submsg->buf);
    if (json_root == NULL)
    {
        return mkernel_internal_json_parse_error;
    }

    json_risk_array = cJSON_GetObjectItem(json_root, "risk");
    domain_size = cJSON_GetArraySize(json_risk_array);
    for (indexa = 0; indexa < domain_size; indexa++)
    {
        json_domain = cJSON_GetArrayItem(json_risk_array, indexa);
        if (NULL == json_domain)
            continue;

        if (NULL != json_domain->string)
        {
            domain_id = ezos_atoi(json_domain->string);
        }

        domain_cmd_size = cJSON_GetArraySize(json_domain);
        if (domain_cmd_size == 0)
        {
            add_domain_risk_control(sdk_kernel, domain_id);
            continue;
        }
        for (indexb = 0; indexb < domain_size; indexb++)
        {
            json_domain_cmd = cJSON_GetArrayItem(json_domain, indexb);
            if (NULL == json_domain_cmd)
                continue;

            add_cmd_risk_control(sdk_kernel, domain_id, json_domain_cmd->valueint);
        }
    }

    cJSON_Delete(json_root);
    return mkernel_internal_succ;
}

static mkernel_internal_error bus_handle_force_offline(ezdev_sdk_kernel *sdk_kernel)
{
    broadcast_runtime_err(TAG_ACCESS, mkiE2ezE(mkernel_internal_force_offline), NULL, 0);
    add_access_risk_control(sdk_kernel);
    return mkernel_internal_succ;
}

mkernel_internal_error access_domain_bus_handle(const ezdev_sdk_kernel_submsg *ptr_submsg)
{
    mkernel_internal_error sdk_error = mkernel_internal_succ;
    switch (ptr_submsg->msg_command_id)
    {
    case DAS_CMD_CENPLT2PUDOMAINCONFIG:
        sdk_error = bus_handle_domainconfig(&g_ezdev_sdk_kernel, ptr_submsg);
        break;
    case DAS_CMD_CENPLT2PURISKCONFIG:
        sdk_error = bus_handle_dev_risk(&g_ezdev_sdk_kernel, ptr_submsg);
        break;
    case DAS_CMD_CENPLT2PUOFFLINE:
        sdk_error = bus_handle_force_offline(&g_ezdev_sdk_kernel);
        break;
    case DAS_CMD_CENPLT2PUSETKEEPALIVETIMEREQ:
        break;
    case DAS_CMD_CENPLT2PUSETLBSDOMAINNAMEBYDASREQ:
    case DAS_CMD_CENPLT2PUSETLBSDOMAINNAMEREQ:
        sdk_error = bus_handle_dev_redirect(&g_ezdev_sdk_kernel, ptr_submsg);
        break;
    default:
        break;
    }
    return sdk_error;
}