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
 * 2021-11-25     xurongjun    first version 
 *******************************************************************************/

#include <string.h>
#include "bulb_led_drv_pwm.h"
#include "ezos_pwm.h"
#include "ezlog.h"

static const char *TAG_LED = "[LED]";

static ez_uint32_t g_led_pwm_cycle_time = 1000;

static ez_uint8_t g_led_white_control_mode = WHITE_CONTROL_CW; //球泡灯的白光控制方式

/* 由于渐变引入的实时变化中的灯泡状态*/
led_current_param_t g_led_current_param =
    {
        .brightness = 100,
        .cct_value = 3000,
        .rgb = 0xff0000};

/* 灯板驱动的方法*/
led_enable_level_t g_led_gpios_level =
    {
        .led_c = LED_GPIO_LEVEL_HIGH,
        .led_w = LED_GPIO_LEVEL_HIGH,
        .led_r = LED_GPIO_LEVEL_HIGH,
        .led_g = LED_GPIO_LEVEL_HIGH,
        .led_b = LED_GPIO_LEVEL_HIGH,
};

#define LED_CIE1931_NUM 381
typedef struct
{
    ez_int16_t cct_value;
    ez_int16_t cie1931_x;
    ez_int16_t cie1931_y;
} led_cie1931_xy_t;

