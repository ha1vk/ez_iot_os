#include <float.h>
#include <math.h>
#include <limits.h>
#include <ezos.h>
#include "cJSON.h"

#include "ez_iot_tsl.h"
#include "tsl_legality.h"
#include "tsl_adapter.h"
#include "tsl_profile.h"

static ez_err_t check_schema_value(const ez_void_t *schema_dsc, const ez_void_t *tsl_value);

#ifdef CONFIG_EZIOT_TSL_LEGALITY_CHECK_STRONG
static int check_object_value(tsl_schema_desc *schema, ez_tsl_value_t *value);
static int check_array_value(tsl_schema_desc *schema, ez_tsl_value_t *value);
static int check_string_value(tsl_schema_desc *schema, ez_tsl_value_t *value);
static int check_int_value(tsl_schema_desc *schema, ez_tsl_value_t *value);
static int check_double_value(tsl_schema_desc *schema, ez_tsl_value_t *value);
#endif

ez_err_t tsl_legality_property_check(const ez_char_t *sn, const ez_tsl_rsc_t *rsc_info, const ez_tsl_key_t *key_info, const ez_tsl_value_t *value)
{
    ez_err_t rv = EZ_TSL_ERR_SUCC;
    tsl_domain_prop *prop = NULL;
    size_t i = 0;
    size_t j = 0;
    size_t k = 0;

    const tsl_capacity_t *capacity = tsl_profile_get_lock((ez_char_t *)sn);
    CHECK_COND_DONE(!capacity, EZ_TSL_ERR_DEV_NOT_FOUND);

    for (i = 0; i < capacity->rsc_num; i++)
    {
        if (0 == ezos_strcmp(rsc_info->res_type, capacity->resource[i].rsc_category))
        {
            break;
        }
    }

    CHECK_COND_DONE(i == capacity->rsc_num, EZ_TSL_ERR_DEV_NOT_FOUND);

    for (j = 0; j < capacity->resource[i].index_num; j++)
    {
        if (0 == ezos_strcmp(rsc_info->local_index, capacity->resource[i].index))
        {
            break;
        }
    }

    CHECK_COND_DONE(j == capacity->resource[i].index_num, EZ_TSL_ERR_INDEX_NOT_FOUND);

    for (j = 0; j < capacity->resource[i].domain_num; j++)
    {
        if (0 == ezos_strcmp(capacity->resource[i].domain[j].identifier, (char *)key_info->domain))
        {
            break;
        }
    }

    CHECK_COND_DONE(j == capacity->resource[i].domain_num, EZ_TSL_ERR_DOMAIN_NOT_FOUND);

    for (k = 0; k < capacity->resource[i].domain[j].prop_num; k++)
    {
        tsl_domain_prop *prop_temp = capacity->resource[i].domain[j].prop + k;
        if (0 == ezos_strcmp(prop_temp->identifier, (char *)key_info->key))
        {
            prop = prop_temp;
            break;
        }
    }

    CHECK_COND_DONE(k == capacity->resource[i].domain[j].prop_num, EZ_TSL_ERR_KEY_NOT_FOUND);

#ifdef CONFIG_EZIOT_TSL_LEGALITY_CHECK_STRONG
    if (value)
    {
        rv = check_schema_value(&prop->prop_desc, value);
    }
#endif

done:
    if (capacity)
    {
        tsl_profile_get_unlock();
    }

    return rv;
}

