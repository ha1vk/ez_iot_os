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
 * XuRongjun (xurongjun@ezvizlife.com) - Device information related interface declaration
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-03-15     xurongjun    first version 
 *******************************************************************************/

#ifndef _DEVICE_INFO_H_
#define _DEVICE_INFO_H_

#include "ezos.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief 
     * 
     * @param buf 
     * @param buf_size 
     * @return ez_bool_t 
     */
    ez_bool_t dev_info_init(ez_char_t *buf, ez_int32_t buf_size);

    /**
     * @brief 
     * 
     * @return const char* 
     */
    const ez_char_t *dev_info_get_type();

    /**
     * @brief 
     * 
     * @return const char* 
     */
    const ez_char_t *dev_info_get_fwver();

    /**
     * @brief 
     * 
     * @return const char* 
     */
    const ez_char_t *dev_info_get_sn();

    /**
     * @brief 
     * 
     * @return const char* 
     */
    const ez_char_t *dev_info_get_vcode();

    /**
     * @brief 
     * 
     * @return ez_int32_t 
     */
    ez_int32_t dev_info_auth_mode();

#ifdef __cplusplus
}
#endif

#endif /* _DEVICE_INFO_H_ */