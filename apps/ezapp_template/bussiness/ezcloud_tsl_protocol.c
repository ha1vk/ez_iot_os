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

#include "ezcloud_tsl_protocol.h"
#include "hal_config.h"
#include "ezlog.h"

#define MAKE_DEFVAL(type, _defval) \
    .value_##type = _defval

#define CHECK_VALUE_INVAILD(value_type)                                                  \
    if (NULL == tsl_value || value_type != tsl_value->type)                              \
    {                                                                                    \
        ezlog_w(TAG_APP, "value_type1:%d, value_type2:%d", value_type, tsl_value->type); \
        ezlog_e(TAG_APP, "tsl value invaild, identify:%s", thiz->identify);              \
        goto done;                                                                       \
    }

static ez_bool_t property_set_wrapper(tsl_prop_impl_t *thiz, const ez_tsl_value_t *tsl_value)
{
    ez_bool_t rv = ez_false;

    switch (tsl_value->type)
    {
    case EZ_TSL_DATA_TYPE_BOOL:
    case EZ_TSL_DATA_TYPE_INT:
    {
        rv = hal_config_set_int(thiz->identify, tsl_value->value_int);
        break;
    }
    case EZ_TSL_DATA_TYPE_DOUBLE:
    {
        rv = hal_config_set_double(thiz->identify, tsl_value->value_double);
        break;
    }
    case EZ_TSL_DATA_TYPE_STRING:
    case EZ_TSL_DATA_TYPE_ARRAY:
    case EZ_TSL_DATA_TYPE_OBJECT:
    {
        rv = hal_config_set_string(thiz->identify, (const ez_char_t *)tsl_value->value);
        break;
    }
    default:
        break;
    }

    if (!rv)
    {
        ezlog_e(TAG_APP, "Error occurred in saving data, identify:%s", thiz->identify);
    }

    return rv;
}

static ez_void_t property_get_wrapper(tsl_prop_impl_t *thiz, ez_tsl_value_t *tsl_value, ez_tsl_data_type_e value_type)
{
    switch (value_type)
    {
    case EZ_TSL_DATA_TYPE_BOOL:
    case EZ_TSL_DATA_TYPE_INT:
    {
        tsl_value->size = sizeof(thiz->value_int);
        hal_config_get_int(thiz->identify, &tsl_value->value_int, thiz->value_int);
        break;
    }
    case EZ_TSL_DATA_TYPE_DOUBLE:
    {
        tsl_value->size = sizeof(thiz->value_double);
        hal_config_get_double(thiz->identify, &tsl_value->value_double, thiz->value_double);
        break;
    }
    case EZ_TSL_DATA_TYPE_STRING:
    case EZ_TSL_DATA_TYPE_ARRAY:
    case EZ_TSL_DATA_TYPE_OBJECT:
    {
        hal_config_get_string(thiz->identify, (ez_char_t *)tsl_value->value, (ez_int32_t *)&tsl_value->size, thiz->value_string);
        break;
    }
    default:
        break;
    }

    tsl_value->type = value_type;
}

static ez_int32_t property_brightness_set(tsl_prop_impl_t *thiz, const ez_tsl_value_t *tsl_value)
{
    ez_int32_t rv = -1;

    CHECK_VALUE_INVAILD(EZ_TSL_DATA_TYPE_INT);

    // TODO 执行业务

    property_set_wrapper(thiz, tsl_value);
    rv = EZ_TSL_ERR_SUCC;
done:
    return rv;
}

static ez_int32_t property_brightness_get(tsl_prop_impl_t *thiz, ez_tsl_value_t *tsl_value)
{
    ez_int32_t rv = -1;

    property_get_wrapper(thiz, tsl_value, EZ_TSL_DATA_TYPE_INT);
    rv = EZ_TSL_ERR_SUCC;
    return rv;
}

static ez_int32_t property_colortemperature_set(tsl_prop_impl_t *thiz, const ez_tsl_value_t *tsl_value)
{
    ez_int32_t rv = -1;

    CHECK_VALUE_INVAILD(EZ_TSL_DATA_TYPE_INT);

    // TODO 执行业务

    property_set_wrapper(thiz, tsl_value);
    rv = EZ_TSL_ERR_SUCC;
done:
    return rv;
}

static ez_int32_t property_colortemperature_get(tsl_prop_impl_t *thiz, ez_tsl_value_t *tsl_value)
{
    ez_int32_t rv = -1;

    property_get_wrapper(thiz, tsl_value, EZ_TSL_DATA_TYPE_INT);
    rv = EZ_TSL_ERR_SUCC;
    return rv;
}