ez_err_t tsl_legality_event_check(const ez_char_t *sn, const ez_tsl_rsc_t *rsc_info, const ez_tsl_key_t *key_info, const ez_tsl_value_t *value)
{
    ez_err_t rv = EZ_TSL_ERR_SUCC;
    tsl_domain_event *event = NULL;
    size_t i = 0;
    size_t j = 0;
    size_t k = 0;

    const tsl_capacity_t *capacity = tsl_profile_get_lock((ez_char_t*)sn);
    CHECK_COND_DONE(!capacity, EZ_TSL_ERR_DEV_NOT_FOUND);

    for (i = 0; i < capacity->rsc_num; i++)
    {
        if (0 == ezos_strcmp(rsc_info->res_type, capacity->resource[i].rsc_category))
        {
            break;
        }
    }

    CHECK_COND_DONE(i == capacity->rsc_num, EZ_TSL_ERR_DEV_NOT_FOUND);

    for (j = 0; j < capacity->resource[i].index_num; j++)
    {
        if (0 == ezos_strcmp(rsc_info->local_index, capacity->resource[i].index))
        {
            break;
        }
    }

    CHECK_COND_DONE(j == capacity->resource[i].index_num, EZ_TSL_ERR_INDEX_NOT_FOUND);

    for (j = 0; j < capacity->resource[i].domain_num; j++)
    {
        if (0 == ezos_strcmp(capacity->resource[i].domain[j].identifier, (char *)key_info->domain))
        {
            break;
        }
    }

    CHECK_COND_DONE(j == capacity->resource[i].domain_num, EZ_TSL_ERR_DOMAIN_NOT_FOUND);

    for (k = 0; k < capacity->resource[i].domain[j].event_num; k++)
    {
        tsl_domain_event *event_temp = capacity->resource[i].domain[j].event + k;

        if (0 == ezos_strcmp(event_temp->identifier, (char *)key_info->key))
        {
            event = event_temp;
            break;
        }
    }

    CHECK_COND_DONE(k == capacity->resource[i].domain[j].event_num, EZ_TSL_ERR_KEY_NOT_FOUND);

#ifdef CONFIG_EZIOT_TSL_LEGALITY_CHECK_STRONG
    if (value)
    {
        rv = check_schema_value(&event->input_schema, value);
    }
#endif

done:
    if (capacity)
    {
        tsl_profile_get_unlock();
    }

    return rv;
}

static ez_err_t check_schema_value(const ez_void_t *schema_dsc, const ez_void_t *tsl_value)
{
    ez_err_t ret = EZ_TSL_ERR_SUCC;

#ifdef CONFIG_EZIOT_TSL_LEGALITY_CHECK_STRONG
    tsl_schema_desc *schema = (tsl_schema_desc *)schema_dsc;
    ez_tsl_value_t *value = (ez_tsl_value_t *)tsl_value;

    if (schema->item_type != value->type)
    {
        if (schema->item_type == EZ_TSL_DATA_TYPE_DOUBLE && value->type == EZ_TSL_DATA_TYPE_INT)
        {
            ez_tsl_value_t tsl_value = {0};
            tsl_value.type = EZ_TSL_DATA_TYPE_DOUBLE;
            tsl_value.value_double = (double)value->value_int;
            tsl_value.size = sizeof(double);
            return check_double_value(schema, &tsl_value);
        }
        else
        {
            ezlog_e(TAG_TSL, "type not match. schema type: %d, value type: %d", schema->item_type, value->type);
            return EZ_TSL_ERR_VALUE_TYPE;
        }
    }

    switch (value->type)
    {
    case EZ_TSL_DATA_TYPE_BOOL:

        break;
    case EZ_TSL_DATA_TYPE_INT:
        ret = check_int_value(schema, value);
        break;
    case EZ_TSL_DATA_TYPE_DOUBLE:
        ret = check_double_value(schema, value);
        break;
    case EZ_TSL_DATA_TYPE_STRING:
        ret = check_string_value(schema, value);
        break;
    case EZ_TSL_DATA_TYPE_ARRAY:
        ret = check_array_value(schema, value);
        break;
    case EZ_TSL_DATA_TYPE_OBJECT:
        ret = check_object_value(schema, value);
        break;

    default:
        break;
    }
#endif //CONFIG_EZIOT_TSL_LEGALITY_CHECK_STRONG

    return ret;
}

#ifdef CONFIG_EZIOT_TSL_LEGALITY_CHECK_STRONG
static int check_int_value(tsl_schema_desc *schema, ez_tsl_value_t *value)
{
    int ret = 0;

    do
    {
        if (schema->type_integer.enum_num != 0)
        {
            ez_bool_t is_hit = ez_false;
            for (size_t i = 0; i < schema->type_integer.enum_num; i++)
            {
                if (value->value_int == *(schema->type_integer.int_enum + i))
                {
                    ezlog_d(TAG_TSL, "int value match: %d -- %d", value->value_int, *(schema->type_integer.int_enum + i));
                    is_hit = ez_true;
                    ret = 0;
                    break;
                }
            }
            if (!is_hit)
            {
                ezlog_e(TAG_TSL, "enum not match.");
                ret = EZ_TSL_ERR_VALUE_ILLEGAL;
            }
            break;
        }

        if (schema->type_integer.multiple != 0)
        {
            if (0 != value->value_int % schema->type_integer.multiple)
            {
                ezlog_e(TAG_TSL, "multiple not match.");
                ret = EZ_TSL_ERR_VALUE_ILLEGAL;
                break;
            }
        }

        if (0 != schema->type_integer.maximum && 0 != schema->type_integer.minimum && schema->type_integer.maximum >= schema->type_integer.minimum)
        {
            if (schema->type_integer.maximum >= value->value_int && value->value_int >= schema->type_integer.minimum)
            {
                ezlog_d(TAG_TSL, "int value legal.");
                ret = 0;
                break;
            }
            else
            {
                ezlog_e(TAG_TSL, "int value illegal.min: %d, cur: %d, max: %d", schema->type_integer.minimum, value->value_int, schema->type_integer.maximum);
                ret = EZ_TSL_ERR_VALUE_ILLEGAL;
                break;
            }
        }

        if (0 != schema->type_integer.exmaximum && 0 != schema->type_integer.exminimum && schema->type_integer.exmaximum > schema->type_integer.exminimum)
        {
            if (schema->type_integer.exmaximum > value->value_int && value->value_int > schema->type_integer.exminimum)
            {
                ezlog_d(TAG_TSL, "int value legal.");
                ret = 0;
                break;
            }
            else
            {
                ezlog_e(TAG_TSL, "int value illegal.exmin: %d, cur: %d, exmax: %d", schema->type_integer.exminimum, value->value_int, schema->type_integer.exmaximum);
                ret = EZ_TSL_ERR_VALUE_ILLEGAL;
                break;
            }
        }
    } while (ez_false);

    return ret;
}

