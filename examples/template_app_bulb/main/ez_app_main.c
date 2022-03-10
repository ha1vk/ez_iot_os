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

#include "ezlog.h"

#include "ezos_time.h" //延迟需要

#ifdef __linux
#include "signal.h"
#endif

#include "ap_distribution.h"

#include "bulb_business.h"
#include "dev_netmgr.h"
#include "kv_imp.h"
#include "eztimer.h"
#include "ezconn.h"
#include "product_config.h"
#include "config_implement.h"
#include "dev_init.h"
#include "product_test.h"

#ifdef HAL_ESP

#include "esp_system.h"
#include "esp_heap_caps.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#endif
extern int uart_init(void);
extern void wifi_info_timer_cb();
ez_int32_t rssi_timer;
static ez_kv_default_node_t m_kv_default_table[] = {
    {EZ_KV_DEFALUT_KEY_MASTERKEY, "", 0}, /* basic kv */
    {EZ_KV_DEFALUT_KEY_TSLMAP, "", 0},    /* tslmap profile map */
    {EZ_KV_DEFALUT_KEY_HUBLIST, "", 0},   /* hub module sublist */
};

void monitor_print_tasks(void)
{

    heap_caps_print_heap_info(0);

    char *szPthreadInfo = (char *)ezos_malloc(2048);
    if (szPthreadInfo)
    {
        printf("rt_state_in----------------------------------------------\r\n");
        ezos_bzero(szPthreadInfo, 2048);
        vTaskGetRunTimeStats(szPthreadInfo);
        printf("%s", szPthreadInfo);
        printf("rt_state_in stack size----------------------------------------------\r\n");
        ezos_bzero(szPthreadInfo, 2048);
        vTaskList(szPthreadInfo);
        printf("%s", szPthreadInfo);
        ezos_free(szPthreadInfo);
        szPthreadInfo = NULL;
        printf("rt_state_out----------------------------------------------\r\n");
    }
    else
    {
        printf("esp_print_tasks malloc memory error\n");
    }
}

static const ez_kv_default_t m_default_kv = {m_kv_default_table, sizeof(m_kv_default_table) / sizeof(ez_kv_default_node_t)};

int app_main(int argc, char **argv)
{
    printf("ez_app, easy your life.\n");
    ezlog_init();
    ezlog_start();
    ezlog_filter_lvl(4);

    kv_init(&m_default_kv);
    uart_init();

    ezconn_wifi_init();

    if (0 != product_config_init() || (0 != config_init()))
    {
        ezlog_a(TAG_AP, "global init failed!");
        return 0;
    }

    set_gpio_config();

    bulb_ctrl_init();

    regular_power_up();

    ap_config_checkupdate();

    // ez_product_test(get_product_subtype());

    if (ap_distribution_check())
    {
        /* 设备通过重启进入配网，检查是否需要配网 */
        ap_distribution_do();
    }
    else if (netmgr_is_wd_done()) //flash 已存有ssid信息
    {
        /* 已配过网，直接上线 */
        wifi_connect_do();
    }
    else
    {
        netmgr_sta_update(net_sta_dile, 0);
    }

    eztimer_create("rssi_timer", (300 * 1000), ez_true, wifi_info_timer_cb);
    return 0;
}
