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

#ifndef _EZCLOUD_LINK_H_
#define _EZCLOUD_LINK_H_

#include "ezos.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief Start iot connecting
     * 
     * @return
     */
    ez_void_t ezcloud_link_start(ez_void_t);

    /**
     * @brief 上报属性
     * 
     * @return ez_void_t 
     */
    ez_void_t ezcloud_tsl_prop_report(ez_char_t *identify);

    /**
     * @brief 重置所有属性
     * 
     * @return ez_void_t 
     */
    ez_void_t ezcloud_tsl_prop_reset(ez_void_t);

#ifdef __cplusplus
}
#endif

#endif /* _EZCLOUD_LINK_H_ */
