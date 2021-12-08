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
 * Chentengfei (xurongjun@ezvizlife.com)
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-12-04     xurongjun    first version 
 *******************************************************************************/

#include "tsl_def.h"
#include "tsl_storage.h"
#include "cJSON.h"
#include "ezlog.h"
#include "s2j.h"
#include "mbedtls/md5.h"
#include "mbedtls/base64.h"

#define TSLMAP_JSON_KEY_SN "sn"
#define TSLMAP_JSON_KEY_HANDLE "handle"
#define EZ_TSL_KEY_TSL_PREFIX "tslpf"

typedef struct
{
    ez_char_t sn[48];
    ez_char_t handle[32];
} tslmap_metadata_t;

/* private function */
static void storage_devinfo2index(ez_char_t *dev_sn, ez_char_t *dev_type, ez_char_t *dev_fwver, ez_char_t index[32]);
static ez_int32_t get_tsl_handle_count(cJSON *json_obj, ez_char_t *handle);
static cJSON *tslmap_metadata_to_json(tslmap_metadata_t *struct_obj);
static void json_to_tslmap_metadata(cJSON *json_obj, tslmap_metadata_t *struct_obj);
static ez_int32_t find_dev_by_sn(cJSON *json_obj, ez_char_t *sn);

/* private member */

ez_void_t tsl_storage_print(ez_void_t)
{
    ezos_kv_print();
}

ez_err_t tsl_storage_load(ez_char_t *dev_sn, ez_char_t **buf, ez_uint32_t *len)
{
    size_t length = CONFIG_EZIOT_TSL_PROFILE_SIZE;
    ez_err_t rv = EZ_KV_ERR_SUCC;
    tslmap_metadata_t tsl_metadata = {0};
    size_t tslmap_len = CONFIG_EZIOT_TSL_PROFILE_MAP_SIZE;
    ez_char_t *pbuf = NULL;
    cJSON *js_root = NULL;
    ez_int32_t index = -1;

    ezos_kv_print();
    CHECK_RV_DONE(ezos_kv_raw_get(EZ_KV_DEFALUT_KEY_TSLMAP, NULL, &tslmap_len));
    CHECK_COND_DONE(0 == tslmap_len, EZ_KV_ERR_NAME);

    pbuf = ezos_malloc(tslmap_len + 1);
    CHECK_COND_DONE(!pbuf, -1);

    ezos_memset(pbuf, 0, tslmap_len + 1);
    CHECK_RV_DONE(ezos_kv_raw_get(EZ_KV_DEFALUT_KEY_TSLMAP, pbuf, &tslmap_len));
    CHECK_COND_DONE(!(js_root = cJSON_Parse(pbuf)), -1);
    SAFE_FREE(pbuf);

    CHECK_COND_DONE(-1 == (index = find_dev_by_sn(js_root, dev_sn)), EZ_KV_ERR_NAME);

    cJSON *js_item = cJSON_GetArrayItem(js_root, index);
    json_to_tslmap_metadata(js_item, &tsl_metadata);

    CHECK_COND_DONE(ezos_kv_raw_get(tsl_metadata.handle, NULL, &length), EZ_KV_ERR_NAME);

    pbuf = ezos_malloc(length + 1);
    CHECK_COND_DONE(!pbuf, -1);

    ezos_memset(pbuf, 0, length + 1);
    CHECK_COND_DONE(ezos_kv_raw_get(tsl_metadata.handle, pbuf, &length), EZ_KV_ERR_NAME);
    ezlog_d(TAG_TSL, "length: %zu, read_buf: %s, ", length, pbuf);

    *buf = pbuf;
    *len = length;
    pbuf = NULL;
done:
    SAFE_FREE(pbuf);
    cJSON_Delete(js_root);

    return rv;
}