static ez_int32_t property_countdowncfg_set(tsl_prop_impl_t *thiz, const ez_tsl_value_t *tsl_value)
{
    ez_int32_t rv = -1;

    CHECK_VALUE_INVAILD(EZ_TSL_DATA_TYPE_OBJECT);

    // TODO 执行业务

    property_set_wrapper(thiz, tsl_value);
    rv = EZ_TSL_ERR_SUCC;
done:
    return rv;
}

static ez_int32_t property_countdowncfg_get(tsl_prop_impl_t *thiz, ez_tsl_value_t *tsl_value)
{
    ez_int32_t rv = -1;
 
    property_get_wrapper(thiz, tsl_value, EZ_TSL_DATA_TYPE_OBJECT);
    rv = EZ_TSL_ERR_SUCC;
    return rv;
}

static ez_int32_t property_lightswitchplan_set(tsl_prop_impl_t *thiz, const ez_tsl_value_t *tsl_value)
{
    ez_int32_t rv = -1;

    CHECK_VALUE_INVAILD(EZ_TSL_DATA_TYPE_OBJECT);

    // TODO 执行业务

    property_set_wrapper(thiz, tsl_value);
    rv = EZ_TSL_ERR_SUCC;
done:
    return rv;
}

static ez_int32_t property_lightswitchplan_get(tsl_prop_impl_t *thiz, ez_tsl_value_t *tsl_value)
{
    ez_int32_t rv = -1;

    property_get_wrapper(thiz, tsl_value, EZ_TSL_DATA_TYPE_OBJECT);
    rv = EZ_TSL_ERR_SUCC;

    return rv;
}

static ez_int32_t property_powerswitch_set(tsl_prop_impl_t *thiz, const ez_tsl_value_t *tsl_value)
{
    ez_int32_t rv = -1;

    CHECK_VALUE_INVAILD(EZ_TSL_DATA_TYPE_BOOL);

    // TODO 执行业务

    property_set_wrapper(thiz, tsl_value);
    rv = EZ_TSL_ERR_SUCC;
done:
    return rv;
}

static ez_int32_t property_powerswitch_get(tsl_prop_impl_t *thiz, ez_tsl_value_t *tsl_value)
{
    ez_int32_t rv = -1;

    property_get_wrapper(thiz, tsl_value, EZ_TSL_DATA_TYPE_BOOL);
    rv = EZ_TSL_ERR_SUCC;

    return rv;
}

static ez_int32_t property_biorhythm_set(tsl_prop_impl_t *thiz, const ez_tsl_value_t *tsl_value)
{
    ez_int32_t rv = -1;

    CHECK_VALUE_INVAILD(EZ_TSL_DATA_TYPE_OBJECT);

    // TODO 执行业务

    property_set_wrapper(thiz, tsl_value);
    rv = EZ_TSL_ERR_SUCC;
done:
    return rv;
}

static ez_int32_t property_biorhythm_get(tsl_prop_impl_t *thiz, ez_tsl_value_t *tsl_value)
{
    ez_int32_t rv = -1;

    property_get_wrapper(thiz, tsl_value, EZ_TSL_DATA_TYPE_OBJECT);
    rv = EZ_TSL_ERR_SUCC;

    return rv;
}

static ez_int32_t property_colorrgb_set(tsl_prop_impl_t *thiz, const ez_tsl_value_t *tsl_value)
{
    ez_int32_t rv = -1;

    CHECK_VALUE_INVAILD(EZ_TSL_DATA_TYPE_STRING);

    // TODO 执行业务

    property_set_wrapper(thiz, tsl_value);
    rv = EZ_TSL_ERR_SUCC;
done:
    return rv;
}

static ez_int32_t property_colorrgb_get(tsl_prop_impl_t *thiz, ez_tsl_value_t *tsl_value)
{
    ez_int32_t rv = -1;

    property_get_wrapper(thiz, tsl_value, EZ_TSL_DATA_TYPE_STRING);
    rv = EZ_TSL_ERR_SUCC;

    return rv;
}

static ez_int32_t property_helpsleep_set(tsl_prop_impl_t *thiz, const ez_tsl_value_t *tsl_value)
{
    ez_int32_t rv = -1;

    CHECK_VALUE_INVAILD(EZ_TSL_DATA_TYPE_ARRAY);

    // TODO 执行业务

    property_set_wrapper(thiz, tsl_value);
    rv = EZ_TSL_ERR_SUCC;
done:
    return rv;
}

