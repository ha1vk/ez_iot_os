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

#ifndef _EZHAL_PWM_H_
#define _EZHAL_PWM_H_

#include "ezos.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct pin_list_s
    {
      ez_uint32_t pin_num;
      ez_uint32_t dutie;
    } pin_list_t;

    /**
      * @brief  init all channels,PWM function initialization, including GPIO, frequency and duty cycle,
      *
      * @param  channel_num PWM channel number, maximum is 8
      * @param  pin_list pin list
      * @param  pwm_period PWM period, unit: us.
      *         e.g. For 1KHz PWM, period is 1000 us. Do not set the period below 20us.
      * 
      * @return
      *     - ESP_OK Success
      *     - ESP_ERR_INVALID_ARG Parameter error
      *     - ESP_FAIL Init error
      */
    ez_int32_t hal_pwm_init(ez_uint8_t channel_num, const pin_list_t *pin_list, ez_uint32_t pwm_period);


    /**
      * @brief  Set the duty cycle of all channels.
      *
      * @note   After set configuration, pwm_start needs to be called to take effect.
      *
      * @param  duties An array that store the duty cycle of each channel,
      *         the array elements number needs to be the same as the number of channels.
      * @return
      *     
      *     
      */
    ez_int32_t hal_pwm_set_duties(int *pwm_duty_period);

    /**
     * @brief This function is  used to start the PWM execution.
     * @param[in] 
     * @return    
     * @note
     * @waring
     */
    ez_int32_t hal_pwm_start();

    /**
     * @brief  This function is mainly used to stop the PWM execution.
     * @param[in]  channel is PWM channel number.
     * @return   
     * @waring
     */
    ez_int32_t hal_pwm_stop();

    /**
     * @brief    This function uninitialize the PWM hardware channel.
     * @param[in] pwm_channal:PWM channel logical .
     * @return   
     * @note
     * @waring
     */
    ez_int32_t hal_pwm_deinit(ez_int32_t pwm_channal);

#ifdef __cplusplus
}
#endif

#endif