ez_err_t tsl_storage_save(ez_char_t *dev_sn, ez_char_t *dev_type, ez_char_t *dev_fwver, ez_char_t *buf, ez_int32_t len)
{
    ez_err_t rv = EZ_KV_ERR_SUCC;
    cJSON *js_root = NULL;
    cJSON *js_tslmap_item = NULL;
    ez_char_t *pbuf = NULL;
    ez_char_t *pbuf_save = NULL;
    ez_char_t handle_curr[32] = {0};
    ez_int32_t index = -1;
    tslmap_metadata_t tsl_metadata = {0};
    size_t tslmap_len = 0;

    CHECK_COND_DONE(len > CONFIG_EZIOT_TSL_PROFILE_SIZE, EZ_KV_ERR_SAVED_FULL);
    storage_devinfo2index(dev_sn, dev_type, dev_fwver, handle_curr);
    ezos_kv_raw_get(EZ_KV_DEFALUT_KEY_TSLMAP, NULL, &tslmap_len);

    if (0 == tslmap_len)
    {
        ezlog_w(TAG_TSL, "tslmap null, try clear all!");
        ezos_kv_del_by_prefix(EZ_TSL_KEY_TSL_PREFIX);
        js_root = cJSON_CreateArray();
        CHECK_COND_DONE(!js_root, -1);
    }
    else
    {
        pbuf = ezos_malloc(tslmap_len + 1);
        CHECK_COND_DONE(!js_root, -1);
        ezos_memset(pbuf, 0, tslmap_len + 1);

        CHECK_COND_DONE(ezos_kv_raw_get(EZ_KV_DEFALUT_KEY_TSLMAP, pbuf, &tslmap_len), -1);
        CHECK_COND_DONE(!(js_root = cJSON_Parse(pbuf)), -1);
        index = find_dev_by_sn(js_root, dev_sn);
    }

    do
    {
        if (-1 == index)
        {
            /* 新增设备，tslmap增加元数据 */
            ezlog_d(TAG_TSL, "add handle 2 tslmap:%s", handle_curr);
            ezos_strncpy(tsl_metadata.sn, dev_sn, sizeof(tsl_metadata.sn) - 1);
            ezos_strncpy(tsl_metadata.handle, handle_curr, sizeof(tsl_metadata.handle) - 1);

            CHECK_COND_DONE(!(js_tslmap_item = tslmap_metadata_to_json(&tsl_metadata)), -1);
            cJSON_AddItemToArray(js_root, js_tslmap_item);
            CHECK_COND_DONE(!(pbuf_save = cJSON_PrintUnformatted(js_root)), -1);
            CHECK_COND_DONE(ezos_strlen(pbuf_save) > CONFIG_EZIOT_TSL_PROFILE_MAP_SIZE, EZ_KV_ERR_SAVED_FULL);
            CHECK_COND_DONE(ezos_kv_raw_set(EZ_KV_DEFALUT_KEY_TSLMAP, pbuf_save, ezos_strlen(pbuf_save)), -1);
            ezlog_d(TAG_TSL, "add handle succ!!");
        }
        else
        {
            cJSON *js_item = cJSON_GetArrayItem(js_root, index);
            json_to_tslmap_metadata(js_item, &tsl_metadata);

            if (0 == ezos_strcmp(tsl_metadata.handle, handle_curr))
            {
                /* 元数据相同，无需更新tslmap */
                break;
            }

            if (1 == get_tsl_handle_count(js_root, tsl_metadata.handle))
            {
                /* 功能描述文件只有当前设备在使用，设备型号或者版本号已发生变更，tslpf文件删除 */
                ezlog_w(TAG_TSL, "ref = 0, del old pf");
                ezos_kv_del(tsl_metadata.handle);
            }

            ezlog_w(TAG_TSL, "tslmap chg, update");
            cJSON_ReplaceItemInObject(js_item, TSLMAP_JSON_KEY_HANDLE, cJSON_CreateString(handle_curr));
            CHECK_COND_DONE(!(pbuf_save = cJSON_PrintUnformatted(js_root)), -1);
            CHECK_RV_DONE(ezos_kv_raw_set(EZ_KV_DEFALUT_KEY_TSLMAP, pbuf_save, ezos_strlen(pbuf_save)));
            ezlog_w(TAG_TSL, "update succ!");
        }
    } while (ez_false);

    /* 只有在tslmap中找不到索引句柄情况下才会保存功能描述文件 */
    if (NULL != buf && 1 == get_tsl_handle_count(js_root, handle_curr))
    {
        ezlog_i(TAG_TSL, "save pf:%s", handle_curr);
        CHECK_COND_DONE(EZ_KV_ERR_SUCC != ezos_kv_raw_set(handle_curr, buf, len), ez_false);
        ezlog_i(TAG_TSL, "save pf succ!");
    }

done:
    SAFE_FREE(pbuf);
    SAFE_FREE(pbuf_save);
    cJSON_Delete(js_root);

    return rv;
}

