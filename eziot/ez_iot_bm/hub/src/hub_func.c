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
 * Brief:
 * 
 * 
 * Change Logs:
 * Date           Author       Notes
 * 2021-11-25    zhangdi29     
 *******************************************************************************/
#include <stdlib.h>
#include "ezos.h"
#include "hub_func.h"
#include "hub_extern.h"
#include "ez_iot_hub.h"
#include "cJSON.h"
#include "mbedtls/md5.h"
#include "misc.h"
#include "eztimer.h"
#include "ezlog.h"
#include "misc.h"
#include "eztimer.h"


#define SAP_DEV_SN_LEN 9
#define SUBLIST_JSON_KEY_STA "sta"
#define SUBLIST_JSON_KEY_SN "sn"
#define SUBLIST_JSON_KEY_VER "ver"
#define SUBLIST_JSON_KEY_TYPE "type"
#define SUBLIST_JSON_KEY_ACCESS "access"
#define HUB_AUTH_SALT "www.88075998.com"
#define EZIOT_HUB_LIST_SIZE 1024 * 8

typedef struct
{
    ez_bool_t connected;
    ez_char_t childdevid[64];                ///< 子设备序列号
    ez_char_t version[64];                   ///< 子设备版本号
    ez_char_t type[64];                      ///< 子设备型号
    ez_char_t FirmwareIdentificationCode[1]; ///< 固件识别码
} hub_subdev_info_report_t;

static ez_hub_callbacks_t g_phub_cbs = {0};

static void *g_hlock = NULL;

static ez_int32_t g_auth_timer = 0;

static ez_int32_t g_unauth_count = 0;

static cJSON *subdev_to_json(void *struct_obj);

static void json_to_subdev(cJSON *json_obj, void *struct_obj);

static void subdev_to_relation_lst(cJSON *json_relation_lst, void *struct_obj);

static void subdev_to_status_lst(cJSON *json_status_lst, void *struct_obj);

static EZ_INT find_dev_by_sn(cJSON *json_obj, ez_char_t *sn);

static ez_hub_callbacks_t *hub_callbacks_get(void);

static void hub_subdev_auth_passed(ez_char_t *subdev_sn);

static void hub_subdev_auth_failure(ez_char_t *subdev_sn);

/**
 * @brief 子设备认证重试
 * 
 * @return EZ_INT 
 */
void auth_retry_timer_cb(void);

static ez_hub_callbacks_t *hub_callbacks_get(void)
{
    return &g_phub_cbs;
}

ez_int_t hub_func_init(const ez_hub_callbacks_t *phub_cbs)
{
    ezos_memcpy((void *)&g_phub_cbs, (const void *)phub_cbs, sizeof(g_phub_cbs));
    g_hlock = ezos_mutex_create();
    if (NULL == g_hlock)
    {
        return EZ_HUB_ERR_INTERNAL;
    }

    g_auth_timer = eztimer_create("auth_retry_timer", (2 * 1000 * 60), ez_false, auth_retry_timer_cb);
    if (g_auth_timer <= 0)
    {
        ezlog_e(TAG_AP, "start auth retry timer failed.");
    }

    return 0;
}

void hub_func_deinit()
{
    ezos_memset((void *)&g_phub_cbs, 0, sizeof(g_phub_cbs));
    eztimer_delete(g_auth_timer);

    ezos_mutex_destroy(g_hlock);
}

ez_err_t hub_add_do(const hub_subdev_info_internal_t *subdev_info)
{
    ez_err_t rv = EZ_HUB_ERR_SUCC;
    size_t length = EZIOT_HUB_LIST_SIZE;
    ez_char_t *pbuf = NULL;
    ez_char_t *pbuf_save = NULL;
    cJSON *js_root = NULL;
    cJSON *js_subdev = NULL;

    ezos_mutex_lock(g_hlock);

    CHECK_COND_DONE(ezos_kv_raw_get((const ez_char_t *)EZ_KV_DEFALUT_KEY_HUBLIST, NULL, &length), EZ_HUB_ERR_STORAGE);

    if (0 == length)
    {
        /* 子设备列表为空，创建列表 */
        js_root = cJSON_CreateArray();
        CHECK_COND_DONE(!js_root, EZ_HUB_ERR_MEMORY);
    }
    else
    {
        /* 防止重复添加 */
        pbuf = (ez_char_t *)malloc(length + 1);
        CHECK_COND_DONE(!pbuf, EZ_HUB_ERR_MEMORY);
        ezos_memset(pbuf, 0, length + 1);

        CHECK_COND_DONE(ezos_kv_raw_get((const ez_char_t *)EZ_KV_DEFALUT_KEY_HUBLIST, (ez_char_t *)pbuf, &length), EZ_HUB_ERR_STORAGE);
        CHECK_COND_DONE(!(js_root = cJSON_Parse(pbuf)), EZ_HUB_ERR_MEMORY);
        CHECK_COND_DONE(-1 != find_dev_by_sn(js_root, (ez_char_t *)subdev_info->sn), EZ_HUB_ERR_SUBDEV_EXISTED);
    }

    /* 子设备数量不能超上限 */
    CHECK_COND_DONE(cJSON_GetArraySize(js_root) >= COMPONENT_HUB_SUBLIST_MAX, EZ_HUB_ERR_OUT_OF_RANGE);

    CHECK_COND_DONE(!(js_subdev = subdev_to_json((void *)subdev_info)), EZ_HUB_ERR_MEMORY);
    cJSON_AddItemToArray(js_root, js_subdev);
    CHECK_COND_DONE(!(pbuf_save = cJSON_PrintUnformatted(js_root)), EZ_HUB_ERR_MEMORY);
    CHECK_COND_DONE(ezos_kv_raw_set((const ez_char_t *)EZ_KV_DEFALUT_KEY_HUBLIST, (ez_char_t *)pbuf_save, ezos_strlen(pbuf_save)), EZ_HUB_ERR_STORAGE);
    ezos_kv_print();
    g_unauth_count++;
    hub_subdev_auth_do((void *)subdev_info);

done:
    ezos_mutex_unlock(g_hlock);

    SAFE_FREE(pbuf);
    SAFE_FREE(pbuf_save);
    cJSON_Delete(js_root);

    if (EZ_HUB_ERR_SUCC != rv)
    {
        ezlog_e(TAG_HUB, "rv:%08x", rv);
    }

    return rv;
}