static led_cie1931_xy_t led_cie1931_xy[LED_CIE1931_NUM] =
    {
        {2700, 4593, 4107}, {2710, 4585, 4105}, {2720, 4577, 4103}, {2730, 4569, 4101}, {2740, 4561, 4099}, 
        {2750, 4553, 4097}, {2760, 4545, 4095}, {2770, 4537, 4093}, {2780, 4529, 4091}, {2790, 4521, 4089}, 
        {2800, 4514, 4087}, {2810, 4506, 4085}, {2820, 4498, 4083}, {2830, 4490, 4081}, {2840, 4483, 4079}, 
        {2850, 4475, 4076}, {2860, 4468, 4074}, {2870, 4460, 4072}, {2880, 4453, 4070}, {2890, 4445, 4068}, 
        {2900, 4438, 4065}, {2910, 4430, 4063}, {2920, 4423, 4061}, {2930, 4416, 4058}, {2940, 4409, 4056}, 
        {2950, 4401, 4054}, {2960, 4394, 4051}, {2970, 4387, 4049}, {2980, 4380, 4047}, {2990, 4373, 4044}, 
        {3000, 4366, 4042}, {3010, 4359, 4039}, {3020, 4352, 4037}, {3030, 4345, 4034}, {3040, 4338, 4032}, 
        {3050, 4331, 4029}, {3060, 4324, 4027}, {3070, 4317, 4024}, {3080, 4311, 4022}, {3090, 4304, 4019}, 
        {3100, 4297, 4017}, {3110, 4290, 4014}, {3120, 4284, 4012}, {3130, 4277, 4009}, {3140, 4271, 4007}, 
        {3150, 4264, 4004}, {3160, 4258, 4001}, {3170, 4251, 3999}, {3180, 4245, 3886}, {3190, 4238, 3993}, 
        {3200, 4232, 3991}, {3210, 4225, 3988}, {3220, 4219, 3985}, {3230, 4213, 3983}, {3240, 4207, 3980}, 
        {3250, 4200, 3977}, {3260, 4194, 3975}, {3270, 4188, 3972}, {3280, 4182, 3969}, {3290, 4176, 3967}, 
        {3300, 4170, 3964}, {3310, 4163, 3961}, {3320, 4157, 3958}, {3330, 4151, 3956}, {3340, 4145, 3953}, 
        {3350, 4139, 3950}, {3360, 4134, 3947}, {3370, 4128, 3945}, {3380, 4122, 3942}, {3390, 4116, 3939}, 
        {3400, 4110, 3936}, {3410, 4104, 3934}, {3420, 4099, 3931}, {3430, 4093, 3928}, {3440, 4087, 3925}, 
        {3450, 4081, 3923}, {3460, 4076, 3920}, {3470, 4070, 3917}, {3480, 4065, 3914}, {3490, 4059, 3911}, 
        {3500, 4053, 3909}, {3510, 4048, 3906}, {3520, 4042, 3903}, {3530, 4037, 3900}, {3540, 4031, 3897}, 
        {3550, 4026, 3894}, {3560, 4021, 3892}, {3570, 4015, 3889}, {3580, 4010, 3886}, {3590, 4005, 3883}, 
        {3600, 3999, 3880}, {3610, 3994, 3878}, {3620, 3989, 3875}, {3630, 3984, 3872}, {3640, 3978, 3869}, 
        {3650, 3973, 3866}, {3660, 3968, 3863}, {3670, 3963, 3861}, {3680, 3958, 3858}, {3690, 3953, 3855}, 
        {3700, 3948, 3852}, {3710, 3942, 3849}, {3720, 3937, 3846}, {3730, 3932, 3844}, {3740, 3927, 3841}, 
        {3750, 3923, 3838}, {3760, 3918, 3835}, {3770, 3913, 3832}, {3780, 3908, 3829}, {3790, 3903, 3827}, 
        {3800, 3898, 3824}, {3810, 3893, 3821}, {3820, 3888, 3818}, {3830, 3884, 3815}, {3840, 3879, 3812}, 
        {3850, 3874, 3810}, {3860, 3869, 3807}, {3870, 3865, 3804}, {3880, 3860, 3801}, {3890, 3855, 3798}, 
        {3900, 3851, 3795}, {3910, 3846, 3793}, {3920, 3841, 3790}, {3930, 3837, 3787}, {3940, 3832, 3784}, 
        {3950, 3828, 3781}, {3960, 3823, 3779}, {3970, 3819, 3776}, {3980, 3814, 3773}, {3990, 3810, 3770}, 
        {4000, 3805, 3767}, {4010, 3800, 3764}, {4020, 3796, 3761}, {4030, 3791, 3759}, {4040, 3787, 3756}, 
        {4050, 3783, 3753}, {4060, 3778, 3750}, {4070, 3774, 3748}, {4080, 3770, 3745}, {4090, 3766, 3742}, 
        {4100, 3761, 3740}, {4110, 3757, 3737}, {4120, 3753, 3434}, {4130, 3749, 3731}, {4140, 3745, 3729}, 
        {4150, 3740, 3726}, {4160, 3736, 3723}, {4170, 3732, 3721}, {4180, 3728, 3718}, {4190, 3724, 3715}, 
        {4200, 3720, 3713}, {4210, 3716, 3710}, {4220, 3712, 3707}, {4230, 3708, 3705}, {4240, 3704, 3702}, 
        {4250, 3700, 3699}, {4260, 3696, 3697}, {4270, 3692, 3694}, {4280, 3688, 3692}, {4290, 3684, 3689}, 
        {4300, 3681, 3686}, {4310, 3677, 3684}, {4320, 3673, 3681}, {4330, 3669, 3678}, {4340, 3665, 3676}, 
        {4350, 3662, 3673}, {4360, 3658, 3671}, {4370, 3654, 3668}, {4380, 3650, 3666}, {4390, 3647, 3663}, 
        {4400, 3643, 3660}, {4410, 3639, 3658}, {4420, 3636, 3655}, {4430, 3632, 3653}, {4440, 3628, 3650}, 
        {4450, 3625, 3648}, {4460, 3621, 3645}, {4470, 3618, 3643}, {4480, 3614, 3640}, {4490, 3611, 3637}, 
        {4500, 3607, 3635}, {4510, 3604, 3632}, {4520, 3600, 3630}, {4530, 3597, 3627}, {4540, 3593, 3625}, 
        {4550, 3590, 3622}, {4560, 3586, 3620}, {4570, 3583, 3617}, {4580, 3580, 3615}, {4590, 3576, 3613}, 
        {4600, 3673, 3610}, {4610, 3569, 3608}, {4620, 3566, 3605}, {4630, 3563, 3603}, {4640, 3559, 3600}, 
        {4650, 3556, 3598}, {4660, 3553, 3595}, {4670, 3550, 3593}, {4680, 3546, 3591}, {4690, 3543, 3588}, 
        {4700, 3540, 3586}, {4710, 3537, 3583}, {4720, 3534, 3581}, {4730, 3530, 3578}, {4740, 3527, 3576}, 
        {4750, 3524, 3574}, {4760, 3521, 3571}, {4770, 3518, 3569}, {4780, 3515, 3567}, {4790, 3512, 3564}, 
        {4800, 3509, 3562}, {4810, 3506, 3560}, {4820, 3503, 3557}, {4830, 3499, 3555}, {4840, 3496, 3552}, 
        {4850, 3493, 3550}, {4860, 3490, 3548}, {4870, 3487, 3546}, {4880, 3485, 3543}, {4890, 3482, 3541}, 
        {4900, 3479, 3539}, {4910, 3476, 3536}, {4920, 3473, 3534}, {4930, 3470, 3532}, {4940, 3467, 3529}, 
        {4950, 3464, 3527}, {4960, 3461, 3525}, {4970, 3458, 3523}, {4980, 3456, 3520}, {4990, 3453, 3518}, 
        {5000, 3450, 3516}, {5010, 3447, 3514}, {5020, 3444, 3511}, {5030, 3442, 3509}, {5040, 3439, 3507}, 
        {5050, 3436, 3505}, {5060, 3433, 3502}, {5070, 3431, 3500}, {5080, 3428, 3498}, {5090, 3425, 3496}, 
        {5100, 3422, 3494}, {5110, 3420, 3492}, {5120, 3417, 3489}, {5130, 3414, 3487}, {5140, 3412, 3485}, 
        {5150, 3409, 3483}, {5160, 3406, 3481}, {5170, 3404, 3478}, {5180, 3401, 3476}, {5190, 3399, 3474}, 
        {5200, 3396, 3472}, {5210, 3394, 3470}, {5220, 3391, 3468}, {5230, 3388, 3466}, {5240, 3386, 3464}, 
        {5250, 3383, 3461}, {5260, 3381, 3459}, {5270, 3378, 3457}, {5280, 3376, 3455}, {5290, 3373, 3453}, 
        {5300, 3371, 3451}, {5310, 3368, 3449}, {5320, 3366, 3447}, {5330, 3363, 3445}, {5340, 3361, 3443}, 
        {5350, 3359, 3441}, {5360, 3356, 3439}, {5370, 3354, 3437}, {5380, 3351, 3434}, {5390, 3349, 3432}, 
        {5400, 3347, 3430}, {5410, 3344, 3428}, {5420, 3342, 3426}, {5430, 3340, 3424}, {5440, 3337, 3422}, 
        {5450, 3335, 3420}, {5460, 3333, 3418}, {5470, 3330, 3416}, {5480, 3328, 3414}, {5490, 3326, 3412}, 
        {5500, 3326, 3410}, {5510, 3321, 3408}, {5520, 3319, 3406}, {5530, 3317, 3404}, {5540, 3314, 3403}, 
        {5550, 3312, 3401}, {5560, 3310, 3399}, {5570, 3308, 3397}, {5580, 3305, 3395}, {5590, 3303, 3393}, 
        {5600, 3301, 3391}, {5610, 3299, 3389}, {5620, 3297, 3387}, {5630, 3295, 3385}, {5640, 3292, 3383}, 
        {5650, 3290, 3381}, {5660, 3288, 3379}, {5670, 3286, 3378}, {5680, 3284, 3376}, {5690, 3282, 3374}, 
        {5700, 3280, 3372}, {5710, 3278, 3370}, {5720, 3275, 3368}, {5730, 3273, 3366}, {5740, 3271, 3364}, 
        {5750, 3269, 3363}, {5760, 3267, 3361}, {5770, 3265, 3359}, {5780, 3263, 3357}, {5790, 3261, 3355}, 
        {5800, 3259, 3353}, {5810, 3257, 3352}, {5820, 3255, 3350}, {5830, 3253, 3348}, {5840, 3251, 3346}, 
        {5850, 3249, 3344}, {5860, 3247, 3342}, {5870, 3245, 3341}, {5880, 3243, 3339}, {5890, 3241, 3337}, 
        {5900, 3239, 3335}, {5910, 3237, 3334}, {5920, 3235, 3332}, {5930, 3233, 3330}, {5940, 3232, 3328}, 
        {5950, 3230, 3326}, {5960, 3228, 3325}, {5970, 3226, 3323}, {5980, 3224, 3321}, {5990, 3222, 3319}, 
        {6000, 3220, 3318}, {6010, 3218, 3316}, {6020, 3216, 3314}, {6030, 3215, 3313}, {6040, 3213, 3311}, 
        {6050, 3211, 3309}, {6060, 3209, 3307}, {6070, 3207, 3306}, {6080, 3205, 3304}, {6090, 3204, 3302}, 
        {6100, 3202, 3301}, {6110, 3200, 3299}, {6120, 3198, 3297}, {6130, 3196, 3296}, {6140, 3195, 3294}, 
        {6150, 3193, 3292}, {6160, 3191, 3291}, {6170, 3189, 3289}, {6180, 3188, 3287}, {6190, 3186, 3286}, 
        {6200, 3184, 3284}, {6210, 3182, 3282}, {6220, 3181, 3281}, {6230, 3179, 3279}, {6240, 3177, 3277}, 
        {6250, 3176, 3276}, {6260, 3174, 3274}, {6270, 3172, 3273}, {6280, 3171, 3271}, {6290, 3169, 3269}, 
        {6300, 3167, 3268}, {6310, 3165, 3266}, {6320, 3164, 3265}, {6330, 3162, 3263}, {6340, 3161, 3261}, 
        {6350, 3159, 3260}, {6360, 3157, 3258}, {6370, 3156, 3257}, {6380, 3154, 3255}, {6390, 3152, 3254}, 
        {6400, 3151, 3252}, {6410, 3149, 3250}, {6420, 3148, 3249}, {6430, 3146, 3247}, {6440, 3144, 3246}, 
        {6450, 3143, 3244}, {6460, 3141, 3243}, {6470, 3140, 3241}, {6480, 3138, 3240}, {6490, 3136, 3238}, 
        {6500, 3135, 3237}
        };

