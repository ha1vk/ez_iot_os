/*******************************************************************************
 * Copyright © 2017-2021 Ezviz Inc.
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
 * Chentengfei (chentengfei5@ezvizlife.com)
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-11-15     chentengfei  first version
 * 2022-12-06     xurongjun    
 *******************************************************************************/

#include <ezos.h>
#include <cJSON.h>
#include "ezlist.h"

#include "tsl_def.h"
#include "tsl_legality.h"
#include "tsl_adapter.h"

const char *tsl_key_version = "version";
const char *tsl_key_resources = "resources";
const char *tsl_key_identifier = "identifier";
const char *tsl_key_rsc_category = "resourceCategory";
const char *tsl_key_local_index = "localIndex";
const char *tsl_key_domains = "domains";
const char *tsl_key_props = "props";
const char *tsl_key_actions = "actions";
const char *tsl_key_events = "events";
const char *tsl_key_access = "access";
const char *tsl_key_schema = "schema";
const char *tsl_key_direction = "direction";
const char *tsl_key_input = "input";
const char *tsl_key_output = "output";
const char *tsl_key_eventtype = "eventType";
const char *tsl_key_type = "type";
const char *tsl_key_data_string = "str";
const char *tsl_key_data_boolean = "bool";
const char *tsl_key_data_integer = "int";
const char *tsl_key_data_number = "num";
const char *tsl_key_data_array = "arr";
const char *tsl_key_data_object = "obj";
const char *tsl_key_data_key = "key";
const char *tsl_key_enum = "enum";
const char *tsl_key_minlength = "mnl";
const char *tsl_key_maxlength = "mxl";
const char *tsl_key_minimum = "mn";
const char *tsl_key_exclusive_minimum = "emn";
const char *tsl_key_maximum = "mx";
const char *tsl_key_exclusive_maximum = "emx";
const char *tsl_key_multiple = "multi";
const char *tsl_key_items = "itm";
const char *tsl_key_min_items = "mnitm";
const char *tsl_key_max_items = "mxitm";
const char *tsl_key_unique_items = "unq";
const char *tsl_key_required = "req";
const char *tsl_key_max_properties = "mnp";
const char *tsl_key_min_properties = "mxp";
const char *tsl_key_dependencies = "dep";
const char *tsl_key_schema_properties = "prop";

typedef struct
{
    ez_node_t node;
    ez_char_t dev_sn[32];
} node_dev_t;

typedef struct
{
    ez_node_t node;
    tsl_capacity_t capacity;
    ez_list_t dev_list;
} node_capacity_t;

/* profile parse */
static ez_err_t profile_parse(char *profile_value, tsl_capacity_t *capacity);
static ez_err_t profile_parse_domain(tsl_rsc_domain *pdomain, ez_int32_t domain_num, cJSON *js_domains);
static ez_err_t profile_parse_domain_props(tsl_domain_prop *p_props, ez_int32_t props_num, cJSON *js_props);
static ez_err_t profile_parse_domain_actions(tsl_domain_action *p_actions, ez_int32_t actions_num, cJSON *js_actions);
static ez_err_t profile_parse_domain_events(tsl_domain_event *p_events, ez_int32_t events_num, cJSON *js_events);
static ez_err_t profile_parse_schema(tsl_schema_desc *schema, cJSON *js_schema);

/* profile free */
static ez_void_t profile_free(tsl_capacity_t *capacity);
static ez_void_t profile_free_domain(tsl_rsc_domain *tsl_domain, ez_int32_t domain_num);
static ez_void_t profile_free_props(tsl_domain_prop *tsl_props, ez_int32_t prop_num);
static ez_void_t profile_free_actions(tsl_domain_action *tsl_actions, ez_int32_t action_num);
static ez_void_t profile_free_events(tsl_domain_event *tsl_events, ez_int32_t event_num);
static ez_void_t profile_free_schema(tsl_schema_desc *tsl_schema);

/* profile manager */
static ez_mutex_t g_mutex = NULL;
static ez_list_t g_profile_list;
static ez_bool_t profile_mutex_init();
static ez_void_t profile_mutex_lock();
static ez_void_t profile_mutex_unlock();
static ez_void_t profile_mutex_deinit();

static ez_void_t profile_list_free(ez_void_t);
static node_capacity_t *profile_find_by_sn(ez_char_t *dev_sn);
static ez_int32_t profile_ref_add(node_capacity_t *node_capacity, ez_char_t *dev_sn);
static ez_int32_t profile_ref_del(node_capacity_t *node_capacity, ez_char_t *dev_sn);

ez_err_t tsl_profile_init(ez_void_t)
{
    if (!profile_mutex_init())
    {
        return EZ_TSL_ERR_MEMORY;
    }

    ezlist_init(&g_profile_list);

    return EZ_TSL_ERR_SUCC;
}

ez_err_t tsl_profile_deinit(ez_void_t)
{
    profile_mutex_lock();
    profile_list_free();
    profile_mutex_unlock();
    profile_mutex_deinit();

    return EZ_TSL_ERR_SUCC;
}

ez_bool_t tsl_profile_check(ez_char_t *dev_sn)
{
    ez_bool_t rv = ez_false;

    profile_mutex_lock();

    if (profile_find_by_sn(dev_sn))
    {
        rv = ez_true;
    }

    profile_mutex_unlock();

    return rv;
}

ez_err_t tsl_profile_reg(ez_char_t *dev_sn, ez_char_t *dev_type, ez_char_t *dev_fwver, ez_char_t *profile)
{
    ez_err_t rv = EZ_TSL_ERR_SUCC;
    ez_int32_t count = 0;
    node_capacity_t *pnode_capacity = (node_capacity_t *)ezos_malloc(sizeof(node_capacity_t));
    ezos_bzero(pnode_capacity, sizeof(node_capacity_t));

    CHECK_COND_DONE(!pnode_capacity, EZ_TSL_ERR_MEMORY);

    rv = profile_parse(profile, &pnode_capacity->capacity);
    CHECK_RV_DONE(rv);

    ezos_strncpy(pnode_capacity->capacity.dev_type, dev_type, sizeof(pnode_capacity->capacity.dev_type) - 1);
    ezos_strncpy(pnode_capacity->capacity.dev_fw_ver, dev_type, sizeof(pnode_capacity->capacity.dev_fw_ver) - 1);
    count = profile_ref_add(pnode_capacity, dev_sn);
    CHECK_COND_DONE(count < 1, EZ_TSL_ERR_MEMORY);

    profile_mutex_lock();
    ezlist_add_last(&g_profile_list, &pnode_capacity->node);
    pnode_capacity = NULL;
    profile_mutex_unlock();

done:
    SAFE_FREE(pnode_capacity);

    return rv;
}

