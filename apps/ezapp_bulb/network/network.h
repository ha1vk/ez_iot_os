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
 * XuRongjun (xurongjun@ezvizlife.com) - network module Interface interface declaration
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-03-16     xurongjun    first version 
 *******************************************************************************/

#ifndef _NETWORK_H_
#define _NETWORK_H_

#include "ezos.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief Network module initialization
     * 
     * @return true for success, false for failed
     */
    ez_bool_t network_init(ez_void_t);

    /**
     * @brief Start network connection
     * 
     * @return true for success, false for failed 
     */
    ez_bool_t network_connect_start(ez_void_t);

    /**
     * @brief Update the judgment condition of the wifi provisioning
     * 
     * @brief Switch on and off multiple times means that the wifi network needs to be configured 
     * 
     * @return ez_void_t
     */
    ez_void_t network_wifi_prov_update(ez_void_t);

    /**
     * @brief Determine whether the wifi network needs to be configured
     * 
     * @return true for success, false for failed
     */
    ez_bool_t network_wifi_prov_need(ez_void_t);

    /**
     * @brief wifi provisioning
     * 
     * @return true for success, false for failed
     */
    ez_bool_t network_wifi_prov_do(ez_void_t);

    /**
     * @brief Wait for the wifi provisioning completed
     * 
     * @return
     */
    ez_void_t network_wifi_prov_waitfor(ez_void_t);

#ifdef __cplusplus
}
#endif

#endif