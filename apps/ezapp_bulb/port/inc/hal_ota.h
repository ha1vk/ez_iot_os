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

#ifndef _HAL_OTA_H_
#define _HAL_OTA_H_

#include "ezos.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief Start the upgrade process
     * 
     * @return see ez_ota_errcode_e
     *      - OTA_CODE_NONE: 0x00
     *      - OTA_CODE_MEM: 0X00500039
     *      - OTA_CODE_DAMAGE: 0X00500038
     *      - OTA_CODE_GENNERAL: 0X0050003B
     */
    ez_err_t hal_ota_begin(ez_size_t image_size);

    /**
     * @brief Write OTA package data to flash
     * 
     * @return see ez_ota_errcode_e
     *      - OTA_CODE_NONE: 0x00
     *      - OTA_CODE_BURN: 0X0050003A
     *      - OTA_CODE_GENNERAL: 0X0050003B
     */
    ez_err_t hal_ota_write(const ez_void_t *data, ez_size_t size);

    /**
     * @brief Finish OTA update and validate newly written app image.
     * 
     * @note The hal_ota_begin() function must be called first. Calling the hal_ota_end() just means that the ota package has 
     *       completed writing and verification, then need to call the hal_ota_action() complete upgrade process.
     * 
     * @return see ez_ota_errcode_e
     *      - OTA_CODE_NONE: 0x00
     *      - OTA_CODE_DIGEST: 0X00500036
     *      - OTA_CODE_SIGN: 0X00500037
     *      - OTA_CODE_DAMAGE: 0X00500038
     *      - OTA_CODE_GENNERAL: 0X0050003B
     */
    ez_err_t hal_ota_end(ez_void_t);

    /**
     * @brief Execute the upgrade command
     * 
     * @note If this function returns OTA_CODE_NONE, the device will run the new firmware after rebooting
     * 
     * @return see ez_ota_errcode_e
     *      - OTA_CODE_NONE: 0x00
     *      - OTA_CODE_GENNERAL: 0X0050003B
     */
    ez_err_t hal_ota_action(ez_void_t);

#ifdef __cplusplus
}
#endif

#endif /* _HAL_OTA_H_ */