ez_bool_t tsl_profile_ref_add(ez_char_t *dev_sn, ez_char_t *dev_type, ez_char_t *dev_fwver)
{
    ez_bool_t rv = ez_false;
    node_capacity_t *pnode_capacity = NULL;

    profile_mutex_lock();

    LIST_FOR_EACH(node_capacity_t, pnode_capacity, &g_profile_list)
    {
        if (0 == ezos_strcmp(dev_type, pnode_capacity->capacity.dev_type) &&
            0 == ezos_strcmp(dev_fwver, pnode_capacity->capacity.dev_fw_ver))
        {
            profile_ref_add(pnode_capacity, dev_sn);
            rv = ez_true;
            break;
        }
    }

    profile_mutex_unlock();

    return rv;
}

ez_void_t tsl_profile_ref_del(ez_char_t *dev_sn)
{
    node_capacity_t *pnode_capacity = NULL;
    ez_int32_t count = 0;

    profile_mutex_lock();

    pnode_capacity = profile_find_by_sn(dev_sn);
    count = profile_ref_del(pnode_capacity, dev_sn);
    if (count < 1)
    {
        ezlog_w(TAG_TSL, "ref = 0, free capacity");
        ezlist_delete(&g_profile_list, &pnode_capacity->node);
        ezlist_clear(&pnode_capacity->dev_list);
        profile_free(&pnode_capacity->capacity);
        ezos_free(pnode_capacity);
    }

    profile_mutex_unlock();
}

const tsl_capacity_t *tsl_profile_get_lock(ez_char_t *dev_sn)
{
    node_capacity_t *pnode_capacity = NULL;

    profile_mutex_lock();

    pnode_capacity = profile_find_by_sn(dev_sn);
    if (!pnode_capacity)
    {
        profile_mutex_unlock();
        return NULL;
    }

    return &pnode_capacity->capacity;
}

ez_void_t tsl_profile_get_unlock(ez_void_t)
{
    profile_mutex_unlock();
}

static ez_bool_t profile_mutex_init()
{
    g_mutex = ezos_mutex_create();
    if (NULL == g_mutex)
    {
        return ez_false;
    }

    return ez_true;
}

static ez_void_t profile_mutex_lock()
{
    if (NULL == g_mutex)
    {
        return;
    }

    ezos_mutex_lock(g_mutex);
}

static ez_void_t profile_mutex_unlock()
{
    if (NULL == g_mutex)
    {
        return;
    }

    ezos_mutex_unlock(g_mutex);
}

static ez_void_t profile_mutex_deinit()
{
    if (NULL == g_mutex)
    {
        return;
    }

    ezos_mutex_destroy(g_mutex);
    g_mutex = NULL;
}

static ez_void_t profile_list_free(ez_void_t)
{
    node_capacity_t *node_capacity = NULL;

    LIST_FOR_EACH(node_capacity_t, node_capacity, &g_profile_list)
    {
        ezlist_clear(&node_capacity->dev_list);
        profile_free(&node_capacity->capacity);
        ezos_free(node_capacity);
    }

    ezlist_clear(&g_profile_list);
}

static ez_int32_t profile_ref_add(node_capacity_t *node_capacity, ez_char_t *dev_sn)
{
    node_dev_t *node_dev = (node_dev_t *)ezos_malloc(sizeof(node_dev_t));
    if (NULL == node_dev)
    {
        return -1;
    }

    ezos_bzero(node_dev, sizeof(node_dev_t));
    ezos_strncpy(node_dev->dev_sn, dev_sn, sizeof(node_dev->dev_sn) - 1);
    ezlist_add_last(&node_capacity->dev_list, &node_dev->node);

    return ezlist_get_size(&node_capacity->dev_list);
}

static ez_int32_t profile_ref_del(node_capacity_t *node_capacity, ez_char_t *dev_sn)
{
    node_dev_t *node_dev = NULL;

    LIST_FOR_EACH(node_dev_t, node_dev, &node_capacity->dev_list)
    {
        if (0 == ezos_strcmp(dev_sn, node_dev->dev_sn))
        {
            ezlist_delete(&node_capacity->dev_list, &node_dev->node);
            ezos_free(node_dev);
            break;
        }
    }

    return ezlist_get_size(&node_capacity->dev_list);
}

#ifndef COMPONENT_TSL_PROFILE_STRIP
static ez_void_t profile_free_schema(tsl_schema_desc *tsl_schema)
{
    switch (tsl_schema->item_type)
    {
    case EZ_TSL_DATA_TYPE_BOOL:
        break;
    case EZ_TSL_DATA_TYPE_DOUBLE:
    {
        if (NULL != tsl_schema->type_number.num_enum)
        {
            ezos_free(tsl_schema->type_number.num_enum);
            tsl_schema->type_number.num_enum = NULL;
        }
    }
    break;
    case EZ_TSL_DATA_TYPE_INT:
    {
        if (NULL != tsl_schema->type_integer.int_enum)
        {
            ezos_free(tsl_schema->type_integer.int_enum);
            tsl_schema->type_integer.int_enum = NULL;
        }
    }
    break;
    case EZ_TSL_DATA_TYPE_STRING:
    {
        if (NULL != tsl_schema->type_string.str_enum)
        {
            ezos_free(tsl_schema->type_string.str_enum);
            tsl_schema->type_string.str_enum = NULL;
        }
    }
    break;
    case EZ_TSL_DATA_TYPE_ARRAY:
    {
        if (NULL != tsl_schema->type_array.item_prop)
        {
            ez_int32_t index = tsl_schema->type_array.prop_num;
            for (int i = 0; i < index; i++)
            {
                profile_free_schema(tsl_schema->type_array.item_prop + i);
            }
            ezos_free(tsl_schema->type_array.item_prop);
            tsl_schema->type_array.item_prop = NULL;
        }
    }
    break;
    case EZ_TSL_DATA_TYPE_OBJECT:
    {
        if (NULL != tsl_schema->type_object.required)
        {
            ezos_free(tsl_schema->type_object.required);
            tsl_schema->type_object.required = NULL;
        }
        if (NULL != tsl_schema->type_object.property)
        {
            ez_int32_t index = tsl_schema->type_object.prop_num;
            for (int i = 0; i < index; i++)
            {
                profile_free_schema(tsl_schema->type_object.property + i);
            }
            ezos_free(tsl_schema->type_object.property);
            tsl_schema->type_object.property = NULL;
        }
    }
    break;

    default:
        break;
    }
}

static ez_void_t profile_free_props(tsl_domain_prop *tsl_props, ez_int32_t prop_num)
{
    ez_int32_t index = 0;
    for (index = 0; index < prop_num; index++)
    {
        tsl_domain_prop *prop = tsl_props + index;
        profile_free_schema(&prop->prop_desc);
    }
}

static ez_void_t profile_free_actions(tsl_domain_action *tsl_actions, ez_int32_t action_num)
{
    ez_int32_t index = 0;
    for (index = 0; index < action_num; index++)
    {
        tsl_domain_action *action = tsl_actions + index;
        profile_free_schema(&action->input_schema);
        profile_free_schema(&action->output_schema);
    }
}