static ez_int32_t property_helpsleep_get(tsl_prop_impl_t *thiz, ez_tsl_value_t *tsl_value)
{
    ez_int32_t rv = -1;

    property_get_wrapper(thiz, tsl_value, EZ_TSL_DATA_TYPE_ARRAY);
    rv = EZ_TSL_ERR_SUCC;

    return rv;
}

static ez_int32_t property_musicrhythm_set(tsl_prop_impl_t *thiz, const ez_tsl_value_t *tsl_value)
{
    ez_int32_t rv = -1;

    CHECK_VALUE_INVAILD(EZ_TSL_DATA_TYPE_STRING);

    // TODO 执行业务
    // do nothing

    property_set_wrapper(thiz, tsl_value);
    rv = EZ_TSL_ERR_SUCC;
done:
    return rv;
}

static ez_int32_t property_musicrhythm_get(tsl_prop_impl_t *thiz, ez_tsl_value_t *tsl_value)
{
    ez_int32_t rv = -1;

    property_get_wrapper(thiz, tsl_value, EZ_TSL_DATA_TYPE_STRING);
    rv = EZ_TSL_ERR_SUCC;

    return rv;
}

static ez_int32_t property_customscenecfg_set(tsl_prop_impl_t *thiz, const ez_tsl_value_t *tsl_value)
{
    ez_int32_t rv = -1;

    CHECK_VALUE_INVAILD(EZ_TSL_DATA_TYPE_OBJECT);

    // TODO 执行业务

    property_set_wrapper(thiz, tsl_value);
    rv = EZ_TSL_ERR_SUCC;
done:
    return rv;
}

static ez_int32_t property_customscenecfg_get(tsl_prop_impl_t *thiz, ez_tsl_value_t *tsl_value)
{
    ez_int32_t rv = -1;

    property_get_wrapper(thiz, tsl_value, EZ_TSL_DATA_TYPE_OBJECT);
    rv = EZ_TSL_ERR_SUCC;

    return rv;
}

static ez_int32_t property_wakeup_set(tsl_prop_impl_t *thiz, const ez_tsl_value_t *tsl_value)
{
    ez_int32_t rv = -1;

    CHECK_VALUE_INVAILD(EZ_TSL_DATA_TYPE_ARRAY);

    // TODO 执行业务

    property_set_wrapper(thiz, tsl_value);
    rv = EZ_TSL_ERR_SUCC;
done:
    return rv;
}

static ez_int32_t property_wakeup_get(tsl_prop_impl_t *thiz, ez_tsl_value_t *tsl_value)
{
    ez_int32_t rv = -1;

    property_get_wrapper(thiz, tsl_value, EZ_TSL_DATA_TYPE_ARRAY);
    rv = EZ_TSL_ERR_SUCC;

    return rv;
}

static ez_int32_t property_workmode_set(tsl_prop_impl_t *thiz, const ez_tsl_value_t *tsl_value)
{
    ez_int32_t rv = -1;

    CHECK_VALUE_INVAILD(EZ_TSL_DATA_TYPE_STRING);

    // TODO 执行业务

    property_set_wrapper(thiz, tsl_value);
    rv = EZ_TSL_ERR_SUCC;
done:
    return rv;
}

static ez_int32_t property_workmode_get(tsl_prop_impl_t *thiz, ez_tsl_value_t *tsl_value)
{
    ez_int32_t rv = -1;

    property_get_wrapper(thiz, tsl_value, EZ_TSL_DATA_TYPE_STRING);
    rv = EZ_TSL_ERR_SUCC;

    return rv;
}

static ez_int32_t property_timezonecompose_set(tsl_prop_impl_t *thiz, const ez_tsl_value_t *tsl_value)
{
    ez_int32_t rv = -1;

    CHECK_VALUE_INVAILD(EZ_TSL_DATA_TYPE_OBJECT);

    // TODO 执行业务

    property_set_wrapper(thiz, tsl_value);
    rv = EZ_TSL_ERR_SUCC;
done:
    return rv;
}

static ez_int32_t property_timezonecompose_get(tsl_prop_impl_t *thiz, ez_tsl_value_t *tsl_value)
{
    ez_int32_t rv = -1;

    property_get_wrapper(thiz, tsl_value, EZ_TSL_DATA_TYPE_OBJECT);
    rv = EZ_TSL_ERR_SUCC;

    return rv;
}