ez_err_t hub_del_do(const ez_char_t *subdev_sn)
{
    ez_err_t rv = EZ_HUB_ERR_SUCC;
    size_t length = EZIOT_HUB_LIST_SIZE;
    ez_char_t *pbuf = NULL;
    ez_char_t *pbuf_save = NULL;
    cJSON *js_root = NULL;
    EZ_INT index = -1;
    hub_subdev_info_internal_t subdev_info = {0};

    ezos_mutex_lock(g_hlock);

    CHECK_COND_DONE(ezos_kv_raw_get((const ez_char_t *)EZ_KV_DEFALUT_KEY_HUBLIST, NULL, &length), EZ_HUB_ERR_STORAGE);
    CHECK_COND_DONE(0 == length, EZ_HUB_ERR_SUBDEV_NOT_FOUND);

    CHECK_COND_DONE(!(pbuf = (ez_char_t *)malloc(length + 1)), EZ_HUB_ERR_MEMORY);
    ezos_memset(pbuf, 0, length + 1);

    CHECK_COND_DONE(ezos_kv_raw_get((const ez_char_t *)EZ_KV_DEFALUT_KEY_HUBLIST, (ez_char_t *)pbuf, &length), EZ_HUB_ERR_STORAGE);
    CHECK_COND_DONE(!(js_root = cJSON_Parse(pbuf)), EZ_HUB_ERR_MEMORY);
    CHECK_COND_DONE(-1 == (index = find_dev_by_sn(js_root, (ez_char_t *)subdev_sn)), EZ_HUB_ERR_SUBDEV_NOT_FOUND);

    cJSON *js_item = cJSON_GetArrayItem(js_root, index);
    CHECK_COND_DONE(!js_item, EZ_HUB_ERR_SUBDEV_NOT_FOUND);
    json_to_subdev(js_item, (void *)&subdev_info);

    cJSON_DeleteItemFromArray(js_root, index);
    CHECK_COND_DONE(!(pbuf_save = cJSON_PrintUnformatted(js_root)), EZ_HUB_ERR_MEMORY);
    CHECK_COND_DONE(ezos_kv_raw_set((const ez_char_t *)EZ_KV_DEFALUT_KEY_HUBLIST, (ez_char_t *)pbuf_save, ezos_strlen(pbuf_save)), EZ_HUB_ERR_STORAGE);

    if (0 == subdev_info.access)
    {
        g_unauth_count--;
        goto done;
    }

done:

    ezos_mutex_unlock(g_hlock);
    SAFE_FREE(pbuf);
    SAFE_FREE(pbuf_save);
    cJSON_Delete(js_root);

    if (EZ_HUB_ERR_SUCC == rv)
    {
        hub_subdev_list_report();
    }
    else
    {
        ezlog_e(TAG_HUB, "rv:%08x", rv);
    }

    return rv;
}

