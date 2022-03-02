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
#include <string.h> 

#include "ezos_time.h" //延迟需要
#include <ezos_sem.h>

#include "ezlog.h"
#include "ez_iot_core_def.h"
#include "ez_iot_core.h"

#include "bulb_led_ctrl.h"
#include "bulb_led_drv_pwm.h"

ez_sem_t ctrl_sem;
led_ctrl_t g_stru_led_ctrl;

extern led_current_param_t g_led_current_param; //灯泡当前的瞬时状态
uint8_t g_b_ModeChanged = 0;                    //灯泡渐变过程中，需要打断渐变过程进入下一次渐变过程

/**@fn		  
 * @brief		  该接口用于获取颜色渐变过程的各个颜色值,当dur_loops 为0时，系统自动计算获取变化次数
 				  通过宏LM_CV_VALUE_STEP_MAX 来控制最大变化次数，最小的次数除以三
 				  通过宏LM_CV_VALUE_STEP_MAX = 150当rgb 三者颜色都在变化的情况下，e.g红色0xff0000-》蓝绿0xffff，则变化50次。
				  单色变化时，变化150次
				  双色变化时，变化75次
			      
 				  根据实际测试，变化间隔在30ms 内，肉眼不会有抖动，
 				  10ms 调用一次会导致线程占用cpu 过长，所以可以选择20和30ms 停留时间，暂定用20ms
 				  呼吸时间由duration 控制，
 				  
 * @param[in]  
 * @param[out] 
 * @return	  
 */
