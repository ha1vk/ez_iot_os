/*******************************************************************************
 * Copyright © 2017-2022 Ezviz Inc.
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
 * 2022-03-20     xurongjun    first version 
 *******************************************************************************/

#include "ezos.h"
#include "ezlog.h"
#include "ez_iot_base.h"

static ez_int32_t ez_base_notice_func(ez_base_event_e event_type, ez_void_t *data, ez_int32_t len)
{
    ez_bind_challenge_t *bind_chanllenge;

    switch (event_type)
    {
    case EZ_EVENT_BINDING:
    {
        /* 设备已绑定 */
        ezlog_d(TAG_APP, "Device is bound");
        //TODO 处理抢占式
    }
    break;
    case EZ_EVENT_UNBINDING:
    {
        /* 设备未绑定 */
        ezlog_d(TAG_APP, "The device is not bound");
        //TODO 清空所有用户数据
    }
    break;
    default:
        break;
    }

    return 0;
}

ez_void_t ezcloud_base_init(ez_void_t)
{
    ez_iot_base_init(ez_base_notice_func);
}