ez_err_t hub_ver_update_do(const ez_char_t *subdev_sn, const ez_char_t *subdev_ver)
{
    ez_err_t rv = EZ_HUB_ERR_SUCC;
    size_t length = EZIOT_HUB_LIST_SIZE;
    ez_char_t *pbuf = NULL;
    ez_char_t *pbuf_save = NULL;
    cJSON *js_root = NULL;
    EZ_INT index = -1;
    hub_subdev_info_internal_t subdev_info_old = {0};
    hub_subdev_info_internal_t subdev_info_new = {0};

    ezos_mutex_lock(g_hlock);

    CHECK_COND_DONE(ezos_kv_raw_get((const ez_char_t *)EZ_KV_DEFALUT_KEY_HUBLIST, NULL, &length), EZ_HUB_ERR_STORAGE);
    CHECK_COND_DONE(0 == length, EZ_HUB_ERR_SUBDEV_NOT_FOUND);

    CHECK_COND_DONE(!(pbuf = (ez_char_t *)malloc(length + 1)), EZ_HUB_ERR_MEMORY);
    ezos_memset(pbuf, 0, length + 1);

    CHECK_COND_DONE(ezos_kv_raw_get((const ez_char_t *)EZ_KV_DEFALUT_KEY_HUBLIST, (ez_char_t *)pbuf, &length), EZ_HUB_ERR_STORAGE);
    CHECK_COND_DONE(!(js_root = cJSON_Parse(pbuf)), EZ_HUB_ERR_MEMORY);
    CHECK_COND_DONE(-1 == (index = find_dev_by_sn(js_root, (ez_char_t *)subdev_sn)), EZ_HUB_ERR_SUBDEV_NOT_FOUND);

    cJSON *js_item = cJSON_GetArrayItem(js_root, index);
    CHECK_COND_DONE(!js_item, EZ_HUB_ERR_SUBDEV_NOT_FOUND);
    json_to_subdev(js_item, (void *)&subdev_info_old);

    cJSON_ReplaceItemInObject(js_item, SUBLIST_JSON_KEY_VER, cJSON_CreateString((ez_char_t *)subdev_ver));
    CHECK_COND_DONE(!(pbuf_save = cJSON_PrintUnformatted(js_root)), EZ_HUB_ERR_MEMORY);
    CHECK_COND_DONE(ezos_kv_raw_set((const ez_char_t *)EZ_KV_DEFALUT_KEY_HUBLIST, (ez_char_t *)pbuf_save, ezos_strlen(pbuf_save)), EZ_HUB_ERR_STORAGE);
    json_to_subdev(js_item, (void *)&subdev_info_new);

done:

    ezos_mutex_unlock(g_hlock);
    SAFE_FREE(pbuf);
    SAFE_FREE(pbuf_save);
    cJSON_Delete(js_root);

    if (EZ_HUB_ERR_SUCC == rv)
    {
        hub_subdev_list_report();
    }
    else
    {
        ezlog_e(TAG_HUB, "rv:%08x", rv);
    }

    return rv;
}

ez_err_t hub_status_update_do(const ez_char_t *subdev_sn, ez_bool_t online)
{
    ez_err_t rv = EZ_HUB_ERR_SUCC;
    size_t length = EZIOT_HUB_LIST_SIZE;
    ez_char_t *pbuf = NULL;
    ez_char_t *pbuf_save = NULL;
    cJSON *js_root = NULL;
    EZ_INT index = -1;

    ezos_mutex_lock(g_hlock);

    CHECK_COND_DONE(ezos_kv_raw_get((const ez_char_t *)EZ_KV_DEFALUT_KEY_HUBLIST, NULL, &length), EZ_HUB_ERR_STORAGE);
    CHECK_COND_DONE(0 == length, EZ_HUB_ERR_SUBDEV_NOT_FOUND);

    CHECK_COND_DONE(!(pbuf = (ez_char_t *)malloc(length + 1)), EZ_HUB_ERR_MEMORY);
    ezos_memset(pbuf, 0, length + 1);

    CHECK_COND_DONE(ezos_kv_raw_get((const ez_char_t *)EZ_KV_DEFALUT_KEY_HUBLIST, (ez_char_t *)pbuf, &length), EZ_HUB_ERR_STORAGE);
    CHECK_COND_DONE(!(js_root = cJSON_Parse(pbuf)), EZ_HUB_ERR_MEMORY);
    CHECK_COND_DONE(-1 == (index = find_dev_by_sn(js_root, (ez_char_t *)subdev_sn)), EZ_HUB_ERR_SUBDEV_NOT_FOUND);

    cJSON *js_item = cJSON_GetArrayItem(js_root, index);
    CHECK_COND_DONE(!js_item, EZ_HUB_ERR_SUBDEV_NOT_FOUND);

    cJSON_ReplaceItemInObject(js_item, SUBLIST_JSON_KEY_STA, cJSON_CreateBool((cJSON_bool)online));
    CHECK_COND_DONE(!(pbuf_save = cJSON_PrintUnformatted(js_root)), EZ_HUB_ERR_MEMORY);
    CHECK_COND_DONE(ezos_kv_raw_set((const ez_char_t *)EZ_KV_DEFALUT_KEY_HUBLIST, (ez_char_t *)pbuf_save, ezos_strlen(pbuf_save)), EZ_HUB_ERR_STORAGE);
done:

    ezos_mutex_unlock(g_hlock);
    SAFE_FREE(pbuf);
    SAFE_FREE(pbuf_save);
    cJSON_Delete(js_root);

    if (EZ_HUB_ERR_SUCC == rv)
    {
        hub_subdev_sta_report();
    }
    else
    {
        ezlog_e(TAG_HUB, "rv:%08x", rv);
    }

    return rv;
}