static ez_void_t profile_free_events(tsl_domain_event *tsl_events, ez_int32_t event_num)
{
    ez_int32_t index = 0;
    if (NULL != tsl_events->event_type)
    {
        ezos_free(tsl_events->event_type);
        tsl_events->event_type = NULL;
    }

    for (index = 0; index < event_num; index++)
    {
        tsl_domain_event *event = tsl_events + index;
        profile_free_schema(&event->input_schema);
        profile_free_schema(&event->output_schema);
    }
}
#endif

static ez_void_t profile_free_domain(tsl_rsc_domain *tsl_domain, ez_int32_t domain_num)
{
    ez_int32_t index = 0;
    for (index = 0; index < domain_num; index++)
    {
        tsl_rsc_domain *domain = tsl_domain + index;
        if (NULL != domain->prop)
        {
#ifndef COMPONENT_TSL_PROFILE_STRIP
            profile_free_props(domain->prop, domain->prop_num);
#endif
            ezos_free(domain->prop);
            domain->prop = NULL;
        }

        if (NULL != domain->action)
        {
#ifndef COMPONENT_TSL_PROFILE_STRIP
            profile_free_actions(domain->action, domain->action_num);
#endif
            ezos_free(domain->action);
            domain->action = NULL;
        }

        if (NULL != domain->event)
        {
#ifndef COMPONENT_TSL_PROFILE_STRIP
            profile_free_events(domain->event, domain->event_num);
#endif
            ezos_free(domain->event);
            domain->event = NULL;
        }
    }
}

static ez_void_t profile_free(tsl_capacity_t *capacity)
{
    ez_int32_t rsc_num = capacity->rsc_num;
    for (int i = 0; i < rsc_num; i++)
    {
        tsl_profile_resource *tsl_rsc = capacity->resource + i;
        if (NULL != tsl_rsc)
        {
            if (NULL != tsl_rsc->index)
            {
                ezos_free(tsl_rsc->index);
                tsl_rsc->index = NULL;
            }

            if (NULL != tsl_rsc->domain)
            {
                profile_free_domain(tsl_rsc->domain, tsl_rsc->domain_num);
                ezos_free(tsl_rsc->domain);
                tsl_rsc->domain = NULL;
            }
        }
    }

    if (NULL != capacity->resource)
    {
        ezos_free(capacity->resource);
        capacity->resource = NULL;
    }
}

