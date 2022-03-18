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
 * liwei (liwei@ezvizlife.com)
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-11-25     liwei    first version 
 *******************************************************************************/

#ifndef _EZHAL_I2C_H_
#define _EZHAL_I2C_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "stdio.h"
#include "ezos_def.h"
//#include "driver/i2c.h"

/**
 * @brief I2C initialization parameters
 */
typedef struct {
    int mode;              /*!< I2C mode */
    int sda_io_num;        /*!< GPIO number for I2C sda signal */
    int sda_pullup_en;  /*!< Internal GPIO pull mode for I2C sda signal*/
    int scl_io_num;        /*!< GPIO number for I2C scl signal */
    int scl_pullup_en;  /*!< Internal GPIO pull mode for I2C scl signal*/
} ez_i2c_config_t;


/**
 * @brief I2C driver install
 *
 * @param i2c_num I2C port number
 * @param mode I2C mode( master or slave )
 *
 * @return
 *     - EZ_OK   Success
 *     - EZ_ERR_INVALID_ARG Parameter error
 *     - EZ_FAIL Driver install error
 */
int ezhal_i2c_driver_install(int i2c_num, int mode);


/**
 * @brief I2C parameter initialization
 *
 * @note It must be used after calling i2c_driver_install
 *
 * @param i2c_num I2C port number
 * @param i2c_conf pointer to I2C parameter settings
 *
 * @return
 *     - EZ_OK Success
 *     - EZ_ERR_INVALID_ARG Parameter error
 */
int ezhal_i2c_param_config(int i2c_num, const ez_i2c_config_t *i2c_conf);

/**
 * @brief test code to write mpu6050
 *
 * 1. send data
 *
 * @param i2c_num I2C port number
 * @param data data to send
 * @param data_len data length
 *
 * @return
 *     - EZ_OK Success
 */
int ezhal_i2c_master_write_slave(int i2c_num, ez_uint8_t *data_wr, size_t size);

#ifdef __cplusplus
}
#endif

#endif
