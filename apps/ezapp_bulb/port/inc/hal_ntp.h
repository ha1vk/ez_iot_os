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
 * XuRongjun (xurongjun@ezvizlife.com) - ntp abstract interface declaration
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-03-16     xurongjun    first version 
 *******************************************************************************/

#ifndef _HAL_NTP_H_
#define _HAL_NTP_H_

#include "ezos.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief Initialize the ntp module and start the timing task
     * 
     * @note use default ntp server
     * 
     * @return ez_void_t
     */
    ez_void_t hal_ntp_init(ez_void_t);

    /**
     * @brief Set ntp server
     * 
     * @param ntp_servername
     * @return ez_void_t
     */
    ez_void_t hal_ntp_set_servername(const ez_char_t *ntp_servername);

    /**
     * @brief Set timing cycle
     * 
     * @param timeval_ms unit
     * @return ez_void_t
     */
    ez_void_t hal_ntp_set_timeval(ez_int32_t timeval_ms);

    /**
     * @brief Deinitialize the timing module and stop the timing task
     * 
     * @return ez_void_t
     */
    ez_void_t hal_ntp_deinit(ez_void_t);

#ifdef __cplusplus
}
#endif

#endif /* _HAL_NTP_H_ */