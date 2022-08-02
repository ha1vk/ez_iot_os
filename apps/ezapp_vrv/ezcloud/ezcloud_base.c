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
#include "network.h"
#include "ezcloud_link.h"
#include "hal_config.h"

#define KV_USER_ID "user_id"

static ez_bool_t is_user_switched(ez_void_t *data)
{
    ez_bool_t rv = ez_false;
    ez_char_t user_id[32] = {0};
    ez_int32_t length = sizeof(user_id);
    ez_bind_info_t *bind_info = (ez_bind_info_t *)data;

    if (NULL == bind_info || 0 == ezos_strlen(bind_info->user_id))
    {
        return rv;
    }

    hal_config_get_string(KV_USER_ID, user_id, &length, "");

    if (0 == length)
    {
        ezlog_d(TAG_APP, "user_id null");
        hal_config_set_string(KV_USER_ID, bind_info->user_id);
    }
    else if (0 == ezos_strcmp(user_id, bind_info->user_id))
    {
        ezlog_d(TAG_APP, "user_id match");
    }
    else
    {
        ezlog_w(TAG_APP, "user_id mismatch, local:%s, new:%s", user_id, bind_info->user_id);
        hal_config_set_string(KV_USER_ID, bind_info->user_id);
        rv = ez_true;
    }

    return rv;
}

static ez_int32_t ez_base_notice_func(ez_base_event_e event_type, ez_void_t *data, ez_int32_t len)
{
    switch (event_type)
    {
    case EZ_EVENT_BINDING:
    {
        /* 设备已绑定 */
        ezlog_d(TAG_APP, "Device is bound");

        /* 设备换用户, 清空原用户数据 */
        if (is_user_switched(data))
        {
            ezcloud_tsl_prop_reset();
        }
    }
    break;
    case EZ_EVENT_UNBINDING:
    {
        /* 设备未绑定/解绑, 清空网络配置及用户数据 */
        ezlog_d(TAG_APP, "The device is not bound");
        network_reset();
        ezcloud_tsl_prop_reset();
    }
    break;
    default:
        break;
    }

    return 0;
}

ez_void_t ezcloud_base_init(ez_void_t)
{
    ez_char_t bind_token[64] = {0};
    ez_int32_t length = sizeof(bind_token);

    ez_iot_base_init(ez_base_notice_func);

    // The binding token can only be used once
    hal_config_get_string("token", bind_token, &length, "");
    hal_config_del("token");

    if (0 != ezos_strlen(bind_token))
    {
        ez_iot_base_bind_near(bind_token);
    }
    else
    {
        ez_iot_base_bind_query();
    }
}

#ifdef CONFIG_EZIOT_COMPONENT_CLI_ENABLE
#include "cli.h"

static void bind(char *buf, int len, int argc, char **argv)
{
    if (argc > 1)
    {
        ez_iot_base_bind_near(argv[1]);
    }
}

EZOS_CLI_EXPORT("bind", "device bind param : <token> e.g bind cf08393f8581407fad8c3d55dae434ff", bind);

#endif
