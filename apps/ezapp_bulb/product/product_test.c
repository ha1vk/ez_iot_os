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
 * ChenTengfei (chentengfei5@ezvizlife.com) - Smart bulb application finished product production test
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-03-15     ChenTengfei  first version 
 *******************************************************************************/

#include "ezos.h"
#include "ezlog.h"
#include "ezconn.h"
#include "ezhal_wifi.h"

#include "product_test.h"
#include "hal_config.h"
#include "product_test.h"
#include "product_test_light.h"
#include "product_config.h"

#define KV_DEFVAL(key) key##_DEFVAL       // Key映射默认值
#define KV_PT_PARA1 "para1"               // 是否已经完成产测1
#define KV_PT_PARA1_DEFVAL 0              // 默认值
#define KV_PT_PARA2 "para2"               // 是否已经完成产测2
#define KV_PT_PARA2_DEFVAL 0              // 默认值
#define KV_PT_AGE_TIME "age_time"         // 已经老化的时间
#define KV_PT_AGE_TIME_DEFVAL 0           // 默认值
#define KV_PT_STAGE3_COUNT "stage3_count" // 进入产测3的次数，5次需要恢复出厂设置
#define KV_PT_STAGE3_COUNT_DEFVAL 0       // 默认值

#define TAG_PROTEST "T_PRODUCT"                // 日志打印TAG
#define AP_LIST_MAX_NUM 20                     // 每次扫描wifi列表的最大个数
#define AP_SCAN_MAX_TIME 2                     // 每次扫描wifi列表的最大个数
#define TEST_WIFI_SSID_PREFIX "EzvizTest001_"  // 产测环境路由前缀，如moduletest001_XX，XX表示老练测试要进行的时间
#define TEST2_WIFI_SSID_PREFIX "EzvizTest002_" // 产测环境路由前缀，如moduletest001_XX，XX表示老练测试要进行的时间
#define MS_PER_MIN (60 * 1000)
#define TEST_WEEK_SIGNAL (-60)

#define STAGE_1_TIME 120 // 第一阶段产测时间，单位s
#define PT_ENDLESS_LOOP(ms) \
    do                      \
    {                       \
        ezos_delay_ms(ms);  \
    } while (1)

ez_bool_t g_stage_1_flag = ez_false;
ez_bool_t g_stage_2_flag = ez_false;
ez_int32_t g_stage_1_time = 0; // 根据用户配置文件获取
ez_int32_t g_stage_2_time = 0; // 根据wifi信息获取

typedef enum
{
    STATE_NO_ROUTE,
    STATE_WEAK_SIGNAL,
    STATE_NORMAL,
} pt_state_e;

static pt_state_e check_test_environment()
{
    pt_state_e pt_state = STATE_NO_ROUTE;
    ezconn_wifi_config(EZCONN_WIFI_MODE_STA);
    ezos_delay_ms(2000);
    ezhal_wifi_list_t ap_list[AP_LIST_MAX_NUM] = {0};
    ez_int32_t wifi_list_num = 0;

    for (ez_int32_t j = 0; j < AP_SCAN_MAX_TIME; j++)
    {
        ezos_memset(&ap_list, 0, sizeof(ap_list));

        wifi_list_num = ezhal_sta_get_scan_list(AP_LIST_MAX_NUM, ap_list);
        if (0 == wifi_list_num)
        {
            ezlog_e(TAG_PROTEST, "wifi scan failed.");
            break;
        }

        for (ez_int32_t i = 0; i < wifi_list_num; ++i)
        {
            //如果产测二未成功，就开始扫描产测二的路由器
            if (!g_stage_2_flag)
            {
                if (0 == ezos_strncmp((char *)ap_list[i].ssid, TEST_WIFI_SSID_PREFIX, ezos_strlen((char *)TEST_WIFI_SSID_PREFIX)))
                {
                    ezos_sscanf((char *)ap_list[i].ssid, "EzvizTest001_%d", &g_stage_2_time);
                    if (1 > g_stage_2_time)
                    {
                        g_stage_2_time = 50;
                    }

                    ezlog_i(TAG_PROTEST, "stage 2 test %d min.", g_stage_2_time);
                    ezlog_d(TAG_PROTEST, "scaned ssid: %s,rssi=%d", ap_list[i].ssid, ap_list[i].rssi);
                    if (ap_list[i].rssi < TEST_WEEK_SIGNAL)
                    {
                        pt_state = STATE_WEAK_SIGNAL;
                        continue;
                    }
                    else
                    {
                        //搜索到强信号则退出来
                        pt_light_stage2_time(g_stage_2_time);
                        pt_state = STATE_NORMAL;
                        break;
                    }
                }
            }
            else //如果产测二已经成功，则扫描产测三的路由器
            {
                if (0 == ezos_strncmp((char *)ap_list[i].ssid, TEST2_WIFI_SSID_PREFIX, ezos_strlen((char *)TEST2_WIFI_SSID_PREFIX)))
                {
                    ezlog_d(TAG_PROTEST, "scaned ssid: %s,rssi=%d", ap_list[i].ssid, ap_list[i].rssi);
                    if (ap_list[i].rssi < TEST_WEEK_SIGNAL)
                    {
                        pt_state = STATE_WEAK_SIGNAL;
                        continue;
                    }
                    else
                    {
                        pt_state = STATE_NORMAL;
                        break;
                    }
                }
            }
        }

        if (g_stage_2_flag || STATE_NORMAL == pt_state) // 包装测试 只扫描一次就退出来
        {
            break;
        }
    }

    ezlog_d(TAG_PROTEST, "check test environment exit.");
    return pt_state;
}

