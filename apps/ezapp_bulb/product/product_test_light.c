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
 * ChenTengfei (chentengfei5@ezvizlife.com) - Smart bulb application Production test light status
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-03-15     ChenTengfei  first version 
 *******************************************************************************/

#include "ezlog.h"
#include "product_test_light.h"
#include "bulb_led_ctrl.h"
#include "product_test.h"
#include "product_config.h"
#include "hal_config.h"

#define TAG "T_PT"

#define KV_DEFVAL(key) key##_DEFVAL       // Key映射默认值
#define KV_PT_AGE_TIME "age_time"         // 已经老化的时间
#define KV_PT_AGE_TIME_DEFVAL 0           // 默认值

static int g_stage_2_time = 50;          // 根据wifi信息获取
static int g_stage_2_age_time_count = 0; // 已经老化的时间
static char g_light_order[8] = {0};
static int g_default_cct = 0;

#define PT_COND_RETURN(cond, rv) \
    do                           \
    {                            \
        if (cond)                \
            return rv;           \
    } while (0)

static int g_pt_light_inited = 0;
static ez_thread_t g_pt_light_thread = NULL;
static int g_pt_light_exit = 0;
static pt_light_mode_e g_pt_light_mode = MODE_MIN;
static int g_product_type = 0x00000; // R-G-B-W-C

static int light_mode_no_route();
static int light_mode_week_singal();
static int light_mode_pt1_normal(int dur_time);
static int light_mode_pt1_retest();
static int light_mode_pt2_normal();
static int light_mode_pt2_end();
static int light_mode_pt3_normal();
static int light_mode_reset_factory();

static int set_product_type(int type)
{
    int ret = 0;
    switch (type)
    {
    case LAMP_W:
        g_product_type = 0x00010;
        break;
    case LAMP_C:
        g_product_type = 0x00001;
        break;
    case LAMP_WC:
    case LAMP_CCT:
        g_product_type = 0x00011;
        break;
    case LAMP_RGB:
        g_product_type = 0x11100;
        break;
    case LAMP_RGBW:
        g_product_type = 0x11110;
        break;
    case LAMP_RGBWC:
    case LAMP_RGBCCT:
        g_product_type = 0x11111;
        break;
    default:
        g_product_type = 0x00000;
        ret = -1;
        break;
    }

    ezlog_i(TAG, "product type: %d.", type);
    return ret;
}

static int light_mode_no_route()
{
    led_ctrl_t ctrl_param = {0};
    ctrl_param.nbrightness = 100;
    ctrl_param.nCctValue = 6500;
    led_ctrl_do_async(&ctrl_param);
    ezos_delay_ms(500);
    ctrl_param.nbrightness = 0;
    led_ctrl_do_async(&ctrl_param);
    ezos_delay_ms(500);
    return 0;
}

static int light_mode_week_singal()
{
    led_ctrl_t ctrl_param = {0};
    ctrl_param.nbrightness = 100;
    ctrl_param.iRgbValue = 0xff0000;
    ctrl_param.nUpDuration = 500;
    led_ctrl_do_async(&ctrl_param);
    ezos_delay_ms(500);
    ctrl_param.nbrightness = 0;
    led_ctrl_do_async(&ctrl_param);
    ezos_delay_ms(500);
    return 0;
}

static int light_mode_pt1_normal(int dur_time)
{
    static int i = 0;
    int len = ezos_strlen(g_light_order);
    len = len ? len : 1;

    led_ctrl_t ctrl_param = {0};

    switch (g_light_order[(i++) % len])
    {
    case 'R':
    {
        if (g_product_type & 0x10000)
        {
            ctrl_param.iRgbValue = 0xff0000;
        }
    }
    break;
    case 'G':
    {
        if (g_product_type & 0x01000)
        {
            ctrl_param.iRgbValue = 0x00ff00;
        }
    }
    break;
    case 'B':
    {
        if (g_product_type & 0x00100)
        {
            ctrl_param.iRgbValue = 0x0000ff;
        }
    }
    break;
    case 'W':
    {
        if (g_product_type & 0x00010)
        {
            ctrl_param.nCctValue = 2700;
        }
    }
    break;
    case 'C':
    {
        if (g_product_type & 0x00001)
        {
            ctrl_param.nCctValue = 6500;
        }
    }
    break;
    default:
        break;
    }
    ctrl_param.nbrightness = 100;
    led_ctrl_do_async(&ctrl_param);
    ezos_delay_ms(dur_time);

    if (g_product_type == 0x00010) //单色灯需要明灭各2s
    {
        ctrl_param.nbrightness = 0;
        ctrl_param.nCctValue = 2700;
        led_ctrl_do_async(&ctrl_param);
        ezos_delay_ms(dur_time);
    }

    if (g_product_type == 0x00001) //单色灯需要明灭各2s
    {
        ctrl_param.nbrightness = 0;
        ctrl_param.nCctValue = 6500;
        led_ctrl_do_async(&ctrl_param);
        ezos_delay_ms(dur_time);
    }

    return 0;
}

