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

#include <stdio.h>

#include <ezlog.h>
#include "ezos_thread.h"
#include "ez_iot_core_def.h"
#include "ez_iot_core.h"

#include "bulb_led_ctrl.h"
#include "bulb_led_drv_pwm.h"
#include "bulb_business.h"

light_param_t g_bulb_param = {0};               //灯泡所有的参数
extern led_current_param_t g_led_current_param; //灯泡实时的亮度
int g_brightness_upper = 100;                   //灯泡实际最高亮度值
int g_brightness_lower = 0;                     //灯泡实际最低亮度值，

int get_brightness_limit()
{
    // todo.
    // initialize the g_brightness_upper and g_brightness_lower

    return 0;
}

//某些灯泡在低亮度时会闪烁，需要做一个转化
int light_brigtness_to_limit(int light_brightness)
{
    int tmp_brightness = light_brightness;

    if (g_brightness_upper != 0 && g_brightness_upper >= g_brightness_lower)
    {
        tmp_brightness = (tmp_brightness / 100.0) * (g_brightness_upper - g_brightness_lower) + g_brightness_lower;
    }

    return tmp_brightness;
}

/**@fn  
 * @brief  
 * @param[in]  
 * @param[out] 
 * @return	  
 */
int turn_on_lamp()
{
    int dstBrigtness = 0;
    led_ctrl_t led_ctrl_cmd = {0};

    if (false == g_bulb_param.swit)
    {
        g_bulb_param.swit = true;
    }
    else
    {
        ezlog_e(TAG_LIGHT, "receive turn on ctrlcmd,but the bulb switch is already turned on.");
        return -1;
    }

    switch (g_bulb_param.mode)
    {
    case LIGHT_COLOR:
        /*同时调节亮度和颜色到指定的值*/
        //led_ctrl_rgb(g_led_current_RGB_light,light_brigtness_to_limit(light_cfg.rgb.led_lm_percentage),g_led_current_RGB_value,light_cfg.rgb.color_value,LM_CV_ONECE_TIME,50);

        led_ctrl_cmd.iRgbValue = g_bulb_param.rgb;
        led_ctrl_cmd.nbrightness = light_brigtness_to_limit(g_bulb_param.brightness);
        led_ctrl_cmd.nUpDuration = led_ctrl_cmd.nbrightness * 10; //变化时间500ms,颜色调节接口，颜色主变，亮度在颜色完成变化后，变化到位

        led_ctrl_do_async(&led_ctrl_cmd);
        break;
    case LIGHT_WHITE:
    {
        led_ctrl_cmd.nCctValue = g_bulb_param.cct;
        led_ctrl_cmd.iRgbValue = 0;
        if (dstBrigtness > 80)
        {
            led_ctrl_cmd.nbrightness = 80;
            led_ctrl_cmd.nUpDuration = 1200; //变化时间1200ms
            led_ctrl_do_async(&led_ctrl_cmd);

            led_ctrl_cmd.nbrightness = dstBrigtness;
            led_ctrl_cmd.nUpDuration = (dstBrigtness - 80) * 10;
            led_ctrl_do_sync(&led_ctrl_cmd);
        }
        else
        {
            led_ctrl_cmd.nbrightness = dstBrigtness;
            led_ctrl_cmd.nUpDuration = dstBrigtness * 10; //变化时间为亮度的百分比*10ms,e.g %10亮度，100ms完成，50%亮度，500ms 完成
            led_ctrl_do_async(&led_ctrl_cmd);
        }
    }
    break;
    case LIGHT_SCENE:
        ezlog_e(TAG_LIGHT, "turn on the scene mode.");
        // todo:
        break;
    default:
        break;
    }
    return 0;
}