static const char *TAG_GPIO = "[GPIO]";

/**
 * @brief    This function initializes the soft PWM channel and pin.
 * @param[in] led_gpio_config: PWM channel config info.
 * @param[in] frequency:PWM frequency.
 * @param[in] led_white_control_mode:bulb white led driver method
 * @return    
 * @note     针对球泡灯（1路，2路冷暖可调，3路rgb彩灯可调，5路rgb彩+冷暖灯可调）的gpio通用配置方法
 * @waring
 */
ez_int8_t bulb_leds_gpio_config(led_gpio_config_t led_gpio_config[LEDC_CHANNEL_MAX], int frequency, int led_white_control_mode)
{
    int i = 0;
    ez_uint32_t pwm_led_duties[5] = {100, 100, 100, 100, 100}; //初始的占空比（亮度），频率为1000的情况下，占空比10%
    ez_uint32_t pwm_led_pin[5] = {0, 0, 0, 0, 0};              //r,g,b,w,c 五路灯珠的实际引脚

    if (led_gpio_config == NULL)
    {
        ezlog_e(TAG_GPIO, "%s  gpio config fail  %d!!!", __FUNCTION__);
        return -1;
    }

    for (i = 0; i < LEDC_CHANNEL_MAX; i++)
    {

        switch (led_gpio_config[i].led_color_name)
        {
        case CONFIG_COLOR_RED:
            pwm_led_pin[LED_COLOR_RED] = led_gpio_config[i].led_gpio_name;
            g_led_gpios_level.led_r = led_gpio_config[i].led_gpio_level;
            break;
        case CONFIG_COLOR_GREEN:
            pwm_led_pin[LED_COLOR_GREEN] = led_gpio_config[i].led_gpio_name;
            g_led_gpios_level.led_g = led_gpio_config[i].led_gpio_level;
            break;
        case CONFIG_COLOR_BLUE:
            pwm_led_pin[LED_COLOR_BLUE] = led_gpio_config[i].led_gpio_name;
            g_led_gpios_level.led_b = led_gpio_config[i].led_gpio_level;
            break;
        case CONFIG_COLOR_WARM:
            ezlog_w(TAG_GPIO, "LED_COLOR_WARM init!!!");
            pwm_led_pin[LED_COLOR_WARM] = led_gpio_config[i].led_gpio_name;
            g_led_gpios_level.led_w = led_gpio_config[i].led_gpio_level;
            break;

        case CONFIG_COLOR_COOL:
            ezlog_w(TAG_GPIO, "LED_COLOR_COOL init!!!");
            pwm_led_pin[LED_COLOR_COOL] = led_gpio_config[i].led_gpio_name;
            g_led_gpios_level.led_c = led_gpio_config[i].led_gpio_level;
            break;
        case CONFIG_CCT:
            ezlog_w(TAG_GPIO, "LED_COLOR_WARM init!!!");
            pwm_led_pin[LED_CCT] = led_gpio_config[i].led_gpio_name;
            g_led_gpios_level.led_cct = led_gpio_config[i].led_gpio_level;
            break;
        case CONFIG_CCT_LIGHT:
            ezlog_w(TAG_GPIO, "LED_COLOR_COOL init!!!");
            pwm_led_pin[LED_CCT_LIGHT] = led_gpio_config[i].led_gpio_name;
            g_led_gpios_level.led_cct_light = led_gpio_config[i].led_gpio_level;
            break;

        default:
            ezlog_e(TAG_GPIO, "unknow gpio config!");
            break;
        }
    }

    ezlog_i(TAG_GPIO, "pin[0] =%d,pin[1] =%d,pin[2] =%d,pin[3] =%d,pin[4] =%d!", pwm_led_pin[0], pwm_led_pin[1], pwm_led_pin[2], pwm_led_pin[3], pwm_led_pin[4]);

    g_led_pwm_cycle_time = 1000 * 1000 / frequency;

    g_led_white_control_mode = led_white_control_mode;

    for (i = 0; i < LEDC_CHANNEL_MAX; i++)
    {
        ezos_pwm_init(i, pwm_led_pin[i], g_led_pwm_cycle_time, pwm_led_duties[i]);
    }
    return 0;
}