ez_err_t hub_subdev_query(const ez_char_t *subdev_sn, hub_subdev_info_internal_t *subdev_info)
{
    ez_err_t rv = EZ_HUB_ERR_SUCC;
    size_t length = EZIOT_HUB_LIST_SIZE;
    ez_char_t *pbuf = NULL;
    ez_char_t *pbuf_save = NULL;
    cJSON *js_root = NULL;
    EZ_INT index = -1;

    ezos_mutex_lock(g_hlock);

    CHECK_COND_DONE(ezos_kv_raw_get((const ez_char_t *)EZ_KV_DEFALUT_KEY_HUBLIST, NULL, &length), EZ_HUB_ERR_STORAGE);
    CHECK_COND_DONE(0 == length, EZ_HUB_ERR_SUBDEV_NOT_FOUND);
    CHECK_COND_DONE(!(pbuf = (ez_char_t *)malloc(length + 1)), EZ_HUB_ERR_MEMORY);
    ezos_memset(pbuf, 0, length + 1);

    CHECK_COND_DONE(ezos_kv_raw_get((const ez_char_t *)EZ_KV_DEFALUT_KEY_HUBLIST, (ez_char_t *)pbuf, &length), EZ_HUB_ERR_STORAGE);
    CHECK_COND_DONE(!(js_root = cJSON_Parse(pbuf)), EZ_HUB_ERR_MEMORY);

    index = find_dev_by_sn(js_root, (ez_char_t *)subdev_sn);
    if (-1 == index)
    {
        rv = EZ_HUB_ERR_SUBDEV_NOT_FOUND;
        goto done;
    }

    cJSON *js_item = cJSON_GetArrayItem(js_root, index);
    CHECK_COND_DONE(!js_item, EZ_HUB_ERR_SUBDEV_NOT_FOUND);
    json_to_subdev(js_item, (void *)subdev_info);

done:
    ezos_mutex_unlock(g_hlock);

    SAFE_FREE(pbuf);
    SAFE_FREE(pbuf_save);
    cJSON_Delete(js_root);

    return rv;
}

ez_err_t hub_subdev_next(hub_subdev_info_internal_t *subdev_info)
{
    ez_err_t rv = EZ_HUB_ERR_SUCC;
    size_t length = EZIOT_HUB_LIST_SIZE;
    ez_char_t *pbuf = NULL;
    ez_char_t *pbuf_save = NULL;
    cJSON *js_root = NULL;
    EZ_INT index = -1;

    ezos_mutex_lock(g_hlock);

    CHECK_COND_DONE(ezos_kv_raw_get((const ez_char_t *)EZ_KV_DEFALUT_KEY_HUBLIST, NULL, &length), EZ_HUB_ERR_STORAGE);
    CHECK_COND_DONE(0 == length, EZ_HUB_ERR_ENUM_END);

    CHECK_COND_DONE(!(pbuf = (ez_char_t *)malloc(length + 1)), EZ_HUB_ERR_MEMORY);
    ezos_memset(pbuf, 0, length + 1);

    CHECK_COND_DONE(ezos_kv_raw_get((const ez_char_t *)EZ_KV_DEFALUT_KEY_HUBLIST, (ez_char_t *)pbuf, &length), EZ_HUB_ERR_STORAGE);
    CHECK_COND_DONE(!(js_root = cJSON_Parse(pbuf)), EZ_HUB_ERR_MEMORY);

    if (0 == ezos_strlen((ez_char_t *)subdev_info->sn))
    {
        index = find_dev_by_sn(js_root, NULL);
        if (-1 == index)
        {
            rv = EZ_HUB_ERR_ENUM_END;
            goto done;
        }
    }
    else
    {
        index = find_dev_by_sn(js_root, (ez_char_t *)subdev_info->sn);
        if (-1 == index || cJSON_GetArraySize(js_root) <= index + 1)
        {
            rv = EZ_HUB_ERR_ENUM_END;
            goto done;
        }

        index++;
    }

    cJSON *js_item = cJSON_GetArrayItem(js_root, index);
    CHECK_COND_DONE(!js_item, EZ_HUB_ERR_SUBDEV_NOT_FOUND);
    json_to_subdev(js_item, (void *)subdev_info);

done:
    ezos_mutex_unlock(g_hlock);

    SAFE_FREE(pbuf);
    SAFE_FREE(pbuf_save);
    cJSON_Delete(js_root);

    if (EZ_HUB_ERR_SUCC != rv && EZ_HUB_ERR_ENUM_END != rv)
    {
        ezlog_e(TAG_HUB, "rv:%08x", rv);
    }

    return rv;
}

ez_err_t hub_clean_do(void)
{
    int32_t rv = EZ_HUB_ERR_SUCC;

    ezos_mutex_lock(g_hlock);
    CHECK_COND_DONE(ezos_kv_raw_set((const ez_char_t *)EZ_KV_DEFALUT_KEY_HUBLIST, (ez_char_t *)"", 0), EZ_HUB_ERR_STORAGE);
    ezos_mutex_unlock(g_hlock);

    hub_subdev_list_report();

done:
    return rv;
}

static EZ_INT find_dev_by_sn(cJSON *json_obj, ez_char_t *sn)
{
    EZ_INT index = -1;

    for (EZ_INT i = 0; i < cJSON_GetArraySize(json_obj); i++)
    {
        cJSON *js_item = cJSON_GetArrayItem(json_obj, i);
        if (NULL == js_item)
        {
            continue;
        }

        cJSON *js_sn = cJSON_GetObjectItem(js_item, SUBLIST_JSON_KEY_SN);
        if (NULL == js_sn)
        {
            continue;
        }

        if (NULL == sn || 0 == ezos_strcmp(js_sn->valuestring, (ez_char_t *)sn))
        {
            index = i;
            break;
        }
    }

    return index;
}

