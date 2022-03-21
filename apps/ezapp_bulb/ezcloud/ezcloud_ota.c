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
#include "device_info.h"
#include "ez_iot_ota.h"

static ez_int32_t ota_event_notify(ez_ota_res_t *pres, ez_ota_event_e event, ez_void_t *data, ez_int32_t len)
{
    ez_int32_t rv = -1;

    //TODO 实现

    return rv;
}

static ez_int32_t download_data_cb(ez_int32_t total_len, ez_int32_t offset, ez_void_t *data, ez_int32_t len, ez_void_t *user_data)
{
    //TODO 烧录

    return 0;
}

static ez_void_t download_result_cb(ez_ota_cb_result_e result, ez_void_t *user_data)
{
    ez_ota_res_t pres = {0};

    if (RESULT_FAILED == result)
    {
        ez_iot_ota_status_fail(&pres, dev_info_get_type(), (ez_int8_t *)"", OTA_CODE_DOWNLOAD);
    }
}

ez_void_t ezcloud_ota_init(ez_void_t)
{
    ez_ota_init_t init_info = {.cb.ota_recv_msg = ota_event_notify};
    ez_ota_res_t ota_res = {0};
    ez_ota_module_t module = {dev_info_get_type(), dev_info_get_fwver()};
    ez_ota_modules_t modules = {1, &module};

    ez_iot_ota_init(&init_info);
    ez_iot_ota_modules_report(&ota_res, &modules, 5000);
    ez_iot_ota_status_ready(&ota_res, module.mod_name);
}