static int check_double_value(tsl_schema_desc *schema, ez_tsl_value_t *value)
{
    int ret = 0;
    do
    {
        if (schema->type_number.enum_num != 0)
        {
            ez_bool_t is_hit = ez_false;
            for (size_t i = 0; i < schema->type_number.enum_num; i++)
            {
                if (value->value_double == *(schema->type_number.num_enum + i))
                {
                    ezlog_d(TAG_TSL, "double value match: %f -- %f", value->value_double, *(schema->type_number.num_enum + i));
                    is_hit = ez_true;
                    ret = 0;
                    break;
                }
            }
            if (!is_hit)
            {
                ezlog_e(TAG_TSL, "enum not match.");
                ret = EZ_TSL_ERR_VALUE_ILLEGAL;
            }
            break;
        }

        if (schema->type_number.multiple != 0)
        {
            if (0 != (value->value_double / schema->type_number.multiple - (int)(value->value_double / schema->type_number.multiple)))
            {
                ezlog_e(TAG_TSL, "multiple not match.");
                ret = EZ_TSL_ERR_VALUE_ILLEGAL;
                break;
            }
        }

        if (0 != schema->type_number.maximum && 0 != schema->type_number.minimum && schema->type_number.maximum >= schema->type_number.minimum)
        {
            if (schema->type_number.maximum >= value->value_double && value->value_double >= schema->type_number.minimum)
            {
                ezlog_d(TAG_TSL, "double value legal.");
                ret = 0;
                break;
            }
            else
            {
                ezlog_e(TAG_TSL, "double value illegal.min: %f, cur: %f, max: %f", schema->type_number.minimum, value->value_double, schema->type_number.maximum);
                ret = EZ_TSL_ERR_VALUE_ILLEGAL;
                break;
            }
        }

        if (0 != schema->type_number.exmaximum && 0 != schema->type_number.exminimum && schema->type_number.exmaximum > schema->type_number.exminimum)
        {
            if (schema->type_number.exmaximum > value->value_double && value->value_double > schema->type_number.exminimum)
            {
                ezlog_d(TAG_TSL, "double value legal.");
                ret = 0;
                break;
            }
            else
            {
                ezlog_e(TAG_TSL, "double value illegal.exmin: %f, cur: %f, exmax: %f", schema->type_number.exminimum, value->value_int, schema->type_number.exmaximum);
                ret = EZ_TSL_ERR_VALUE_ILLEGAL;
                break;
            }
        }
    } while (ez_false);

    return ret;
}

