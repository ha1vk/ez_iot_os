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
 * XuRongjun (xurongjun@ezvizlife.com) - Device Bluetooth Low Energy abstract interface declaration
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-03-18     xurongjun    first version 
 *******************************************************************************/

#ifndef _HAL_BLE_H_
#define _HAL_BLE_H_

#include "ezos.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief Receive APDU (Application Protocol Data Unit)
     * 
     * @param data read buffer
     * @param length length of data
     * @return 0 for success, -1 for failed 
     */
    typedef ez_int32_t (*ble_recv_func_t)(const ez_char_t *data, ez_int32_t length);

    /**
     * @brief Initialize and start the ble module, ready to send and receive data
     * 
     * @return true for success, false for failed
     */
    ez_bool_t hal_ble_start(ble_recv_func_t recv_func);

    /**
     * @brief Send APDU (Application Protocol Data Unit)
     * 
     * @param data send buffer
     * @param length length of data
     * @return 0 for success, -1 for failed 
     */
    ez_int32_t hal_ble_send(const ez_char_t *data, ez_int32_t length);

    /**
     * @brief start the ble module, Stop sending and receiving data
     * 
     * @return 
     */
    ez_void_t hal_ble_stop(ez_void_t);

#ifdef __cplusplus
}
#endif

#endif /* _HAL_CONFIG_H_ */
