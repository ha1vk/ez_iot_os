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
 * XuRongjun (xurongjun@ezvizlife.com)
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-03-20     xurongjun    first version 
 *******************************************************************************/

#include "ezos.h"
#include "ezlog.h"
#include "ezos_time.h"
#include "ez_iot_ota.h"
#include "device_info.h"
#include "hal_config.h"
#include "hal_ota.h"
#include "mbedtls/md5.h"
#include "misc.h"

static ez_bool_t is_upgrading = ez_false;

typedef struct
{
    ez_int8_t mod_name[56];         ///< 模块名称
    ez_int8_t degist[32 + 1];       ///< 升级包的摘要值,包含'\0'
    mbedtls_md5_context degist_ctx; ///< 摘要句柄
} package_ctx_t;

static ez_int32_t download_data_cb(ez_uint32_t total_len, ez_uint32_t offset, ez_void_t *data, ez_uint32_t len, ez_void_t *user_data)
{
    ez_int32_t rv = 0;
    ez_uint16_t progress = 0;
    package_ctx_t *package_ctx = (package_ctx_t *)user_data;
    static ez_uint16_t last_progress = -1;

    if (0 == total_len)
    {
        rv = OTA_CODE_FILE_SIZE_RANGE;
        goto done;
    }

    if (0 == offset)
    {
        last_progress = 0;
        mbedtls_md5_init(&package_ctx->degist_ctx);
        mbedtls_md5_starts(&package_ctx->degist_ctx);
    }

    mbedtls_md5_update(&package_ctx->degist_ctx, data, len);
    rv = hal_ota_write(data, len);
    if (OTA_CODE_NONE != rv)
    {
        goto done;
    }

    // 间隔10%上报一次, 前50%为下载
    progress = (offset + len) * 50 / total_len;
    if ((progress - last_progress) / 10 > 0)
    {
        last_progress = progress;
        ez_iot_ota_progress_report(NULL, package_ctx->mod_name, OTA_STATE_DOWNLOADING, last_progress);
    }

done:

    if (OTA_CODE_NONE != rv)
    {
        ezlog_e(TAG_OTA, "write rv:%d", rv);
    }

    return rv;
}

static ez_void_t download_result_cb(ez_ota_cb_result_e result, ez_void_t *user_data)
{
    ez_err_t rv;
    package_ctx_t *package_ctx = (package_ctx_t *)user_data;
    ez_uchar_t degist_result[16] = {0};
    ez_uchar_t degist_hex_up[32 + 1] = {0};
    ez_uchar_t degist_hex[32 + 1] = {0};

    if (RESULT_FAILED == result)
    {
        rv = OTA_CODE_DOWNLOAD;
        goto err;
    }

    // 下载完成
    ez_iot_ota_progress_report(NULL, package_ctx->mod_name, OTA_STATE_DOWNLOAD_COMPLETED, 50);
    ezos_delay_ms(2000);

    // 校验摘要、烧录
    ez_iot_ota_progress_report(NULL, package_ctx->mod_name, OTA_STATE_BURNING, 60);
    mbedtls_md5_finish(&package_ctx->degist_ctx, degist_result);
    bin2hexstr(degist_result, sizeof(degist_result), 1, degist_hex_up);
    bin2hexstr(degist_result, sizeof(degist_result), 0, degist_hex);

    if (0 != ezos_strcmp((ez_char_t *)degist_hex_up, (ez_char_t *)package_ctx->degist) &&
        0 != ezos_strcmp((ez_char_t *)degist_hex, (ez_char_t *)package_ctx->degist))
    {
        rv = OTA_CODE_DIGEST;
        ezlog_e(TAG_OTA, "md5 mismatch, md5 result:%s, %s", degist_hex_up, degist_hex);
        goto err;
    }

    rv = hal_ota_end();
    if (OTA_CODE_NONE != rv)
    {
        goto err;
    }

    // 烧录完成
    ez_iot_ota_progress_report(NULL, package_ctx->mod_name, OTA_STATE_BURNING_COMPLETED, 70);
    ezos_delay_ms(2000);

    ez_iot_ota_status_succ(NULL, package_ctx->mod_name);
    ezos_delay_ms(2000);

    // 切换分区并重启
    rv = hal_ota_action();
    if (OTA_CODE_NONE != rv)
    {
        goto err;
    }

    ezos_reboot();

err:
    ezlog_e(TAG_OTA, "result rv:%d", rv);
    ez_iot_ota_status_fail(NULL, package_ctx->mod_name, (ez_int8_t *)"", rv);
    mbedtls_md5_free(&package_ctx->degist_ctx);
    ezos_free(user_data);
    is_upgrading = ez_false;
}