EZ_INT hub_subdev_list_report(void)
{
    EZ_INT rv = 0;
    size_t length = EZIOT_HUB_LIST_SIZE;
    ez_char_t *pbuf = NULL;
    ez_char_t *pbuf_report = NULL;
    cJSON *js_root = NULL;
    hub_subdev_info_internal_t subdev_info = {0};
    cJSON *subdev_lst = cJSON_CreateArray();

    ezos_mutex_lock(g_hlock);

    CHECK_COND_DONE(!subdev_lst, EZ_HUB_ERR_MEMORY);
    CHECK_COND_DONE(ezos_kv_raw_get((const ez_char_t *)EZ_KV_DEFALUT_KEY_HUBLIST, NULL, &length), EZ_HUB_ERR_STORAGE);

    if (0 == length)
    {
        CHECK_COND_DONE(hub_send_msg_to_platform("{}", ezos_strlen("{}"), kPu2CenPltHubReportRelationShipReq, EZDEVSDK_HUB_REQ, 0), EZ_HUB_ERR_INTERNAL);
        goto done;
    }

    CHECK_COND_DONE(!(pbuf = (ez_char_t *)malloc(length + 1)), EZ_HUB_ERR_MEMORY);
    ezos_memset(pbuf, 0, length + 1);

    CHECK_COND_DONE(ezos_kv_raw_get((const ez_char_t *)EZ_KV_DEFALUT_KEY_HUBLIST, (ez_char_t *)pbuf, &length), EZ_HUB_ERR_STORAGE);
    CHECK_COND_DONE(!(js_root = cJSON_Parse(pbuf)), EZ_HUB_ERR_MEMORY);
    ezlog_i(TAG_HUB, "buf:%s", pbuf);
    SAFE_FREE(pbuf);

    for (size_t i = 0; i < cJSON_GetArraySize(js_root); i++)
    {
        cJSON *js_item = cJSON_GetArrayItem(js_root, i);
        ezos_memset((void *)&subdev_info, 0, sizeof(subdev_info));
        json_to_subdev(js_item, (void *)&subdev_info);
        if (0 == subdev_info.access)
        {
            continue;
        }

        subdev_to_relation_lst(subdev_lst, (void *)&subdev_info);
    }

    CHECK_COND_DONE(!(pbuf_report = cJSON_PrintUnformatted(subdev_lst)), EZ_HUB_ERR_MEMORY);
    CHECK_COND_DONE(hub_send_msg_to_platform(pbuf_report, ezos_strlen(pbuf_report), kPu2CenPltHubReportRelationShipReq, EZDEVSDK_HUB_REQ, 0), EZ_HUB_ERR_INTERNAL);

done:
    ezos_mutex_unlock(g_hlock);

    SAFE_FREE(pbuf);
    SAFE_FREE(pbuf_report);
    cJSON_Delete(js_root);
    cJSON_Delete(subdev_lst);

    if (EZ_HUB_ERR_SUCC != rv)
    {
        ezlog_e(TAG_HUB, "rv:%08x", rv);
    }

    return rv;
}

EZ_INT hub_subdev_sta_report(void)
{
    EZ_INT rv = 0;
    size_t length = EZIOT_HUB_LIST_SIZE;
    ez_char_t *pbuf = NULL;
    ez_char_t *pbuf_report = NULL;
    cJSON *js_root = NULL;
    hub_subdev_info_internal_t subdev_info = {0};
    cJSON *subdev_lst = cJSON_CreateArray();

    ezos_mutex_lock(g_hlock);

    CHECK_COND_DONE(!subdev_lst, EZ_HUB_ERR_MEMORY);
    CHECK_COND_DONE(ezos_kv_raw_get((const ez_char_t *)EZ_KV_DEFALUT_KEY_HUBLIST, NULL, &length), EZ_HUB_ERR_STORAGE);
    CHECK_COND_DONE(0 == length, EZ_HUB_ERR_SUBDEV_NOT_FOUND);

    CHECK_COND_DONE(!(pbuf = (ez_char_t *)malloc(length + 1)), EZ_HUB_ERR_MEMORY);
    ezos_memset(pbuf, 0, length + 1);

    CHECK_COND_DONE(ezos_kv_raw_get((const ez_char_t *)EZ_KV_DEFALUT_KEY_HUBLIST, (ez_char_t *)pbuf, &length), EZ_HUB_ERR_STORAGE);
    CHECK_COND_DONE(!(js_root = cJSON_Parse(pbuf)), EZ_HUB_ERR_MEMORY);
    SAFE_FREE(pbuf);

    for (size_t i = 0; i < cJSON_GetArraySize(js_root); i++)
    {
        cJSON *js_item = cJSON_GetArrayItem(js_root, i);
        ezos_memset((void *)&subdev_info, 0, sizeof(subdev_info));
        json_to_subdev(js_item, (void *)&subdev_info);
        subdev_to_status_lst(subdev_lst, (void *)&subdev_info);
    }

    CHECK_COND_DONE(!(pbuf_report = cJSON_PrintUnformatted(subdev_lst)), EZ_HUB_ERR_MEMORY);
    CHECK_COND_DONE(hub_send_msg_to_platform(pbuf_report, ezos_strlen(pbuf_report), kPu2CenPltHubReportOnlineStatusReq, EZDEVSDK_HUB_REQ, 0), EZ_HUB_ERR_INTERNAL);

done:
    ezos_mutex_unlock(g_hlock);

    SAFE_FREE(pbuf);
    SAFE_FREE(pbuf_report);
    cJSON_Delete(js_root);
    cJSON_Delete(subdev_lst);

    if (EZ_HUB_ERR_SUCC != rv)
    {
        ezlog_e(TAG_HUB, "rv:%08x", rv);
    }

    return rv;
}

