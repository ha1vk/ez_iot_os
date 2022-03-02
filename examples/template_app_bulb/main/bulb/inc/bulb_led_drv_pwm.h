#ifndef _BULB_PWM_DRIVER_H_
#ifdef __cplusplus
extern "C"
{
#endif
#define _BULB_PWM_DRIVER_H_
#include <stdio.h>
#include "ezos_def.h"

    typedef enum
    {
        CONFIG_COLOR_COOL = 0,
        CONFIG_COLOR_WARM,
        CONFIG_COLOR_RED,
        CONFIG_COLOR_GREEN,
        CONFIG_COLOR_BLUE,
        CONFIG_CCT,
        CONFIG_CCT_LIGHT,
    } config_led_color_name_e;

    /*灯控逻辑口*/
    typedef enum
    {
        LED_COLOR_RED,
        LED_COLOR_GREEN,
        LED_COLOR_BLUE,
        LED_COLOR_WARM = 3,
        LED_COLOR_COOL,
        LED_CCT = 3,
        LED_CCT_LIGHT,
    } led_color_name_e;

    typedef enum
    {
        LED_GPIO_UNKNOW = 0,
        LED_GPIO_4 = 4,
        LED_GPIO_5 = 5,
        LED_GPIO_12 = 12,
        LED_GPIO_13 = 13,
        LED_GPIO_14 = 14,
    } led_gpio_name_e;

#define LED_GPIO_ALL_PIN ((1ULL << LED_GPIO_4) | (1ULL << LED_GPIO_5) | (1ULL << LED_GPIO_12) | (1ULL << LED_GPIO_13) | (1ULL << LED_GPIO_14))

    typedef enum
    {
        LED_GPIO_LEVEL_LOW = 0,
        LED_GPIO_LEVEL_HIGH = 1,
    } led_gpio_level_e;

    typedef enum
    {
        LED_GPIO_MODE_INPUT = 1UL << (0),
        LED_GPIO_MODE_OUTPUT = 1UL << (1),
    } led_gpio_mode_e;

    typedef enum
    {
        LED_GPIO_DRIVER_MODE_IO = 0,
        LED_GPIO_DRIVER_MODE_PWM = 1,
        LED_GPIO_DRIVER_MODE_I2C = 2,
        LED_GPIO_DRIVER_MODE_SPI = 3,
    } led_gpio_driver_mode_e;

    typedef enum
    {
        INTR_DISABLE = 0,
        INTR_POSEDGE = 1,
        INTR_NEGEDGE = 2,
        INTR_ANYEDGE = 3,
        INTR_LOW_LEVEL = 4,
        INTR_HIGH_LEVEL = 5,
    } intr_type_e;

    typedef enum
    {
        FUNC_MIN,
        FUNC_RESTART = 1,
        FUNC_RESET_WIFI = 2,
        FUNC_RESET = 3,
        FUNC_TEST = 5,
    } gpio_func_e;

    /* 管脚与颜色的映射到一个结构体*/
    typedef struct
    {
        config_led_color_name_e led_color_name; //0：冷光 ；1：暖光；2：红 ；3：绿；4：蓝；5：CCT；6：亮度
        led_gpio_name_e led_gpio_name;          //led maps to the real hardware pin,
        led_gpio_level_e led_gpio_level;        //1：高电平驱动有效，0低电平驱动有效
        led_gpio_mode_e led_gpio_mode;          //input:0 ,output:1
        intr_type_e intr_type;                  //I/O为输入方式时使用的中断方式,I/O为输出时无效
    } led_gpio_config_t;

    typedef struct
    {
        led_gpio_config_t led_gpio_config[5];
        led_gpio_driver_mode_e led_gpio_driver_mode;
        ez_uint8_t white_control_mode;    //cct or cw 驱动
        union
        {
            ez_uint32_t pwm_frequency;
            ez_uint32_t i2c_reserved;
            ez_uint32_t spi_reserved;
            ez_uint32_t io_reserved;
        } driver_config;
    } led_gpio_init_t;

#define LM_CV_ONECE_TIME 20 //缓慢变化调整的间隔时间,20ms设定时，最低亮度时（小于10流明）会出现闪烁
#define LM_CV_ONECE_TIME_SHORT 10

#define LM_CV_VALUE_STEP_MAX 150 //颜色变换算法中最多150步调整,根据LM_CV_ONECE_TIME最多3s 时间调整完呼吸,

#define LM_CV_STEP_MAX_SMART 1000 //亮度调节支持千分之一变化
#define LM_CV_STEP_MAX 100        //亮度调节支持百分之一变化

#define LEDC_CHANNEL_MAX 5 //最多支持5路灯，5路pwm 控制不同颜色led

    typedef enum
    {
        WHITE_CONTROL_CW = 0,  //冷暖灯珠驱动按时
        WHITE_CONTROL_CCT = 1, //CCT+ 亮度驱动方式
    } led_white_control_method_e;

    typedef struct
    {
        ez_int16_t brightness; //灯的亮度0-1000
        ez_int16_t cct_value;  //2700~6500
        ez_int8_t res[2];
        ez_int32_t rgb; //rgb的值
    } led_current_param_t;

    /* 此结构体用以定义驱动电路的使能方式，某些灯板以低电平有效驱动，有些灯板以高电平有效驱动*/
    typedef struct
    {

        led_gpio_level_e led_r;
        led_gpio_level_e led_g;
        led_gpio_level_e led_b;
        union
        {
            led_gpio_level_e led_w;
            led_gpio_level_e led_cct;
        };
        union
        {
            led_gpio_level_e led_c;
            led_gpio_level_e led_cct_light;
        };
    } led_enable_level_t;

    typedef struct
    {
        unsigned char R;
        unsigned char G;
        unsigned char B;
    } COLOR_RGB;

    /**
 * @brief    This function initializes the soft PWM channel and pin.
 * @param[in] led_gpio_config: PWM channel config info.
 * @param[in] frequency:PWM frequency.
 * @param[in] led_white_control_mode:bulb white led driver method
 * @return    
 * @note     针对球泡灯（1路，2路冷暖可调，3路rgb彩灯可调，5路rgb彩+冷暖灯可调）的gpio通用配置方法
 * @waring
 */
    ez_int8_t bulb_leds_gpio_config(led_gpio_init_t * led_gpio_init);

    /**@fn		  
 * @brief		  点亮cct灯
 * @param[in]  		cct_value：色温，led_lm_percentage 亮度，范围0-1000千分之一精度设置,由非smart函数拷贝而来，改变了宏值
 * @param[out] 
 * @return	  
 */
    ez_int8_t bulb_leds_cct_config(ez_int16_t cct_value, ez_int16_t led_lm_percentage);

    /**@fn		  
 * @brief		  
 * @param[in]  
 * @param[out] 
 * @return	  
 */
    ez_int8_t bulb_leds_RGB_config(ez_int32_t color_value, ez_int16_t led_lm_percentage);

    void Ezviz_Led_stop();

    void Ezviz_Led_start();

    void Ezviz_Led_restart();

#ifdef __cplusplus
}
#endif
#endif