#ifndef COMPONENT_TSL_PROFILE_STRIP
static ez_int32_t profile_parse_schema(tsl_schema_desc *schema, cJSON *js_schema)
{
    ez_int32_t rv = EZ_TSL_ERR_SUCC;

    if (NULL == schema || NULL == js_schema)
    {
        ezlog_e(TAG_TSL, "parse schema param error.");
        return EZ_TSL_ERR_PARAM_INVALID;
    }

    ezlog_v(TAG_TSL, "&&&&&&&&&&&&&&&& schema &&&&&&&&&&");
    do
    {
        cJSON *js_type = cJSON_GetObjectItem(js_schema, tsl_key_type);
        if (NULL == js_type || cJSON_String != js_type->type)
        {
            ezlog_e(TAG_TSL, "schema type absent.");
            break;
        }
        if (0 == ezos_strcmp(js_type->valuestring, tsl_key_data_string))
        {
            schema->item_type = EZ_TSL_DATA_TYPE_STRING;
        }
        else if (0 == ezos_strcmp(js_type->valuestring, tsl_key_data_boolean))
        {
            schema->item_type = EZ_TSL_DATA_TYPE_BOOL;
        }
        else if (0 == ezos_strcmp(js_type->valuestring, tsl_key_data_integer))
        {
            schema->item_type = EZ_TSL_DATA_TYPE_INT;
        }
        else if (0 == ezos_strcmp(js_type->valuestring, tsl_key_data_number))
        {
            schema->item_type = EZ_TSL_DATA_TYPE_DOUBLE;
        }
        else if (0 == ezos_strcmp(js_type->valuestring, tsl_key_data_object))
        {
            schema->item_type = EZ_TSL_DATA_TYPE_OBJECT;
        }
        else if (0 == ezos_strcmp(js_type->valuestring, tsl_key_data_array))
        {
            schema->item_type = EZ_TSL_DATA_TYPE_ARRAY;
        }
        else
        {
            schema->item_type = EZ_TSL_DATA_TYPE_MAX;
        }

        cJSON *js_key = cJSON_GetObjectItem(js_schema, tsl_key_data_key);
        if (NULL != js_key && cJSON_String == js_key->type)
        {
            ezos_strncpy(schema->prop_key, js_key->valuestring, sizeof(schema->prop_key) - 1);
            ezlog_v(TAG_TSL, "schema prop_key: %s", schema->prop_key);
        }

        switch (schema->item_type)
        {
        case EZ_TSL_DATA_TYPE_BOOL:
            break;

        case EZ_TSL_DATA_TYPE_INT:
        {
            cJSON *js_min = cJSON_GetObjectItem(js_schema, tsl_key_minimum);
            if (NULL != js_min && cJSON_Number == js_min->type)
            {
                schema->type_integer.minimum = js_min->valueint;
                ezlog_v(TAG_TSL, "integer minimum: %d", schema->type_integer.minimum);
            }
            else
            {
                js_min = cJSON_GetObjectItem(js_schema, tsl_key_exclusive_minimum);
                if (NULL != js_min && cJSON_Number == js_min->type)
                {
                    schema->type_integer.exminimum = js_min->valueint;
                    ezlog_v(TAG_TSL, "integer ex minimum: %d", schema->type_integer.exminimum);
                }
            }

            cJSON *js_max = cJSON_GetObjectItem(js_schema, tsl_key_maximum);
            if (NULL != js_max && cJSON_Number == js_max->type)
            {
                schema->type_integer.maximum = js_max->valueint;
                ezlog_v(TAG_TSL, "integer maximum: %d", schema->type_integer.maximum);
            }
            else
            {
                js_max = cJSON_GetObjectItem(js_schema, tsl_key_exclusive_maximum);
                if (NULL != js_max && cJSON_Number == js_max->type)
                {
                    schema->type_integer.exmaximum = js_max->valueint;
                    ezlog_v(TAG_TSL, "integer ex maximum: %d", schema->type_integer.exmaximum);
                }
            }

            cJSON *js_multi = cJSON_GetObjectItem(js_schema, tsl_key_multiple);
            if (NULL != js_multi && cJSON_Number == js_multi->type)
            {
                schema->type_integer.multiple = js_multi->valuedouble;
                ezlog_v(TAG_TSL, "integer multi: %d", schema->type_integer.multiple);
            }

            cJSON *js_enum = cJSON_GetObjectItem(js_schema, tsl_key_enum);
            if (NULL != js_enum && cJSON_Array == js_enum->type)
            {
                ez_int32_t array_size = cJSON_GetArraySize(js_enum);
                if (0 == array_size)
                {
                    ezlog_e(TAG_TSL, "integer enum size 0.");
                    break;
                }

                schema->type_integer.int_enum = (int *)ezos_malloc(sizeof(int) * array_size);
                if (NULL == schema->type_integer.int_enum)
                {
                    ezlog_e(TAG_TSL, "memory not enough.");
                    break;
                }
                ezos_memset(schema->type_integer.int_enum, 0, sizeof(int) * array_size);

                schema->type_integer.enum_num = array_size;
                ezlog_v(TAG_TSL, "integer enum size: %d", schema->type_integer.enum_num);

                for (int i = 0; i < array_size; i++)
                {
                    cJSON *js_int = cJSON_GetArrayItem(js_enum, i);
                    if (NULL != js_int && cJSON_Number == js_int->type)
                    {
                        *(schema->type_integer.int_enum + i) = js_int->valueint;
                        ezlog_v(TAG_TSL, "integer enum %d: %d", i, *(schema->type_integer.int_enum + i));
                    }
                }
            }
        }
        break;

        case EZ_TSL_DATA_TYPE_DOUBLE:
        {
            cJSON *js_min = cJSON_GetObjectItem(js_schema, tsl_key_minimum);
            if (NULL != js_min && cJSON_Number == js_min->type)
            {
                schema->type_number.minimum = js_min->valuedouble;
                ezlog_v(TAG_TSL, "number minimum: %f", schema->type_number.minimum);
            }
            else
            {
                js_min = cJSON_GetObjectItem(js_schema, tsl_key_exclusive_minimum);
                if (NULL != js_min && cJSON_Number == js_min->type)
                {
                    schema->type_number.exminimum = js_min->valuedouble;
                    ezlog_v(TAG_TSL, "number ex minimum: %f", schema->type_number.exminimum);
                }
            }

            cJSON *js_max = cJSON_GetObjectItem(js_schema, tsl_key_maximum);
            if (NULL != js_max && cJSON_Number == js_max->type)
            {
                schema->type_number.maximum = js_max->valuedouble;
                ezlog_v(TAG_TSL, "number maximum: %f", schema->type_number.maximum);
            }
            else
            {
                js_max = cJSON_GetObjectItem(js_schema, tsl_key_exclusive_maximum);
                if (NULL != js_max && cJSON_Number == js_max->type)
                {
                    schema->type_number.exmaximum = js_max->valuedouble;
                    ezlog_v(TAG_TSL, "number ex maximum: %f", schema->type_number.exmaximum);
                }
            }

            cJSON *js_multi = cJSON_GetObjectItem(js_schema, tsl_key_multiple);
            if (NULL != js_multi && cJSON_Number == js_multi->type)
            {
                schema->type_number.multiple = js_multi->valuedouble;
                ezlog_v(TAG_TSL, "number multi: %f", schema->type_number.multiple);
            }

            cJSON *js_enum = cJSON_GetObjectItem(js_schema, tsl_key_enum);
            if (NULL != js_enum && cJSON_Array == js_enum->type)
            {
                ez_int32_t array_size = cJSON_GetArraySize(js_enum);
                if (0 == array_size)
                {
                    ezlog_e(TAG_TSL, "number enum size 0");
                    break;
                }

                schema->type_number.num_enum = (double *)ezos_malloc(sizeof(double) * array_size);
                if (NULL == schema->type_number.num_enum)
                {
                    ezlog_e(TAG_TSL, "memory not enough.");
                    break;
                }
                ezos_memset(schema->type_number.num_enum, 0, sizeof(double) * array_size);

                schema->type_number.enum_num = array_size;
                ezlog_v(TAG_TSL, "number enum size: %d", schema->type_number.enum_num);

                for (int i = 0; i < array_size; i++)
                {
                    cJSON *js_num = cJSON_GetArrayItem(js_enum, i);
                    if (NULL != js_num && cJSON_Number == js_num->type)
                    {
                        *(schema->type_number.num_enum + i) = js_num->valuedouble;
                        ezlog_v(TAG_TSL, "number enum %d: %f", i, *(schema->type_number.num_enum + i));
                    }
                }
            }
        }
        break;

        case EZ_TSL_DATA_TYPE_STRING:
        {
            cJSON *js_maxlen = cJSON_GetObjectItem(js_schema, tsl_key_maxlength);
            if (NULL != js_maxlen && cJSON_Number == js_maxlen->type)
            {
                schema->type_string.max_len = js_maxlen->valueint;
                ezlog_v(TAG_TSL, "string max len: %d", schema->type_string.max_len);
            }
            cJSON *js_minlen = cJSON_GetObjectItem(js_schema, tsl_key_minlength);
            if (NULL != js_minlen && cJSON_Number == js_minlen->type)
            {
                schema->type_string.min_len = js_minlen->valueint;
                ezlog_v(TAG_TSL, "string min len: %d", schema->type_string.min_len);
            }

            cJSON *js_enum = cJSON_GetObjectItem(js_schema, tsl_key_enum);
            if (NULL != js_enum && cJSON_Array == js_enum->type)
            {
                ez_int32_t array_size = cJSON_GetArraySize(js_enum);
                if (0 == array_size)
                {
                    ezlog_e(TAG_TSL, "string enum size 0.");
                    break;
                }

                schema->type_string.str_enum = (char *)ezos_malloc(MAX_STRING_ENUM_LENGTH * array_size);
                if (NULL == schema->type_string.str_enum)
                {
                    ezlog_e(TAG_TSL, "memory not enough.");
                    break;
                }
                ezos_memset(schema->type_string.str_enum, 0, MAX_STRING_ENUM_LENGTH * array_size);

                schema->type_string.enum_num = array_size;
                ezlog_v(TAG_TSL, "string enum size: %d", schema->type_string.enum_num);

                for (int i = 0; i < array_size; i++)
                {
                    cJSON *js_str = cJSON_GetArrayItem(js_enum, i);
                    if (NULL != js_str && cJSON_String == js_str->type)
                    {
                        ezos_strncpy(schema->type_string.str_enum + (MAX_STRING_ENUM_LENGTH * i), js_str->valuestring, MAX_STRING_ENUM_LENGTH - 1);
                        ezlog_v(TAG_TSL, "string enum %d: %s", i, schema->type_string.str_enum + (MAX_STRING_ENUM_LENGTH * i));
                    }
                }
            }
        }
        break;

        case EZ_TSL_DATA_TYPE_ARRAY:
        {
            cJSON *js_minitems = cJSON_GetObjectItem(js_schema, tsl_key_min_items);
            if (NULL != js_minitems && cJSON_Number == js_minitems->type)
            {
                schema->type_array.minItem = js_minitems->valueint;
                ezlog_v(TAG_TSL, "array min items: %d", schema->type_array.minItem);
            }

            cJSON *js_maxitems = cJSON_GetObjectItem(js_schema, tsl_key_max_items);
            if (NULL != js_maxitems && cJSON_Number == js_maxitems->type)
            {
                schema->type_array.maxItem = js_maxitems->valueint;
                ezlog_v(TAG_TSL, "array max items: %d", schema->type_array.maxItem);
            }

            cJSON *js_items = cJSON_GetObjectItem(js_schema, tsl_key_items);
            if (NULL == js_items || cJSON_Object != js_items->type)
            {
                ezlog_e(TAG_TSL, "js_items absent.");
                rv = EZ_TSL_ERR_PARAM_INVALID;
                break;
            }

            schema->type_array.item_prop = (tsl_schema_desc *)ezos_malloc(sizeof(tsl_schema_desc));
            if (NULL == schema->type_array.item_prop)
            {
                ezlog_e(TAG_TSL, "memory not enough.");
                rv = EZ_TSL_ERR_MEMORY;
                break;
            }
            ezos_memset(schema->type_array.item_prop, 0, sizeof(tsl_schema_desc));
            schema->type_array.prop_num = 1;
            ezlog_v(TAG_TSL, "array prop size: %d", schema->type_array.prop_num);

            profile_parse_schema(schema->type_array.item_prop, js_items);
        }
        break;

        case EZ_TSL_DATA_TYPE_OBJECT:
        {
            cJSON *js_max_props = cJSON_GetObjectItem(js_schema, tsl_key_max_properties);
            if (NULL != js_max_props && cJSON_Number == js_max_props->type)
            {
                schema->type_object.max_props = js_max_props->valueint;
                ezlog_v(TAG_TSL, "obj max prop num: %d", schema->type_object.max_props);
            }

            cJSON *js_min_props = cJSON_GetObjectItem(js_schema, tsl_key_min_properties);
            if (NULL != js_min_props && cJSON_Number == js_min_props->type)
            {
                schema->type_object.min_props = js_min_props->valueint;
                ezlog_v(TAG_TSL, "obj min prop num: %d", schema->type_object.min_props);
            }

            cJSON *js_required = cJSON_GetObjectItem(js_schema, tsl_key_required);
            if (NULL != js_required && cJSON_Array == js_required->type)
            {
                ez_int32_t arr_size = cJSON_GetArraySize(js_required);
                if (0 != arr_size)
                {
                    schema->type_object.required = (char *)ezos_malloc(MAX_ARR_REQUIRE_LENGTH * arr_size);
                    if (NULL == schema->type_object.required)
                    {
                        ezlog_e(TAG_TSL, "memory not enough.");
                        rv = EZ_TSL_ERR_MEMORY;
                        break;
                    }
                    ezos_memset(schema->type_object.required, 0, MAX_ARR_REQUIRE_LENGTH * arr_size);

                    schema->type_object.req_num = arr_size;
                    ezlog_v(TAG_TSL, "schema obj required size: %d.", schema->type_object.req_num);

                    for (int i = 0; i < arr_size; i++)
                    {
                        char *enum_str = schema->type_object.required + MAX_ARR_REQUIRE_LENGTH * i;
                        cJSON *js_str = cJSON_GetArrayItem(js_required, i);
                        if (NULL != js_str && cJSON_String == js_str->type)
                        {
                            ezos_strncpy(enum_str, js_str->valuestring, MAX_ARR_REQUIRE_LENGTH - 1);
                            ezlog_v(TAG_TSL, "schema required index: %d, %s", i, enum_str);
                        }
                    }
                }
                else
                {
                    ezlog_w(TAG_TSL, "object required size 0.");
                }
            }

            cJSON *js_obj_prop = cJSON_GetObjectItem(js_schema, tsl_key_schema_properties);
            if (NULL != js_obj_prop && cJSON_Array == js_obj_prop->type)
            {
                ez_int32_t arr_size = cJSON_GetArraySize(js_obj_prop);
                if (0 != arr_size)
                {
                    schema->type_object.property = (tsl_schema_desc *)ezos_malloc(sizeof(tsl_schema_desc) * arr_size);
                    if (NULL == schema->type_object.property)
                    {
                        ezlog_e(TAG_TSL, "memory not enough.");
                        rv = EZ_TSL_ERR_MEMORY;
                        break;
                    }
                    ezos_memset(schema->type_object.property, 0, sizeof(tsl_schema_desc) * arr_size);

                    schema->type_object.prop_num = arr_size;
                    ezlog_v(TAG_TSL, "schema prop size: %d", schema->type_object.prop_num);

                    for (int i = 0; i < arr_size; i++)
                    {
                        tsl_schema_desc *sub_schema = schema->type_object.property + sizeof(tsl_schema_desc) * i;
                        cJSON *js_prop = cJSON_GetArrayItem(js_obj_prop, i);
                        if (NULL != js_prop && cJSON_Object == js_prop->type)
                        {
                            profile_parse_schema(sub_schema, js_prop);
                        }
                    }
                }
                else
                {
                    ezlog_w(TAG_TSL, "obj prop size 0.");
                }
            }
        }
        break;

        default:
            break;
        }

    } while (ez_false);

    return rv;
}
#endif

