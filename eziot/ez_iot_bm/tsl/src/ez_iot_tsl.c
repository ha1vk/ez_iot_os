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
 * XuRongjun (xurongjun@ezvizlife.com) - TSL(Thing Specification Language) user interface implement
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-11-15     xurongjun    first version 
 *******************************************************************************/

#include "ez_iot_tsl_def.h"
#include "ez_iot_tsl.h"
#include "ez_iot_tsl_adapter.h"
#include "ez_iot_tsl_schema.h"
#include "ez_iot_core_def.h"
#include "ez_iot_core_lowlvl.h"
#include "uuid.h"
#include "cJSON.h"

static ez_bool_t g_tsl_is_inited = ez_false;

static ez_err_t make_event_value(const ez_tsl_value_t *value, ez_tsl_value_t *tsl_value);

ez_err_t ez_iot_tsl_init(ez_tsl_callbacks_t *ptsl_cbs)
{
    FUNC_IN();

    ez_err_t rv = EZ_TSL_ERR_SUCC;

    if (g_tsl_is_inited)
    {
        goto done;
    }

    CHECK_COND_DONE(NULL == ptsl_cbs, EZ_TSL_ERR_PARAM_INVALID);
    CHECK_COND_DONE(NULL == ptsl_cbs->action2dev, EZ_TSL_ERR_PARAM_INVALID);
    CHECK_COND_DONE(NULL == ptsl_cbs->property2cloud, EZ_TSL_ERR_PARAM_INVALID);
    CHECK_COND_DONE(NULL == ptsl_cbs->property2dev, EZ_TSL_ERR_PARAM_INVALID);
    CHECK_RV_DONE(ez_iot_tsl_adapter_init(ptsl_cbs));
    CHECK_COND_DONE(0 != ez_iot_shadow_init(), EZ_TSL_ERR_MEMORY);

    g_tsl_is_inited = ez_true;
done:
    FUNC_OUT();

    return rv;
}

EZOS_API ez_err_t ez_iot_tsl_property_report(const ez_char_t *sn, const ez_tsl_rsc_t *rsc_info, const ez_tsl_key_t *key_info, const ez_tsl_value_t *value)
{
    FUNC_IN();

    ez_err_t rv = EZ_TSL_ERR_SUCC;

    ez_shadow_res_t shadow_res = {0};
    ez_shadow_value_t shadow_value = {0};
    ez_shadow_value_t *pvalue = NULL;
    ez_char_t res_type[32] = {0};
    ez_tsl_rsc_t _rsc_info;

    CHECK_COND_DONE(ez_false == g_tsl_is_inited, EZ_TSL_ERR_NOT_INIT);
    CHECK_COND_DONE(NULL == sn, EZ_TSL_ERR_PARAM_INVALID);
    CHECK_COND_DONE(NULL == rsc_info, EZ_TSL_ERR_PARAM_INVALID);
    CHECK_COND_DONE(NULL == key_info, EZ_TSL_ERR_PARAM_INVALID);
    CHECK_COND_DONE(NULL == key_info->domain, EZ_TSL_ERR_PARAM_INVALID);

    _rsc_info.local_index = (rsc_info->local_index) ? rsc_info->local_index : (ez_char_t *)"0";
    _rsc_info.res_type = (rsc_info->res_type) ? rsc_info->res_type : res_type;
    tsl_find_property_rsc_by_keyinfo(sn, key_info, res_type, sizeof(res_type));

    /* Legality check */
    rv = ez_iot_tsl_property_value_legal(sn, &_rsc_info, key_info, value);
    CHECK_RV_DONE(rv);

    if (NULL != value)
    {
        shadow_value.length = value->size;
        shadow_value.type = value->type;
        switch (value->type)
        {
        case EZ_TSL_DATA_TYPE_BOOL:
        case EZ_TSL_DATA_TYPE_INT:
            shadow_value.value_int = value->value_int;
            ezlog_d(TAG_TSL, "tsl value:%d", value->value_int);
            break;
        case EZ_TSL_DATA_TYPE_DOUBLE:
            shadow_value.value_double = value->value_double;
            ezlog_d(TAG_TSL, "tsl value:%lf", value->value_double);
            break;
        case EZ_TSL_DATA_TYPE_STRING:
        case EZ_TSL_DATA_TYPE_ARRAY:
        case EZ_TSL_DATA_TYPE_OBJECT:
            shadow_value.value = value->value;
            ezlog_d(TAG_TSL, "tsl value:%s", (char *)value->value);
            break;
        default:
            CHECK_COND_DONE(1, EZ_TSL_ERR_PARAM_INVALID);
            break;
        }

        pvalue = &shadow_value;
    }

    ezos_strncpy((char *)shadow_res.dev_serial, (char *)sn, sizeof(shadow_res.dev_serial) - 1);
    ezos_strncpy((char *)shadow_res.res_type, (char *)res_type, sizeof(shadow_res.res_type) - 1);
    shadow_res.local_index = ezos_atoi((char *)_rsc_info.local_index);

    CHECK_COND_DONE(ez_iot_shadow_push(&shadow_res, key_info->domain, key_info->key, pvalue), EZ_TSL_ERR_GENERAL);

done:
    FUNC_OUT();

    return rv;
}