static int check_string_value(tsl_schema_desc *schema, ez_tsl_value_t *value)
{
    int ret = 0;
    do
    {
        if (schema->type_string.enum_num != 0)
        {
            ez_bool_t is_hit = ez_false;
            for (size_t i = 0; i < schema->type_string.enum_num; i++)
            {
                if (0 == ezos_strcmp((char *)value->value, schema->type_string.str_enum + MAX_STRING_ENUM_LENGTH * i))
                {
                    ezlog_d(TAG_TSL, "string value match: %s -- %s", (char *)value->value, schema->type_string.str_enum + MAX_STRING_ENUM_LENGTH * i);
                    is_hit = ez_true;
                    ret = 0;
                    break;
                }
            }
            if (!is_hit)
            {
                ezlog_e(TAG_TSL, "enum not match.");
                ret = EZ_TSL_ERR_VALUE_ILLEGAL;
            }
            break;
        }

        int len = ezos_strlen((char *)value->value);
        if (0 != schema->type_string.min_len && 0 != schema->type_string.max_len && schema->type_string.min_len <= schema->type_string.max_len)
        {
            if (schema->type_string.min_len <= len && len <= schema->type_string.max_len)
            {
                ezlog_d(TAG_TSL, "string value len legal.");
                ret = 0;
                break;
            }
            else
            {
                ezlog_e(TAG_TSL, "string value len illegal. min_len: %d, cur_len: %d, max_len: %d", schema->type_string.min_len, len, schema->type_string.max_len);
                ret = EZ_TSL_ERR_VALUE_ILLEGAL;
                break;
            }
        }
    } while (ez_false);

    return ret;
}

static int tranform_value_from_json(ez_tsl_value_t *prop_value, cJSON *js_prop, char **str_msg)
{
    int ret = 0;

    switch (js_prop->type)
    {
    case cJSON_False:
        prop_value->value_bool = ez_false;
        prop_value->size = sizeof(ez_bool_t);
        prop_value->type = EZ_TSL_DATA_TYPE_BOOL;
        break;
    case cJSON_True:
        prop_value->value_bool = ez_true;
        prop_value->size = sizeof(ez_bool_t);
        prop_value->type = EZ_TSL_DATA_TYPE_BOOL;
        break;
    case cJSON_Number:
    {
        ezlog_d(TAG_TSL, "js prop int: %d, js prop double: %f", js_prop->valueint, js_prop->valuedouble);
        if (DBL_EPSILON >= fabs(js_prop->valuedouble - js_prop->valueint || js_prop->valueint == INT_MAX || js_prop->valueint == INT_MIN))
        {
            prop_value->value_int = js_prop->valueint;
            prop_value->size = sizeof(int);
            prop_value->type = EZ_TSL_DATA_TYPE_INT;
        }
        else
        {
            prop_value->value_double = js_prop->valuedouble;
            prop_value->size = sizeof(double);
            prop_value->type = EZ_TSL_DATA_TYPE_DOUBLE;
        }
        ezlog_d(TAG_TSL, "prop int: %d, prop double: %f", prop_value->value_int, prop_value->value_double);
    }
    break;
    case cJSON_String:
        prop_value->size = ezos_strlen(js_prop->valuestring);
        prop_value->type = EZ_TSL_DATA_TYPE_STRING;
        *str_msg = (char *)ezos_malloc(prop_value->size + 1);
        if (NULL == *str_msg)
        {
            ezlog_e(TAG_TSL, "memory not enough.");
            ret = -1;
            break;
        }
        ezos_strcpy(*str_msg, js_prop->valuestring);
        prop_value->value = *str_msg;
        break;
    case cJSON_Array:
        *str_msg = cJSON_PrintUnformatted(js_prop);
        if (NULL != *str_msg)
        {
            prop_value->size = ezos_strlen(*str_msg);
            prop_value->type = EZ_TSL_DATA_TYPE_ARRAY;
            prop_value->value = *str_msg;
        }
        else
        {
            ret = -1;
        }

        break;
    case cJSON_Object:
        *str_msg = cJSON_PrintUnformatted(js_prop);
        if (NULL != *str_msg)
        {
            prop_value->size = ezos_strlen(*str_msg);
            prop_value->type = EZ_TSL_DATA_TYPE_OBJECT;
            prop_value->value = *str_msg;
        }
        else
        {
            ret = -1;
        }
        break;

    default:
        break;
    }
    if (NULL != *str_msg)
    {
        ezlog_d(TAG_TSL, "str_msg: %s", *str_msg);
    }

    return ret;
}

