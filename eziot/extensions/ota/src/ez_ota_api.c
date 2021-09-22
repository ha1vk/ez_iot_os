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
* Contributors:
 *    shenhongyin - initial API and implementation and/or initial documentation
 *******************************************************************************/
#include "ez_ota.h"
#include "ez_ota_extern.h"
#include "ez_ota_user.h"
#include "ez_sdk_log.h"
#include "ez_sdk_ota.h"
#include <string.h>

static int g_ota_inited = 0;

#ifdef __cplusplus
extern "C"
{
#endif

    EZ_OS_API_EXTERN ez_err_e ez_iot_ota_init(ota_init_t *pota_init)
    {
        ez_err_e suc = ez_errno_succ;
        if (!pota_init)
        {
            return ez_errno_ota_param_invalid;
        }
        if (!pota_init->cb.ota_recv_msg)
        {
            ez_log_e(TAG_OTA, "ota_init, cb recv_msg err\n");
            return ez_errno_ota_param_invalid;
        }
        ez_ota_user_init(pota_init->cb);
        suc = ez_ota_extern_init();
        if (ez_errno_succ != suc)
        {
            ez_log_e(TAG_OTA, "ota_extern_init err\n");
            return suc;
        }
        g_ota_inited = 1;
        ez_log_d(TAG_OTA, "ez_ota_init success\n");

        return suc;
    }

    EZ_OS_API_EXTERN ez_err_e ez_iot_ota_modules_report(const ota_res_t *pres, const ota_modules_t *pmodules, uint32_t timeout_ms)
    {
        if (0 == g_ota_inited)
        {
            return ez_errno_ota_not_init;
        }
        if (NULL == pmodules)
        {
            return ez_errno_ota_param_invalid;
        }

        return ezdev_ota_module_info_report(pres, pmodules, timeout_ms);
    }

    EZ_OS_API_EXTERN ez_err_e ez_iot_ota_status_ready(const ota_res_t *pres, int8_t *pmodule)
    {
        if (!g_ota_inited)
        {
            return ez_errno_ota_not_init;
        }
        return ez_progress_report(pres, pmodule, NULL, ota_code_none, ota_state_ready, 0);
    }

    EZ_OS_API_EXTERN ez_err_e ez_iot_ota_status_succ(const ota_res_t *pres, int8_t *pmodule)
    {
        if (!g_ota_inited)
        {
            return ez_errno_ota_not_init;
        }
        return ez_progress_report(pres, pmodule, NULL, ota_code_none, ota_state_succ, 0);
    }

    EZ_OS_API_EXTERN ez_err_e ez_iot_ota_status_fail(const ota_res_t *pres, int8_t *pmodule, int8_t *perr_msg, ota_errcode_e code)
    {
        if (!g_ota_inited)
        {
            return ez_errno_ota_not_init;
        }
        return ez_progress_report(pres, pmodule, perr_msg, code, ota_state_failed, 0);
    }

    EZ_OS_API_EXTERN ez_err_e ez_iot_ota_progress_report(const ota_res_t *pres, int8_t *pmodule, ota_status_e status, int16_t progress)
    {
        if (!g_ota_inited)
        {
            return ez_errno_ota_not_init;
        }
        if (progress <= 0 || progress > 100)
        {
            return ez_errno_ota_param_invalid;
        }
        return ez_progress_report(pres, pmodule, NULL, ota_code_none, status, progress);
    }

    EZ_OS_API_EXTERN ez_err_e ez_iot_ota_download(ota_download_info_t *input_info, get_file_cb file_cb, notify_cb notify, void *user_data)
    {
        if (!g_ota_inited)
        {
            return ez_errno_ota_not_init;
        }
        if (NULL == input_info || 0 == strlen((char *)input_info->url) || input_info->block_size <= 0 || input_info->total_size <= 0 || NULL == file_cb || NULL == notify)
        {
            return ez_errno_ota_param_invalid;
        }

        return ez_ota_file_download(input_info, file_cb, notify, user_data);
    }

    EZ_OS_API_EXTERN ez_err_e ez_iot_ota_deinit()
    {
        if (!g_ota_inited)
        {
            return ez_errno_ota_not_init;
        }
        g_ota_inited = 0;
        ez_ota_extern_fini();
        return ez_errno_succ;
    }

#ifdef __cplusplus
}
#endif
