/*******************************************************************************
 * Copyright ? 2017-2021 Ezviz Inc.
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
 * XuRongjun (xurongjun@ezvizlife.com) - entry of user application, global init
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-03-08     xurongjun    first version
 *******************************************************************************/

#include "ezos.h"
#include "ezlog.h"
#include "hal_config.h"
#include "device_info.h"
#include "network.h"
#include "ezcloud_link.h"

static void boot_info_show(void)
{
    //TODO 输出固件关键信息

    ezos_printf("ezapp, easy your life!\r\n");
    ezos_printf("fwver:%s\r\n", dev_info_get_fwver());

    // 初始化日志模块，默认日志等级为WARN
    ezlog_init();
    ezlog_start();
    ezlog_filter_lvl(EZ_ELOG_LVL_WARN);
}

static void board_init(void)
{
    //TODO 初始化芯片关键驱动、中断
}

static void factory_data_load(void)
{
    //TODO 初始化存储模块, 加载设备出厂配置

    const ez_int32_t max_len = 2048;
    ez_int32_t read_len = 0;
    ez_char_t *buf = ezos_malloc(max_len);
    hal_config_init();

    ezos_bzero(buf, max_len);
    read_len = hal_config_lic_load((ez_char_t *)buf, sizeof(buf));
    if (!dev_info_init(buf, read_len))
    {
        ezlog_a(TAG_APP, "Invalid lic!");
    }

    ezos_free(buf);
}

static int integrity_check()
{
    // TODO 出厂数据校验/签名校验
    // 暂时不校验

    // TODO 是否完成产测
    

    return 0;
}

static void app_global_init(void)
{
    // TODO 初始化各业务模块以及组件

    network_init();
    network_wifi_prov_update();
    if (network_wifi_prov_need())
    {
        network_wifi_prov_do();
        network_wifi_prov_waitfor();
    }

    network_connect_start();
    ezcloud_link_start();

    // 2.定时计划启动

    // 3.场景启动等等
}

static void app_entry_done()
{
    //TODO 回收资源或开启监控
}

int app_main(void)
{
    boot_info_show();

    board_init();

    factory_data_load();

    if (0 != integrity_check())
    {
        goto done;
    }

    app_global_init();
done:
    app_entry_done();

    return 0;
}