static ez_int32_t profile_parse_domain_props(tsl_domain_prop *p_props, ez_int32_t props_num, cJSON *js_props)
{
    if (NULL == js_props || NULL == p_props || 0 >= props_num)
    {
        ezlog_e(TAG_TSL, "parse domain props param error.");
        return EZ_TSL_ERR_PARAM_INVALID;
    }

    ezlog_v(TAG_TSL, "@@@@@@@@@@@@ props @@@@@@@@@@");
    ez_int32_t rv = EZ_TSL_ERR_SUCC;
    for (int i = 0; i < props_num; i++)
    {
        tsl_domain_prop *prop = p_props + i;
        cJSON *js_obj = cJSON_GetArrayItem(js_props, i);
        if (NULL == js_obj || cJSON_Object != js_obj->type)
        {
            ezlog_e(TAG_TSL, "prop object absent.");
            rv = EZ_TSL_ERR_PARAM_INVALID;
            break;
        }
        ezlog_v(TAG_TSL, "prop index: %d", i);

        cJSON *js_identify = cJSON_GetObjectItem(js_obj, tsl_key_identifier);
        if (NULL == js_identify || cJSON_String != js_identify->type)
        {
            ezlog_e(TAG_TSL, "prop identify absent");
            rv = EZ_TSL_ERR_PARAM_INVALID;
            break;
        }
        ezos_strncpy(prop->identifier, js_identify->valuestring, sizeof(prop->identifier) - 1);
        ezlog_v(TAG_TSL, "prop identifier: %s", prop->identifier);

        cJSON *js_access = cJSON_GetObjectItem(js_obj, tsl_key_access);
        if (NULL == js_access || cJSON_String != js_access->type)
        {
            ezlog_e(TAG_TSL, "prop schema absent.");
            rv = EZ_TSL_ERR_PARAM_INVALID;
            break;
        }
        if (NULL != ezos_strstr(js_access->valuestring, "r"))
        {
            prop->access = ACCESS_READ;
        }
        if (NULL != ezos_strstr(js_access->valuestring, "w"))
        {
            prop->access |= ACCESS_WRITE;
        }
        ezlog_v(TAG_TSL, "prop access: %d", prop->access);

#ifndef COMPONENT_TSL_PROFILE_STRIP
        cJSON *js_ver = cJSON_GetObjectItem(js_obj, tsl_key_version);
        if (NULL != js_ver && cJSON_String == js_ver->type)
        {
            ezos_strncpy(prop->version, js_ver->valuestring, sizeof(prop->version) - 1);
            ezlog_v(TAG_TSL, "prop version: %s", prop->version);
        }

        cJSON *js_schema = cJSON_GetObjectItem(js_obj, tsl_key_schema);
        if (NULL == js_schema || cJSON_Object != js_schema->type)
        {
            ezlog_e(TAG_TSL, "prop schema absent.");
            rv = EZ_TSL_ERR_PARAM_INVALID;
            break;
        }

        rv = profile_parse_schema(&prop->prop_desc, js_schema);
        if (0 != rv)
        {
            ezlog_e(TAG_TSL, "parse schema error.");
            break;
        }
#endif
    }

    return rv;
}