int get_rgb_Change_loops(int32_t start_rgb_value, int32_t stop_rgb_value, int *set_rgb_value_rgb, int dur_loops)
{
    COLOR_RGB rgb_start, rgb_stop;
    uint8_t start_r = 0, stop_r = 0, start_g = 0, stop_g = 0, start_b = 0, stop_b = 0;
    int loop_r = 0, loop_g = 0, loop_b = 0, loop_r_cnt = 0, loop_g_cnt = 0, loop_b_cnt = 0, max_loop = 0;
    int loop_count = 0, total_r = 0, total_g = 0, total_b = 0, total_rgb = 0;
    int j = 0;

    if (dur_loops > LM_CV_VALUE_STEP_MAX)
    {
        ezlog_v(TAG_APP, "change loops is too big,dur_loops=%d!", dur_loops);
        return -1;
    }

    if (start_rgb_value == stop_rgb_value)
    {
        ezlog_v(TAG_APP, "rgb change loops return 0 because start_rgb_value is same with stop_rgb_value!");
        return 0;
    }

    rgb_start.R = (start_rgb_value >> 16) & 0xff;
    rgb_start.G = (start_rgb_value >> 8) & 0xff;
    rgb_start.B = start_rgb_value & 0xff;

    rgb_stop.R = (stop_rgb_value >> 16) & 0xff;
    rgb_stop.G = (stop_rgb_value >> 8) & 0xff;
    rgb_stop.B = stop_rgb_value & 0xff;

    if (set_rgb_value_rgb == NULL)
    {
        ezlog_v(TAG_APP, "MEMORY not enough!");
        return -1;
    }

    memset(set_rgb_value_rgb, 0, LM_CV_VALUE_STEP_MAX * 4);

    if (rgb_start.R > rgb_stop.R)
    {
        total_r = rgb_start.R - rgb_stop.R;
    }
    else if (rgb_start.R < rgb_stop.R)
    {
        total_r = rgb_stop.R - rgb_start.R;
    }

    if (rgb_start.G > rgb_stop.G)
    {
        total_g = rgb_start.G - rgb_stop.G;
    }
    else if (rgb_start.G < rgb_stop.G)
    {
        total_g = rgb_stop.G - rgb_start.G;
    }

    if (rgb_start.B > rgb_stop.B)
    {
        total_b = rgb_start.B - rgb_stop.B;
    }
    else if (rgb_start.B < rgb_stop.B)
    {
        total_b = rgb_stop.B - rgb_start.B;
    }

    total_rgb = total_r + total_g + total_b;
    if (0 != dur_loops) /* 按整个调整时间算需要分片的次数，若颜色差值不大，循环次数减少，用于场景变化*/
    {
        loop_r = (total_r > dur_loops) ? dur_loops : total_r;
        loop_g = (total_g > dur_loops) ? dur_loops : total_g;
        loop_b = (total_b > dur_loops) ? dur_loops : total_b;
    }
    else /* 原有逻辑暂时不变，用于颜色进度条实时调整颜色*/
    {
        loop_r = LM_CV_VALUE_STEP_MAX * total_r / total_rgb;
        loop_g = LM_CV_VALUE_STEP_MAX * total_g / total_rgb;
        loop_b = LM_CV_VALUE_STEP_MAX * total_b / total_rgb;
    }
    ezlog_v(TAG_APP, "r=%d g=%d b=%d total=%d", total_r, total_g, total_b, total_rgb);
    ezlog_v(TAG_APP, "r=%f g=%f b=%f", 1.0 * total_r / total_rgb, 1.0 * total_g / total_rgb, 1.0 * total_b / total_rgb);
    ezlog_v(TAG_APP, "r=%d g=%d b=%d,max_loop=%d", loop_r, loop_g, loop_b, max_loop);

    max_loop = loop_r;
    if (max_loop < loop_g)
        max_loop = loop_g;
    if (max_loop < loop_b)
        max_loop = loop_b;

    start_r = rgb_start.R;
    stop_r = rgb_stop.R;
    start_g = rgb_start.G;
    stop_g = rgb_stop.G;
    start_b = rgb_start.B;
    stop_b = rgb_stop.B;

    loop_r_cnt = 0;
    loop_g_cnt = 0;
    loop_b_cnt = 0;

    //ESP_LOGE(TAG_LED, "RRRR %d",(start_a-start_b)/count_t);
    for (j = 1; j <= max_loop; j++)
    {
        if (loop_r_cnt < loop_r)
        {
            loop_r_cnt++;
            if (start_r > stop_r)
            {
                rgb_start.R = start_r - (start_r - stop_r) * j / loop_r;
            }
            else if (start_r < stop_r)
            {
                rgb_start.R = start_r + (stop_r - start_r) * j / loop_r;
            }
        }

        if (loop_g_cnt < loop_g)
        {
            loop_g_cnt++;
            if (start_g > stop_g)
            {
                rgb_start.G = start_g - (start_g - stop_g) * j / loop_g;
            }
            else if (start_g < stop_g)
            {
                rgb_start.G = start_g + (stop_g - start_g) * j / loop_g;
            }
        }

        if (loop_b_cnt < loop_b)
        {
            loop_b_cnt++;
            if (start_b > stop_b)
            {
                rgb_start.B = start_b - (start_b - stop_b) * j / loop_b;
            }
            else if (start_b < stop_b)
            {
                rgb_start.B = start_b + (stop_b - start_b) * j / loop_b;
            }
        }

        set_rgb_value_rgb[loop_count++] = (rgb_start.R << 16) | (rgb_start.G << 8) | rgb_start.B;
    }
    ezlog_v(TAG_APP, "get_rgb_change_loops=%d", loop_count);
    return loop_count;
}

/**@fn		  
 * @brief		灯泡颜色变化控制
 * @param[in] dst_lm:颜色变化到目标亮度，0~100   
 * @param[in] dst_rgb：目标颜色				
 * @param[in] sleep_ms 一般设定为20ms或者30ms ，此时灯泡连续变化不会闪烁。
 * @param[in] rgb_loops  是由外部传递进来，表示预期的变化的次数，但实际由于颜色或者亮度差值没那么大，
 *  				变化过程可能达不到指定的次数,rgb_loops  为0 ，表示由算法自动调整变化时间， 
 * 					sleep_ms*chg_loops表示灯泡实际变化持续的预期时间
 * @param[out] 
 * @return	  
 */