static int light_mode_pt1_retest()
{
    return light_mode_pt1_normal(500);
}

static int light_mode_pt2_normal()
{
    static int cur_exec_time = 1;
    led_ctrl_t ctrl_param = {0};
    if (cur_exec_time % 60 == 0)
    {
        g_stage_2_age_time_count++;
        ezlog_d(TAG, "stage 2 age time count: %d", g_stage_2_age_time_count);
        hal_config_set_int(KV_PT_AGE_TIME, g_stage_2_age_time_count);
    }

    switch (g_product_type)
    {
    case 0x00010:
    {
        ctrl_param.nbrightness = 100;
        ctrl_param.nCctValue = 2700;
    }
    break;
    case 0x00001:
    {
        ctrl_param.nbrightness = 100;
        ctrl_param.nCctValue = 6500;
    }
    break;

    case 0x11100:
    {
        ctrl_param.nbrightness = 100;
        ctrl_param.iRgbValue = 0xffffff;
    }
    break;
    case 0x00011:
    {
        if (g_stage_2_age_time_count < g_stage_2_time / 2)
        {
            ctrl_param.nbrightness = 100;
            ctrl_param.nCctValue = 2700;
        }
        else
        {
            ctrl_param.nbrightness = 100;
            ctrl_param.nCctValue = 6500;
        }
    }
    break;
    case 0x11110:
    {
        if (g_stage_2_age_time_count < g_stage_2_time / 2)
        {
            ctrl_param.nbrightness = 100;
            ctrl_param.nCctValue = 2700;
        }
        else
        {
            ctrl_param.nbrightness = 100;
            ctrl_param.iRgbValue = 0xffffff;
        }
    }
    break;
    case 0x11111:
    {
        if (g_stage_2_age_time_count < g_stage_2_time * 2 / 5)
        {
            ctrl_param.nbrightness = 100;
            ctrl_param.nCctValue = 2700;
        }
        else if (g_stage_2_age_time_count < g_stage_2_time * 4 / 5)
        {
            ctrl_param.nbrightness = 100;
            ctrl_param.nCctValue = 6500;
        }
        else
        {
            ctrl_param.nbrightness = 100;
            ctrl_param.iRgbValue = 0xffffff;
        }
    }
    break;
    default:
        break;
    }
    led_ctrl_do_async(&ctrl_param);
    ezos_delay_ms(1000);
    cur_exec_time++;

    return 0;
}

static int light_mode_pt2_end()
{
    led_ctrl_t ctrl_param = {0};

    switch (g_product_type)
    {
    case 0x00010:
    case 0x00011:
    {
        ctrl_param.nbrightness = 10;
        ctrl_param.nCctValue = 2700;
    }
    break;
    case 0x00001:
    {
        ctrl_param.nbrightness = 10;
        ctrl_param.nCctValue = 6500;
    }
    break;

    case 0x11100:
    case 0x11110:
    case 0x11111:
    {
        ctrl_param.nbrightness = 10;
        ctrl_param.iRgbValue = 0x00ff00;
    }
    break;
    default:
        break;
    }
    led_ctrl_do_async(&ctrl_param);
    ezos_delay_ms(1000);

    return 0;
}

static int light_mode_pt3_normal()
{
    static int i = 0;
    int len = ezos_strlen(g_light_order);
    len = len ? len : 1;

    led_ctrl_t ctrl_param = {0};

    switch (g_light_order[(i++) % len])
    {
    case 'R':
    {
        if (g_product_type & 0x10000)
        {
            ctrl_param.iRgbValue = 0xff0000;
        }
    }
    break;
    case 'G':
    {
        if (g_product_type & 0x01000)
        {
            ctrl_param.iRgbValue = 0x00ff00;
        }
    }
    break;
    case 'B':
    {
        if (g_product_type & 0x00100)
        {
            ctrl_param.iRgbValue = 0x0000ff;
        }
    }
    break;
    case 'W':
    {
        if (g_product_type & 0x00010)
        {
            ctrl_param.nCctValue = 2700;
        }
    }
    break;
    case 'C':
    {
        if (g_product_type & 0x00001)
        {
            ctrl_param.nCctValue = 6500;
        }
    }
    break;
    default:
        break;
    }
    ctrl_param.nbrightness = 100;
    ctrl_param.nUpDuration = 2000;
    led_ctrl_do_async(&ctrl_param);
    ezos_delay_ms(2000);

    ctrl_param.nbrightness = 0;
    led_ctrl_do_async(&ctrl_param);
    ezos_delay_ms(2000);
    return 0;
}
static int light_mode_reset_factory()
{
    return light_mode_no_route();
}

