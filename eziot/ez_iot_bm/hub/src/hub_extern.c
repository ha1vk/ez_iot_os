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
#include "hub_extern.h"
#include "ezos_gconfig.h"
#include "ezos_def.h"
#include "ez_iot_core.h"
#include "ez_iot_core_def.h"
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


int hub_extern_finit()
{
    return 0;
}

