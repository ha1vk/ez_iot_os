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

#include "ezlog.h"

enum
{
    kPu2CenPltHubReportRelationShipReq = 0x00000001,
    kPu2CenPltHubReportRelationShipRsp = 0x00000002,
    kPu2CenPltHubReportOnlineStatusReq = 0x00000003,
    kPu2CenPltHubReportOnlineStatusRsp = 0x00000004,
    kPu2CenPltHubAuthChildDeviceReq = 0x000000010,
    kPu2CenPltHubAuthChildDeviceRsp = 0x000000011,
    kPu2CenPltHubTransferMsgReq = 0x00000007,
    kPu2CenPltHubTransferMsgRsp = 0x00000008,
    kCenPlt2PuAddChildDeviceReq = 0x00004E80,
    kCenPlt2PuAddChildDeviceRsp = 0x00004E81,
    kCenPlt2PuDeleteChildDeviceReq = 0x00004E82,
    kCenPlt2PuDeleteChildDeviceRsp = 0x00004E83,
};

#define EZDEVSDK_HUB_RSP 1
#define EZDEVSDK_HUB_REQ 0

#define hub_module_id 7002
#define hub_module_name "hub"
#define hub_module_version "V2.0.0"



EZ_INT hub_extern_init();

EZ_INT hub_extern_finit();

EZ_INT hub_send_msg_to_platform(const EZ_CHAR *msg, EZ_INT msg_len, EZ_INT cmd_id, EZ_UCHAR msg_response, EZ_UINT msg_seq);

#define CHECK_COND_RETURN(cond, errcode) \
        if ((cond))                          \
        {                                    \
            ezlog_e(TAG_HUB, "cond return:0x%x,errcode:0x%x",cond, errcode);    \
            return (errcode);                \
        }

#define CHECK_COND_DONE(cond, errcode)                                   \
    if ((cond))                                                          \
    {                                                                    \
        ezlog_e(TAG_HUB, "cond done:0x%x,errcode:0x%x", cond, errcode); \
        rv = (errcode);                                                  \
        goto done;                                                       \
    }

#define CHECK_RV_DONE(errcode)                      \
    if (0 != errcode)                               \
    {                                               \
        ezlog_e(TAG_HUB, "errcode:0x%x", errcode); \
        rv = (errcode);                             \
        goto done;                                  \
    }

#define SAFE_FREE(p)  \
    if (p)            \
    {                 \
        ezos_free(p); \
        p = NULL;     \
    }

#define CJSON_SAFE_DELETE(p) \
    if (p)                   \
    {                        \
        cJSON_Delete(p);   \
        p = NULL;            \
    }