static ez_int32_t profile_parse_domain_actions(tsl_domain_action *p_actions, ez_int32_t actions_num, cJSON *js_actions)
{
    if (NULL == js_actions || NULL == p_actions || 0 >= actions_num)
    {
        ezlog_e(TAG_TSL, "parse domain actions param error.");
        return EZ_TSL_ERR_PARAM_INVALID;
    }

    ezlog_v(TAG_TSL, "@@@@@@@@@@@@ actions @@@@@@@@@@");
    ez_int32_t rv = EZ_TSL_ERR_SUCC;
    for (int i = 0; i < actions_num; i++)
    {
        tsl_domain_action *action = p_actions + i;

        cJSON *js_obj = cJSON_GetArrayItem(js_actions, i);
        if (NULL == js_obj || cJSON_Object != js_obj->type)
        {
            rv = EZ_TSL_ERR_GENERAL;
            ezlog_e(TAG_TSL, "action absent.");
            break;
        }

        cJSON *js_identify = cJSON_GetObjectItem(js_obj, tsl_key_identifier);
        if (NULL == js_identify || cJSON_String != js_identify->type)
        {
            rv = EZ_TSL_ERR_PARAM_INVALID;
            ezlog_e(TAG_TSL, "identify absent.");
            break;
        }
        ezos_strncpy(action->identifier, js_identify->valuestring, sizeof(action->identifier) - 1);
        ezlog_v(TAG_TSL, "action identifier: %s", action->identifier);

#ifndef COMPONENT_TSL_PROFILE_STRIP
        cJSON *js_ver = cJSON_GetObjectItem(js_obj, tsl_key_version);
        if (NULL != js_ver && cJSON_String == js_ver->type)
        {
            ezos_strncpy(action->version, js_ver->valuestring, sizeof(action->version) - 1);
            ezlog_v(TAG_TSL, "action version: %s", action->version);
        }

        cJSON *js_direction = cJSON_GetObjectItem(js_obj, tsl_key_direction);
        if (NULL == js_direction || cJSON_String != js_direction->type)
        {
            rv = EZ_TSL_ERR_PARAM_INVALID;
            ezlog_e(TAG_TSL, "direction absent.");
            break;
        }
        ezos_strncpy(action->direction, js_direction->valuestring, sizeof(action->direction) - 1);
        ezlog_v(TAG_TSL, "action direction: %s", action->direction);

        cJSON *js_input = cJSON_GetObjectItem(js_obj, tsl_key_input);
        if (NULL != js_input && cJSON_Object == js_input->type)
        {
            cJSON *js_schema = cJSON_GetObjectItem(js_input, tsl_key_schema);
            if (NULL == js_schema || cJSON_Object != js_schema->type)
            {
                rv = EZ_TSL_ERR_PARAM_INVALID;
                ezlog_e(TAG_TSL, "action input schema absent.");
                break;
            }

            rv = profile_parse_schema(&action->input_schema, js_schema);
            if (rv != 0)
            {
                ezlog_e(TAG_TSL, "actiom input absent.");
                break;
            }
        }

        cJSON *js_output = cJSON_GetObjectItem(js_obj, tsl_key_output);
        if (NULL != js_output && cJSON_Object == js_output->type)
        {
            cJSON *js_schema = cJSON_GetObjectItem(js_output, tsl_key_schema);
            if (NULL == js_schema || cJSON_Object != js_schema->type)
            {
                rv = EZ_TSL_ERR_PARAM_INVALID;
                ezlog_e(TAG_TSL, "action output schema absent.");
                break;
            }
            rv = profile_parse_schema(&action->output_schema, js_schema);
            if (rv != 0)
            {
                ezlog_e(TAG_TSL, "action output absent.");
                break;
            }
        }
#endif
    }

    return rv;
}

static ez_int32_t profile_parse_domain_events(tsl_domain_event *p_events, ez_int32_t events_num, cJSON *js_events)
{
    if (NULL == js_events || NULL == p_events || 0 >= events_num)
    {
        ezlog_e(TAG_TSL, "parse domain events param error.");
        return EZ_TSL_ERR_PARAM_INVALID;
    }
    ez_int32_t rv = EZ_TSL_ERR_SUCC;

    ez_int32_t arr_size = cJSON_GetArraySize(js_events);
    for (size_t i = 0; i < arr_size; i++)
    {
        tsl_domain_event *event = p_events + i;
        cJSON *js_obj = cJSON_GetArrayItem(js_events, i);
        if (NULL == js_obj || cJSON_Object != js_obj->type)
        {
            rv = EZ_TSL_ERR_GENERAL;
            ezlog_e(TAG_TSL, "event absent.");
            break;
        }

        cJSON *js_identify = cJSON_GetObjectItem(js_obj, tsl_key_identifier);
        if (NULL == js_identify || cJSON_String != js_identify->type)
        {
            rv = EZ_TSL_ERR_PARAM_INVALID;
            ezlog_e(TAG_TSL, "identify absent");
            break;
        }
        ezos_strncpy(event->identifier, js_identify->valuestring, sizeof(event->identifier) - 1);
        ezlog_v(TAG_TSL, "event identifier: %s", event->identifier);

#ifndef COMPONENT_TSL_PROFILE_STRIP
        cJSON *js_ver = cJSON_GetObjectItem(js_obj, tsl_key_version);
        if (NULL != js_ver && cJSON_String == js_ver->type)
        {
            ezos_strncpy(event->version, js_ver->valuestring, sizeof(event->version) - 1);
            ezlog_v(TAG_TSL, "event version: %s", event->version);
        }

        cJSON *js_event_type = cJSON_GetObjectItem(js_obj, tsl_key_eventtype);
        if (NULL == js_event_type || cJSON_Array != js_event_type->type)
        {
            rv = EZ_TSL_ERR_PARAM_INVALID;
            ezlog_e(TAG_TSL, "eventType absent.");
            break;
        }

        ez_int32_t arr_size = cJSON_GetArraySize(js_event_type);
        if (0 != arr_size)
        {
            event->event_type = (char *)ezos_malloc(MAX_EVENT_TYPE_KEY_LENGTH * arr_size);
            if (NULL == event->event_type)
            {
                ezlog_e(TAG_TSL, "memory not enough.");
                rv = EZ_TSL_ERR_MEMORY;
                break;
            }
            ezos_memset(event->event_type, 0, MAX_EVENT_TYPE_KEY_LENGTH * arr_size);

            event->enum_num = arr_size;

            for (int j = 0; j < arr_size; j++)
            {
                char *type = event->event_type + MAX_EVENT_TYPE_KEY_LENGTH * j;
                cJSON *js_type = cJSON_GetArrayItem(js_event_type, i);
                if (NULL != js_type && cJSON_String == js_type->type)
                {
                    ezos_strncpy(type, js_type->valuestring, MAX_EVENT_TYPE_KEY_LENGTH - 1);
                }
            }
        }
        else
        {
            ezlog_w(TAG_TSL, "event type arr size 0.");
        }

        cJSON *js_input = cJSON_GetObjectItem(js_obj, tsl_key_input);
        if (NULL != js_input && cJSON_Object == js_input->type)
        {
            cJSON *js_schema = cJSON_GetObjectItem(js_input, tsl_key_schema);
            if (NULL == js_schema || cJSON_Object != js_schema->type)
            {
                rv = EZ_TSL_ERR_GENERAL;
                ezlog_e(TAG_TSL, "event input schema absent.");
                break;
            }

            rv = profile_parse_schema(&event->input_schema, js_schema);
            if (rv != 0)
            {
                ezlog_e(TAG_TSL, "event schema parse error.");
                break;
            }
        }

        cJSON *js_output = cJSON_GetObjectItem(js_obj, tsl_key_output);
        if (NULL != js_output && cJSON_Object == js_output->type)
        {
            cJSON *js_schema = cJSON_GetObjectItem(js_output, tsl_key_schema);
            if (NULL == js_schema || cJSON_Object != js_schema->type)
            {
                rv = EZ_TSL_ERR_GENERAL;
                ezlog_e(TAG_TSL, "event output schema absent.");
                break;
            }

            rv = profile_parse_schema(&event->output_schema, js_schema);
            if (rv != 0)
            {
                ezlog_e(TAG_TSL, "event schema parse error.");
                break;
            }
        }
#endif
    }

    return rv;
}