/**@fn  
 * @brief  交互正常使用的是色温加亮度进行调节，对于cw 驱动的灯板，需要先转换在设置。
 * @param[in]     色温
 * @param[out]    色温转化为暖光的占比
 * @return  
 */
static float bulb_leds_ctrl_cct2Duty(ez_int16_t cct_value)
{
    double duty = -1;
    ez_int16_t i = 0, cie1931_x_temp = 0; // cie1931_y_temp = 0;
    ez_int8_t find_cct_flag = 0;

    for (i = 0; i < LED_CIE1931_NUM; i++)
    {
        if (led_cie1931_xy[i].cct_value == cct_value)
        {
            cie1931_x_temp = led_cie1931_xy[i].cie1931_x;
            //cie1931_y_temp = led_cie1931_xy[i].cie1931_y;
            find_cct_flag = 1;
            break;
        }
    }

    if (0 == find_cct_flag)
    {
        ezlog_e(TAG_LED, "cie1931_x_y can't find!");
        return -1;
    }

    if (3135 == cie1931_x_temp) //色温为6500 转化而来，duty 应该为0 liwei@20200819
    {
        duty = 0;
    }
    else
    {
        /* 色温为2700 ，得出duty=1*/
        duty = 1.0 / (1.0 + (3237 * 1.0 / 4107) * ((4593 - cie1931_x_temp) * 1.0 / (cie1931_x_temp - 3135)));
    }

    return (duty * 100);
}

