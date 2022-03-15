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
 * XuRongjun (xurongjun@ezvizlife.com) - Product profile interface declaration
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-03-15     xurongjun    first version 
 *******************************************************************************/

#ifndef _PRODUCT_CONFIG_H_
#define _PRODUCT_CONFIG_H_

#include "ezos.h"

#ifdef __cplusplus
extern "C"
{
#endif

    ez_bool_t product_config_init(ez_char_t *buf, ez_int32_t buf_size);

    /**
     * @brief 获取国家码
     * 
     * @param buf 
     * @param buf_size 
     * @return ez_char_t 
     */
    const ez_char_t* product_config_get_cc(ez_void_t);

    /**
     * @brief 获取ap前缀
     * 
     * @param buf 
     * @param buf_size 
     * @return ez_char_t 
     */
    const ez_char_t* product_config_get_ap_prefix(ez_void_t);

    /**
     * @brief 获取默认色温
     * 
     * @return ez_int32_t 
     */
    ez_int32_t product_config_get_cct(ez_void_t);

#ifdef __cplusplus
}
#endif

#endif