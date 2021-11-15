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
 * xurongjun (xurongjun@ezvizlife.com)
 * 
 * Change Logs:
 * Date           Author       Notes
 * 2021-11-15     xurongjun    first version
*******************************************************************************/

#ifndef _EZOS_SYSTEM_H_
#define _EZOS_SYSTEM_H_

#include "ezos_def.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief  Get one random 32-bit word from hardware RNG
     *
     * @return Random value between 0 and UINT32_MAX
     */
    unsigned int ezos_random(void);

    /**
     * @brief get device uuid
     * 
     * @param uuid Put the uuid of the device, in no more than the length of the buffer,
     *             the result is null-terminated.
     * @param len The length of the buffer, with a maximum value of 128.
     * @return On success, return the length of the actual copy, excluding the end sign(\n). On error, zero is returned 
     */
    int ezos_get_uuid(char *uuid, short len);

#ifdef __cplusplus
}
#endif

#endif