static ez_int32_t profile_parse_domain(tsl_rsc_domain *pdomain, ez_int32_t domain_num, cJSON *js_domains)
{
    if (NULL == pdomain || 0 >= domain_num)
    {
        ezlog_e(TAG_TSL, "profile_parse_domain param error.");
        return EZ_TSL_ERR_PARAM_INVALID;
    }

    ez_int32_t rv = EZ_TSL_ERR_SUCC;

    ezlog_v(TAG_TSL, "--------------------------- domain ------------------------");
    for (int j = 0; j < domain_num; j++)
    {
        cJSON *js_domain = cJSON_GetArrayItem(js_domains, j);
        if (NULL == js_domain || cJSON_Object != js_domain->type)
        {
            ezlog_e(TAG_TSL, "domain absent.");
            break;
        }
        ezlog_v(TAG_TSL, "################ domain index: %d ###############", j);

        tsl_rsc_domain *domain = pdomain + j;
        cJSON *js_iden = cJSON_GetObjectItem(js_domain, tsl_key_identifier);
        if (NULL == js_iden || cJSON_String != js_iden->type)
        {
            ezlog_e(TAG_TSL, "domain iden absent.");
            rv = EZ_TSL_ERR_PARAM_INVALID;
            break;
        }
        ezos_strncpy(domain->identifier, js_iden->valuestring, sizeof(domain->identifier) - 1);
        ezlog_v(TAG_TSL, "domain identifier: %s", domain->identifier);

        cJSON *js_props = cJSON_GetObjectItem(js_domain, tsl_key_props);
        if (NULL == js_props || cJSON_Array != js_props->type)
        {
            ezlog_w(TAG_TSL, "domain prop absent.");
            // rv = EZ_TSL_ERR_PARAM_INVALID;
            goto actions;
        }

        ez_int32_t props_num = cJSON_GetArraySize(js_props);
        if (0 == props_num)
        {
            ezlog_w(TAG_TSL, "props array size 0.");
            goto actions;
        }

        domain->prop = (tsl_domain_prop *)ezos_malloc(sizeof(tsl_domain_prop) * props_num);
        if (NULL == domain->prop)
        {
            ezlog_e(TAG_TSL, "memory not enough.");
            rv = EZ_TSL_ERR_MEMORY;
            break;
        }
        ezos_memset(domain->prop, 0, sizeof(tsl_domain_prop) * props_num);

        domain->prop_num = props_num;
        ezlog_v(TAG_TSL, "domain props num: %d", domain->prop_num);

        rv = profile_parse_domain_props(domain->prop, domain->prop_num, js_props);
        if (0 != rv)
        {
            ezlog_e(TAG_TSL, "domain props parse failed.");
            break;
        }

    actions:
    {
        cJSON *js_actions = cJSON_GetObjectItem(js_domain, tsl_key_actions);
        if (NULL == js_actions || cJSON_Array != js_actions->type)
        {
            ezlog_w(TAG_TSL, "domain action absent.");
            // rv = EZ_TSL_ERR_PARAM_INVALID;
            goto events;
        }
        ez_int32_t actions_num = cJSON_GetArraySize(js_actions);
        if (0 == actions_num)
        {
            ezlog_w(TAG_TSL, "action array size 0.");
            goto events;
        }

        domain->action = (tsl_domain_action *)ezos_malloc(sizeof(tsl_domain_action) * actions_num);
        if (NULL == domain->action)
        {
            ezlog_e(TAG_TSL, "memory not enough.");
            rv = EZ_TSL_ERR_MEMORY;
            break;
        }
        ezos_memset(domain->action, 0, sizeof(tsl_domain_action) * actions_num);

        domain->action_num = actions_num;
        ezlog_v(TAG_TSL, "domain actions num: %d", domain->action_num);

        rv = profile_parse_domain_actions(domain->action, domain->action_num, js_actions);
        if (0 != rv)
        {
            ezlog_e(TAG_TSL, "domain actions parse failed.");
            break;
        }
    }

    events:
    {
        cJSON *js_events = cJSON_GetObjectItem(js_domain, tsl_key_events);
        if (NULL == js_events || cJSON_Array != js_events->type)
        {
            ezlog_w(TAG_TSL, "domain events absent.");
            // rv = EZ_TSL_ERR_PARAM_INVALID;
            continue;
        }

        ez_int32_t events_num = cJSON_GetArraySize(js_events);
        if (0 == events_num)
        {
            ezlog_w(TAG_TSL, "events array size 0.");
            continue;
        }

        domain->event = (tsl_domain_event *)ezos_malloc(sizeof(tsl_domain_event) * events_num);
        if (NULL == domain->event)
        {
            ezlog_e(TAG_TSL, "memory not enough.");
            rv = EZ_TSL_ERR_MEMORY;
            break;
        }
        ezos_memset(domain->event, 0, sizeof(tsl_domain_event) * events_num);

        domain->event_num = events_num;
        ezlog_v(TAG_TSL, "domain events num: %d", domain->event_num);

        rv = profile_parse_domain_events(domain->event, domain->event_num, js_events);
        if (0 != rv)
        {
            ezlog_e(TAG_TSL, "domain events parse failed.");
            break;
        }
    }
    }

    return rv;
}

