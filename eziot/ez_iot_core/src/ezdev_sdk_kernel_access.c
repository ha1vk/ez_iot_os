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
#include "ezdev_sdk_kernel_access.h"
#include "mkernel_internal_error.h"
#include "sdk_kernel_def.h"
#include "lbs_transport.h"
#include "das_transport.h"
#include "ezdev_sdk_kernel_risk_control.h"
#include "ezdev_sdk_kernel_event.h"
#include "ezdev_sdk_kernel_struct.h"
#include "sdk_kernel_def.h"
#include "utils.h"
#include "ezxml.h"
#include "aes_support.h"

LBS_TRANSPORT_INTERFACE
DAS_TRANSPORT_INTERFACE
EZDEV_SDK_KERNEL_RISK_CONTROL_INTERFACE
EZDEV_SDK_KERNEL_EVENT_INTERFACE
AES_SUPPORT_INTERFACE

ezdev_sdk_kernel g_ezdev_sdk_kernel;

static mkernel_internal_error cnt_state_lbs_redirect(ezdev_sdk_kernel *sdk_kernel, EZDEV_SDK_UINT8 nUpper)
{
    mkernel_internal_error sdk_error = mkernel_internal_succ;
    int type = 0;
    if (ezos_strcmp("", (const char *)sdk_kernel->dev_id) == 0)
    {
        ezlog_i(TAG_CORE, "dev_id is empty");
        sdk_error = lbs_redirect_createdevid_with_auth(sdk_kernel, nUpper);
        type = 0;
    }
    else
    {
        if (ezos_strcmp("", (const char *)sdk_kernel->master_key) == 0)
        {
            ezlog_i(TAG_CORE, "masterkey is empty");
            sdk_error = lbs_redirect_with_auth(sdk_kernel, nUpper);
            type = 1;
        }
        else
        {
            sdk_error = lbs_redirect(sdk_kernel);
            type = 2;
        }
    }

    ezlog_i(TAG_CORE, "cnt_state_lbs_redirect result:%d, type:%d", sdk_error, type);

    if (sdk_error == mkernel_internal_platform_devid_inconformity)
    {
        ezlog_i(TAG_CORE, "memset dev_id in memory");
        ezos_memset(sdk_kernel->dev_id, 0, ezdev_sdk_devid_len);
    }

    if (sdk_error == mkernel_internal_platform_masterkey_invalid)
    {
        ezlog_i(TAG_CORE, "memset masterkey in memory");
        ezos_memset(sdk_kernel->master_key, 0, ezdev_sdk_masterkey_len);
    }
    if (sdk_error == mkernel_internal_platform_lbs_signcheck_error && nUpper != 0)
    {
        sdk_error = cnt_state_lbs_redirect(sdk_kernel, 0);
        ezlog_i(TAG_CORE, "lbs return :%d, and nUpper != 0", sdk_error);
    }
    return sdk_error;
}

static mkernel_internal_error cnt_state_das_reged(ezdev_sdk_kernel *sdk_kernel)
{
    mkernel_internal_error sdk_error = mkernel_internal_succ;
    das_unreg(sdk_kernel);
    sdk_error = das_reg(sdk_kernel);
    return sdk_error;
}

static mkernel_internal_error cnt_state_das_fast_reg(ezdev_sdk_kernel *sdk_kernel)
{
    return das_light_reg_v2(sdk_kernel);
}

static mkernel_internal_error cnt_state_das_work(ezdev_sdk_kernel *sdk_kernel)
{
    mkernel_internal_error sdk_error = mkernel_internal_succ;
    sdk_error = das_yield(sdk_kernel);
    return sdk_error;
}

static mkernel_internal_error cnt_state_das_retry(ezdev_sdk_kernel *sdk_kernel)
{
    return das_light_reg(sdk_kernel);
}

