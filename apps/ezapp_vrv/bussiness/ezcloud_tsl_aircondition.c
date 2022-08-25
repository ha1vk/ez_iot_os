#include "ezcloud_tsl_aircondition.h"
#include "ezcloud_tsl_storage.h"
#include "ezlog.h"

#ifdef CONFIG_EZIOT_COMPONENT_CLI_ENABLE
#include "cli.h"
#include "device_info.h"
#endif

ez_int32_t aircondition_property_PowerSwitch_get(tsl_prop_impl_t *thiz, const ez_tsl_rsc_t *rsc_info, ez_tsl_value_t *tsl_value)
{
    ez_int32_t rv = -1;
    ez_int32_t _defval = 16;

    property_get_wrapper(thiz, rsc_info, tsl_value, EZ_TSL_DATA_TYPE_BOOL, &_defval);

    rv = EZ_TSL_ERR_SUCC;
    return rv;
}

ez_int32_t aircondition_property_CountdownCfg_get(tsl_prop_impl_t *thiz, const ez_tsl_rsc_t *rsc_info, ez_tsl_value_t *tsl_value)
{
    ez_int32_t rv = -1;
    ez_char_t *_defval = "{\"enable\":false,\"switch\":true,\"timeRemaining\":60}";

    property_get_wrapper(thiz, rsc_info, tsl_value, EZ_TSL_DATA_TYPE_OBJECT, _defval);

    rv = EZ_TSL_ERR_SUCC;
    return rv;
}

ez_int32_t aircondition_property_Temperature_get(tsl_prop_impl_t *thiz, const ez_tsl_rsc_t *rsc_info, ez_tsl_value_t *tsl_value)
{
    ez_int32_t rv = -1;
    ez_double_t _defval = 16;

    property_get_wrapper(thiz, rsc_info, tsl_value, EZ_TSL_DATA_TYPE_DOUBLE, &_defval);

    rv = EZ_TSL_ERR_SUCC;
    return rv;
}

ez_int32_t aircondition_property_ACException_get(tsl_prop_impl_t *thiz, const ez_tsl_rsc_t *rsc_info, ez_tsl_value_t *tsl_value)
{
    ez_int32_t rv = -1;
    ez_char_t *_defval = "{\"errorCode\":\"0x0\"}";

    property_get_wrapper(thiz, rsc_info, tsl_value, EZ_TSL_DATA_TYPE_OBJECT, _defval);

    rv = EZ_TSL_ERR_SUCC;
    return rv;
}

ez_int32_t aircondition_property_RoomTemperature_get(tsl_prop_impl_t *thiz, const ez_tsl_rsc_t *rsc_info, ez_tsl_value_t *tsl_value)
{
    ez_int32_t rv = -1;
    ez_double_t _defval = 16;

    property_get_wrapper(thiz, rsc_info, tsl_value, EZ_TSL_DATA_TYPE_DOUBLE, &_defval);

    rv = EZ_TSL_ERR_SUCC;
    return rv;
}

ez_int32_t aircondition_property_WindSpeedLevel_get(tsl_prop_impl_t *thiz, const ez_tsl_rsc_t *rsc_info, ez_tsl_value_t *tsl_value)
{
    ez_int32_t rv = -1;
    ez_char_t *_defval = "auto";

    property_get_wrapper(thiz, rsc_info, tsl_value, EZ_TSL_DATA_TYPE_STRING, _defval);

    rv = EZ_TSL_ERR_SUCC;
    return rv;
}

ez_int32_t aircondition_property_WorkMode_get(tsl_prop_impl_t *thiz, const ez_tsl_rsc_t *rsc_info, ez_tsl_value_t *tsl_value)
{
    ez_int32_t rv = -1;
    ez_char_t *_defval = "auto";

    property_get_wrapper(thiz, rsc_info, tsl_value, EZ_TSL_DATA_TYPE_STRING, _defval);

    rv = EZ_TSL_ERR_SUCC;
    return rv;
}