static ez_int32_t property_netstatus_get(tsl_prop_impl_t *thiz, ez_tsl_value_t *tsl_value)
{
    ez_int32_t rv = -1;

    property_get_wrapper(thiz, tsl_value, EZ_TSL_DATA_TYPE_OBJECT);
    rv = EZ_TSL_ERR_SUCC;

    return rv;
}

static ez_int32_t action_getcountdown_down(tsl_action_impl_t *thiz, const ez_tsl_value_t *value_in, ez_tsl_value_t *value_out)
{
    ez_int32_t rv = -1;

    // TODO 执行业务

    rv = EZ_TSL_ERR_SUCC;
    return rv;
}

tsl_prop_impl_t g_tsl_prop_lst[] = {
    {"Brightness", "LightCtrl", "global", "0", property_brightness_set, property_brightness_get, MAKE_DEFVAL(int, 100)},
    {"ColorTemperature", "LightCtrl", "global", "0", property_colortemperature_set, property_colortemperature_get, MAKE_DEFVAL(int, 100)},
    {"CountdownCfg", "LightCtrl", "global", "0", property_countdowncfg_set, property_countdowncfg_get, MAKE_DEFVAL(string, "{}")},
    {"LightSwitchPlan", "LightCtrl", "global", "0", property_lightswitchplan_set, property_lightswitchplan_get, MAKE_DEFVAL(string, "{}")},
    {"PowerSwitch", "PowerMgr", "global", "0", property_powerswitch_set, property_powerswitch_get, MAKE_DEFVAL(int, 1)},
    {"Biorhythm", "RGBLightCtrl", "global", "0", property_biorhythm_set, property_biorhythm_get, MAKE_DEFVAL(string, "{}")},
    {"ColorRgb", "RGBLightCtrl", "global", "0", property_colorrgb_set, property_colorrgb_get, MAKE_DEFVAL(string, "#FF0000")},
    {"HelpSleep", "RGBLightCtrl", "global", "0", property_helpsleep_set, property_helpsleep_get, MAKE_DEFVAL(string, "{}")},
    {"MusicRhythm", "RGBLightCtrl", "global", "0", property_musicrhythm_set, property_musicrhythm_get, MAKE_DEFVAL(string, "off")},
    {"CustomSceneCfg", "RGBLightCtrl", "global", "0", property_customscenecfg_set, property_customscenecfg_get, MAKE_DEFVAL(string, "{}")},
    {"WakeUp", "RGBLightCtrl", "global", "0", property_wakeup_set, property_wakeup_get, MAKE_DEFVAL(string, "{}")},
    {"WorkMode", "RGBLightCtrl", "global", "0", property_workmode_set, property_workmode_get, MAKE_DEFVAL(string, "white")},
    {"TimeZoneCompose", "TimeMgr", "global", "0", property_timezonecompose_set, property_timezonecompose_get, MAKE_DEFVAL(string, "{\"timeFormat\":\"0\",\"timeZone\": \"UTC+08:00\",\"tzCode\": 42,\"daylightSavingTime\": 1,\"offsetTime\": 0,\"startMonth\": 0,\"startWeekIndex\": \"\",\"startWeekDay\": \"\",\"startTime\": \"08:00:00\",\"endMonth\": 0,\"endWeekIndex\": \"\",\"endWeekDay\": \"\",\"endTime\": \"08:00:00\"}")},
    {"NetStatus", "WifiStatus", "global", "0", NULL, property_netstatus_get, MAKE_DEFVAL(string, "{}")},
    {NULL, NULL, NULL, NULL, NULL, NULL, 0},
};

tsl_action_impl_t g_tsl_action_lst[] = {
    {"GetCountdown", "LightCtrl", "global", "0", action_getcountdown_down},
    {NULL, NULL, NULL, NULL, NULL},
};

#ifdef CONFIG_EZIOT_COMPONENT_CLI_ENABLE
#include "cli.h"

static void show_kv(char *buf, int len, int argc, char **argv)
{
    ez_char_t kv_buf[2048] = {0};
    ez_int32_t kv_buf_len = sizeof(kv_buf);

    if (argc > 1)
    {
        hal_config_get_string(argv[1], kv_buf, &kv_buf_len, "");
        ezlog_hexdump(TAG_APP, 16, kv_buf, kv_buf_len);
    }
    else
    {
        hal_config_print();
    }
}

EZOS_CLI_EXPORT("showkv", "show kv data, param : <key> e.g showkv or showkv Brightness", show_kv);

#endif