static mkernel_internal_error cnt_lbs_redirect_do(ezdev_sdk_kernel *sdk_kernel)
{
    mkernel_internal_error sdk_error = mkernel_internal_succ;

    if (sdk_kernel->lbs_redirect_times &&
        !ezcore_time_isexpired_bydiff(&sdk_kernel->cnt_state_timer, sdk_kernel->lbs_redirect_times * 2000))
    {
        return sdk_error;
    }

    ezcore_time_countdown(&sdk_kernel->cnt_state_timer, 0);
    ezlog_v(TAG_CORE, "cnt_state_lbs_redirect, times:%d", sdk_kernel->lbs_redirect_times);
    sdk_error = cnt_state_lbs_redirect(sdk_kernel, 1);

    if (sdk_error == mkernel_internal_succ)
    {
        sdk_kernel->cnt_state = sdk_cnt_redirected;
        sdk_kernel->lbs_redirect_times = 0;
        sdk_kernel->das_retry_times = 0;
    }
    else
    {
        if (mkernel_internal_net_connect_error == sdk_error || mkernel_internal_net_gethostbyname_error == sdk_error || mkernel_internal_platform_lbs_auth_type_need_rematch == sdk_error)
        {
            sdk_kernel->lbs_redirect_times = 1;
        }
        else
        {
            if (++sdk_kernel->lbs_redirect_times >= 60)
                sdk_kernel->lbs_redirect_times = 60;
        }
    }

    return sdk_error;
}

static mkernel_internal_error cnt_das_reg_do(ezdev_sdk_kernel *sdk_kernel)
{
    mkernel_internal_error sdk_error = mkernel_internal_succ;

    if (sdk_kernel->das_retry_times && !ezcore_time_isexpired_bydiff(&sdk_kernel->cnt_state_timer, sdk_kernel->das_retry_times * 2000))
    {
        return sdk_error;
    }

    ezcore_time_countdown(&sdk_kernel->cnt_state_timer, 0);
    ezlog_v(TAG_CORE, "cnt_state_das_reged, times:%d", sdk_kernel->das_retry_times);

    sdk_error = cnt_state_das_reged(sdk_kernel);
    if (sdk_error == mkernel_internal_mqtt_session_exist)
    {
        sdk_error = cnt_state_das_fast_reg(sdk_kernel);
    }

    if (sdk_error == mkernel_internal_succ)
    {
        sdk_kernel->cnt_state = sdk_cnt_das_reged;
        sdk_kernel->lbs_redirect_times = 0;
        sdk_kernel->das_retry_times = 0;
        if (sdk_kernel->entr_state == sdk_entrance_switchover)
        {
            ez_kernel_switchover_context_t context = {0};
            context.das_udp_port = sdk_kernel->redirect_das_info.das_udp_port;
            ezos_memcpy(context.das_ip, sdk_kernel->redirect_das_info.das_address, ezdev_sdk_ip_max_len);
            ezos_memcpy(context.lbs_ip, sdk_kernel->server_info.server_ip, ezdev_sdk_ip_max_len);
            ezos_memcpy(context.session_key, sdk_kernel->session_key, ezdev_sdk_sessionkey_len);
            ezos_memcpy(context.lbs_domain, sdk_kernel->server_info.server_name, ezdev_sdk_ip_max_len);
            broadcast_user_event(SDK_KERNEL_EVENT_SWITCHOVER, (void *)&context, sizeof(context));
            sdk_kernel->entr_state = sdk_entrance_normal;
        }
        else
        {
            ez_kernel_sessionkey_context_t context = {0};
            context.das_udp_port = sdk_kernel->redirect_das_info.das_udp_port;
            context.das_port = sdk_kernel->redirect_das_info.das_port;
            context.das_socket = ezdev_sdk_kernel_get_das_socket(sdk_kernel);
            ezos_memcpy(context.das_ip, sdk_kernel->redirect_das_info.das_address, ezdev_sdk_ip_max_len);
            ezos_memcpy(context.lbs_ip, sdk_kernel->server_info.server_ip, ezdev_sdk_ip_max_len);
            ezos_memcpy(context.session_key, sdk_kernel->session_key, ezdev_sdk_sessionkey_len);
            ezos_memcpy(context.das_domain, sdk_kernel->redirect_das_info.das_domain, ezdev_sdk_ip_max_len);
            ezos_memcpy(context.das_serverid, sdk_kernel->redirect_das_info.das_serverid, ezdev_sdk_ip_max_len);
            broadcast_user_event(SDK_KERNEL_EVENT_ONLINE, (void *)&context, sizeof(context));
        }
    }
    else
    {
        if (sdk_kernel->das_retry_times++ >= 5)
        {
            sdk_kernel->cnt_state = sdk_cnt_unredirect;
            sdk_kernel->lbs_redirect_times = 0;
            sdk_kernel->das_retry_times = 0;
        }
    }

    return sdk_error;
}