ez_int_t hub_subdev_auth_do(void *subdev_info)
{
    hub_subdev_info_internal_t *_subdev_info = (hub_subdev_info_internal_t *)subdev_info;
    ez_int_t rv = -1;
    ez_char_t sn[64] = {0};
    ez_char_t MAC[32 + 1] = {0};
    ez_char_t md5[16] = {0};
    ez_char_t *pbuf_report = NULL;
    mbedtls_md5_context md5_ctx;

    cJSON *auth_info = cJSON_CreateObject();
    CHECK_COND_RETURN(!auth_info, EZ_HUB_ERR_MEMORY);

    if (0 == _subdev_info->authm)
        ezos_strncpy(sn, _subdev_info->sn, sizeof(sn) - 1);
    else
        ezos_snprintf(sn, sizeof(sn) - 1, "%s:%s", _subdev_info->type, _subdev_info->sn);

    {
        //生成MAC方法：md5(md5(md5(verifycode+childserial) + www.88075998.com))

        //首次MAC
        mbedtls_md5_init(&md5_ctx);
        mbedtls_md5_starts(&md5_ctx);
        mbedtls_md5_update(&md5_ctx, (unsigned char *)_subdev_info->vcode, ezos_strlen(_subdev_info->vcode));
        mbedtls_md5_update(&md5_ctx, (unsigned char *)sn, ezos_strlen(sn));
        mbedtls_md5_finish(&md5_ctx, (unsigned char*)md5);
        mbedtls_md5_free(&md5_ctx);
        bin2hexstr((unsigned char *)md5, 16, 1, (unsigned char *)MAC);

        //第二次MAC
        mbedtls_md5_init(&md5_ctx);
        mbedtls_md5_starts(&md5_ctx);
        mbedtls_md5_update(&md5_ctx, (unsigned char *)MAC, ezos_strlen(MAC));
        mbedtls_md5_update(&md5_ctx, (unsigned char *)HUB_AUTH_SALT, ezos_strlen(HUB_AUTH_SALT));
        mbedtls_md5_finish(&md5_ctx, (unsigned char *)md5);
        mbedtls_md5_free(&md5_ctx);
        bin2hexstr((unsigned char *)md5, 16, 1, (unsigned char *)MAC);

        //计算摘要值
        mbedtls_md5((unsigned char *)MAC, ezos_strlen(MAC), (unsigned char *)md5);
        bin2hexstr((unsigned char *)md5, 16, 1, (unsigned char *)MAC);
    }

    cJSON_AddStringToObject(auth_info, "childserial", sn);
    cJSON_AddStringToObject(auth_info, "childverifycode", MAC);
    cJSON_AddNumberToObject(auth_info, "devAccessMode", _subdev_info->authm);
    CHECK_COND_DONE(!(pbuf_report = cJSON_PrintUnformatted(auth_info)), EZ_HUB_ERR_MEMORY);

    rv = hub_send_msg_to_platform(pbuf_report, ezos_strlen(pbuf_report), kPu2CenPltHubAuthChildDeviceReq, EZDEVSDK_HUB_REQ, 0);

done:
    SAFE_FREE(pbuf_report);
    cJSON_Delete(auth_info);

    return rv;
}

void hub_subdev_auth_done(void *buf, EZ_INT len)
{
    cJSON *js_root = NULL;
    cJSON *js_sn;
    cJSON *js_result;
    ez_char_t sn[16 + 1];

    if (NULL == buf || 0 == len)
    {
        ezlog_e(TAG_HUB, "PDU invaild!");
        goto done;
    }

    // {"childserial":"4LYV8SK7UKLBOUOVS6HXVX:ARZCWQSRG3CW","result":1}
    if (NULL == (js_root = cJSON_Parse(buf)))
    {
        ezlog_e(TAG_HUB, "PDU invaild, json format!");
        goto done;
    }

    js_sn = cJSON_GetObjectItem(js_root, "childserial");
    js_result = cJSON_GetObjectItem(js_root, "result");

    if (NULL == js_sn || NULL == js_result ||
        cJSON_String != js_sn->type || cJSON_Number != js_result->type)
    {
        ezlog_e(TAG_HUB, "PDU invaild, obj!");
        goto done;
    }

    if (SAP_DEV_SN_LEN == ezos_strlen(js_sn->valuestring))
    {
        ezos_strncpy(sn, js_sn->valuestring, sizeof(sn) - 1);
    }
    else
    {
        /* license类型设备需要截取deviceName */
        ezos_sscanf(js_sn->valuestring, "%*[^:]:%s", sn);
    }

    ezlog_i(TAG_HUB, "sn:%s", sn);
    switch (js_result->valueint)
    {
    case 0:
        hub_subdev_auth_passed(sn);
        break;
    case 1:
        hub_subdev_auth_failure(sn);
        break;
    default:
        ezlog_e(TAG_HUB, "Server internal err");
        break;
    }

done:
    CJSON_SAFE_DELETE(js_root);
}