/**@fn  
 * @brief  点亮cct灯
 * @param[in]  cct_value：色温，led_lm_percentage 亮度，范围0-1000千分之一精度设置,由非smart函数拷贝而来，改变了宏值
 * @param[out]
 * @return
 */
ez_int8_t bulb_leds_cct_config(ez_int16_t cct_value, ez_int16_t led_lm_percentage)
{
    float config_duty = 0, tmp_duty = 0;
    ez_uint32_t pwm_led_duties[5] = {0, 0, 0, 0, 0};

    ezlog_v(TAG_LED, "cct = %d  smart_lm= %d", cct_value, led_lm_percentage);

    g_led_current_param.brightness = led_lm_percentage;

    g_led_current_param.cct_value = cct_value;

    if (g_led_white_control_mode == 1) // cct  驱动方式
    {
        if (cct_value < 2700)
        {
            tmp_duty = 0;
        }
        else if (cct_value > 6500)
        {
            tmp_duty = 100;
        }
        else
        {
            tmp_duty = (cct_value - 2700) / 38;
        }

        pwm_led_duties[LED_CCT] = tmp_duty * g_led_pwm_cycle_time / 100;

        if (g_led_gpios_level.led_c == LED_GPIO_LEVEL_LOW)
        {
            pwm_led_duties[LED_CCT_LIGHT] = g_led_pwm_cycle_time - pwm_led_duties[3];
        }

        pwm_led_duties[LED_CCT_LIGHT] = led_lm_percentage * g_led_pwm_cycle_time / LM_CV_STEP_MAX_SMART; //0~1000的精度

        if (g_led_gpios_level.led_cct_light == LED_GPIO_LEVEL_LOW)
        {
            pwm_led_duties[LED_CCT_LIGHT] = g_led_pwm_cycle_time - pwm_led_duties[4];
        }
    }
    else //CW 驱动方式
    {
        cct_value = cct_value / 10 * 10; //cct 最小10的变化，由led_cie1931_xy_t限制

        config_duty = bulb_leds_ctrl_cct2Duty(cct_value);
        if (config_duty == -1)
        {
            ezlog_e(TAG_LED, "get config duty fail, cct_valu %d!\n", cct_value);
            return -1;
        }

        tmp_duty = config_duty * g_led_pwm_cycle_time / 100;

        pwm_led_duties[LED_COLOR_WARM] = tmp_duty * led_lm_percentage / LM_CV_STEP_MAX_SMART;

        if (g_led_gpios_level.led_w == LED_GPIO_LEVEL_LOW)
        {
            pwm_led_duties[LED_COLOR_WARM] = g_led_pwm_cycle_time - pwm_led_duties[LED_COLOR_WARM];
        }

        pwm_led_duties[LED_COLOR_COOL] = (g_led_pwm_cycle_time - tmp_duty) * led_lm_percentage / LM_CV_STEP_MAX_SMART;

        if (g_led_gpios_level.led_c == LED_GPIO_LEVEL_LOW)
        {
            pwm_led_duties[LED_COLOR_COOL] = g_led_pwm_cycle_time - pwm_led_duties[LED_COLOR_WARM];
        }
    }

    //ezlog_e(TAG_LED, "mode =  %d duties[3] = %d  duties[4]= %d",g_led_cct_control_mode_flag, pwm_led_duties[3], pwm_led_duties[4]);

    for (int i = 0; i < LEDC_CHANNEL_MAX; i++)
    {
        ezos_pwm_set_duty(i, pwm_led_duties[i]);
    }

    ezos_pwm_start();

    return 0;
}