static int light_mode_ap_start()
{
    led_ctrl_t ctrl_param = {0};
    ctrl_param.nbrightness = 5;
    ctrl_param.nCctValue = g_default_cct;
    ctrl_param.nUpDuration = 1500;
    led_ctrl_do_async(&ctrl_param);
    ezos_delay_ms(1500);
    ctrl_param.nbrightness = 75;
    led_ctrl_do_async(&ctrl_param);
    ezos_delay_ms(1500);
    return 0;
}

static int light_app_client_conn()
{
    led_ctrl_t ctrl_param = {0};
    ctrl_param.nbrightness = 100;
    ctrl_param.nCctValue = g_default_cct;
    led_ctrl_do_async(&ctrl_param);
    ezos_delay_ms(1000);
    return 0;
}

static int light_ap_conn_route()
{
    return light_mode_ap_start();
}

static int light_ap_conn_succ()
{
    return light_app_client_conn();
}

static int light_ap_conn_fail()
{
    return light_app_client_conn();
}

static int light_ap_timeout()
{
    return light_app_client_conn();
}

static int lighting_with_mode()
{
    switch (g_pt_light_mode)
    {
    case MODE_MIN:
        ezos_delay_ms(100);
        break;
    case MODE_NO_ROUTE:
        light_mode_no_route();
        break;
    case MODE_WEAK_SIGNAL:
        light_mode_week_singal();
        break;

    case MODE_PT1_NORMAL:
        light_mode_pt1_normal(1000);
        break;
    case MODE_PT1_RETEST:
        light_mode_pt1_retest();
        break;

    case MODE_PT2_NORMAL:
        light_mode_pt2_normal();
        break;
    case MODE_PT2_END:
        light_mode_pt2_end();
        break;

    case MODE_PT3_NORMAL:
        light_mode_pt3_normal();
        break;

    case MODE_RESET_FACTORY:
        light_mode_reset_factory();
        break;

    case MODE_AP_START:
        light_mode_ap_start();
        break;

    case MODE_AP_CLIENT_CONN:
        light_app_client_conn();
        break;

    case MODE_AP_CONN_ROUTE:
        light_ap_conn_route();
        break;
    case MODE_AP_CONN_FAIL:
        light_ap_conn_fail();
        break;

    case MODE_AP_CONN_SUCC:
        light_ap_conn_succ();
        break;

    case MODE_AP_TIMEOUT:
        light_ap_timeout();
        break;
    default:
        break;
    }
    return 0;
}

static void pt_light_fun(void *param)
{
    while (!g_pt_light_exit)
    {
        lighting_with_mode();
    }

    ezlog_i(TAG, "pt_light thread exit.");
}

int pt_light_set_mode(pt_light_mode_e mode)
{
    PT_COND_RETURN(0 == g_pt_light_inited, -1);
    PT_COND_RETURN(0 != g_pt_light_exit, -2);

    g_pt_light_mode = mode;
    return 0;
}

int pt_light_init(int type)
{
    int ret = 0;
    PT_COND_RETURN(0 != g_pt_light_inited, -1);
    const bulb_test_param_t *bulb_test = get_product_test_param();

    set_product_type(type);

    hal_config_get_int(KV_PT_AGE_TIME, &g_stage_2_age_time_count, KV_DEFVAL(KV_PT_AGE_TIME));
    ezos_strncpy(g_light_order, bulb_test->order, sizeof(g_light_order) - 1);

    if (0 == ezos_strlen(g_light_order))
    {
        ezlog_e(TAG, "get light order failed.");
        ezos_strcpy(g_light_order, "RGBWC");
    }

    ezlog_i(TAG, "light order: %s .", g_light_order);
    g_default_cct = product_config_default_cct();

    ret = ezos_thread_create(&g_pt_light_thread, "pt_light", pt_light_fun, NULL, 1024 * 2, 5);
    if (0 != ret)
    {
        ezlog_e(TAG, "create pt thread failed.");
        return ret;
    }

    g_pt_light_exit = 0;
    g_pt_light_inited = 1;
    return ret;
}

int pt_light_stage2_time(int stage2_time)
{
    g_stage_2_time = stage2_time;
    return 0;
}

int pt_light_deinit()
{
    PT_COND_RETURN(0 == g_pt_light_inited, -1);

    g_pt_light_exit = 1;
    g_pt_light_inited = 0;
    return 0;
}