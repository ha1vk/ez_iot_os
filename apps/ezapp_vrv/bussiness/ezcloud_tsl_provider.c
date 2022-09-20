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
 * XuRongjun (xurongjun@ezvizlife.com) - Device Thing Specification Language Protocol
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-03-22     xurongjun    first version
 *******************************************************************************/

#include "ezcloud_tsl_provider.h"
#include "ezcloud_tsl_control.h"
#include "ezcloud_tsl_aircondition.h"
#include "ezcloud_tsl_airfresh.h"
#include "ezcloud_tsl_storage.h"
#include "device_info.h"
#include "hal_config.h"
#include "ezlog.h"
#include "cJSON.h"

tsl_prop_impl_t g_tsl_prop_lst[] = {
    // 静态资源 - 控制器
    {"Resources", "DynamicResource", "global", "0", NULL, control_property_Resource_get},
    {"NetStatus", "WifiStatus", "global", "0", NULL, control_property_NetStatus_get},

    // 动态资源 - 空调
    {"PowerSwitch", "PowerMgr", "AirCondition", NULL, aircondition_property_PowerSwitch_set, aircondition_property_PowerSwitch_get},
    {"CountdownCfg", "LightCtrl", "AirCondition", NULL, aircondition_property_CountdownCfg_set, aircondition_property_CountdownCfg_get},
    {"Temperature", "AirConditionCtrl", "AirCondition", NULL, aircondition_property_Temperature_set, aircondition_property_Temperature_get},
    {"ACException", "AirConditionCtrl", "AirCondition", NULL, NULL, aircondition_property_ACException_get},
    {"RoomTemperature", "AirConditionCtrl", "AirCondition", NULL, aircondition_property_RoomTemperature_set, aircondition_property_RoomTemperature_get},
    {"WindSpeedLevel", "AirConditionCtrl", "AirCondition", NULL, aircondition_property_WindSpeedLevel_set, aircondition_property_WindSpeedLevel_get},
    {"WorkMode", "AirConditionCtrl", "AirCondition", NULL, aircondition_property_WorkMode_set, aircondition_property_WorkMode_get},

    // 动态资源 - 新风机
    {"PowerSwitch", "PowerMgr", "AirFresh", NULL, airfresh_property_PowerSwitch_set, airfresh_property_PowerSwitch_get},
    {"CountdownCfg", "LightCtrl", "AirFresh", NULL, airfresh_property_CountdownCfg_set, airfresh_property_CountdownCfg_get},
    {"WindSpeedLevel", "AirConditionCtrl", "AirFresh", NULL, airfresh_property_WindSpeedLevel_set, airfresh_property_WindSpeedLevel_get},
    {"WorkMode", "AirConditionCtrl", "AirFresh", NULL, airfresh_property_WorkMode_set, airfresh_property_WorkMode_get},

    {NULL, NULL, NULL, NULL, NULL, NULL},
};

tsl_action_impl_t g_tsl_action_lst[] = {
    {NULL, NULL, NULL, NULL, NULL},
};

ez_int32_t provider_dynamic_rsc_query(const ez_char_t *res_type, ez_char_t index_lst[][4], ez_int32_t max_count)
{
    ez_int32_t count = 0;
    cJSON *js_root = NULL;
    ez_char_t buf[1024] = {0};
    ez_tsl_value_t value = {.value = buf, .size = sizeof(buf)};
    ez_tsl_rsc_t rsc_info = {.res_type = g_tsl_prop_lst[0].res_type, .local_index = g_tsl_prop_lst[0].index};

    property_get_wrapper(&g_tsl_prop_lst[0], &rsc_info, &value, EZ_TSL_DATA_TYPE_ARRAY, "");

    js_root = cJSON_Parse(value.value);
    if (NULL == js_root)
    {
        goto done;
    }

    for (size_t i = 0; i < cJSON_GetArraySize(js_root); i++)
    {
        cJSON *js_object = cJSON_GetArrayItem(js_root, i);
        cJSON *js_rc = cJSON_GetObjectItem(js_object, "rc");
        if (0 != ezos_strcmp(js_rc->valuestring, res_type))
        {
            continue;
        }

        cJSON *js_index_lst = cJSON_GetObjectItem(js_object, "index");
        if (NULL == js_index_lst || cJSON_Array != js_index_lst->type)
        {
            break;
        }

        for (size_t j = 0; j < cJSON_GetArraySize(js_index_lst); j++)
        {
            cJSON *js_index = cJSON_GetArrayItem(js_index_lst, j);
            if (j + 1 > max_count)
            {
                break;
            }

            ezos_strncpy(index_lst[j], js_index->valuestring, 3);
            count++;
        }
    }

done:
    cJSON_free(js_root);
    return count;
}

ez_void_t provider_dynamic_rsc_report(ez_void_t)
{
    tsl_prop_impl_t ctx = {"Resources", "DynamicResource", "global", "0", NULL, control_property_Resource_get};
    ez_tsl_rsc_t rsc_info = {ctx.res_type, ctx.index};
    ez_tsl_key_t key_info = {ctx.domain, ctx.identify};

    // 上报更新属性
    ez_iot_tsl_property_report(dev_info_get_sn(), &rsc_info, &key_info, NULL);
}

ez_void_t provider_dynamic_rsc_reset(ez_void_t)
{
    tsl_prop_impl_t ctx = {"Resources", "DynamicResource", "global", "0", NULL, control_property_Resource_get};
    ez_tsl_rsc_t rsc_info = {ctx.res_type, ctx.index};
    ez_tsl_key_t key_info = {ctx.domain, ctx.identify};
    ez_char_t *_defval = "[{\"rc\":\"AirCondition\",\"rid\":\"AirCondition\",\"index\":[]},{\"rc\":\"AirFresh\",\"rid\":\"AirFreshRes\",\"index\":[]}]";

    // 保存至flash
    ez_tsl_value_t tsl_value = {.type = EZ_TSL_DATA_TYPE_ARRAY, .size = ezos_strlen(_defval), .value = _defval};
    property_set_wrapper(&ctx, &rsc_info, &tsl_value);

    // 上报更新属性
    ez_iot_tsl_property_report(dev_info_get_sn(), &rsc_info, &key_info, NULL);
}

#ifdef CONFIG_EZIOT_COMPONENT_CLI_ENABLE
#include "cli.h"

static void show_kv(char *buf, int len, int argc, char **argv)
{
    ez_char_t kv_buf[2048] = {0};
    ez_int32_t kv_buf_len = sizeof(kv_buf);

    if (argc > 1)
    {
        hal_config_get_string(argv[1], kv_buf, &kv_buf_len, "");
        ezlog_hexdump(TAG_APP, 16, (ez_uchar_t *)kv_buf, kv_buf_len);
    }
    else
    {
        hal_config_print();
    }
}

EZOS_CLI_EXPORT("showkv", "show kv data, param : <key> e.g showkv or showkv Brightness", show_kv);

#endif