ez_err_t ez_iot_tsl_event_report(const ez_char_t *sn, const ez_tsl_rsc_t *rsc_info, const ez_tsl_key_t *key_info, const ez_tsl_value_t *value)
{
    FUNC_IN();

    ez_err_t rv = EZ_TSL_ERR_SUCC;

    ez_tsl_value_t tsl_value = {0};
    ez_char_t res_type[32] = {0};
    ez_tsl_rsc_t _rsc_info;

    CHECK_COND_DONE(ez_false == g_tsl_is_inited, EZ_TSL_ERR_NOT_INIT);
    CHECK_COND_DONE(NULL == sn, EZ_TSL_ERR_PARAM_INVALID);
    CHECK_COND_DONE(NULL == rsc_info, EZ_TSL_ERR_PARAM_INVALID);
    CHECK_COND_DONE(NULL == key_info, EZ_TSL_ERR_PARAM_INVALID);
    CHECK_COND_DONE(NULL == key_info->domain, EZ_TSL_ERR_PARAM_INVALID);
    CHECK_COND_DONE(make_event_value(value, &tsl_value), EZ_TSL_ERR_MEMORY);

    _rsc_info.local_index = (rsc_info->local_index) ? rsc_info->local_index : (ez_char_t *)"0";
    _rsc_info.res_type = (rsc_info->res_type) ? rsc_info->res_type : res_type;
    tsl_find_event_rsc_by_keyinfo(sn, key_info, res_type, sizeof(res_type));

    /* Legality check */
    rv = ez_iot_tsl_event_value_legal(sn, &_rsc_info, key_info, &tsl_value);
    CHECK_RV_DONE(rv);

    ez_kernel_pubmsg_v3_t pubmsg = {0};
    pubmsg.msg_qos = QOS_T1;
    pubmsg.msg_body = tsl_value.value;
    pubmsg.msg_body_len = tsl_value.size;

    ezos_strncpy(pubmsg.resource_type, _rsc_info.res_type, sizeof(pubmsg.resource_type) - 1);
    ezos_strncpy(pubmsg.resource_id, _rsc_info.local_index, sizeof(pubmsg.resource_id) - 1);
    ezos_strncpy(pubmsg.module, TSL_MODULE_NAME, sizeof(pubmsg.module) - 1);
    ezos_strncpy(pubmsg.method, TSL_EVENT_METHOD_NAME, sizeof(pubmsg.module) - 1);
    ezos_strncpy(pubmsg.msg_type, TSL_MSG_TYPE_REPORT, sizeof(pubmsg.msg_type) - 1);
    ezos_snprintf(pubmsg.ext_msg, sizeof(pubmsg.ext_msg), "%s/%s", key_info->domain, key_info->key);

    if (0 == ezos_strcmp((char *)sn, (char *)ez_kernel_getdevinfo_bykey("dev_subserial")))
    {
        ezos_strncpy(pubmsg.sub_serial, "global", sizeof(pubmsg.sub_serial) - 1);
    }
    else
    {
        ezos_strncpy(pubmsg.sub_serial, (char *)sn, sizeof(pubmsg.sub_serial) - 1);
    }

    CHECK_COND_DONE(ez_kernel_send_v3(&pubmsg), EZ_TSL_ERR_GENERAL);

done:
    FUNC_OUT();

    SAFE_FREE(tsl_value.value);

    return rv;
}

