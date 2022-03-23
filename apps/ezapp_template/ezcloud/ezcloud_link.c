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
 * XuRongjun (xurongjun@ezvizlife.com)
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-03-20     xurongjun    first version 
 *******************************************************************************/

#include "ezcloud_link.h"
#include "ez_iot_core.h"
#include "ez_iot_core_def.h"
#include "device_info.h"
#include "hal_config.h"
#include "ezlog.h"

#define EZCLOUD_DEFAULT_DOMAIN "devcn.eziot.com"
#define EZCLOUD_DEFAULT_PORT 8666
static ez_bool_t g_is_inited = ez_false;

static ez_int32_t ez_event_notice_func(ez_event_e event_type, ez_void_t *data, ez_int32_t len)
{
    switch (event_type)
    {
    case EZ_EVENT_ONLINE:
        ezlog_d(TAG_APP, "dev online!");
        break;
    case EZ_EVENT_OFFLINE:
        ezlog_d(TAG_APP, "dev offline!");
        break;
    case EZ_EVENT_DEVID_UPDATE:
        ezlog_i(TAG_APP, "devid update, new devid:");
        ezlog_hexdump(TAG_APP, 16, data, len);
        //TODO 需要固化devid
        break;
    default:
        break;
    }

    return 0;
}

ez_void_t ezcloud_core_init(ez_void_t)
{
    ez_dev_info_t m_dev_info = {0};
    ez_server_info_t m_lbs_addr = {EZCLOUD_DEFAULT_DOMAIN, EZCLOUD_DEFAULT_PORT};
    ez_int32_t length = sizeof(m_lbs_addr.host) - 1;

    hal_config_get_string("domain", m_lbs_addr.host, &length, EZCLOUD_DEFAULT_DOMAIN);

    m_dev_info.auth_mode = dev_info_auth_mode();
    ezos_strncpy(m_dev_info.dev_type, dev_info_get_type(), sizeof(m_dev_info.dev_type) - 1);
    ezos_strncpy(m_dev_info.dev_firmwareversion, dev_info_get_fwver(), sizeof(m_dev_info.dev_firmwareversion) - 1);
    ezos_strncpy(m_dev_info.dev_subserial, dev_info_get_sn(), sizeof(m_dev_info.dev_subserial) - 1);
    ezos_strncpy(m_dev_info.dev_verification_code, dev_info_get_vcode(), sizeof(m_dev_info.dev_verification_code) - 1);

    ez_iot_core_init(&m_lbs_addr, &m_dev_info, ez_event_notice_func);
    ez_iot_core_start();
}

ez_void_t ezcloud_link_start(ez_void_t)
{
    if (g_is_inited)
    {
        return;
    }

    ezcloud_core_init();

#ifdef CONFIG_EZIOT_BASE_ENABLE
    extern ez_void_t ezcloud_base_init(ez_void_t);
    ezcloud_base_init();
#endif

#ifdef CONFIG_EZIOT_OTA_ENABLE
    extern ez_void_t ezcloud_ota_init(ez_void_t);
    ezcloud_ota_init();
#endif

#ifdef CONFIG_EZIOT_TSL_ENABLE
    extern ez_void_t ezcloud_tsl_init(ez_void_t);
    ezcloud_tsl_init();
#endif

    g_is_inited = ez_true;
}