static ez_int32_t profile_parse(char *profile_value, tsl_capacity_t *capacity)
{
    ez_int32_t rv = EZ_TSL_ERR_SUCC;

    if (NULL == capacity || NULL == profile_value)
    {
        ezlog_e(TAG_TSL, "profile param error.");
        return EZ_TSL_ERR_PARAM_INVALID;
    }

    cJSON *js_root = NULL;
    do
    {
        js_root = cJSON_Parse(profile_value);
        if (NULL == js_root)
        {
            ezlog_e(TAG_TSL, "profile parse failed.");
            rv = EZ_TSL_ERR_MEMORY;
            break;
        }

        cJSON *js_ver = cJSON_GetObjectItem(js_root, tsl_key_version);
        if (NULL == js_ver || cJSON_String != js_ver->type)
        {
            ezlog_e(TAG_TSL, "version absent.");
            rv = EZ_TSL_ERR_PARAM_INVALID;
            break;
        }

        cJSON *js_src = cJSON_GetObjectItem(js_root, tsl_key_resources);
        if (NULL == js_src || cJSON_Array != js_src->type)
        {
            ezlog_e(TAG_TSL, "resources absent.");
            rv = EZ_TSL_ERR_PARAM_INVALID;
            break;
        }

        ez_int32_t array_size = cJSON_GetArraySize(js_src);
        if (0 == array_size)
        {
            ezlog_e(TAG_TSL, "array size 0.");
            rv = EZ_TSL_ERR_GENERAL;
            break;
        }

        capacity->resource = (tsl_profile_resource *)ezos_malloc(sizeof(tsl_profile_resource) * array_size);
        if (NULL == capacity->resource)
        {
            ezlog_e(TAG_TSL, "memory not enough.");
            rv = EZ_TSL_ERR_MEMORY;
            break;
        }
        ezos_memset(capacity->resource, 0, sizeof(tsl_profile_resource) * array_size);

        capacity->rsc_num = array_size;
        ezlog_v(TAG_TSL, "resources num: %d", capacity->rsc_num);

        for (int i = 0; i < array_size; i++)
        {
            tsl_profile_resource *p_src = capacity->resource + i;
            cJSON *js_obj = cJSON_GetArrayItem(js_src, i);
            if (NULL == js_obj)
            {
                ezlog_e(TAG_TSL, "array item %d error.", i);
                rv = EZ_TSL_ERR_GENERAL;
                break;
            }

            cJSON *js_category = cJSON_GetObjectItem(js_obj, tsl_key_rsc_category);
            if (NULL == js_category || cJSON_String != js_category->type)
            {
                ezlog_e(TAG_TSL, "category absent.");
                rv = EZ_TSL_ERR_PARAM_INVALID;
                break;
            }
            ezos_strncpy(p_src->rsc_category, js_category->valuestring, sizeof(p_src->rsc_category) - 1);
            ezlog_v(TAG_TSL, "source category: %s", p_src->rsc_category);

            cJSON *js_index = cJSON_GetObjectItem(js_obj, tsl_key_local_index);
            if (NULL == js_index || cJSON_Array != js_index->type)
            {
                ezlog_e(TAG_TSL, "local_index absent.");
                rv = EZ_TSL_ERR_PARAM_INVALID;
                break;
            }

            ez_int32_t index_size = cJSON_GetArraySize(js_index);
            if (0 == index_size)
            {
                ezlog_e(TAG_TSL, "local_index absent.");
                rv = EZ_TSL_ERR_GENERAL;
                break;
            }

            p_src->index = (char *)ezos_malloc(MAX_LOCAL_INDEX_LENGTH * index_size);
            if (NULL == p_src->index)
            {
                ezlog_e(TAG_TSL, "memory not enough");
                rv = EZ_TSL_ERR_MEMORY;
                break;
            }
            ezos_memset(p_src->index, 0, MAX_LOCAL_INDEX_LENGTH * index_size);

            p_src->index_num = index_size;
            ezlog_v(TAG_TSL, "resource local index num: %d", p_src->index_num);

            for (int j = 0; j < index_size; j++)
            {
                cJSON *js_idx = cJSON_GetArrayItem(js_index, j);
                if (NULL == js_idx || cJSON_String != js_idx->type)
                {
                    ezlog_e(TAG_TSL, "local index absent.");
                    break;
                }
                ezos_strncpy(p_src->index + i, js_idx->valuestring, MAX_LOCAL_INDEX_LENGTH - 1);
                ezlog_v(TAG_TSL, "resource local index: %d, %s", j, p_src->index + j * MAX_LOCAL_INDEX_LENGTH);
            }

            cJSON *js_domain = cJSON_GetObjectItem(js_obj, tsl_key_domains);
            if (NULL == js_domain || cJSON_Array != js_domain->type)
            {
                ezlog_e(TAG_TSL, "domain absent.");
                rv = EZ_TSL_ERR_PARAM_INVALID;
                break;
            }
            ez_int32_t domain_size = cJSON_GetArraySize(js_domain);
            if (0 == domain_size)
            {
                ezlog_e(TAG_TSL, "domain array size 0.");
                rv = EZ_TSL_ERR_GENERAL;
                break;
            }

            p_src->domain = (tsl_rsc_domain *)ezos_malloc(sizeof(tsl_rsc_domain) * domain_size);
            if (NULL == p_src->domain)
            {
                ezlog_e(TAG_TSL, "memory not enough.");
                rv = EZ_TSL_ERR_MEMORY;
                break;
            }
            ezos_memset(p_src->domain, 0, sizeof(tsl_rsc_domain) * domain_size);

            p_src->domain_num = domain_size;
            ezlog_v(TAG_TSL, "resource domain num: %d", p_src->domain_num);

            rv = profile_parse_domain(p_src->domain, p_src->domain_num, js_domain);
            if (0 != rv)
            {
                ezlog_e(TAG_TSL, "parse domain failed.");
                break;
            }
        }
    } while (ez_false);

    if (0 != rv)
    {
        profile_free(capacity);
    }

    if (NULL != js_root)
    {
        cJSON_Delete(js_root);
    }

    return rv;
}

static node_capacity_t *profile_find_by_sn(ez_char_t *dev_sn)
{
    node_capacity_t *node_capacity = NULL;
    node_dev_t *node_dev = NULL;

    LIST_FOR_EACH(node_capacity_t, node_capacity, &g_profile_list)
    {
        LIST_FOR_EACH(node_dev_t, node_dev, &node_capacity->dev_list)
        {
            if (0 == ezos_strcmp(dev_sn, node_dev->dev_sn))
            {
                return node_capacity;
            }
        }
    }

    return NULL;
}