ez_err_t ez_iot_tsl_check_value_legal(const char *key, int type, const ez_tsl_devinfo_t *dev_info, ez_tsl_value_t *value)
{
    if (!g_tsl_is_inited)
    {
        ezlog_w(TAG_TSL, "tsl not inited.");
        return -1;
    }
#ifdef EZ_IOT_TSL_NEED_SCHEMA
    return check_value_legal(key, type, dev_info, value);
#endif
    return 0;
}

ez_err_t ez_iot_tsl_deinit(void)
{
    FUNC_IN();

    ez_err_t rv = EZ_TSL_ERR_SUCC;

    CHECK_COND_DONE(ez_false == g_tsl_is_inited, EZ_TSL_ERR_NOT_INIT);
    ez_iot_shadow_deini();
    ez_iot_tsl_adapter_deinit();

    g_tsl_is_inited = ez_false;

done:
    FUNC_OUT();

    return rv;
}

EZOS_API ez_err_t ez_iot_tsl_reg(ez_tsl_devinfo_t *pevinfo, ez_char_t *profile)
{
    FUNC_IN();

    ez_err_t rv = EZ_TSL_ERR_SUCC;
    ez_tsl_devinfo_t devinfo = {0};

    CHECK_COND_DONE(ez_false == g_tsl_is_inited, EZ_TSL_ERR_NOT_INIT);

    if (!pevinfo)
    {
        devinfo.dev_subserial = (ez_char_t *)ez_kernel_getdevinfo_bykey("dev_subserial");
        devinfo.dev_type = (ez_char_t *)ez_kernel_getdevinfo_bykey("dev_type");
        devinfo.dev_firmwareversion = (ez_char_t *)ez_kernel_getdevinfo_bykey("dev_firmwareversion");
        pevinfo = &devinfo;
    }
    else
    {
        CHECK_COND_DONE(NULL == pevinfo->dev_subserial, EZ_TSL_ERR_PARAM_INVALID);
        CHECK_COND_DONE(NULL == pevinfo->dev_type, EZ_TSL_ERR_PARAM_INVALID);
        CHECK_COND_DONE(NULL == pevinfo->dev_firmwareversion, EZ_TSL_ERR_PARAM_INVALID);
    }

    ez_iot_tsl_adapter_add(pevinfo, profile);

done:
    FUNC_OUT();

    return rv;
}

ez_err_t ez_iot_tsl_unreg(ez_tsl_devinfo_t *pevinfo)
{
    FUNC_IN();

    ez_err_t rv = EZ_TSL_ERR_SUCC;
    ez_tsl_devinfo_t devinfo = {0};

    CHECK_COND_DONE(ez_false == g_tsl_is_inited, EZ_TSL_ERR_NOT_INIT);

    if (!pevinfo)
    {
        devinfo.dev_subserial = (ez_char_t *)ez_kernel_getdevinfo_bykey("dev_subserial");
        devinfo.dev_type = (ez_char_t *)ez_kernel_getdevinfo_bykey("dev_type");
        devinfo.dev_firmwareversion = (ez_char_t *)ez_kernel_getdevinfo_bykey("dev_firmwareversion");
        pevinfo = &devinfo;
    }
    else
    {
        CHECK_COND_DONE(NULL == pevinfo->dev_subserial, EZ_TSL_ERR_PARAM_INVALID);
        CHECK_COND_DONE(NULL == pevinfo->dev_type, EZ_TSL_ERR_PARAM_INVALID);
        CHECK_COND_DONE(NULL == pevinfo->dev_firmwareversion, EZ_TSL_ERR_PARAM_INVALID);
    }

    ez_iot_tsl_adapter_del(pevinfo);

done:
    FUNC_OUT();

    return rv;
}

