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

static void boot_info_show(void)
{
    ezos_printf("ezapp, easy your life!\r\n");

    ezlog_init();
    ezlog_start();
    ezlog_filter_lvl(EZ_ELOG_LVL_DEBUG);
}

static void system_init(void)
{
}

static void factory_data_load(void)
{
    static unsigned char buf[4096];
    hal_config_init();

    // 加载产品配置文件
    ezos_bzero(buf, sizeof(buf));
    hal_config_product_load((ez_char_t *)buf, sizeof(buf));
    ezlog_hexdump(TAG_APP, 32, buf, sizeof(buf));

    // 加载设备license
    ezos_bzero(buf, sizeof(buf));
    hal_config_lic_load((ez_char_t *)buf, sizeof(buf));
    ezlog_hexdump(TAG_APP, 32, buf, sizeof(buf));
}

static void app_global_init(void)
{
    // TODO1 业务模块初始化
    // TODO1
}

static int integrity_check()
{
    // 出厂数据校验

    // 是否完成产测

    return 0;
}

static void app_entry_done()
{
}

int app_main(void)
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

    return 0;
}