ez_err_t tsl_storage_del(ez_char_t *dev_sn)
{
    ez_err_t rv = EZ_KV_ERR_SUCC;
    cJSON *js_root = NULL;
    ez_char_t *pbuf = NULL;
    ez_char_t *pbuf_save = NULL;
    ez_int32_t index = -1;
    tslmap_metadata_t tsl_metadata = {0};
    size_t tslmap_len = 0;

    CHECK_RV_DONE(ezos_kv_raw_get(EZ_KV_DEFALUT_KEY_TSLMAP, NULL, &tslmap_len));
    CHECK_COND_DONE(0 == tslmap_len, EZ_KV_ERR_SUCC);

    pbuf = ezos_malloc(tslmap_len + 1);
    CHECK_COND_DONE(!pbuf, -1);
    ezos_memset(pbuf, 0, tslmap_len + 1);

    CHECK_RV_DONE(ezos_kv_raw_get(EZ_KV_DEFALUT_KEY_TSLMAP, pbuf, &tslmap_len));
    CHECK_COND_DONE(!(js_root = cJSON_Parse(pbuf)), -1);

    CHECK_COND_DONE(-1 == (index = find_dev_by_sn(js_root, dev_sn)), -1);
    cJSON *js_item = cJSON_GetArrayItem(js_root, index);
    json_to_tslmap_metadata(js_item, &tsl_metadata);

    if (1 == get_tsl_handle_count(js_root, tsl_metadata.handle))
    {
        /* 功能描述文件只有当前设备在使用，设备型号或者版本号已发生变更，tslpf文件删除 */
        ezlog_w(TAG_TSL, "ref = 0, del");
        CHECK_COND_DONE(ezos_kv_del(tsl_metadata.handle), ez_false);
        ezlog_w(TAG_TSL, "del pf succ!");
    }

    ezlog_w(TAG_TSL, "tslmap chg, update");
    cJSON_DeleteItemFromArray(js_root, index);
    CHECK_COND_DONE(!(pbuf_save = cJSON_PrintUnformatted(js_root)), -1);
    CHECK_RV_DONE(ezos_kv_raw_set(EZ_KV_DEFALUT_KEY_TSLMAP, pbuf_save, ezos_strlen(pbuf_save)));
    ezlog_w(TAG_TSL, "update succ!");

done:
    SAFE_FREE(pbuf);
    SAFE_FREE(pbuf_save);
    cJSON_Delete(js_root);

    return rv;
}

static void storage_devinfo2index(ez_char_t *dev_sn, ez_char_t *dev_type, ez_char_t *dev_fwver, ez_char_t index[32])
{
    ez_uchar_t md5_output[16] = {0};
    ez_uchar_t base64_output[24 + 1] = {0};
    size_t olen = sizeof(base64_output);

    mbedtls_md5_context md5_ctx = {0};

    mbedtls_md5_init(&md5_ctx);
    mbedtls_md5_starts(&md5_ctx);

    mbedtls_md5_update(&md5_ctx, (unsigned char*)dev_type, ezos_strlen(dev_type));
    mbedtls_md5_update(&md5_ctx, (unsigned char*)dev_fwver, ezos_strlen(dev_fwver));
    mbedtls_md5_finish(&md5_ctx, md5_output);

    mbedtls_base64_encode(base64_output, sizeof(base64_output), &olen, md5_output, sizeof(md5_output));
    ezos_memset(index, 0, 32);
    ezos_snprintf(index, 32, "%s_%s", EZ_TSL_KEY_TSL_PREFIX, base64_output);
}

static ez_int32_t get_tsl_handle_count(cJSON *json_obj, ez_char_t *handle)
{
    ez_int32_t count = 0;

    for (int i = 0; i < cJSON_GetArraySize(json_obj); i++)
    {
        cJSON *js_item = cJSON_GetArrayItem(json_obj, i);
        if (NULL == js_item)
        {
            continue;
        }

        cJSON *js_sn = cJSON_GetObjectItem(js_item, TSLMAP_JSON_KEY_HANDLE);
        if (NULL == js_sn)
        {
            continue;
        }

        if (0 == ezos_strcmp(js_sn->valuestring, handle))
        {
            count++;
            break;
        }
    }

    return count;
}

static cJSON *tslmap_metadata_to_json(tslmap_metadata_t *struct_obj)
{
    s2j_create_json_obj(metadata_json);
    s2j_json_set_basic_element(metadata_json, struct_obj, string, sn);
    s2j_json_set_basic_element(metadata_json, struct_obj, string, handle);

    return metadata_json;
}

static void json_to_tslmap_metadata(cJSON *json_obj, tslmap_metadata_t *struct_obj)
{
    cJSON *json_temp = NULL;

    s2j_struct_get_basic_element_ex(struct_obj, json_obj, string, sn, "");
    s2j_struct_get_basic_element_ex(struct_obj, json_obj, string, handle, "");
}

static ez_int32_t find_dev_by_sn(cJSON *json_obj, ez_char_t *sn)
{
    ez_int32_t index = -1;

    for (int i = 0; i < cJSON_GetArraySize(json_obj); i++)
    {
        cJSON *js_item = cJSON_GetArrayItem(json_obj, i);
        if (NULL == js_item)
        {
            continue;
        }

        cJSON *js_sn = cJSON_GetObjectItem(js_item, TSLMAP_JSON_KEY_SN);
        if (NULL == js_sn)
        {
            continue;
        }

        if (0 == strcmp(js_sn->valuestring, sn))
        {
            index = i;
            break;
        }
    }

    return index;
}