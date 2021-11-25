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

#include "ez_iot_base_extern.h"
#include "ez_iot_base_def.h"
#include "ez_iot_base_protocol.h"
#include "ez_iot_core_lowlvl.h"
#include "ez_iot_core_def.h"

ez_void_t base_extend_start_cb(ez_void_t *pUser);
ez_void_t base_extend_stop_cb(ez_void_t *pUser);
ez_void_t base_extend_data_route_cb(ez_kernel_submsg_t *ptr_submsg, ez_void_t *pUser);
ez_void_t base_extend_event_cb(ez_kernel_event_e *ptr_event, ez_void_t *pUser);

ez_err_t base_extern_init(ez_void_t)
{
    ez_kernel_extend_t extern_info;
    memset(&extern_info, 0, sizeof(extern_info));

    extern_info.domain_id = BASE_DOMAIN_ID;
    extern_info.pUser = NULL;
    extern_info.ezdev_sdk_kernel_extend_start = base_extend_start_cb;
    extern_info.ezdev_sdk_kernel_extend_stop = base_extend_stop_cb;
    extern_info.ezdev_sdk_kernel_extend_data_route = base_extend_data_route_cb;
    extern_info.ezdev_sdk_kernel_extend_event = base_extend_event_cb;

    ezos_strncpy(extern_info.extend_module_name, BASE_DOMAIN_NAME, sizeof(extern_info.extend_module_name) - 1);
    ezos_strncpy(extern_info.extend_module_version, BASE_DOMAIN_VER, sizeof(extern_info.extend_module_version) - 1);

    int ret = ez_kernel_extend_load(&extern_info);
    return ret;
}

ez_void_t base_extern_deinit(ez_void_t)
{
    //do nothing
}

ez_void_t base_extend_start_cb(ez_void_t *pUser)
{
}

ez_void_t base_extend_stop_cb(ez_void_t *pUser)
{
}

ez_void_t base_extend_data_route_cb(ez_kernel_submsg_t *ptr_submsg, ez_void_t *pUser)
{
    if (ptr_submsg == NULL)
    {
        return;
    }

    ez_int32_t cmd_id = ptr_submsg->msg_command_id;
    ez_uint32_t msg_seq = ptr_submsg->msg_seq;
    ez_int32_t result_code = 0;

    ezlog_w(TAG_BASE, "cmd:%d", cmd_id);
    ezlog_d(TAG_BASE, "seq:%d, len:%d, data:%s", msg_seq, ptr_submsg->buf_len, ptr_submsg->buf);

    switch (cmd_id)
    {
    case kCenPlt2PuSetUserIdReq:
        result_code = cloud2dev_xml_req_bushandle(ptr_submsg->buf, ptr_submsg->buf_len,
                                                  kCenPlt2PuSetUserIdReq, msg_seq, base_protocol_bind_status_notice_req);
        break;
    case kPu2CenPltGetUserListRsp:
        result_code = base_protocol_bind_status_query_rsp(ptr_submsg->buf, ptr_submsg->buf_len);
        break;
    case kPu2CenPltBindUserWithTokenRsp:
        ///< do nonthing
        break;
    case Pu2CenPltQueryFeatureProfileRsp:
        result_code = base_protocol_query_profile_rsp(ptr_submsg->buf, ptr_submsg->buf_len);
        break;
    case kCenPlt2PuBindUserTouchReq:
        result_code = cloud2dev_json_req_bushandle(ptr_submsg->buf, ptr_submsg->buf_len,
                                                   kCenPlt2PuBindUserTouchRsp, msg_seq, base_protocol_bind_challenge_req);
        break;
    default:
        break;
    }

    if (0 != result_code)
    {
        ezlog_e(TAG_BASE, "rv:%d", result_code);
    }
}

ez_void_t base_extend_event_cb(ez_kernel_event_e *ptr_event, ez_void_t *pUser)
{
}