ez_int32_t aircondition_property_PowerSwitch_set(tsl_prop_impl_t *thiz, const ez_tsl_rsc_t *rsc_info, const ez_tsl_value_t *tsl_value)
{
    ez_int32_t rv = EZ_TSL_ERR_SUCC;

    // TODO 执行业务(控制电源的开关)
    if (EZ_TSL_ERR_SUCC != rv)
    {
        goto done;
    }

    property_set_wrapper(thiz, rsc_info, tsl_value);
done:

    return rv;
}

ez_int32_t aircondition_property_CountdownCfg_set(tsl_prop_impl_t *thiz, const ez_tsl_rsc_t *rsc_info, const ez_tsl_value_t *tsl_value)
{
    ez_int32_t rv = EZ_TSL_ERR_SUCC;

    // TODO 执行业务(开关倒计时任务)
    if (EZ_TSL_ERR_SUCC != rv)
    {
        goto done;
    }

    property_set_wrapper(thiz, rsc_info, tsl_value);
done:
    return rv;
}

ez_int32_t aircondition_property_Temperature_set(tsl_prop_impl_t *thiz, const ez_tsl_rsc_t *rsc_info, const ez_tsl_value_t *tsl_value)
{
    ez_int32_t rv = EZ_TSL_ERR_SUCC;

    // TODO 执行业务(设置空调温度)
    if (EZ_TSL_ERR_SUCC != rv)
    {
        goto done;
    }

    property_set_wrapper(thiz, rsc_info, tsl_value);
done:
    return rv;
}

ez_int32_t aircondition_property_RoomTemperature_set(tsl_prop_impl_t *thiz, const ez_tsl_rsc_t *rsc_info, const ez_tsl_value_t *tsl_value)
{
    ez_int32_t rv = EZ_TSL_ERR_SUCC;

    // TODO 执行业务(设置室内温度)
    if (EZ_TSL_ERR_SUCC != rv)
    {
        goto done;
    }

    property_set_wrapper(thiz, rsc_info, tsl_value);
    rv = EZ_TSL_ERR_SUCC;
done:
    return rv;
}

ez_int32_t aircondition_property_WindSpeedLevel_set(tsl_prop_impl_t *thiz, const ez_tsl_rsc_t *rsc_info, const ez_tsl_value_t *tsl_value)
{
    ez_int32_t rv = EZ_TSL_ERR_SUCC;

    // TODO 执行业务(设置风速档位)
    if (EZ_TSL_ERR_SUCC != rv)
    {
        goto done;
    }

    property_set_wrapper(thiz, rsc_info, tsl_value);
    rv = EZ_TSL_ERR_SUCC;
done:
    return rv;
}

ez_int32_t aircondition_property_WorkMode_set(tsl_prop_impl_t *thiz, const ez_tsl_rsc_t *rsc_info, const ez_tsl_value_t *tsl_value)
{
    ez_int32_t rv = EZ_TSL_ERR_SUCC;

    // TODO 执行业务(设置空调工作模式)
    if (EZ_TSL_ERR_SUCC != rv)
    {
        goto done;
    }

    property_set_wrapper(thiz, rsc_info, tsl_value);
done:
    return rv;
}

#ifdef CONFIG_EZIOT_COMPONENT_CLI_ENABLE

static void ps_set(char *buf, int len, int argc, char **argv)
{
    tsl_prop_impl_t ctx = {"PowerSwitch", "PowerMgr", "AirCondition", NULL, aircondition_property_PowerSwitch_set, aircondition_property_PowerSwitch_get};
    ez_tsl_rsc_t rsc_info = {ctx.res_type, "0"};
    ez_tsl_key_t key_info = {ctx.domain, ctx.identify};

    if (argc >= 2)
    {
        rsc_info.local_index = argv[1];

        // 保存至flash
        ez_tsl_value_t tsl_value = {.type = EZ_TSL_DATA_TYPE_BOOL, .size = ezos_strlen(argv[2]), .value_int = ezos_atoi(argv[2])};
        property_set_wrapper(&ctx, &rsc_info, &tsl_value);

        // 上报更新属性
        ez_iot_tsl_property_report(dev_info_get_sn(), &rsc_info, &key_info, NULL);
    }
}

EZOS_CLI_EXPORT("ps_set", "PowerSwitch, param : <index> <payload>", ps_set);

#endif