int turn_off_lamp()
{
    int srcBrigtness = g_led_current_param.cct_value;
    led_ctrl_t led_ctrl_cmd = {0};

    /* 系统测试缺陷：

    已设置倒计时任务，通过app首页切换灯泡开关，查看效果  结果：倒计时任务不取消
    现象二：已设置倒计时任务，通过app详情页切换灯泡开关，查看效果  结果：倒计时任务取消
    修改时区，夏令时，开灯，关灯，统一都取消倒计时任务
    */

    if (true == g_bulb_param.swit)
    {
        g_bulb_param.swit = false;
    }
    else
    {
        ezlog_e(TAG_LIGHT, "receive turn off ctrlcmd,but the bulb switch is already turned off.");
        return -1;
    }

    switch (g_bulb_param.mode)
    {
    case LIGHT_COLOR:
    {
        led_ctrl_cmd.iRgbValue = g_led_current_param.rgb;
        led_ctrl_cmd.nbrightness = 0;
        led_ctrl_cmd.nUpDuration = srcBrigtness * 10; //变化时间根据亮度来变化
        led_ctrl_do_async(&led_ctrl_cmd);
    }
    break;
    case LIGHT_WHITE:
    {
        led_ctrl_cmd.nCctValue = g_led_current_param.cct_value;

        led_ctrl_cmd.iRgbValue = 0;
        if (srcBrigtness > 80)
        {

            led_ctrl_cmd.nbrightness = 80;
            led_ctrl_cmd.nUpDuration = (srcBrigtness - 80) * 10; //变化时间100ms
            led_ctrl_do_async(&led_ctrl_cmd);

            led_ctrl_cmd.nbrightness = 0;
            led_ctrl_cmd.nUpDuration = 1200; //变化时间1200ms，就白灯最高亮度熄灭采用特殊的延时时间
            led_ctrl_do_sync(&led_ctrl_cmd);
        }
        else
        {
            led_ctrl_cmd.nbrightness = 0;
            led_ctrl_cmd.nUpDuration = srcBrigtness * 10; //变化时间100ms
            led_ctrl_do_async(&led_ctrl_cmd);
        }
    }
    break;

    case LIGHT_SCENE:
    case LIGHT_MUSIC: //暂时没用上，理论上灯泡在律动情况下，不会收到关灯指令，APP 的一切操作之前都需要关掉律动功能。
    {

        /* 场景合音乐律动的状态不确定，因不知初始状态(白灯或者彩灯)，需要关两次，因为本身为变化的，这里立即关闭灯泡
         	彩灯正常比较暗，先关彩灯，不会太明显（否则彩灯色块下，白灯方式关闭，会用白灯方式将亮度调到0，显示为闪烁），再关白灯时，若此时亮度已经为0，不会在闪烁
			后续灯控都可以采用此方式 进行控制，不需要 区分灯的产品类型，灯的类型可以在底层屏蔽，白光灯关两次，彩光灯关闭底层将不执行
         */
        led_ctrl_cmd.nbrightness = 0;
        led_ctrl_cmd.nUpDuration = 10; //立即关闭
        led_ctrl_cmd.iRgbValue = g_led_current_param.rgb;
        led_ctrl_do_async(&led_ctrl_cmd);

        led_ctrl_cmd.iRgbValue = 0; //rgbvalue 置0 采用白光方式调亮度
        led_ctrl_cmd.nCctValue = g_led_current_param.cct_value;
        led_ctrl_do_sync(&led_ctrl_cmd); //等待上一条指令执行完毕后执行白灯关闭
    }

    break;
    default:
        break;
    }

    return 0;
}

// note 1-100 亮度
int adjust_light_brightness(int light_brightness)
{
    led_ctrl_t led_ctrl_cmd = {0};
    int iChangeRange = 10;

    ezlog_i(TAG_LIGHT, "adjust_light_brightness in.");

    if (g_bulb_param.brightness == light_brightness) //设置的亮度与设备APP 记录的亮度一致，不变化，正常APP 不会进入
    {
        ezlog_d(TAG_LIGHT, "brightness is not changed.");
        return 0;
    }

    if (g_bulb_param.brightness > light_brightness)
    {
        iChangeRange = g_bulb_param.brightness - light_brightness; //采用实际亮度值进行计算渐变时间
    }
    else
    {
        iChangeRange = light_brightness - g_bulb_param.brightness;
    }
    int tmp_brightness = light_brigtness_to_limit(light_brightness);
    /*调节到目标亮度*/
    led_ctrl_cmd.nbrightness = tmp_brightness;

    led_ctrl_cmd.nUpDuration = iChangeRange * 10; //根据亮度差值来计算变化时间

    if (tmp_brightness > 100 || tmp_brightness < 0)
    {
        ezlog_e(TAG_LIGHT, "light brightness param error.");
        return -1;
    }

    switch (g_bulb_param.mode)
    {
    case LIGHT_COLOR:

        led_ctrl_cmd.iRgbValue = g_bulb_param.rgb;
        if (0 != led_ctrl_do_async(&led_ctrl_cmd))
        {
            ezlog_e(TAG_LIGHT, "color-mode adjust light_color failed.");
            return -1;
        }
        break;
    case LIGHT_WHITE:
    {
        led_ctrl_cmd.nCctValue = g_bulb_param.cct;
        if (0 != led_ctrl_do_async(&led_ctrl_cmd))
        {
            ezlog_e(TAG_LIGHT, "white-mode adjust light_color failed.");
            return -1;
        }
    }
    break;
    default:
        ezlog_e(TAG_LIGHT, "mode =%d,is not support adjust bright.");
        break;
    }

    g_bulb_param.brightness = light_brightness;

    return 0;
}

