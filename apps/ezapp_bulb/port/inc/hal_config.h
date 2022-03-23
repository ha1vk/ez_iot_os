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
 * XuRongjun (xurongjun@ezvizlife.com) - Device configuration abstract interface declaration
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-03-14     xurongjun    first version 
 *******************************************************************************/

#ifndef _HAL_CONFIG_H_
#define _HAL_CONFIG_H_

#include "ezos.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief Initialize the configuration module
     * 
     * @return true for success, false for failed
     */
    ez_bool_t hal_config_init(ez_void_t);

    /**
     * @brief Get Device Product Profile
     * 
     * @param[out] val data read buffer
     * @param[in] len the maximum length of val buffer
     * @return read data length
     */
    ez_int32_t hal_config_product_load(ez_char_t *val, ez_int32_t len);

    /**
     * @brief Get device license
     * 
     * @param[out] val data read buffer
     * @param[in] len the maximum length of val buffer
     * @return read data length
     */
    ez_int32_t hal_config_lic_load(ez_char_t *val, ez_int32_t len);

    /**
     * @brief Read kv (type int)
     * 
     * @param key 
     * @param _defval 
     * @return ez_bool_t 
     */
    ez_bool_t hal_config_get_int(const ez_char_t *key, ez_int32_t *val, ez_int32_t _defval);

    /**
     * @brief Save kv (type int)
     * 
     * @param key 
     * @param val 
     * @return ez_bool_t 
     */
    ez_bool_t hal_config_set_int(const ez_char_t *key, ez_int32_t val);

    /**
     * @brief Read kv (type double)
     * 
     * @param key 
     * @param _defval 
     * @return ez_bool_t 
     */
    ez_bool_t hal_config_get_double(const ez_char_t *key, ez_double_t *val, ez_double_t _defval);

    /**
     * @brief Save kv (type double)
     * 
     * @param key 
     * @param val 
     * @return ez_bool_t 
     */
    ez_bool_t hal_config_set_double(const ez_char_t *key, ez_double_t val);

    /**
     * @brief Read kv (type string)
     * 
     * @param key 
     * @param val 
     * @param _defval 
     * @return ez_bool_t 
     */
    ez_bool_t hal_config_get_string(const ez_char_t *key, ez_char_t *val, ez_int32_t *len, const ez_char_t *_defval);

    /**
     * @brief Save kv (type string)
     * 
     * @param key 
     * @param val 
     * @return ez_bool_t 
     */
    ez_bool_t hal_config_set_string(const ez_char_t *key, const ez_char_t *val);

    /**
     * @brief Del kv
     * 
     * @param key 
     * @return 
     */
    ez_void_t hal_config_del(const ez_char_t *key);

    /**
     * @brief Restore to factory state
     * 
     * @return true for success, false for failed 
     */
    ez_bool_t hal_config_reset_factory(ez_void_t);

    /**
     * @brief 
     * 
     * @return ez_void_t 
     */
    ez_void_t hal_config_print();

#ifdef __cplusplus
}
#endif

#endif /* _HAL_CONFIG_H_ */