static cJSON *subdev_to_json(void *struct_obj)
{
    hub_subdev_info_internal_t *subdev_obj = (hub_subdev_info_internal_t *)struct_obj;
    cJSON *subdev_json = cJSON_CreateObject();
    CHECK_COND_RETURN(!subdev_obj, NULL);

    S2J_JSON_SET_int_ELEMENT(subdev_json, subdev_obj, sta);
    S2J_JSON_SET_int_ELEMENT(subdev_json, subdev_obj, authm);
    S2J_JSON_SET_string_ELEMENT(subdev_json, subdev_obj, type);
    S2J_JSON_SET_string_ELEMENT(subdev_json, subdev_obj, sn);
    S2J_JSON_SET_string_ELEMENT(subdev_json, subdev_obj, vcode);
    S2J_JSON_SET_string_ELEMENT(subdev_json, subdev_obj, ver);
    S2J_JSON_SET_string_ELEMENT(subdev_json, subdev_obj, uuid);
    S2J_JSON_SET_int_ELEMENT(subdev_json, subdev_obj, access);

    return subdev_json;
}

static void json_to_subdev(cJSON *json_obj, void *struct_obj)
{
    hub_subdev_info_internal_t *struct_obj_internal = (hub_subdev_info_internal_t *)struct_obj;
    cJSON *json_temp = NULL;

    json_temp = cJSON_GetObjectItem(json_obj, "sta");
    if (json_temp)
        (struct_obj_internal)->sta = json_temp->valueint;
    else
        (struct_obj_internal)->sta = 0;

    json_temp = cJSON_GetObjectItem(json_obj, "authm");
    if (json_temp)
        (struct_obj_internal)->authm = json_temp->valueint;
    else
        (struct_obj_internal)->authm = 0;

    json_temp = cJSON_GetObjectItem(json_obj, "type");
    if (json_temp)
        ezos_strncpy((struct_obj_internal)->type, json_temp->valuestring, sizeof((struct_obj_internal)->type) - 1);
    else
        ezos_strncpy((struct_obj_internal)->type, "", sizeof((struct_obj_internal)->type) - 1);

    json_temp = cJSON_GetObjectItem(json_obj, "sn");
    if (json_temp)
        ezos_strncpy((struct_obj_internal)->sn, json_temp->valuestring, sizeof((struct_obj_internal)->sn) - 1);
    else
        ezos_strncpy((struct_obj_internal)->sn, "", sizeof((struct_obj_internal)->sn) - 1);

    json_temp = cJSON_GetObjectItem(json_obj, "vcode");
    if (json_temp)
        ezos_strncpy((struct_obj_internal)->vcode, json_temp->valuestring, sizeof((struct_obj_internal)->vcode) - 1);
    else
        ezos_strncpy((struct_obj_internal)->vcode, "", sizeof((struct_obj_internal)->vcode) - 1);

    json_temp = cJSON_GetObjectItem(json_obj, "ver");
    if (json_temp)
        ezos_strncpy((struct_obj_internal)->ver, json_temp->valuestring, sizeof((struct_obj_internal)->ver) - 1);
    else
        ezos_strncpy((struct_obj_internal)->ver, "", sizeof((struct_obj_internal)->ver) - 1);

    json_temp = cJSON_GetObjectItem(json_obj, "uuid");
    if (json_temp)
        ezos_strncpy((struct_obj_internal)->uuid, json_temp->valuestring, sizeof((struct_obj_internal)->uuid) - 1);
    else
        ezos_strncpy((struct_obj_internal)->uuid, "", sizeof((struct_obj_internal)->uuid) - 1);

    json_temp = cJSON_GetObjectItem(json_obj, "access");
    if (json_temp)
        (struct_obj_internal)->access = json_temp->valueint;
    else
        (struct_obj_internal)->access = 0;
}

static void subdev_to_relation_lst(cJSON *json_relation_lst, void *struct_obj)
{
    hub_subdev_info_internal_t *struct_obj_internal = (hub_subdev_info_internal_t *)struct_obj;
    hub_subdev_info_report_t struct_obj_report = {0};

    cJSON *subdev_json = cJSON_CreateObject();
    struct_obj_report.connected = struct_obj_internal->sta;
    ezos_strncpy(struct_obj_report.childdevid, struct_obj_internal->sn, sizeof(struct_obj_report.childdevid) - 1);
    ezos_strncpy(struct_obj_report.type, struct_obj_internal->type, sizeof(struct_obj_report.childdevid) - 1);
    ezos_strncpy(struct_obj_report.version, struct_obj_internal->ver, sizeof(struct_obj_report.childdevid) - 1);

    S2J_JSON_SET_int_ELEMENT(subdev_json, &struct_obj_report, connected);
    S2J_JSON_SET_string_ELEMENT(subdev_json, &struct_obj_report, childdevid);
    S2J_JSON_SET_string_ELEMENT(subdev_json, &struct_obj_report, version);
    S2J_JSON_SET_string_ELEMENT(subdev_json, &struct_obj_report, type);
    S2J_JSON_SET_string_ELEMENT(subdev_json, &struct_obj_report, FirmwareIdentificationCode);

    cJSON_AddItemToArray(json_relation_lst, subdev_json);
}

