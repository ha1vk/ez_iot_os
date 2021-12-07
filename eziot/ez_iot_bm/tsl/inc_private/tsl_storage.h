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

#include "ezos.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief Show tsl data
     * 
     * @return N/A 
     */
    ez_void_t tsl_storage_print(ez_void_t);

    /**
     * @brief Read profile by device serial number
     * 
     * @param[in] dev_sn Device serial number
     * @param[out] buf Profile data
     * @param[out] len Profile length
     * @return 0 for success, -1 for general error, EZ_KV_ERR_NAME means not found
     */
    ez_err_t tsl_storage_load(ez_char_t *dev_sn, ez_char_t **buf, ez_uint32_t *len);

    /**
     * @brief Save profile
     * 
     * @param[in] dev_sn Device serial number
     * @param[in] dev_type Device model
     * @param[in] dev_fwver Firmware version
     * @param[in] buf Profile data
     * @param[in] len Profile length
     * @return 0 for success, -1 for general error, EZ_KV_ERR_NAME means not found 
     */
    ez_err_t tsl_storage_save(ez_char_t *dev_sn, ez_char_t *dev_type, ez_char_t *dev_fwver, ez_char_t *buf, ez_int32_t len);

    /**
     * @brief Del profil
     * 
     * @param dev_sn Device serial number
     * @return 0 for success, -1 for general error
     */
    ez_err_t tsl_storage_del(ez_char_t *dev_sn);

#ifdef __cplusplus
}
#endif