#include "ezcloud_tsl_airfresh.h"
#include "ezcloud_tsl_storage.h"
#include "ezlog.h"

ez_int32_t airfresh_property_PowerSwitch_get(tsl_prop_impl_t *thiz, const ez_tsl_rsc_t *rsc_info, ez_tsl_value_t *tsl_value)
{
    ez_int32_t rv = -1;
    ez_int32_t _defval = 16;

    property_get_wrapper(thiz, rsc_info, tsl_value, EZ_TSL_DATA_TYPE_BOOL, &_defval);

    rv = EZ_TSL_ERR_SUCC;
    return rv;
}

ez_int32_t airfresh_property_CountdownCfg_get(tsl_prop_impl_t *thiz, const ez_tsl_rsc_t *rsc_info, ez_tsl_value_t *tsl_value)
{
    ez_int32_t rv = -1;
    ez_char_t *_defval = "{\"enable\":false,\"switch\":true,\"timeRemaining\":60}";

    property_get_wrapper(thiz, rsc_info, tsl_value, EZ_TSL_DATA_TYPE_OBJECT, _defval);

    rv = EZ_TSL_ERR_SUCC;
    return rv;
}

ez_int32_t airfresh_property_WindSpeedLevel_get(tsl_prop_impl_t *thiz, const ez_tsl_rsc_t *rsc_info, ez_tsl_value_t *tsl_value)
{
    ez_int32_t rv = -1;
    ez_char_t *_defval = "auto";

    property_get_wrapper(thiz, rsc_info, tsl_value, EZ_TSL_DATA_TYPE_STRING, _defval);

    rv = EZ_TSL_ERR_SUCC;
    return rv;
}

ez_int32_t airfresh_property_WorkMode_get(tsl_prop_impl_t *thiz, const ez_tsl_rsc_t *rsc_info, ez_tsl_value_t *tsl_value)
{
    ez_int32_t rv = -1;
    ez_char_t *_defval = "auto";

    property_get_wrapper(thiz, rsc_info, tsl_value, EZ_TSL_DATA_TYPE_STRING, _defval);

    rv = EZ_TSL_ERR_SUCC;
    return rv;
}

ez_int32_t airfresh_property_PowerSwitch_set(tsl_prop_impl_t *thiz, const ez_tsl_rsc_t *rsc_info, const ez_tsl_value_t *tsl_value)
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

ez_int32_t airfresh_property_CountdownCfg_set(tsl_prop_impl_t *thiz, const ez_tsl_rsc_t *rsc_info, const ez_tsl_value_t *tsl_value)
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

ez_int32_t airfresh_property_WindSpeedLevel_set(tsl_prop_impl_t *thiz, const ez_tsl_rsc_t *rsc_info, const ez_tsl_value_t *tsl_value)
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

ez_int32_t airfresh_property_WorkMode_set(tsl_prop_impl_t *thiz, const ez_tsl_rsc_t *rsc_info, const ez_tsl_value_t *tsl_value)
{
    ez_int32_t rv = EZ_TSL_ERR_SUCC;

    // TODO 执行业务(设置工作模式)
    if (EZ_TSL_ERR_SUCC != rv)
    {
        goto done;
    }

    property_set_wrapper(thiz, rsc_info, tsl_value);
done:
    return rv;
}