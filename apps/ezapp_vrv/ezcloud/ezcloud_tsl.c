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
#include "ez_iot_tsl.h"
#include "ezcloud_tsl_provider.h"
#include "ezcloud_link.h"
#include "hal_config.h"
#include "device_info.h"

static ez_void_t ezcloud_tsl_prop_report(tsl_prop_impl_t *thiz, ez_char_t *index)
{
    ez_tsl_rsc_t rsc_info = {.res_type = thiz->res_type, .local_index = index};
    ez_tsl_key_t key_info = {.domain = thiz->domain, .key = thiz->identify};
    ez_iot_tsl_property_report(dev_info_get_sn(), (const ez_tsl_rsc_t *)&rsc_info, (const ez_tsl_key_t *)&key_info, NULL);
}

// 开机全量上报属性
static ez_void_t tsl_prop_reportall(ez_void_t)
{
    for (ez_int32_t i = 0; g_tsl_prop_lst[i].identify; i++)
    {
        // 静态资源
        if (NULL != g_tsl_prop_lst[i].index)
        {
            ezcloud_tsl_prop_report(&g_tsl_prop_lst[i], g_tsl_prop_lst[i].index);
            continue;
        }

        // 动态资源
        ez_char_t index_lst[CONFIG_EZIOT_COMPONENT_APP_MAX_RESOURCE][4] = {0};
        ez_int32_t count = provider_dynamic_rsc_query(g_tsl_prop_lst[i].res_type, index_lst, sizeof(index_lst) / sizeof(index_lst[0]));
        for (size_t j = 0; j < count; j++)
        {
            ezcloud_tsl_prop_report(&g_tsl_prop_lst[i], index_lst[j]);
        }
    }
}

static ez_int32_t tsl_action2dev(const ez_char_t *sn, const ez_tsl_rsc_t *rsc_info, const ez_tsl_key_t *key_info,
                                 const ez_tsl_value_t *value_in, ez_tsl_value_t *value_out)
{
    ez_int32_t rv = -1;

    for (ez_int32_t i = 0; g_tsl_action_lst[i].identify; i++)
    {
        if (0 != ezos_strcmp(rsc_info->res_type, g_tsl_prop_lst[i].res_type))
        {
            continue;
        }

        if (0 != ezos_strcmp(key_info->domain, g_tsl_action_lst[i].domain))
        {
            continue;
        }

        if (0 != ezos_strcmp(key_info->key, g_tsl_action_lst[i].identify))
        {
            continue;
        }

        rv = g_tsl_action_lst[i].func_do(&g_tsl_action_lst[i], rsc_info, value_in, value_out);
        break;
    }

    return rv;
}

static ez_int32_t tsl_property2cloud(const ez_char_t *sn, const ez_tsl_rsc_t *rsc_info, const ez_tsl_key_t *key_info, ez_tsl_value_t *value_out)
{
    ez_int32_t rv = -1;

    for (ez_int32_t i = 0; g_tsl_prop_lst[i].identify; i++)
    {
        if (0 != ezos_strcmp(rsc_info->res_type, g_tsl_prop_lst[i].res_type))
        {
            continue;
        }

        if (0 != ezos_strcmp(key_info->domain, g_tsl_prop_lst[i].domain))
        {
            continue;
        }

        if (0 != ezos_strcmp(key_info->key, g_tsl_prop_lst[i].identify))
        {
            continue;
        }

        if (NULL == g_tsl_prop_lst[i].func_get)
        {
            break;
        }

        rv = g_tsl_prop_lst[i].func_get(&g_tsl_prop_lst[i], rsc_info, value_out);
        break;
    }

    return rv;
}

static ez_int32_t tsl_property2dev(const ez_char_t *sn, const ez_tsl_rsc_t *rsc_info, const ez_tsl_key_t *key_info, const ez_tsl_value_t *value)
{
    ez_int32_t rv = -1;

    for (ez_int32_t i = 0; g_tsl_prop_lst[i].identify; i++)
    {
        if (0 != ezos_strcmp(rsc_info->res_type, g_tsl_prop_lst[i].res_type))
        {
            continue;
        }

        if (0 != ezos_strcmp(key_info->domain, g_tsl_prop_lst[i].domain))
        {
            continue;
        }

        if (0 != ezos_strcmp(key_info->key, g_tsl_prop_lst[i].identify))
        {
            continue;
        }

        if (NULL == g_tsl_prop_lst[i].func_set)
        {
            break;
        }

        rv = g_tsl_prop_lst[i].func_set(&g_tsl_prop_lst[i], rsc_info, value);
        ez_iot_tsl_property_report(sn, rsc_info, key_info, NULL);
        break;
    }

    return rv;
}

static ez_int32_t tsl_notice(ez_tsl_event_e event_type, ez_void_t *data, ez_int32_t len)
{
    switch (event_type)
    {
    case EZ_EVENT_PROPERTY_FULL_REPORT:
    {
        tsl_prop_reportall();
        break;
    }
    default:
    {
        break;
    }
    }

    return 0;
}

ez_void_t ezcloud_tsl_init(ez_void_t)
{
    ez_tsl_callbacks_t tsl_things_cbs = {tsl_notice, tsl_action2dev, tsl_property2cloud, tsl_property2dev};

    ez_iot_tsl_init(&tsl_things_cbs);
    ez_iot_tsl_reg(NULL);
}

ez_void_t ezcloud_tsl_prop_reset(ez_void_t)
{
    //do nothing

    /* 临时逻辑兼容，绑定清空关联资源 */
    provider_dynamic_rsc_reset();
}