#include <ezcloud_tsl_storage.h>
#include <hal_config.h>
#include <ezlog.h>

ez_void_t property_get_wrapper(tsl_prop_impl_t *thiz, const ez_tsl_rsc_t *rsc_info, ez_tsl_value_t *tsl_value, ez_tsl_data_type_e value_type, ez_void_t *_defval)
{
    ez_char_t key[64] = {0};
    ezos_snprintf(key, sizeof(key), "%s_%s_%s_%s", rsc_info->res_type, rsc_info->local_index, thiz->domain, thiz->identify);

    switch (value_type)
    {
    case EZ_TSL_DATA_TYPE_BOOL:
    case EZ_TSL_DATA_TYPE_INT:
    {
        tsl_value->size = sizeof(ez_int32_t);
        hal_config_get_int(key, &tsl_value->value_int, *(ez_int32_t *)_defval);
        break;
    }
    case EZ_TSL_DATA_TYPE_DOUBLE:
    {
        tsl_value->size = sizeof(ez_double_t);
        hal_config_get_double(key, &tsl_value->value_double, *(ez_double_t *)_defval);
        break;
    }
    case EZ_TSL_DATA_TYPE_STRING:
    case EZ_TSL_DATA_TYPE_ARRAY:
    case EZ_TSL_DATA_TYPE_OBJECT:
    {
        hal_config_get_string(key, (ez_char_t *)tsl_value->value, (ez_int32_t *)&tsl_value->size, (ez_char_t *)_defval);
        break;
    }
    default:
        break;
    }

    tsl_value->type = value_type;
}

ez_bool_t property_set_wrapper(tsl_prop_impl_t *thiz, const ez_tsl_rsc_t *rsc_info, const ez_tsl_value_t *tsl_value)
{
    ez_bool_t rv = ez_false;
    ez_char_t key[96] = {0};
    ezos_snprintf(key, sizeof(key), "%s_%s_%s_%s", rsc_info->res_type, rsc_info->local_index, thiz->domain, thiz->identify);

    switch (tsl_value->type)
    {
    case EZ_TSL_DATA_TYPE_BOOL:
    case EZ_TSL_DATA_TYPE_INT:
    {
        rv = hal_config_set_int(key, tsl_value->value_int);
        break;
    }
    case EZ_TSL_DATA_TYPE_DOUBLE:
    {
        rv = hal_config_set_double(key, tsl_value->value_double);
        break;
    }
    case EZ_TSL_DATA_TYPE_STRING:
    case EZ_TSL_DATA_TYPE_ARRAY:
    case EZ_TSL_DATA_TYPE_OBJECT:
    {
        rv = hal_config_set_string(key, (const ez_char_t *)tsl_value->value);
        break;
    }
    default:
        break;
    }

    if (!rv)
    {
        ezlog_e(TAG_APP, "Error occurred in saving data, identify:%s", key);
    }

    return rv;
}