int32_t led_ctrl_rgb(int dst_lm, int dst_rgb, int sleep_ms, int rgb_loops)
{

    int j = 0;
    int32_t set_rgb_value_rgb[LM_CV_VALUE_STEP_MAX] = {0};
    int32_t rgb_count_loop;
    int ret = 0;
    int tmp_lm = 0;
    int src_lm = g_led_current_param.brightness;
    int src_rgb = g_led_current_param.rgb;

    if (dst_lm < 0 || dst_lm > 100)
    {
        ezlog_e(TAG_APP, "led_ctrl_rgb brightness param is error src_lm=%d,dst_lm=%d ", src_lm, dst_lm);
        return -1;
    }

    if (src_rgb < 0 || src_rgb > 0xffffff || dst_rgb < 0 || dst_rgb > 0xffffff)
    {
        ezlog_e(TAG_APP, "led_ctrl_rgb color Rgb param is error src_lm=%d,dst_lm=%d ", src_lm, dst_lm);
        return -1;
    }

    if (sleep_ms < LM_CV_ONECE_TIME_SHORT)
    {
        sleep_ms = LM_CV_ONECE_TIME_SHORT;
    }

    if (rgb_loops > LM_CV_VALUE_STEP_MAX)
    {
        rgb_loops = LM_CV_VALUE_STEP_MAX;
    }

    dst_lm *= 10;
    ezlog_v(TAG_APP, "the src_rgb is 0x%x ,dst_rgb is 0x%x,rgb_loop=%d,sleep_ms=%d,src_lm=%d,dst_lm=%d\n ", src_rgb, dst_rgb, rgb_loops, sleep_ms, src_lm, dst_lm);

    /* 进入彩光调节测试模式。
	   若颜色开始值和结束值不一致，则进行颜色呼吸调节，亮度采用dst_lm
	   若颜色开始值和结束值一致，且亮度不一致，则根据亮度进行循环呼吸
	   若颜色值和亮度值都一致，则设定到指定的颜色查看灯的颜色是否正确
	   
	*/
    if (dst_rgb != src_rgb)
    {

        memset(set_rgb_value_rgb, 0, LM_CV_VALUE_STEP_MAX * 4);
        /* 彩光变化比白光变化复杂，先计算出整个过程中各颜色调节值，得出实际需要的变化次数
		   而白灯，则不需要这一步	 
		*/
        rgb_count_loop = get_rgb_Change_loops(src_rgb, dst_rgb, set_rgb_value_rgb, rgb_loops);

        for (j = 0; j < rgb_count_loop; j++)
        {
            if (src_lm == dst_lm)
            {
                tmp_lm = dst_lm;
            }
            else
            {
                /*最大变化次数为颜色渐变的次数，
				  若亮度值差异较小，根据肉眼在低亮度变化区域比较敏感，亮度值在亮度较高侧随着颜色变化进行调节，
				*/
                if (dst_lm > src_lm) //亮度值由低变高
                {
                    if ((dst_lm - src_lm) > rgb_count_loop)
                    {
                        tmp_lm = src_lm + (dst_lm - src_lm) * (j + 1) / (rgb_count_loop);
                    }
                    else
                    {
                        if ((dst_lm - src_lm) >= (rgb_count_loop - j))
                        {
                            tmp_lm = j + dst_lm - rgb_count_loop + 1; //src_lm + j-((rgb_count_loop-1)-(dst_lm-src_lm))亮度值在后半程随着颜色变化进行调节，直到目标值,测试用例，亮度值变化小，变化格数大,算法，j取两端值要符合条件
                        }
                        else
                        {
                            tmp_lm = src_lm;
                        }
                    }
                }
                else //亮度值由高变低
                {
                    if ((src_lm - dst_lm) > rgb_count_loop)
                    {
                        tmp_lm = src_lm - (src_lm - dst_lm) * (j + 1) / (rgb_count_loop);
                    }
                    else
                    {
                        if ((src_lm - j) >= dst_lm)
                        {
                            tmp_lm = src_lm - j; //亮度值先随着颜色变化进行调节减小，直到目标值
                        }
                        else
                        {
                            tmp_lm = dst_lm;
                        }
                    }
                }
            }

            if (g_b_ModeChanged)
            {
                break;
            }

            ret = bulb_leds_RGB_config(set_rgb_value_rgb[j], tmp_lm);
            if (ret == -1)
            {
                break;
            }
            ezos_delay_ms(sleep_ms);
        }
    }
    else
    {

        if (0 == rgb_loops)
        {
            rgb_loops = 1;
        }
        if (src_lm < dst_lm)
        {
            for (j = src_lm; j <= dst_lm; j++)
            {
                //app 设定的变化总时间是1000ms （1000/20=50=chg_loops），灯泡的亮度变化值50%~60%，500,502,504..600
                if ((dst_lm - src_lm) >= rgb_loops)
                {
                    tmp_lm = src_lm + (dst_lm - src_lm) * (j - src_lm + 1) / rgb_loops;
                }
                else //app 设定的变化总时间是3000ms(150=chg_loops) ，灯泡的亮度变化值50-60%，比颜色提前结束变化，所有亮度值已遍历到，且达到指定亮度
                {
                    tmp_lm = j;
                }

                if (g_b_ModeChanged)
                {
                    break;
                }

                bulb_leds_RGB_config(src_rgb, tmp_lm);
                ezos_delay_ms(sleep_ms);
                if ((j - src_lm + 1) == rgb_loops)
                {
                    break; //所有亮度由低到高都遍历到，循环chg_loops后，提前退出循环
                }
            }
        }
        else if (src_lm > dst_lm)
        {
            for (j = src_lm; j >= dst_lm; j--)
            {
                //app 设定的变化总时间是1000ms （1000/20=50=chg_loops），灯泡的亮度变化值60%-50%，600,598,596..502,0
                if ((src_lm - dst_lm) >= rgb_loops)
                {
                    tmp_lm = src_lm - (src_lm - dst_lm) * (src_lm - j + 1) / rgb_loops;
                }
                else //app 设定的变化总时间是3000ms(3000/20=150=chg_loops) ，灯泡的亮度变化值60%-50%，用最精细的精度延迟变化，也会提前结束变化,所有亮度值已遍历到，已达到指定亮度
                {
                    tmp_lm = j;
                }

                if (g_b_ModeChanged)
                {
                    break;
                }

                bulb_leds_RGB_config(src_rgb, tmp_lm);
                ezos_delay_ms(sleep_ms);

                if ((src_lm - j + 1) == rgb_loops)
                {
                    break; //所有亮度由高到低都遍历到，循环chg_loops后,退出循环
                }
            }
        }
        else
        {
            bulb_leds_RGB_config(src_rgb, src_lm);
            for (j = 0; j < rgb_loops; j++)
            {
                if (g_b_ModeChanged)
                {
                    break;
                }

                ezos_delay_ms(sleep_ms);
            }
        }
    }
    return 0;
}