char g_time_zone[8] = {0};

static int get_format_time(char *format_time)
{
    ezos_time_t time_now = ezos_time(NULL);
    struct ezos_tm ptm_time_now = {0};

    if (NULL == ezos_localtime(&time_now, &ptm_time_now))
    {
        return 0;
    }

    struct ezos_timeval tv = {0};
    ezos_gettimeofday(&tv, NULL);
    int time_ms = tv.tv_usec / 1000;

    if (ezos_strlen(g_time_zone) == 0)
    {
        ezos_sprintf(format_time, "%04d-%02d-%02dT%02d:%02d:%02d.%03d+00:00", ptm_time_now.tm_year + 1900, ptm_time_now.tm_mon + 1, ptm_time_now.tm_mday,
                     ptm_time_now.tm_hour, ptm_time_now.tm_min, ptm_time_now.tm_sec, time_ms);
    }
    else
    {
        ezos_sprintf(format_time, "%04d-%02d-%02dT%02d:%02d:%02d.%03d%s", ptm_time_now.tm_year + 1900, ptm_time_now.tm_mon + 1, ptm_time_now.tm_mday,
                     ptm_time_now.tm_hour, ptm_time_now.tm_min, ptm_time_now.tm_sec, time_ms, g_time_zone);
    }

    return 0;
}

static ez_err_t make_event_value(const ez_tsl_value_t *value, ez_tsl_value_t *tsl_value)
{
    ez_err_t rv = EZ_TSL_ERR_SUCC;
    cJSON *js_root = NULL;
    ez_bool_t has_basic = ez_false;

    if (NULL == value || NULL == value->value || 0 == ezos_strlen((char *)value->value))
    {
        js_root = cJSON_CreateObject();
        CHECK_COND_DONE(NULL == js_root, EZ_TSL_ERR_MEMORY);
    }
    else
    {
        js_root = cJSON_Parse((char *)value->value);
        CHECK_COND_DONE(NULL == js_root, EZ_TSL_ERR_MEMORY);

        cJSON *js_obj = NULL;
        cJSON_ArrayForEach(js_obj, js_root)
        {
            if (0 == ezos_strcmp(js_obj->string, "basic"))
            {
                has_basic = ez_true;
                break;
            }
        }
    }

    if (!has_basic)
    {
        cJSON *js_basic = NULL;
        char format_time[32] = {0};
        ieee_uuid_t uuuuid = {0};
        char uuid[64 + 1] = {0};

        js_basic = cJSON_CreateObject();
        CHECK_COND_DONE(NULL == js_basic, EZ_TSL_ERR_MEMORY);
        get_format_time(format_time);
        cJSON_AddStringToObject(js_basic, "dateTime", format_time);

        CreateUUID(&uuuuid);
        CreateStringUUID(uuuuid, uuid);
        cJSON_AddStringToObject(js_basic, "UUID", uuid);
        cJSON_AddItemToObject(js_root, "basic", js_basic);
    }

    tsl_value->value = cJSON_PrintUnformatted(js_root);
    CHECK_COND_DONE(NULL == tsl_value->value, EZ_TSL_ERR_MEMORY);
    tsl_value->size = ezos_strlen((char *)tsl_value->value);
    tsl_value->type = EZ_TSL_DATA_TYPE_OBJECT;

done:
    cJSON_Delete(js_root);

    return rv;
}