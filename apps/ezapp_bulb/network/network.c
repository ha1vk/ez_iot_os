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
 * XuRongjun (xurongjun@ezvizlife.com) - network module Interface interface implement
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-03-20     xurongjun    first version 
 *******************************************************************************/

#include "network.h"
#include "ezlog.h"
#include "ezhal_wifi.h"
#include "product_config.h"
#include "device_info.h"
#include "hal_config.h"
#include "eztimer.h"
#include "ezconn.h"
#include "product_test_light.h"

static ez_void_t power_on_num_clear();
static ez_void_t wifi_provisioning_result(ezconn_state_e err_code, ezconn_wifi_info_t *wifi_info);
static ez_sem_t g_sem_prov = NULL;

ez_bool_t network_init(ez_void_t)
{
    if (NULL == g_sem_prov)
    {
        g_sem_prov = ezos_sem_create(0, 1);
    }
}

ez_bool_t network_connect_start(ez_void_t)
{
    ez_char_t wifi_ssid[33] = {0};
    ez_char_t wifi_pwd[65] = {0};

    ez_int32_t length = sizeof(wifi_ssid) - 1;
    hal_config_get_string("wifi_ssid", wifi_ssid, &length, "");

    if (0 == length)
    {
        ezlog_e(TAG_APP, "wifi_ssid len range!");
        return ez_false;
    }

    length = sizeof(wifi_pwd) - 1;
    hal_config_get_string("wifi_pwd", wifi_pwd, &length, "");

    ezhal_wifi_config(EZOS_WIFI_MODE_STA);
    ezhal_sta_connect(wifi_ssid, wifi_pwd);
}

ez_void_t network_wifi_prov_update(ez_void_t)
{
    ez_int32_t power_on_num;

    hal_config_get_int("power_on_num", &power_on_num, 0);
    ezlog_i(TAG_APP, "power on num[%d]", power_on_num);

    power_on_num++;
    hal_config_set_int("power_on_num", power_on_num);
    eztimer_create("power_on_num_clear", (10 * 1000), ez_false, power_on_num_clear);
}

ez_bool_t network_wifi_prov_need(ez_void_t)
{
    ez_int32_t cond_upper;
    ez_int32_t cond_lower;
    ez_int32_t power_on_num;

    product_config_get_wd_condition(&cond_lower, &cond_upper);
    hal_config_get_int("power_on_num", &power_on_num, 0);

    if (power_on_num >= cond_lower && power_on_num <= cond_upper)
    {
        return ez_true;
    }

    return ez_false;
}

ez_bool_t network_wifi_prov_do(ez_void_t)
{
    ezconn_dev_info_t dev_info = {0};
    ezconn_ap_info_t ap_info = {0};
    ez_char_t ap_ssid[33] = {0};
    ez_char_t *ap_prefix = product_config_get_wd_prefix();
    ez_char_t *ap_suffix = dev_info_get_sn() + ezos_strlen(dev_info_get_sn()) - 9;

    snprintf(ap_ssid, sizeof(ap_ssid - 1), "%1.23s_%s", ap_prefix, ap_suffix);

    ezos_strncpy(ap_info.ap_ssid, ap_ssid, sizeof(ap_info.ap_ssid) - 1);
    ap_info.auth_mode = 0;
    ap_info.channel = 1;
    ap_info.ap_timeout = product_config_get_wd_period();
    ap_info.apsta_coexist = ez_true;

    ezos_strncpy(dev_info.dev_serial, dev_info_get_sn(), sizeof(dev_info.dev_serial));
    ezos_strncpy(dev_info.dev_type, dev_info_get_type(), sizeof(dev_info.dev_type));
    ezos_strncpy(dev_info.dev_version, dev_info_get_fwver(), sizeof(dev_info.dev_version));

    ezconn_wifi_config(EZCONN_WIFI_MODE_APSTA);
    ezconn_ap_start(&ap_info, &dev_info, wifi_provisioning_result);
    pt_light_set_mode(MODE_AP_START);

    return ez_true;
}

ez_void_t network_wifi_prov_waitfor(ez_void_t)
{
    if (NULL == g_sem_prov)
    {
        return;
    }

    ezlog_w(TAG_AP, "prov waitfor begin");
    ezos_sem_wait(g_sem_prov, product_config_get_wd_period() * 1000);
    ezlog_w(TAG_AP, "prov waitfor end");

    ezconn_ap_stop();
}

static ez_void_t power_on_num_clear()
{
    ez_int_t power_on_num = 0;
    if (0 != hal_config_set_int("power_on_num", power_on_num))
    {
        ezlog_e(TAG_APP, "set value failed. key: power_on_num");
    }
}

static ez_void_t wifi_provisioning_result(ezconn_state_e err_code, ezconn_wifi_info_t *wifi_info)
{
    switch (err_code)
    {
    case EZCONN_STATE_APP_CONNECTED:
    {
        ezlog_w(TAG_AP, "app connected.");
        pt_light_set_mode(MODE_AP_CLIENT_CONN);
        break;
    }
    case EZCONN_STATE_SUCC:
    {
        ezlog_w(TAG_AP, "wifi config success.");
        pt_light_set_mode(MODE_AP_CONN_SUCC);
        ezlog_i(TAG_AP, "ssid: %s", wifi_info->ssid);
        ezlog_i(TAG_AP, "password: %s", wifi_info->password);
        ezlog_i(TAG_AP, "token: %s", wifi_info->token);
        ezlog_i(TAG_AP, "domain: %s", wifi_info->domain);

        ezos_delay_ms(1000);
        pt_light_deinit();

        hal_config_set_string("wifi_ssid", wifi_info->ssid);
        hal_config_set_string("wifi_pwd", wifi_info->password);
        hal_config_set_string("wifi_cc", wifi_info->cc);
        hal_config_set_string("domain", wifi_info->domain);
        if (0 != ezos_strlen(wifi_info->device_id))
        {
            hal_config_set_string("device_id", wifi_info->device_id);
        }

        ezos_sem_post(g_sem_prov);
        break;
    }
    case EZCONN_STATE_CONNECTING_ROUTE:
    {
        ezlog_w(TAG_AP, "connecting route.");
        pt_light_set_mode(MODE_AP_CONN_ROUTE);
        break;
    }
    case EZCONN_STATE_CONNECT_FAILED:
    {
        ezlog_w(TAG_AP, "connect failed.");
        ezos_delay_ms(1000);
        pt_light_deinit();
        break;
    }
    case EZCONN_STATE_WIFI_CONFIG_TIMEOUT:
    {
        ezlog_w(TAG_AP, "wifi config timeout.");
        pt_light_set_mode(MODE_AP_TIMEOUT);
        ezos_delay_ms(1000);
        pt_light_deinit();
        break;
    }
    default:
    {
        break;
    }
    }
}