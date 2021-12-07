/*******************************************************************************
*
*               COPYRIGHT (c) 2015-2016 Broadlink Corporation
*                         All Rights Reserved
*
* The source code contained or described herein and all documents
* related to the source code ("Material") are owned by Broadlink
* Corporation or its licensors.  Title to the Material remains
* with Broadlink Corporation or its suppliers and licensors.
*
* The Material is protected by worldwide copyright and trade secret
* laws and treaty provisions. No part of the Material may be used,
* copied, reproduced, modified, published, uploaded, posted, transmitted,
* distributed, or disclosed in any way except in accordance with the
* applicable license agreement.
*
* No license under any patent, copyright, trade secret or other
* intellectual property right is granted to or conferred upon you by
* disclosure or delivery of the Materials, either expressly, by
* implication, inducement, estoppel, except in accordance with the
* applicable license agreement.
*
* Unless otherwise agreed by Broadlink in writing, you may not remove or
* alter this notice or any other notice embedded in Materials by Broadlink
* or Broadlink's suppliers or licensors in any way.
*
*******************************************************************************/

#ifndef __DNA_PWM_H
#define __DNA_PWM_H

#ifdef __cplusplus
    extern "C" {
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
int ezos_pwm_init(int pwm_channal,int pwm_led_pin,int pwm_period, int pwm_duty_period);

/**
 * @brief    This function uninitialize the PWM hardware channel.
 * @param[in] pwm_channal:PWM channel logical .
 * @return   
 * @note
 * @waring
 */
int ezos_pwm_deinit(int pwm_channal);


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
int ezos_pwm_set_duty(int pwm_channal, int pwm_duty_period);

/**
 * @brief This function is  used to start the PWM execution.
 * @param[in] 
 * @return    
 * @note
 * @waring
 */
int ezos_pwm_start();

/**
 * @brief  This function is mainly used to stop the PWM execution.
 * @param[in]  channel is PWM channel number.
 * @return   
 * @waring
 */
int ezos_pwm_stop();

#ifdef __cplusplus
}
#endif

#endif