static ez_int32_t read_product_test_config()
{
    const bulb_test_param_t *test_param = get_product_test_param();

    hal_config_get_int(KV_PT_PARA1, &g_stage_1_flag, KV_DEFVAL(KV_PT_PARA1));
    hal_config_get_int(KV_PT_PARA2, &g_stage_1_flag, KV_DEFVAL(KV_PT_PARA2));

    ezlog_i(TAG_PROTEST, "stage 1 flag: %d", g_stage_1_flag);
    ezlog_i(TAG_PROTEST, "stage 2 flag: %d", g_stage_2_flag);

    g_stage_1_time = test_param->step1time;

    if (20 > g_stage_1_time || g_stage_1_time > 600)
    {
        g_stage_1_time = STAGE_1_TIME;
    }
    ezlog_i(TAG_PROTEST, "stage 1 time : %d s.", g_stage_1_time);

    return 0;
}

static ez_int32_t act_no_route()
{
    ezlog_d(TAG_PROTEST, "%s enter.", __FUNCTION__);
    pt_light_set_mode(MODE_NO_ROUTE);
    PT_ENDLESS_LOOP(100);
    return 0;
}

static ez_int32_t act_weak_signal()
{
    ezlog_d(TAG_PROTEST, "%s enter.", __FUNCTION__);
    pt_light_set_mode(MODE_WEAK_SIGNAL);
    PT_ENDLESS_LOOP(100);
    return 0;
}

static ez_int32_t act_pt1_normal()
{
    ezlog_d(TAG_PROTEST, "%s enter.", __FUNCTION__);
    ez_int32_t stage_2_age_time = 0;
    ez_int32_t delay_time = 0;
    hal_config_get_int(KV_PT_AGE_TIME, &stage_2_age_time, KV_DEFVAL(KV_PT_AGE_TIME));

    if (stage_2_age_time > 0)
    {
        pt_light_set_mode(MODE_PT1_RETEST);
        delay_time = 12500;
    }
    else
    {
        pt_light_set_mode(MODE_PT1_NORMAL);
        delay_time = 60000;
    }
    ezos_delay_ms(delay_time);

    ezlog_d(TAG_PROTEST, "pt2 normal enter");
    pt_light_set_mode(MODE_PT2_NORMAL);
    ezos_delay_ms((g_stage_2_time - stage_2_age_time) * 60 * 1000);

    ezlog_d(TAG_PROTEST, "pt2 end enter");
    pt_light_set_mode(MODE_PT2_END);

    g_stage_2_flag = ez_true;
    hal_config_set_int(KV_PT_PARA2, g_stage_2_flag);
    hal_config_set_int(KV_PT_AGE_TIME, stage_2_age_time);
    PT_ENDLESS_LOOP(100);

    return 0;
}

static ez_int32_t act_pt3_normal()
{
    ezlog_d(TAG_PROTEST, "%s enter.", __FUNCTION__);
    ez_int32_t ret = 0;
    ez_int32_t stage_3_count = 0;

    hal_config_get_int(KV_PT_STAGE3_COUNT, &stage_3_count, KV_DEFVAL(KV_PT_STAGE3_COUNT));
    stage_3_count++;
    hal_config_set_int(KV_PT_STAGE3_COUNT, stage_3_count);

    if (stage_3_count >= 5)
    {
        ezlog_i(TAG_PROTEST, "5 enter to stage 3,factory set");
        hal_config_reset_factory();
        pt_light_set_mode(MODE_RESET_FACTORY);
        PT_ENDLESS_LOOP(100);
    }
    else
    {
        pt_light_set_mode(MODE_PT3_NORMAL);
        PT_ENDLESS_LOOP(100);
    }

    return ret;
}

ez_int32_t ez_product_test(ez_int32_t type)
{
    ez_int32_t ret = 0;

    ret = pt_light_init(type);
    if (0 != ret)
    {
        ezlog_e(TAG_PROTEST, "pt_light init failed.");
        return ret;
    }

    ret = read_product_test_config();
    if (0 != ret)
    {
        ezlog_e(TAG_PROTEST, "read product test failed.");
        return ret;
    }
    pt_state_e pt_state = check_test_environment();
    switch (pt_state)
    {
    case STATE_NO_ROUTE:
        if (!g_stage_2_flag)
        {
            ret = act_no_route();
        }
        else
        {
            ret = 0;
        }
        break;
    case STATE_WEAK_SIGNAL:
        ret = act_weak_signal();
        break;
    case STATE_NORMAL:
        if (!g_stage_2_flag)
        {
            ret = act_pt1_normal();
        }
        else
        {
            ret = act_pt3_normal();
        }

        break;
    default:
        break;
    }

    return ret;
}

ez_int32_t is_product_test_done()
{
    ez_int32_t step2 = 0;
    hal_config_get_int(KV_PT_PARA2, &step2, KV_DEFVAL(KV_PT_PARA2));

    return step2;
}