/**@fn  
 * @brief  Ezviz_Led_RGB_Config拷贝而来，led_lm_percentage 为0-1000
 * @param[in]  
 * @param[out] 
 * @return	  
 */
ez_int8_t bulb_leds_RGB_config(ez_int32_t color_value, ez_int16_t led_lm_percentage)
{
    ez_uint32_t red = 0, green = 0, blue = 0;
    ez_uint32_t pwm_led_duties[5] = {0, 0, 0, 0, 0};

    g_led_current_param.brightness = led_lm_percentage;

    g_led_current_param.rgb = color_value;

    red = ((color_value >> 16) & 0xff) * g_led_pwm_cycle_time / 255;
    pwm_led_duties[LED_COLOR_RED] = red * 1.0 * led_lm_percentage / LM_CV_STEP_MAX_SMART; //0~1000的精度

    if (g_led_gpios_level.led_r == LED_GPIO_LEVEL_LOW)
    {
        pwm_led_duties[LED_COLOR_RED] = g_led_pwm_cycle_time - pwm_led_duties[LED_COLOR_RED];
    }

    green = ((color_value >> 8) & 0xff) * g_led_pwm_cycle_time / 255;

    pwm_led_duties[LED_COLOR_GREEN] = green * 1.0 * led_lm_percentage / LM_CV_STEP_MAX_SMART; //0~1000的精度

    if (g_led_gpios_level.led_g == LED_GPIO_LEVEL_LOW)
    {
        pwm_led_duties[LED_COLOR_GREEN] = g_led_pwm_cycle_time - pwm_led_duties[LED_COLOR_GREEN];
    }

    blue = ((color_value >> 0) & 0xff) * g_led_pwm_cycle_time / 255;

    pwm_led_duties[LED_COLOR_BLUE] = blue * 1.0 * led_lm_percentage / LM_CV_STEP_MAX_SMART; //0~1000的精度

    if (g_led_gpios_level.led_b == LED_GPIO_LEVEL_LOW)
    {
        pwm_led_duties[LED_COLOR_BLUE] = g_led_pwm_cycle_time - pwm_led_duties[LED_COLOR_BLUE];
    }

    //ezlog_d(TAG_LED, "%s g_led_current_RGB_light=%d red=%d  green=%d  blue=%d!!!", __FUNCTION__, g_led_current_RGB_light,pwm_led_duties[0], pwm_led_duties[1], pwm_led_duties[2]);

    for (int i = 0; i < LEDC_CHANNEL_MAX; i++)
    {
        ezos_pwm_set_duty(i, pwm_led_duties[i]);
    }
    ezos_pwm_start();

    return 0;
}

void bulb_led_stop()
{
    ezos_pwm_stop();
}

void bulb_led_start()
{
    ezos_pwm_start();
}

void bulb_led_restart()
{
    ezos_pwm_stop(0);
    ezos_pwm_start();
}