static mkernel_internal_error cnt_das_work_do(ezdev_sdk_kernel *sdk_kernel)
{
    mkernel_internal_error sdk_error = mkernel_internal_succ;

    sdk_error = cnt_state_das_work(sdk_kernel);
    if (mkernel_internal_succ != sdk_error &&
        mkernel_internal_queue_empty != sdk_error)
    {
        ezlog_e(TAG_CORE, "cnt_das_work_do, code:%d", sdk_error);
        if (sdk_error == mkernel_internal_das_need_reconnect)
        {
            sdk_kernel->cnt_state = sdk_cnt_das_break;
        }
    }

    return sdk_error;
}

static mkernel_internal_error cnt_das_retry_do(ezdev_sdk_kernel *sdk_kernel)
{
    mkernel_internal_error sdk_error = mkernel_internal_succ;

    if (sdk_kernel->das_retry_times && !ezcore_time_isexpired_bydiff(&sdk_kernel->cnt_state_timer, sdk_kernel->das_retry_times * 1000))
    {
        return sdk_error;
    }

    ezcore_time_countdown(&sdk_kernel->cnt_state_timer, 0);
    ezlog_v(TAG_CORE, "cnt_state_das_retry, times:%d", sdk_kernel->das_retry_times);

    sdk_error = cnt_state_das_retry(sdk_kernel);
    if (sdk_error == mkernel_internal_succ)
    {
        sdk_kernel->cnt_state = sdk_cnt_das_reged;
        sdk_kernel->lbs_redirect_times = 0;
        sdk_kernel->das_retry_times = 0;
        broadcast_user_event_reconnect_success();
    }
    else
    {
        if (sdk_kernel->das_retry_times++ >= 3)
        {
            ez_kernel_offline_context_t context = {0};

            context.last_error = sdk_error;
            ezos_memcpy(context.das_ip, sdk_kernel->redirect_das_info.das_address, ezdev_sdk_ip_max_len);
            ezos_memcpy(context.lbs_ip, sdk_kernel->server_info.server_ip, ezdev_sdk_ip_max_len);
            ezlog_e(TAG_CORE, "broadcast_user_event, sdk_kernel_event_break, code:%d", sdk_error);
            broadcast_user_event(SDK_KERNEL_EVENT_BREAK, (void *)&context, sizeof(context));

            sdk_kernel->cnt_state = sdk_cnt_unredirect;
            sdk_kernel->lbs_redirect_times = 0;
            sdk_kernel->das_retry_times = 0;
        }
    }

    return sdk_error;
}

mkernel_internal_error access_stop_yield(ezdev_sdk_kernel *sdk_kernel)
{
    mkernel_internal_error sdk_error = mkernel_internal_succ;
    sdk_error = das_unreg(sdk_kernel);
    sdk_kernel->cnt_state = sdk_cnt_unredirect;
    return sdk_error;
}

mkernel_internal_error access_server_yield(ezdev_sdk_kernel *sdk_kernel)
{
    mkernel_internal_error sdk_error = mkernel_internal_succ;

    if (check_access_risk_control(sdk_kernel))
    {
        return mkernel_internal_force_offline;
    }

    switch (sdk_kernel->cnt_state)
    {
    case sdk_cnt_unredirect:
    {
        sdk_error = cnt_lbs_redirect_do(sdk_kernel);
        break;
    }
    case sdk_cnt_redirected:
    {
        sdk_error = cnt_das_reg_do(sdk_kernel);
        break;
    }
    case sdk_cnt_das_reged:
    {
        sdk_error = cnt_das_work_do(sdk_kernel);
        break;
    }
    case sdk_cnt_das_break:
    {
        sdk_error = cnt_das_retry_do(sdk_kernel);
        break;
    }
    default:
    {
        sdk_error = mkernel_internal_internal_err;
    }
    }

    if (mkernel_internal_mqtt_blacklist == sdk_error)
    {
        add_access_risk_control(sdk_kernel);
        ezlog_a(TAG_CORE, "access risk control, code:%d", sdk_error);
    }
    else if (mkernel_internal_mqtt_redirect == sdk_error)
    {
        ez_kernel_offline_context_t context = {0};
        context.last_error = sdk_error;
        ezos_memcpy(context.das_ip, sdk_kernel->redirect_das_info.das_address, ezdev_sdk_ip_max_len);
        ezos_memcpy(context.lbs_ip, sdk_kernel->server_info.server_ip, ezdev_sdk_ip_max_len);
        broadcast_user_event(SDK_KERNEL_EVENT_BREAK, (void *)&context, sizeof(context));

        sdk_kernel->cnt_state = sdk_cnt_unredirect;
        sdk_kernel->lbs_redirect_times = 0;
        sdk_kernel->das_retry_times = 0;
        ezlog_d(TAG_CORE, "redirect, code:%d", sdk_error);
    }

    return sdk_error;
}