static ez_int32_t ota_download_fun(ez_ota_upgrade_info_t *upgrade_infos, ez_int32_t file_index)
{
    ez_ota_download_info_t download_info = {0};
    package_ctx_t *package_ctx = (package_ctx_t *)ezos_malloc(sizeof(package_ctx_t));
    if (NULL == package_ctx)
    {
        return OTA_CODE_MEM;
    }

    ezos_memset(package_ctx, 0, sizeof(package_ctx_t));
    ezos_strncpy((ez_char_t *)package_ctx->degist, (ez_char_t *)upgrade_infos->pota_files[file_index].degist, sizeof(package_ctx->degist) - 1);
    ezos_strncpy((ez_char_t *)package_ctx->mod_name, (ez_char_t *)upgrade_infos->pota_files[file_index].mod_name, sizeof(package_ctx->mod_name));

    ezos_snprintf((ez_char_t *)download_info.url, sizeof(download_info.url) - 1, "http://%s", (ez_char_t *)upgrade_infos->pota_files[file_index].url);
    ezos_strncpy((ez_char_t *)download_info.degist, (ez_char_t *)upgrade_infos->pota_files[file_index].degist, sizeof(download_info.degist) - 1);
    download_info.block_size = 1024;
    download_info.timeout_s = 60 * 5;
    download_info.retry_max = upgrade_infos->retry_max;
    download_info.total_size = upgrade_infos->pota_files[file_index].size;

    if (EZ_OTA_ERR_SUCC != ez_iot_ota_download(&download_info, download_data_cb, download_result_cb, (ez_void_t *)package_ctx))
    {
        return OTA_CODE_MEM;
    }

    return EZ_OTA_ERR_SUCC;
}

static ez_void_t show_upgrade_info(ez_ota_upgrade_info_t *upgrade_infos)
{
    ezlog_i(TAG_APP, "file_num:%d", upgrade_infos->file_num);
    ezlog_i(TAG_APP, "retry_max:%d", upgrade_infos->retry_max);
    ezlog_i(TAG_APP, "interval:%d", upgrade_infos->interval);

    for (ez_int32_t i = 0; i < upgrade_infos->file_num; i++)
    {
        ezlog_i(TAG_APP, "pota_files[%d]->module:%s", i, upgrade_infos->pota_files[i].mod_name);
        ezlog_i(TAG_APP, "pota_files[%d]->url:%s", i, upgrade_infos->pota_files[i].url);
        ezlog_i(TAG_APP, "pota_files[%d]->fw_ver:%s", i, upgrade_infos->pota_files[i].fw_ver);
        ezlog_i(TAG_APP, "pota_files[%d]->degist:%s", i, upgrade_infos->pota_files[i].degist);
        ezlog_i(TAG_APP, "pota_files[%d]->size:%d", i, upgrade_infos->pota_files[i].size);
        if (upgrade_infos->pota_files[i].pdiffs)
        {
            ezlog_i(TAG_APP, "pdiffs.degist: %s", upgrade_infos->pota_files[i].pdiffs->degist);
            ezlog_i(TAG_APP, "pdiffs.fw_ver_src: %s", upgrade_infos->pota_files[i].pdiffs->fw_ver_dst);
            ezlog_i(TAG_APP, "pdiffs.url: %s", upgrade_infos->pota_files[i].pdiffs->url);
            ezlog_i(TAG_APP, "pdiffs.size: %d", upgrade_infos->pota_files[i].pdiffs->size);
        }
    }
}

static ez_int32_t ota_event_notify(ez_ota_res_t *pres, ez_ota_event_e event, ez_void_t *data, ez_int32_t len)
{
    ez_int32_t rv = -1;
    ez_ota_upgrade_info_t *upgrade_infos = NULL;

    switch (event)
    {
    case START_UPGRADE:
    {
        upgrade_infos = (ez_ota_upgrade_info_t *)data;
        if (NULL == upgrade_infos || sizeof(ez_ota_upgrade_info_t) != len ||
            upgrade_infos->file_num <= 0 || NULL == upgrade_infos->pota_files)
        {
            break;
        }

        if (is_upgrading)
        {
            break;
        }

        is_upgrading = ez_true;
        show_upgrade_info(upgrade_infos);

        rv = hal_ota_begin(upgrade_infos->pota_files->size);
        if (OTA_CODE_NONE != rv)
        {
            break;
        }

        rv = ota_download_fun(upgrade_infos, 0);
        if (0 != rv)
        {
            break;
        }
    }
    break;
    default:
        break;
    }

    if (OTA_CODE_NONE == rv)
    {
        ez_iot_ota_progress_report(NULL, upgrade_infos->pota_files[0].mod_name, OTA_STATE_STARTING, OTA_PROGRESS_MIN);
    }
    else
    {
        is_upgrading = ez_false;
        ezlog_e(TAG_APP, "ota begin err, rv = :%d", rv);
    }

    return rv;
}

ez_void_t ezcloud_ota_init(ez_void_t)
{
    ez_ota_init_t init_info = {.cb.ota_recv_msg = ota_event_notify};
    ez_ota_module_t module = {(ez_int8_t *)dev_info_get_type(), (ez_int8_t *)dev_info_get_fwver()};
    ez_ota_modules_t modules = {1, &module};

    ez_iot_ota_init(&init_info);
    ez_iot_ota_modules_report(NULL, &modules, 5000);
    ez_iot_ota_status_ready(NULL, (ez_int8_t *)dev_info_get_type());
}