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
 * Brief:
 * 
 * 
 * Change Logs:
 * Date           Author       Notes
 * 2021-11-25    zhangdi29     
 *******************************************************************************/
#include <string.h>
#include "hub_extern.h"
#include "ezos_gconfig.h"
#include "ezos_def.h"
#include "ez_iot_core.h"
#include "ez_iot_core_def.h"
#include "ez_iot_core_lowlvl.h"
#include "hub_func.h"
#include "ezlog.h"


#define hub_cmd_version "v1.0.0"

static void hub_extend_start_cb(ez_void_t *pUser)
{
    hub_subdev_list_report();
}

static void hub_extend_stop_cb(ez_void_t *pUser)
{
}

static void hub_extend_data_route_cb(ez_kernel_submsg_t *ptr_submsg, ez_void_t *pUser)
{
    if (ptr_submsg == NULL)
    {
        return;
    }

    ezlog_w(TAG_HUB, "cmd:%d", ptr_submsg->msg_command_id);
    ezlog_d(TAG_HUB, "seq:%d, len:%d, data:%s", ptr_submsg->msg_seq, ptr_submsg->buf_len, (char*)ptr_submsg->buf);

    if (kPu2CenPltHubAuthChildDeviceRsp == ptr_submsg->msg_command_id)
    {
        hub_subdev_auth_done(ptr_submsg->buf, ptr_submsg->buf_len);
    }
}

static void hub_extend_event_cb(ez_kernel_event_t *ptr_event, ez_void_t *pUser)
{
    if (SDK_KERNEL_EVENT_ONLINE == ptr_event->event_type ||
        SDK_KERNEL_EVENT_SWITCHOVER == ptr_event->event_type)
    {
        hub_subdev_list_report();
    }
}


ez_int_t hub_extern_init()
{
    ez_kernel_extend_t extern_info;
    memset(&extern_info, 0, sizeof(ez_kernel_extend_t));

    extern_info.domain_id = hub_module_id;
    extern_info.pUser = NULL;
    extern_info.ezdev_sdk_kernel_extend_start = hub_extend_start_cb;
    extern_info.ezdev_sdk_kernel_extend_stop = hub_extend_stop_cb;
    extern_info.ezdev_sdk_kernel_extend_data_route = hub_extend_data_route_cb;
    extern_info.ezdev_sdk_kernel_extend_event = hub_extend_event_cb;

    strncpy(extern_info.extend_module_name, hub_module_name, ezdev_sdk_extend_name_len);
    strncpy(extern_info.extend_module_version, hub_module_version, version_max_len);

    ez_int_t ret = ez_kernel_extend_load(&extern_info);
    return ret;
}

int hub_extern_finit()
{
    return 0;
}

int hub_send_msg_to_platform(const ez_char_t *msg, ez_int_t msg_len, ez_int_t cmd_id, ez_uchar_t msg_response, ez_uint_t msg_seq)
{
    ez_kernel_pubmsg_t pubmsg;
    memset(&pubmsg, 0, sizeof(ez_kernel_pubmsg_t));

    pubmsg.msg_response = msg_response;
    pubmsg.msg_seq = msg_seq;

    pubmsg.msg_body = (unsigned char *)msg;
    pubmsg.msg_body_len = msg_len;

    pubmsg.msg_domain_id = hub_module_id;
    pubmsg.msg_command_id = cmd_id;

    strncpy(pubmsg.command_ver, hub_module_version, version_max_len - 1);

    ez_err_t sdk_error = ez_kernel_send(&pubmsg);
    if (sdk_error != EZ_HUB_ERR_SUCC)
    {
        return -1;
    }

    return 0;

}

