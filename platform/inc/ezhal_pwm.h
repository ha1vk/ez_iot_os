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

#ifdef __cplusplus
extern "C"
{
#endif

#include "stdio.h"
#include "ezos_def.h"

    /**
     * @brief    This function initializes the PWM hardware channel
     * @param[in] pwm_channal:PWM channel logical
     * @param[in] pwm_led_pin:Gpio used by PWM channel.
     * @param[in] period: PWM period, unit: us.
                        e.g. For 1KHz PWM, period is 1000 us.
    * @param[in] pwm_led_duty.:original duty cycle of pwm channel
    * @return    
    *     - OK :Success
    *     - FAIL: Init error
    * @note
    * @waring
    */
    int ezhal_pwm_init(int pwm_channal, int pwm_led_pin, int pwm_period, int pwm_duty_period);

    /**
     * @brief    This function uninitialize the PWM hardware channel.
     * @param[in] pwm_channal:PWM channel logical .
     * @return   
     * @note
     * @waring
     */
    int ezhal_pwm_deinit(int pwm_channal);

    /**
     * @brief Set the duty cycle of a PWM channel.
     *        Set the time that high level or low(if you invert the output of this channel)
     *        signal will last, the duty cycle cannot exceed the period.
     * @param[in] pwm_channal is PWM channel number.
     * @param[in] pwm_duty_cycle is PWM real period time, unit: us.
     * @return   
     * @note   
     * @waring
     */
    int ezhal_pwm_set_duty(int pwm_channal, int pwm_duty_period);

    /**
     * @brief This function is  used to start the PWM execution.
     * @param[in] 
     * @return    
     * @note
     * @waring
     */
    int ezhal_pwm_start();

    /**
     * @brief  This function is mainly used to stop the PWM execution.
     * @param[in]  channel is PWM channel number.
     * @return   
     * @waring
     */
    int ezhal_pwm_stop();

#ifdef __cplusplus
}
#endif

#endif