int adjust_light_cct(int light_cct)
{

    led_ctrl_t led_ctrl_cmd = {0};
    int iChangeRange = 10;

    if (g_bulb_param.cct == light_cct)
    {
        ezlog_d(TAG_LIGHT, "light cct is not changed.");
        return 0;
    }

    if (2700 > light_cct || 6500 < light_cct)
    {
        ezlog_e(TAG_LIGHT, "light cct param error.");
        return -1;
    }
    if (g_bulb_param.cct > light_cct)
    {
        iChangeRange = g_bulb_param.cct - light_cct; //色温变化的时长
    }
    else
    {
        iChangeRange = light_cct - g_bulb_param.cct; //色温变化的时长
    }
    led_ctrl_cmd.nbrightness = g_bulb_param.brightness;
    led_ctrl_cmd.nCctValue = light_cct;
    led_ctrl_cmd.nUpDuration = iChangeRange * 2;

    if (0 != led_ctrl_do_async(&led_ctrl_cmd))
    {
        ezlog_e(TAG_LIGHT, "white-mode adjust light_color failed.");
        return -1;
    }

    g_bulb_param.cct = light_cct;
    g_bulb_param.mode = LIGHT_WHITE;

    return 0;
}

/*只调节颜色，不调节亮度*/
int adjust_light_rgb(int light_rgb)
{
    int iRet = 0;
    led_ctrl_t led_ctrl_cmd = {0};

    if (g_bulb_param.rgb == light_rgb)
    {
        ezlog_d(TAG_LIGHT, "light rgb is not changed.");
        return 0;
    }

    if (light_rgb > 0xffffff || light_rgb < 0)
    {
        ezlog_e(TAG_LIGHT, "light rgb param error.");
        return -1;
    }

    /* 由当前颜色调节到目标颜色，当前亮度与目标亮度一致都是g_led_current_RGB_light*/
    led_ctrl_cmd.iRgbValue = light_rgb;
    led_ctrl_cmd.nbrightness = g_bulb_param.brightness;
    led_ctrl_cmd.nUpDuration = 0; //变化时间由内部算法自己控制

    iRet = led_ctrl_do_async(&led_ctrl_cmd);
#if 0
    if (0 != Ezviz_Led_RGB_Config_CV(light_rgb, -1))
    {
        return -1;
    }
#endif

    g_bulb_param.rgb = light_rgb;
    g_bulb_param.mode = LIGHT_COLOR;

    /* 将g_bulb_param 参数保存到flash 中*/
    printf("\n to_do DEBUG in line (%d) and function (%s)): save bulb param to flash\n ", __LINE__, __func__);

    return iRet;
}

int reset_light(int mode)
{
    ezlog_w(TAG_LIGHT, "reset light.mode =%d", mode);

    if (0 == mode) //完全恢复出厂
    {
        printf("\n to_do DEBUG in line (%d) and function (%s)): \n ", __LINE__, __func__);
    }
    else if (2 == mode) //除开网络，产测参数，服务器以外的信息
    {
        printf("\n to_do DEBUG in line (%d) and function (%s)): \n ", __LINE__, __func__);
        //todo
    }
    else if (3 == mode) //除开产测参数，以外的信息,退货换到其他用户手里
    {
        printf("\n to_do DEBUG in line (%d) and function (%s)): \n ", __LINE__, __func__);
        //todo
    }
    else
    {
        printf("\n to_do DEBUG in line (%d) and function (%s)): \n ", __LINE__, __func__);
        //todo
    }

    //todo

    return 0;
}

int restart_light()
{
    ezlog_w(TAG_LIGHT, "restart system.");
    printf("\n to_do DEBUG in line (%d) and function (%s)): \n ", __LINE__, __func__);

    return 0;
}

bool get_light_switch()
{
    return g_bulb_param.swit;
}

int get_light_brightness()
{
    return g_bulb_param.brightness;
}

int get_light_color_temperature()
{
    return g_bulb_param.cct;
}

int get_light_color_rgb()
{
    return g_bulb_param.rgb;
}

int get_light_mode()
{
    return g_bulb_param.mode;
}

void bulb_ctrl_init()
{
    ez_err_t rv = EZ_CORE_ERR_SUCC;

    /* 此处需要实际从配置文件中读取灯泡的参数*/
    g_bulb_param.swit = 1;
    g_bulb_param.brightness = 100;
    g_bulb_param.mode = LIGHT_WHITE;
    g_bulb_param.cct = 3000;
    g_bulb_param.rgb = 0xFF0000;

    ez_thread_t bulb_ctrl_thread;
    const ez_char_t *bulb_ctrl_thread_name = "bulb_ctrl_main";
    rv = ezos_thread_create(&bulb_ctrl_thread, bulb_ctrl_thread_name, led_ctrl_Task, (void *)bulb_ctrl_thread_name, 4 * 1024, 5);
    if (EZ_CORE_ERR_SUCC != rv)
    {
        ezlog_i(TAG_APP, "creat bulb_ctrl thread error ,no memory");
    }
}