static void subdev_to_status_lst(cJSON *json_status_lst, void *struct_obj)
{
    hub_subdev_info_internal_t *struct_obj_internal = (hub_subdev_info_internal_t *)struct_obj;
    hub_subdev_info_report_t struct_obj_report = {0};

    cJSON *subdev_json = cJSON_CreateObject();
    struct_obj_report.connected = struct_obj_internal->sta;
    ezos_strncpy(struct_obj_report.childdevid, struct_obj_internal->sn, sizeof(struct_obj_report.childdevid) - 1);
    ezos_strncpy(struct_obj_report.type, struct_obj_internal->type, sizeof(struct_obj_report.childdevid) - 1);
    ezos_strncpy(struct_obj_report.version, struct_obj_internal->ver, sizeof(struct_obj_report.childdevid) - 1);

    s2j_json_set_basic_element(subdev_json, &struct_obj_report, int, connected);
    s2j_json_set_basic_element(subdev_json, &struct_obj_report, string, childdevid);

    cJSON_AddItemToArray(json_status_lst, subdev_json);
}

static void hub_subdev_auth_passed(ez_char_t *subdev_sn)
{
    ezlog_w(TAG_HUB, "auth_passed");

    ez_err_t rv = EZ_HUB_ERR_SUCC;
    size_t length = EZIOT_HUB_LIST_SIZE;
    ez_char_t *pbuf = NULL;
    ez_char_t *pbuf_save = NULL;
    cJSON *js_root = NULL;
    EZ_INT index = -1;
    hub_subdev_info_internal_t subdev_info = {0};

    ezos_mutex_lock(g_hlock);

    CHECK_COND_DONE(ezos_kv_raw_get((const ez_char_t *)EZ_KV_DEFALUT_KEY_HUBLIST, NULL, &length), EZ_HUB_ERR_STORAGE);
    CHECK_COND_DONE(0 == length, EZ_HUB_ERR_SUBDEV_NOT_FOUND);

    CHECK_COND_DONE(!(pbuf = (ez_char_t *)malloc(length + 1)), EZ_HUB_ERR_MEMORY);
    ezos_memset(pbuf, 0, length + 1);

    CHECK_COND_DONE(ezos_kv_raw_get((const ez_char_t *)EZ_KV_DEFALUT_KEY_HUBLIST, (ez_char_t *)pbuf, &length), EZ_HUB_ERR_STORAGE);

    CHECK_COND_DONE(!(js_root = cJSON_Parse(pbuf)), EZ_HUB_ERR_MEMORY);
    CHECK_COND_DONE(-1 == (index = find_dev_by_sn(js_root, (ez_char_t *)subdev_sn)), EZ_HUB_ERR_SUBDEV_NOT_FOUND);

    cJSON *js_item = cJSON_GetArrayItem(js_root, index);
    CHECK_COND_DONE(!js_item, EZ_HUB_ERR_SUBDEV_NOT_FOUND);
    json_to_subdev(js_item, (void *)&subdev_info);

    cJSON_ReplaceItemInObject(js_item, SUBLIST_JSON_KEY_ACCESS, cJSON_CreateNumber(1));
    CHECK_COND_DONE(!(pbuf_save = cJSON_PrintUnformatted(js_root)), EZ_HUB_ERR_MEMORY);
    CHECK_COND_DONE(ezos_kv_raw_set((const ez_char_t *)EZ_KV_DEFALUT_KEY_HUBLIST, (ez_char_t *)pbuf_save, ezos_strlen(pbuf_save)), EZ_HUB_ERR_STORAGE);

    hub_callbacks_get()->recv_event(EZ_EVENT_SUBDEV_ADD_SUCC, (void *)subdev_info.sn, ezos_strlen(subdev_info.sn));

done:
    ezos_mutex_unlock(g_hlock);

    SAFE_FREE(pbuf);
    SAFE_FREE(pbuf_save);
    cJSON_Delete(js_root);

    if (EZ_HUB_ERR_SUCC == rv)
    {
        hub_subdev_list_report();
    }
    else
    {
        ezlog_e(TAG_HUB, "rv:%08x", rv);
    }
}

static void hub_subdev_auth_failure(ez_char_t *subdev_sn)
{
    ezlog_w(TAG_HUB, "auth_failure");
    hub_subdev_info_internal_t subdev_info = {0};

    hub_subdev_query(subdev_sn, &subdev_info);
    hub_callbacks_get()->recv_event(EZ_EVENT_SUBDEV_ADD_FAIL, (void *)&subdev_info, sizeof(ez_subdev_info_t));
    hub_del_do(subdev_sn);
}

void auth_retry_timer_cb(void)
{
    ezlog_d(TAG_HUB, "auth retry cb in");
    ez_err_t rv = 0;
    hub_subdev_info_internal_t subdev_info = {0};

    CHECK_COND_DONE(!(0 < g_unauth_count), 0);
    ezlog_i(TAG_HUB, "count:%d", g_unauth_count);

    do
    {
        rv = hub_subdev_next(&subdev_info);
        if (EZ_HUB_ERR_ENUM_END == rv)
        {
            ezlog_v(TAG_HUB, "auth retry, hub enum end!");
            rv = EZ_HUB_ERR_SUCC;
            break;
        }
        else if (EZ_HUB_ERR_SUCC != rv)
        {
            ezlog_e(TAG_HUB, "err occur in subdev enum, rv = 0x%08x", rv);
            break;
        }

        if (1 == subdev_info.access)
        {
            continue;
        }

        if (hub_subdev_auth_do((void *)&subdev_info))
        {
            ezlog_e(TAG_HUB, "auth do, err occur");
        }
    } while (0 == rv);

done:
    ezlog_d(TAG_HUB, "auth retry cb out");
    return;
}