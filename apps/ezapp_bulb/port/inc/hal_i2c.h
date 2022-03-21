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
 * 2021-11-25     liwei        first version 
 *******************************************************************************/

#ifndef _EZHAL_I2C_H_
#define _EZHAL_I2C_H_

#include "ezos.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef enum
    {
        BP5758 = 0, /*!< I2C chip code */
    } hal_i2c_chip_e;

    /**
     * @brief I2C initialization parameters
     */
    typedef struct
    {
        hal_i2c_chip_e chip_code;
        ez_int32_t mode;          /*!< I2C mode */
        ez_int32_t sda_io_num;    /*!< GPIO number for I2C sda signal */
        ez_int32_t sda_pullup_en; /*!< Internal GPIO pull mode for I2C sda signal*/
        ez_int32_t scl_io_num;    /*!< GPIO number for I2C scl signal */
        ez_int32_t scl_pullup_en; /*!< Internal GPIO pull mode for I2C scl signal*/
    } hal_i2c_config_t;

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
    ez_int32_t hal_i2c_init(const hal_i2c_config_t *i2c_conf);

    /**
     * @brief set the led channel duties to i2c slaver
     *
     * @param pwm_duty_period: An array that store the duty cycle of each channel,
     * @return
     *     - EZ_OK Success
     */
    ez_int32_t hal_i2c_led_set_duties(ez_int32_t *pwm_duty_period);

    /**
    * @brief This function is  used to stop the i2c execution,let the slaver sleep;
    * @param[in] 
    * @return    
    * @note
    * @waring
    */
    ez_int32_t hal_i2c_led_stop();

#ifdef __cplusplus
}
#endif

#endif