static int check_array_value(tsl_schema_desc *schema, ez_tsl_value_t *value)
{
    int ret = 0;
    cJSON *js_root = NULL;
    do
    {
        js_root = cJSON_Parse((char *)value->value);
        if (NULL == js_root || cJSON_Array != js_root->type)
        {
            ezlog_e(TAG_TSL, "array value parse failed.");
            ret = EZ_TSL_ERR_VALUE_ILLEGAL;
            break;
        }
        int arr_num = cJSON_GetArraySize(js_root);
        if (0 != schema->type_array.minItem && 0 != schema->type_array.maxItem && schema->type_array.minItem <= schema->type_array.maxItem)
        {
            if (schema->type_array.minItem < arr_num || schema->type_object.max_props < arr_num)
            {
                ezlog_e(TAG_TSL, "array value props not match. min: %d, cur: %d, max: %d", schema->type_array.minItem, arr_num, schema->type_array.maxItem);
                ret = EZ_TSL_ERR_VALUE_ILLEGAL;
                break;
            }
        }

        cJSON *js_prop = NULL;
        cJSON_ArrayForEach(js_prop, js_root)
        {
            ez_tsl_value_t prop_value = {0};
            char *str_msg = NULL;
            if (0 != tranform_value_from_json(&prop_value, js_prop, &str_msg))
            {
                if (NULL != str_msg)
                {
                    ezos_free(str_msg);
                }
                ret = EZ_TSL_ERR_VALUE_ILLEGAL;
                break;
            }

            for (size_t i = 0; i < schema->type_array.prop_num; i++)
            {
                tsl_schema_desc *tsl_schema = (tsl_schema_desc *)(schema->type_array.item_prop + sizeof(tsl_schema_desc) * i);
                ret = check_schema_value(tsl_schema, &prop_value);
            }

            if (NULL != str_msg)
            {
                ezos_free(str_msg);
            }

            if (0 != ret)
            {
                break;
            }
        }

    } while (ez_false);
    if (NULL != js_root)
    {
        cJSON_Delete(js_root);
    }

    return ret;
}

static int check_object_value(tsl_schema_desc *schema, ez_tsl_value_t *value)
{
    int ret = 0;
    cJSON *js_root = NULL;
    do
    {
        js_root = cJSON_Parse((char *)value->value);
        if (NULL == js_root)
        {
            ezlog_e(TAG_TSL, "object value parse failed.");
            ret = EZ_TSL_ERR_VALUE_ILLEGAL;
            break;
        }

        int obj_num = cJSON_GetArraySize(js_root);
        if (0 != schema->type_object.min_props && 0 != schema->type_object.max_props && schema->type_object.min_props <= schema->type_object.max_props)
        {
            if (schema->type_object.min_props > obj_num || schema->type_object.max_props < obj_num)
            {
                ezlog_e(TAG_TSL, "object value props not match. min: %d, cur: %d, max: %d", schema->type_object.min_props, obj_num, schema->type_object.max_props);
                ret = EZ_TSL_ERR_VALUE_ILLEGAL;
                break;
            }
        }

        if (0 != schema->type_object.req_num)
        {
            for (size_t i = 0; i < schema->type_object.req_num; i++)
            {
                cJSON *js_require = cJSON_GetObjectItem(js_root, schema->type_object.required + MAX_ARR_REQUIRE_LENGTH * i);
                if (NULL == js_require)
                {
                    ezlog_e(TAG_TSL, "object require absent: %s", schema->type_object.required + MAX_ARR_REQUIRE_LENGTH * i);
                    ret = EZ_TSL_ERR_VALUE_ILLEGAL;
                    break;
                }
            }
        }
        if (0 != ret)
        {
            break;
        }

        cJSON *js_prop = NULL;
        cJSON_ArrayForEach(js_prop, js_root)
        {
            ez_tsl_value_t prop_value = {0};
            char *str_msg = NULL;
            if (0 != tranform_value_from_json(&prop_value, js_prop, &str_msg))
            {
                if (NULL != str_msg)
                {
                    ezos_free(str_msg);
                }
                ret = EZ_TSL_ERR_VALUE_ILLEGAL;
                break;
            }
            ez_bool_t is_hit = ez_false;
            for (size_t i = 0; i < schema->type_object.prop_num; i++)
            {
                tsl_schema_desc *tsl_schema = (tsl_schema_desc *)(schema->type_object.property + sizeof(tsl_schema_desc) * i);
                if (0 == ezos_strcmp(tsl_schema->prop_key, js_prop->string))
                {
                    is_hit = ez_true;
                    ret = check_schema_value(tsl_schema, &prop_value);
                    break;
                }
            }
            if (NULL != str_msg)
            {
                ezos_free(str_msg);
            }
            if (!is_hit)
            {
                ezlog_e(TAG_TSL, "identifier not match: %s", js_prop->string);
                ret = EZ_TSL_ERR_VALUE_ILLEGAL;
            }
            if (0 != ret)
            {
                break;
            }
        }
        if (0 != ret)
        {
            break;
        }

    } while (ez_false);

    if (NULL != js_root)
    {
        cJSON_Delete(js_root);
    }

    return ret;
}

#endif //CONFIG_EZIOT_TSL_LEGALITY_CHECK_STRONG