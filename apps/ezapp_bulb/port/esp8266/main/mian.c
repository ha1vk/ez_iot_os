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
#include "esp_system.h"
#include "ezlog.h"

static void boot_info_show(void)
{
    ezos_printf("ezapp, easy your life!\r\n");
}

static void system_init(void)
{
}

static void factory_data_load(void)
{
    // 加载出厂数据

    // 加载用户数据
}

static void app_global_init(void)
{
    //TODO1 业务模块初始化
    //TODO1 
}

static int integrity_check()
{
    // 出厂数据校验

    // 是否完成产测

    // 是否已配网

    return 0;
}

static void app_entry_done()
{
    //do nothing
}

void app_main(void)
{
    boot_info_show();

    system_init();

    factory_data_load();

    app_global_init();

    if (0 != integrity_check())
    {
        ezlog_a(TAG_APP, "Integrity check failed!");
        goto done;
    }

    // ezcloud_iot_link();
done:
    app_entry_done();
}