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
 * XuRongjun (xurongjun@ezvizlife.com) - Product profile interface declaration
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-03-15     xurongjun    first version 
 *******************************************************************************/

#ifndef _PRODUCT_CONFIG_H_
#define _PRODUCT_CONFIG_H_

#include "ezos.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct
    {
        ez_char_t name;       // GPIO的名字，目前有IO 4、5、12、13、14
        ez_char_t enable;     // 0:低电平 ；1:高电平
        ez_char_t mode;       // 1: 输入 ；2：输出
        ez_char_t drive_mode; // IO驱动方式 0:PWM驱动 1：I2C 2:i/o 3:spi
        ez_char_t light;      // 0：冷光 ；1：暖光；2：红 ；3：绿；4：蓝；5：CCT；6：亮度,(5,6须同时存在）
        ez_char_t i2c;        //LED_I2C_SDA=0;LED_I2C_CLK=1;
        union                 // 驱动参数
        {
            ez_int32_t intr_type;     // IO中断触发方式，0：disable, 1:上升沿，2：下降沿，3：上升沿和下降沿 4：低电平 5：高电平
            ez_int32_t pwm_frequency; //灯控应用，PWM频率（默认1000HZ）
        };
    } io_config_t;

    typedef struct
    {
        ez_char_t order[6];   //亮灯的顺序，C：冷光 ；W	:暖光；B：蓝色；G：绿色；R：红色，用户可以随意配置
        ez_int32_t step1time; //第一个阶段持续的时间，单位s，默认60s
        ez_char_t holder[2];  //占位符，保持字节对齐
    } bulb_test_param_t;

    /**
     * @brief Product configuration module initialization
     * 
     * @param[in] buf data buffer
     * @param[in] buf_size length of buffer
     * @return true for success, false for failed 
     */
    ez_bool_t product_config_init(ez_char_t *buf, ez_int32_t buf_size);

    /**
     * @brief Get the wireless distribution network connection name prefix 
     * 
     * @return !=NULL;<23
     */
    const ez_char_t *product_config_get_wd_prefix(ez_void_t);

    /**
     * @brief Get the  wireless distribution network window period
     * 
     * @return >0; minutes 
     */
    ez_int32_t product_config_get_wd_period(ez_void_t);

    /**
     * @brief Get the start conditions of the distribution network process
     * 
     * @param lower reboot times >= lower
     * @param upper reboot times =< upper
     * @return 
     */
    ez_void_t product_config_get_wd_condition(ez_int32_t *lower, ez_int32_t *upper);

    /**
     * @brief Get the default color temperature value
     * 
     * @return >=2700;<=6500
     */
    ez_int32_t product_config_default_cct(ez_void_t);

    /**
     * @brief Get io config array
     * 
     * @param[out] Points to the io config array
     * @return Number of io_config; <=5
     */
    ez_int32_t product_config_io(io_config_t **io_config);

    /**
     * @brief Get the product subtype
     * 
     * @return >0 
     */
    ez_int32_t product_config_subtype(ez_void_t);

    /**
     * @brief Get product ptid (For SAP devices, the ptid is equivalent to the device model)
     * 
     * @return !NULL; <32 
     */
    const ez_char_t *product_config_ptid(ez_void_t);

    /**
     * @brief Get the default nickname of the device, display on the mobile application. 
     * 
     * @return !NULL; <32 
     */
    const ez_char_t *product_config_default_nickname(ez_void_t);

    /**
     * @brief Get the product test param
     * 
     * @return !NULL
     */
    const bulb_test_param_t *get_product_test_param(ez_void_t);

#ifdef __cplusplus
}
#endif

#endif