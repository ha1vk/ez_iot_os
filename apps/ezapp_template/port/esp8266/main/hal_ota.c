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
 * XuRongjun (xurongjun@ezvizlife.com) - Device ota abstract interface declaration
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-03-16     xurongjun    first version 
 *******************************************************************************/

#include "ezlog.h"
#include "esp_ota_ops.h"

static esp_ota_handle_t g_update_handle;

// OTA_CODE_NONE: 0x00
// OTA_CODE_MEM: 0X00500039
// OTA_CODE_DAMAGE: 0X00500038
// OTA_CODE_GENNERAL: 0X0050003B
ez_err_t hal_ota_begin(ez_size_t image_size)
{
    ez_err_t rv = 0x00;
    esp_err_t esp_rv;
    const esp_partition_t *pUpdate_partition = esp_ota_get_next_update_partition(NULL);
    if (NULL == pUpdate_partition)
    {
        rv = 0X0050003B;
        goto done;
    }

    esp_rv = esp_ota_begin(pUpdate_partition, image_size, &g_update_handle);
    switch (esp_rv)
    {
    case ESP_OK:
        rv = 0X00000000;
        break;
    case ESP_ERR_NO_MEM:
        rv = 0X00500039;
        break;
    case ESP_ERR_INVALID_SIZE:
        rv = 0X00500038;
        break;
    default:
        rv = 0X0050003B;
        break;
    }

done:
    return rv;
}

// OTA_CODE_NONE: 0x00
// OTA_CODE_BURN: 0X0050003A
// OTA_CODE_GENNERAL: 0X0050003B
ez_err_t hal_ota_write(const ez_void_t *data, ez_size_t size)
{
    ez_err_t rv = 0x00;
    esp_err_t esp_rv;

    esp_rv = esp_ota_write(g_update_handle, data, size);
    switch (esp_rv)
    {
    case ESP_OK:
        rv = 0X00000000;
        break;
    case ESP_ERR_FLASH_OP_FAIL:
    case ESP_ERR_FLASH_OP_TIMEOUT:
        rv = 0X0050003A;
        break;
    default:
        rv = 0X0050003B;
        break;
    }

    return rv;
}

// OTA_CODE_NONE: 0x00
// OTA_CODE_DIGEST: 0X00500036
// OTA_CODE_SIGN: 0X00500037
// OTA_CODE_DAMAGE: 0X00500038
// OTA_CODE_GENNERAL: 0X0050003B
ez_err_t hal_ota_end(ez_void_t)
{
    ez_err_t rv = 0x00;
    esp_err_t esp_rv;

    esp_rv = esp_ota_end(g_update_handle);
    switch (esp_rv)
    {
    case ESP_OK:
        rv = 0X00000000;
        break;
    case ESP_ERR_OTA_VALIDATE_FAILED:
        rv = 0X00500038;
        break;
    default:
        rv = 0X0050003B;
        break;
    }

    return rv;
}

// OTA_CODE_NONE: 0x00
// OTA_CODE_GENNERAL: 0X0050003B
ez_err_t hal_ota_action(ez_void_t)
{
    ez_err_t rv = 0x00;

    if (ESP_OK != esp_ota_set_boot_partition(esp_ota_get_next_update_partition(NULL)))
    {
        rv = 0X0050003B;
    }

    return rv;
}