/**@fn		  
 * @brief		灯泡显示方法控制 白光灯,控制精度是千分之一，
				亮度变化，则色温辅助变化，亮度变化完，色温也需要变化完（亮度假设在10步内完成，色温也要10步完成）
				亮度不变，则色温主变（可以兼容色温渐变 的 DIY需求）
 				以上主要考虑实际消费场景没有要求色温在渐变占主要（产品经理需要再定下）
 * @param[in]  
 * @param[out] 
 * @return	  
 */

int32_t led_ctrl_cct(int dst_cct, int dst_lm, int sleep_ms, int chg_loops)
{
    int j = 0;
    int tmp_lm = 0;
    int tmpCct = 0;

    if (sleep_ms < LM_CV_ONECE_TIME_SHORT)
    {
        sleep_ms = LM_CV_ONECE_TIME_SHORT;
    }

    if (chg_loops > LM_CV_VALUE_STEP_MAX)
    {
        chg_loops = LM_CV_VALUE_STEP_MAX;
    }

    if (0 == chg_loops)
    {
        chg_loops = 1;
    }

    int src_cct = g_led_current_param.cct_value;
    int src_lm = g_led_current_param.brightness;

    ezlog_v(TAG_APP, "the src_cct is %d ,dst_cct is %d,chg_loop=%d,sleep_ms=%d,src_lm=%d,dst_lm=%d\n ", src_cct, dst_cct, chg_loops, sleep_ms, src_lm, dst_lm);
    /* 若亮度开始值和结束值不一致，则进行循环调亮度
	   若亮度开始值和结束值一致，则设置到指定的亮度以及指定的色温。
	*/
    //src_lm *= 10;
    dst_lm *= 10;

    if (src_lm != dst_lm) //亮度主变，色温辅助变化
    {
        if (dst_lm > src_lm) //亮度值由低变高
        {
            for (j = src_lm; j <= dst_lm; j++)
            {
                //app 设定的变化总时间是1000ms （1000/20=50=chg_loops），灯泡的亮度变化值50%~60%，500,502,504..600
                if ((dst_lm - src_lm) >= chg_loops)
                {
                    tmp_lm = src_lm + (dst_lm - src_lm) * (j - src_lm) / chg_loops;
                    tmpCct = src_cct + (dst_cct - src_cct) * (j - src_lm) / chg_loops; //所有色温值遍历循环chg_loops后，到达dst_cct
                }
                else //app 设定的变化总时间是3000ms(150=chg_loops) ，灯泡的亮度变化值50-60%，会提前结束变化，所有亮度值已遍历到，且达到指定亮度
                {
                    tmp_lm = j;
                    tmpCct = src_cct + (dst_cct - src_cct) * (j - src_lm) / (dst_lm - src_lm); //所有色温值遍历循环(src_lm - dst_lm)后，到达dst_cct
                }
                if (g_b_ModeChanged)
                {
                    break;
                }
                bulb_leds_cct_config(tmpCct, tmp_lm);
                ezos_delay_ms(sleep_ms);
                if ((j - src_lm) == chg_loops)
                {
                    break; //所有亮度由低到高都遍历，循环chg_loops后，还没到指定总的变换时间，提前退出循环循环
                }
            }
        }
        else //亮度值由高变低
        {
            for (j = src_lm; j >= dst_lm; j--)
            {
                //app 设定的变化总时间是1000ms （1000/20=50=chg_loops），灯泡的亮度变化值60%-50%，600,598,596..502,0
                if ((src_lm - dst_lm) >= chg_loops)
                {
                    tmp_lm = src_lm - (src_lm - dst_lm) * (src_lm - j) / chg_loops;
                    tmpCct = src_cct + (dst_cct - src_cct) * (src_lm - j) / chg_loops; //所有色温值遍历循环chg_loops后，到达dst_cct
                }
                else //app 设定的变化总时间是3000ms(3000/20=150=chg_loops) ，灯泡的亮度变化值60%-50%，会提前结束变化,所有亮度值已遍历到，切达到指定亮度
                {
                    tmp_lm = j;
                    tmpCct = src_cct + (dst_cct - src_cct) * (src_lm - j) / (src_lm - dst_lm); //所有色温值遍历循环(src_lm - dst_lm)后，到达dst_cct
                }
                if (g_b_ModeChanged)
                {
                    break;
                }
                bulb_leds_cct_config(tmpCct, tmp_lm);
                ezos_delay_ms(sleep_ms);
                if ((src_lm - j) == chg_loops)
                {
                    break; //所有亮度由高到低都遍历，循环chg_loops后，还没到指定总的变换时间，提前退出循环
                }
            }
        }
    }
    else //亮度值不变，色温主变
    {
        if (dst_cct > src_cct)
        {
            for (j = src_cct; j <= dst_cct; j++)
            {
                //app 设定的变化总时间是1000ms （1000/20=50=chg_loops），灯泡的色温变化值2700~6500，2700,，2776，2852。。。6500
                if ((dst_cct - src_cct) >= chg_loops)
                {
                    tmpCct = src_cct + (dst_cct - src_cct) * (j - src_cct) / chg_loops; //所有色温值遍历循环chg_loops后，到达dst_cct
                }
                else //app 设定的变化总时间是100000ms(5000=chg_loops) ，色温变化值2700~6500，所有色温值已遍历到，且达到指定亮度
                {
                    tmpCct = j; //所有色温值遍历完，到达dst_cct
                }

                if (g_b_ModeChanged)
                {
                    break;
                }
                bulb_leds_cct_config(tmpCct, src_lm);
                ezos_delay_ms(sleep_ms);
                if ((j - src_cct) == chg_loops)
                {
                    break; //所有色温由低到高遍历，循环chg_loops后，还没到指定总的变换时间，提前退出循环
                }
            }
        }
        else
        {
            for (j = src_cct; j >= dst_cct; j--)
            {
                //app 设定的变化总时间是1000ms （1000/20=50=chg_loops），灯泡的亮度变化值60%-50%，600,598,596..502,0
                if ((src_cct - dst_cct) >= chg_loops)
                {
                    tmpCct = src_cct + (dst_cct - src_cct) * (src_cct - j) / chg_loops; //所有色温值遍历循环chg_loops后，到达dst_cct
                }
                else //app 设定的变化总时间是3000ms(3000/20=150=chg_loops) ，灯泡的亮度变化值60%-50%，会提前结束变化,所有亮度值已遍历到，切达到指定亮度
                {
                    tmpCct = j; //所有色温值遍历后，到达dst_cct
                }
                if (g_b_ModeChanged)
                {
                    break;
                }
                bulb_leds_cct_config(tmpCct, src_lm);
                ezos_delay_ms(sleep_ms);
                if ((src_cct - j) == chg_loops)
                {
                    break; //所有色温由高到低都遍历，循环chg_loops后，还没到指定总的变换时间，提前退出循环
                }
            }
        }
    }

    return 0;
}