mkernel_internal_error ezdev_sdk_kernel_inner_send(const ezdev_sdk_kernel_pubmsg *pubmsg)
{
    ezdev_sdk_kernel_pubmsg_exchange *new_pubmsg_exchange = NULL;
    mkernel_internal_error kernel_internal_error = mkernel_internal_succ;

    ezlog_v(TAG_CORE, "ezdev_sdk_kernel_inner_send: domain:%d ,cmd:%d, seq:%d,len:%d, string:%s", pubmsg->msg_domain_id, pubmsg->msg_command_id, pubmsg->msg_seq, pubmsg->msg_body_len, pubmsg->msg_body);

    if (pubmsg->msg_body == NULL || pubmsg->msg_body_len == 0)
    {
        return mkernel_internal_input_param_invalid;
    }

    if (pubmsg->msg_body_len > CONFIG_EZIOT_CORE_MESSAGE_SIZE_MAX)
    {
        return mkernel_internal_msg_len_overrange;
    }

    new_pubmsg_exchange = (ezdev_sdk_kernel_pubmsg_exchange *)ezos_malloc(sizeof(ezdev_sdk_kernel_pubmsg_exchange));
    if (new_pubmsg_exchange == NULL)
    {
        return mkernel_internal_malloc_error;
    }

    ezos_memset(new_pubmsg_exchange, 0, sizeof(ezdev_sdk_kernel_pubmsg_exchange));
    ezos_strncpy(new_pubmsg_exchange->msg_conntext.command_ver, pubmsg->command_ver, version_max_len - 1);
    new_pubmsg_exchange->msg_conntext.msg_response = pubmsg->msg_response;
    new_pubmsg_exchange->msg_conntext.msg_qos = pubmsg->msg_qos;
    new_pubmsg_exchange->msg_conntext.msg_seq = pubmsg->msg_seq;
    new_pubmsg_exchange->msg_conntext.msg_domain_id = pubmsg->msg_domain_id;
    new_pubmsg_exchange->msg_conntext.msg_command_id = pubmsg->msg_command_id;

    new_pubmsg_exchange->msg_conntext.msg_body = (ez_char_t *)ezos_malloc(pubmsg->msg_body_len);
    if (new_pubmsg_exchange->msg_conntext.msg_body == NULL)
    {
        ezos_free(new_pubmsg_exchange);
        new_pubmsg_exchange = NULL;

        ezlog_e(TAG_CORE, "malloc err:%d", pubmsg->msg_body_len);
        return mkernel_internal_malloc_error;
    }
    ezos_memset(new_pubmsg_exchange->msg_conntext.msg_body, 0, pubmsg->msg_body_len);
    new_pubmsg_exchange->msg_conntext.msg_body_len = pubmsg->msg_body_len;
    ezos_memcpy(new_pubmsg_exchange->msg_conntext.msg_body, pubmsg->msg_body, pubmsg->msg_body_len);

    new_pubmsg_exchange->max_send_count = CONFIG_EZIOT_CORE_DEFAULT_PUBLISH_RETRY;
    kernel_internal_error = das_send_pubmsg_async(&g_ezdev_sdk_kernel, new_pubmsg_exchange);
    ezlog_i(TAG_CORE, "das_send_pubmsg_async offline msg send ,error code:%d", kernel_internal_error);
    if (kernel_internal_error != mkernel_internal_succ)
    {
        if (new_pubmsg_exchange != NULL)
        {
            if (new_pubmsg_exchange->msg_conntext.msg_body != NULL)
            {
                ezos_free(new_pubmsg_exchange->msg_conntext.msg_body);
                new_pubmsg_exchange->msg_conntext.msg_body = NULL;
            }

            ezos_free(new_pubmsg_exchange);
            new_pubmsg_exchange = NULL;
        }
    }

    return kernel_internal_error;
}