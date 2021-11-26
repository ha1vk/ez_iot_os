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
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-11-25     ZOUJINWEI    first version 
 *******************************************************************************/
#include "ez_ota.h"
#include "ez_ota_extern.h"
#include "ez_ota_user.h"
#include "ezlog.h"
#include "ez_iot_ota.h"
#include <ezos.h>

static int g_ota_inited = 0;

#ifdef __cplusplus
extern "C"
{
#endif

	EZOS_API ez_err_t ez_iot_ota_init(ez_ota_init_t *pota_init)
    {
        ez_err_t suc = EZ_OTA_ERR_SUCC;
        if (!pota_init)
        {
            return EZ_OTA_ERR_PARAM_INVALID;
        }
        if (!pota_init->cb.ota_recv_msg)
        {
            ezlog_e(TAG_OTA, "ota_init, cb recv_msg err\n");
            return EZ_OTA_ERR_PARAM_INVALID;
        }
        ez_ota_user_init(pota_init->cb);
        suc = ez_ota_extern_init();
        if (EZ_OTA_ERR_SUCC != suc)
        {
            ezlog_e(TAG_OTA, "ota_extern_init err\n");
            return suc;
        }
        g_ota_inited = 1;
        ezlog_d(TAG_OTA, "ez_ota_init success\n");

        return suc;
    }

	EZOS_API ez_err_t ez_iot_ota_modules_report(const ez_ota_res_t *pres, const ez_ota_modules_t *pmodules, ez_int32_t timeout_ms)
    {
        if (0 == g_ota_inited)
        {
            return EZ_OTA_ERR_NOT_INIT;
        }
        if (NULL == pmodules)
        {
            return EZ_OTA_ERR_PARAM_INVALID;
        }

        return ezdev_ota_module_info_report(pres, pmodules, timeout_ms);
    }

	EZOS_API ez_err_t ez_iot_ota_status_ready(const ez_ota_res_t *pres, ez_int8_t *pmodule)
    {
        if (!g_ota_inited)
        {
            return EZ_OTA_ERR_NOT_INIT;
        }
        return ez_progress_report(pres, pmodule, NULL, EZ_OTA_ERR_SUCC, ota_state_ready, 0);
    }

	EZOS_API ez_err_t ez_iot_ota_status_succ(const ez_ota_res_t *pres, ez_int8_t *pmodule)
    {
        if (!g_ota_inited)
        {
            return EZ_OTA_ERR_NOT_INIT;
        }
        return ez_progress_report(pres, pmodule, NULL, EZ_OTA_ERR_SUCC, ota_state_succ, 0);
    }

	EZOS_API ez_err_t ez_iot_ota_status_fail(const ez_ota_res_t *pres, ez_int8_t *pmodule, ez_int8_t *perr_msg, ez_ota_errcode_t code)
    {
        if (!g_ota_inited)
        {
            return EZ_OTA_ERR_NOT_INIT;
        }
        return ez_progress_report(pres, pmodule, perr_msg, code, ota_state_failed, 0);
    }

	EZOS_API ez_err_t ez_iot_ota_progress_report(const ez_ota_res_t *pres, ez_int8_t *pmodule, ez_ota_status_t status, ez_int16_t progress)
    {
        if (!g_ota_inited)
        {
            return EZ_OTA_ERR_NOT_INIT;
        }
        if (progress <= 0 || progress > 100)
        {
            return EZ_OTA_ERR_PARAM_INVALID;
        }
        return ez_progress_report(pres, pmodule, NULL, EZ_OTA_ERR_SUCC, status, progress);
    }

	EZOS_API ez_err_t ez_iot_ota_download(ez_ota_download_info_t *input_info, get_file_cb file_cb, notify_cb notify, void *user_data)
    {
        if (!g_ota_inited)
        {
            return EZ_OTA_ERR_NOT_INIT;
        }
        if (NULL == input_info || 0 == ezos_strlen((char *)input_info->url) || input_info->block_size <= 0 || input_info->total_size <= 0 || NULL == file_cb || NULL == notify)
        {
            return EZ_OTA_ERR_PARAM_INVALID;
        }

        return ez_ota_file_download(input_info, file_cb, notify, user_data);
    }

	EZOS_API ez_err_t ez_iot_ota_deinit()
    {
        if (!g_ota_inited)
        {
            return EZ_OTA_ERR_NOT_INIT;
        }
        g_ota_inited = 0;
        ez_ota_extern_fini();
        return EZ_OTA_ERR_SUCC;
    }

#ifdef __cplusplus
}
#endif
