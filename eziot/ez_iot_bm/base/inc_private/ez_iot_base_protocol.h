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

#ifndef _EZ_IOT_BASE_PROTOCOL_H_
#define _EZ_IOT_BASE_PROTOCOL_H_

#include <ezos.h>

#define kCenPlt2PuDomainConfig 0x00000001
#define kPu2CenPltGetUserListReq 0X00003445
#define kPu2CenPltGetUserListRsp 0X00003446
#define kCenPlt2PuSetUserIdReq 0X00004947
#define kCenPlt2PuSetUserIdRsp 0X00004948
#define kPu2CenPltBindUserWithTokenReq 0X00003802
#define kPu2CenPltBindUserWithTokenRsp 0X00003803
#define Pu2CenPltQueryFeatureProfileReq 0x0003870
#define Pu2CenPltQueryFeatureProfileRsp 0x0003871
#define kCenPlt2PuBindUserTouchReq 0x00004F14
#define kCenPlt2PuBindUserTouchRsp 0x00004F15
#define kPu2CenPltReportBindUserTouchWithTokenReq 0x00003808
#define kPu2CenPltReportBindUserTouchWithTokenRsp 0x00003809

#ifdef __cplusplus
extern "C"
{
#endif

    typedef ez_err_t (*handle_proc_fun)(ez_void_t *buf, ez_int32_t len);

    ///< 下行请求响应
    ez_err_t cloud2dev_xml_req_bushandle(ez_void_t *buf, ez_int32_t len, ez_int32_t rsp_cmd,
                                         ez_uint32_t seq, handle_proc_fun proc_func);

    ez_err_t cloud2dev_json_req_bushandle(ez_void_t *buf, ez_int32_t len, ez_int32_t rsp_cmd,
                                         ez_uint32_t seq, handle_proc_fun proc_func);

    ///< 绑定状态变更
    ez_err_t base_protocol_bind_status_notice_req(ez_void_t *buf, ez_int32_t len);

    ///< 绑定关系查询
    ez_err_t base_protocol_bind_status_query_req();
    ez_err_t base_protocol_bind_status_query_rsp(ez_void_t *buf, ez_int32_t len);

    ///< 近场绑定
    ez_err_t base_protocol_near_bind_req(ez_char_t *token);

    ///< 物理接触绑定
    ez_err_t base_protocol_bind_challenge_req(ez_void_t *buf, ez_int32_t len);
    ez_err_t base_protocol_bind_response_req(ez_int32_t response_code);

    ///< 物模型查询
    ez_err_t base_protocol_query_profile_req(const ez_char_t *req_msg, ez_void_t* func_rsp);
    ez_err_t base_protocol_query_profile_rsp(ez_void_t *buf, ez_int32_t len);

#ifdef __cplusplus
}
#endif

#endif