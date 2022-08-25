#include <ezlog.h>
#include "ezcloud_tsl_control.h"
#include "ezcloud_tsl_storage.h"

#ifdef CONFIG_EZIOT_COMPONENT_CLI_ENABLE
#include "cli.h"
#include "device_info.h"
#endif

ez_int32_t control_property_Resource_get(tsl_prop_impl_t *thiz, const ez_tsl_rsc_t *rsc_info, ez_tsl_value_t *tsl_value)
{
    ez_int32_t rv = -1;
    ez_char_t *_defval = "[{\"rc\":\"AirCondition\",\"rid\":\"AirCondition\",\"index\":[\"1\",\"2\"]},{\"rc\":\"AirFresh\",\"rid\":\"AirFreshRes\",\"index\":[\"130\"]}]";

    // sample1 两个空调: [{\"rc\":\"AirCondition\",\"rid\":\"AirCondition\",\"index\":[\"1\",\"2\"]}]
    // sample2 一个新风: [{\"rc\":\"AirFresh\",\"rid\":\"AirFreshRes\",\"index\":[\"1\"]}]
    // sample3 两个空调 + 一个新风: [{\"rc\":\"AirCondition\",\"rid\":\"AirCondition\",\"index\":[\"1\",\"2\"]},{\"rc\":\"AirFresh\",\"rid\":\"AirFreshRes\",\"index\":[\"129\"]}]

    property_get_wrapper(thiz, rsc_info, tsl_value, EZ_TSL_DATA_TYPE_ARRAY, _defval);

    rv = EZ_TSL_ERR_SUCC;
    return rv;
}

ez_int32_t control_property_NetStatus_get(tsl_prop_impl_t *thiz, const ez_tsl_rsc_t *rsc_info, ez_tsl_value_t *tsl_value)
{
    ez_int32_t rv = -1;
    ez_char_t *_defval = "{\"type\":\"\",\"address\":\"\",\"mask\":\"\",\"gateway\":\"\",\"signal\":\"\",\"ssid\":\"\"}";

    // TODO 获取信号强度、ip、掩码、ssid

    tsl_value->type = EZ_TSL_DATA_TYPE_OBJECT;
    tsl_value->size = ezos_strlen(_defval);
    ezos_strncpy(tsl_value->value, _defval, ezos_strlen(_defval));

    rv = EZ_TSL_ERR_SUCC;
    return rv;
}

#ifdef CONFIG_EZIOT_COMPONENT_CLI_ENABLE

static void rsc_set(char *buf, int len, int argc, char **argv)
{
    tsl_prop_impl_t ctx = {"Resources", "DynamicResource", "global", "0", NULL, control_property_Resource_get};
    ez_tsl_rsc_t rsc_info = {ctx.res_type, ctx.index};
    ez_tsl_key_t key_info = {ctx.domain, ctx.identify};

    if (argc > 0)
    {
        // 保存至flash
        ez_tsl_value_t tsl_value = {.type = EZ_TSL_DATA_TYPE_ARRAY, .size = ezos_strlen(argv[1]), .value = argv[1]};
        property_set_wrapper(&ctx, &rsc_info, &tsl_value);

        // 上报更新属性
        ez_iot_tsl_property_report(dev_info_get_sn(), &rsc_info, &key_info, NULL);
    }
}

EZOS_CLI_EXPORT("rsc_set", "Dynamic Resource, param : <payload>", rsc_set);

#endif