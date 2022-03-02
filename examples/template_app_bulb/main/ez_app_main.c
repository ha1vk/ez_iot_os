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

#include "product_config.h"
#include "config_implement.h"
#include "dev_init.h"

#ifdef HAL_ESP

#include "esp_system.h"
#include "esp_heap_caps.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#endif
extern int uart_init(void);
static ez_kv_default_node_t m_kv_default_table[] = {
    {EZ_KV_DEFALUT_KEY_MASTERKEY, "", 0}, /* basic kv */
    {EZ_KV_DEFALUT_KEY_TSLMAP, "", 0},    /* tslmap profile map */
    {EZ_KV_DEFALUT_KEY_HUBLIST, "", 0},   /* hub module sublist */
};



void monitor_print_tasks(void)
{
#ifdef HAL_ESP

	heap_caps_print_heap_info(0);

    char *szPthreadInfo = (char *) ezos_malloc(2048);
	if(szPthreadInfo)
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
#else
    printf("\n to_do DEBUG in line (%d) and function (%s)): \n ",__LINE__, __func__);  
#endif
}


static const ez_kv_default_t m_default_kv = {m_kv_default_table, sizeof(m_kv_default_table) / sizeof(ez_kv_default_node_t)};

#ifdef HAL_ESP
int app_main(int argc, char **argv) 
#else
int main(int argc, char **argv) 

#endif
{

    ezlog_init();
    ezlog_start();
    ezlog_filter_lvl(5);

    ezos_time_t time = 0;
        

    kv_init(&m_default_kv);
    uart_init();

    #ifdef __linux
    /*mqtt 底层发数据有可能因为服务器关闭重连收到此消息，默认是杀死进程，此处忽略此信号*/
    signal(SIGPIPE, SIG_IGN);
    #endif

    if (0 != product_config_init())
    {
        ezlog_a(TAG_AP, "global init faild!");
        return 0;
    }
    config_print();

    set_gpio_config();
 
    #ifdef HAL_ESP
    ap_config_checkupdate();

    if (ap_distribution_check())
    {
       /* 设备通过重启进入配网，检查是否需要配网 */
       ap_distribution_do();
       
    }
    else if (netmgr_is_wd_done())   //flash 已存有ssid信息
    {
       /* 已配过网，直接上线 */
       wifi_connect_do();
    }
    else
    {
       netmgr_sta_update(net_sta_dile, 0);
    }
    #else
    
    if (0 != register_server())
    {
        printf("\n LW_PRINT DEBUG in line (%d) and function (%s)): \n ",__LINE__, __func__);

        return;
    }
    #endif

    bulb_ctrl_init();   
    kv_print();

    /* 主（父）线程循环,否则进程退出*/
    while (1)
    {

        ezos_delay_ms(30000);   
        time = ezos_time(NULL);
        printf("\n to_do DEBUG in line (%d) and function (%s)): now time=%ld\n ",__LINE__, __func__,time); 

        monitor_print_tasks();
        kv_print();

        printf("\n to_do DEBUG1 in line (%d) and function (%s)): now time=%ld\n ",__LINE__, __func__,time);
    }
    return 0;
}