/*
	通用控制接口，所有的控灯走这个指令，包括音乐律动，开关灯，调节颜色，调节亮度等
	解决灯控业务场景并发情况下产生的多任务灯控相互干扰。

	最初设计为音乐律动场景，单条指令支持如下场景：
	1.从当前颜色调整到目标颜色，支持白光调节到彩色，彩色到彩色，当前工作模式为彩光或者音乐律动
	2.从当前亮度调节到目标亮度，支持白光亮度调整，彩光亮度调整，开关灯场景
	        白光模式，音乐律动，场景，彩光模式都支持。
	3.从当前色温调整到目标色温，白光模式，场景模式，音乐律动模式

*/
int ledctrl_do(led_ctrl_t struLedCtrl)
{
    int chgLoops = 0;
    int i = 0;

    /*开始执行一次柔和渐变过程前，先将g_b_modechanged 置EZ_FALSE，避免外部置ture，不执行渐变*/
    if (g_b_ModeChanged)
    {
        g_b_ModeChanged = EZ_FALSE;
    }

    if (struLedCtrl.iRgbValue > 0)
    {

        chgLoops = struLedCtrl.nUpDuration / LM_CV_ONECE_TIME_SHORT;

        led_ctrl_rgb(struLedCtrl.nbrightness, struLedCtrl.iRgbValue, LM_CV_ONECE_TIME_SHORT, chgLoops);

        if (struLedCtrl.nSpeed > 0)
        {

            chgLoops = struLedCtrl.nSpeed / LM_CV_ONECE_TIME_SHORT;
            for (i = 0; i < chgLoops; i++)
            {
                if (g_b_ModeChanged)
                {
                    break;
                }
                ezos_delay_ms(LM_CV_ONECE_TIME_SHORT);
            }
        }

        if (struLedCtrl.nLowBrightness > 0)
        {
            chgLoops = struLedCtrl.nDownDuration / LM_CV_ONECE_TIME_SHORT;
            led_ctrl_rgb(struLedCtrl.nLowBrightness, struLedCtrl.iRgbValue, LM_CV_ONECE_TIME_SHORT, chgLoops);
        }
    }
    else //冷暖灯,亮度变化
    {
        chgLoops = struLedCtrl.nUpDuration / LM_CV_ONECE_TIME_SHORT;
        led_ctrl_cct(struLedCtrl.nCctValue, struLedCtrl.nbrightness, LM_CV_ONECE_TIME_SHORT, chgLoops);
        if (struLedCtrl.nSpeed > 0)
        {
            chgLoops = struLedCtrl.nSpeed / LM_CV_ONECE_TIME_SHORT;
            for (i = 0; i < chgLoops; i++)
            {
                if (g_b_ModeChanged)
                {
                    break;
                }
                ezos_delay_ms(LM_CV_ONECE_TIME_SHORT);
            }
        }
        if (struLedCtrl.nLowBrightness > 0)
        {
            chgLoops = struLedCtrl.nDownDuration / LM_CV_ONECE_TIME_SHORT;
            led_ctrl_cct(struLedCtrl.nCctValue, struLedCtrl.nLowBrightness, LM_CV_ONECE_TIME_SHORT, chgLoops);
        }
    }

    if (g_b_ModeChanged)
    {
        g_b_ModeChanged = EZ_FALSE;
    }
    return 0;
}

/*支持控灯渐变，同时支持快速切换渐变过程，执行新的渐变过程*/
int led_ctrl_do_async(led_ctrl_t *struLedCtrl)
{

    g_b_ModeChanged = EZ_TRUE; //通知灯控任务加速处理呼吸

    memcpy(&g_stru_led_ctrl, struLedCtrl, sizeof(g_stru_led_ctrl));

    ezos_sem_post(ctrl_sem);

#if 0
    if (xQueueSend(led_ctrl_cmd_queue, struLedCtrl, portTICK_PERIOD_MS) != EZ_TRUE) 
    {
		ezlog_e(TAG_APP, "music send to queue first faild!!!\n");
		g_b_ModeChanged = EZ_TRUE; //通知灯控任务加速处理呼吸
		/* 灯控指令结束一条后会立即消费一条，这里等待队列空，不退出*/
		if (xQueueSend(led_ctrl_cmd_queue, struLedCtrl, portMAX_DELAY) != EZ_TRUE) 
		{
			ezlog_e(TAG_APP, "music send to queue faild!!!\n");       					
		}
	}
#endif

    return 0;
}

/*支持控灯渐变，会等上一条控灯指令执行完毕*/
int led_ctrl_do_sync(led_ctrl_t *struLedCtrl)
{
    memcpy(&g_stru_led_ctrl, struLedCtrl, sizeof(g_stru_led_ctrl));

    ezos_sem_post(ctrl_sem);

#if 0
	/* 灯控指令结束一条后会立即消费一条，这里等待队列空，不退出*/
	if (xQueueSend(led_ctrl_cmd_queue, struLedCtrl, portMAX_DELAY) != EZ_TRUE) 
	{
		ezlog_e(TAG_APP, "music send to queue faild!!!\n");       					
	}
#endif

    return 0;
}

void led_ctrl_Task()
{

    ezlog_i(TAG_APP, "enter %s", __func__);
    led_ctrl_t struMusicCtrlCmd;
    ctrl_sem = ezos_sem_create(0, 1);

    while (1)
    {
        ezos_sem_wait(ctrl_sem, 1000000);

        ledctrl_do(g_stru_led_ctrl);
    }

    return;
}
