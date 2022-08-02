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
#include "device_info.h"
#include "hal_config.h"
#include "eztimer.h"
#include "ezconn.h"

#define KV_WIFI_SSID "wifi_ssid"
#define KV_WIFI_PWD "wifi_pwd"
#define KV_WIFI_CC "wifi_cc"
#define KV_PN_NUM "power_on_num"
#define KV_EZLOUCD_DOMIAN "domain"
#define KV_EZLOUCD_TOKEN "token"

static ez_void_t power_on_num_clear();
static ez_void_t wifi_provisioning_result(ezconn_state_e err_code, ezconn_wifi_info_t *wifi_info);
static ez_sem_t g_sem_prov = NULL;

ez_bool_t network_init(ez_void_t)
{
    if (NULL == g_sem_prov)
    {
        g_sem_prov = ezos_sem_create(0, 1);
    }

    ezconn_wifi_init();
    return ez_true;
}

ez_bool_t network_connect_start(ez_void_t)
{
    ez_char_t wifi_ssid[33] = {0};
    ez_char_t wifi_pwd[65] = {0};

    ez_int32_t length = sizeof(wifi_ssid) - 1;
    hal_config_get_string(KV_WIFI_SSID, wifi_ssid, &length, "");

    if (0 == length)
    {
        ezlog_e(TAG_APP, "wifi_ssid len range!");
        return ez_false;
    }

    length = sizeof(wifi_pwd) - 1;
    hal_config_get_string(KV_WIFI_PWD, wifi_pwd, &length, "");

    ezhal_wifi_config(EZOS_WIFI_MODE_STA);
    ezhal_sta_connect(wifi_ssid, wifi_pwd);

    return ez_true;
}

ez_void_t network_connect_stop(ez_void_t)
{
    ezhal_sta_stop();
}

ez_void_t network_wifi_prov_update(ez_void_t)
{
    ez_int32_t power_on_num;

    hal_config_get_int(KV_PN_NUM, &power_on_num, 0);
    ezlog_i(TAG_APP, "power on num[%d]", power_on_num);

    power_on_num++;
    hal_config_set_int(KV_PN_NUM, power_on_num);
    eztimer_create("power_on_num_clear", (10 * 1000), ez_false, power_on_num_clear);
}

ez_bool_t network_wifi_prov_need(ez_void_t)
{
    ez_int32_t cond_upper = 5;
    ez_int32_t cond_lower = 3;
    ez_int32_t power_on_num;

    hal_config_get_int(KV_PN_NUM, &power_on_num, 0);

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
    const ez_char_t *ap_prefix = "EZVIZ";
    const ez_char_t *ap_suffix = dev_info_get_sn() + ezos_strlen(dev_info_get_sn()) - 9;

    snprintf(ap_info.ap_ssid, sizeof(ap_info.ap_ssid) - 1, "%1.23s_%s", ap_prefix, ap_suffix);
    ap_info.auth_mode = 0;
    ap_info.channel = 1;
    ap_info.ap_timeout = 15;
    ap_info.apsta_coexist = ez_true;

    ezos_strncpy(dev_info.dev_serial, dev_info_get_sn(), sizeof(dev_info.dev_serial));
    ezos_strncpy(dev_info.dev_type, dev_info_get_type(), sizeof(dev_info.dev_type));
    ezos_strncpy(dev_info.dev_version, dev_info_get_fwver(), sizeof(dev_info.dev_version));

    ezconn_wifi_config(EZCONN_WIFI_MODE_APSTA);
    ezconn_ap_start(&ap_info, &dev_info, wifi_provisioning_result);

    return ez_true;
}

ez_void_t network_wifi_prov_waitfor(ez_void_t)
{
    if (NULL == g_sem_prov)
    {
        return;
    }

    ezlog_w(TAG_AP, "prov waitfor begin");
    ezos_sem_wait(g_sem_prov, 15 * 1000 * 60);
    ezlog_w(TAG_AP, "prov waitfor end");

    ezconn_ap_stop();
}

ez_void_t network_reset(ez_void_t)
{
    hal_config_del(KV_WIFI_SSID);
    hal_config_del(KV_WIFI_PWD);
    hal_config_del(KV_WIFI_CC);
    hal_config_del(KV_EZLOUCD_DOMIAN);
    hal_config_del(KV_EZLOUCD_TOKEN);
}

static ez_void_t power_on_num_clear()
{
    hal_config_set_int(KV_PN_NUM, 0);
    ezlog_d(TAG_APP, "power_on_num clean");
}

static ez_void_t wifi_provisioning_result(ezconn_state_e err_code, ezconn_wifi_info_t *wifi_info)
{
    switch (err_code)
    {
    case EZCONN_STATE_APP_CONNECTED:
    {
        ezlog_w(TAG_AP, "app connected.");
        break;
    }
    case EZCONN_STATE_SUCC:
    {
        ezlog_w(TAG_AP, "wifi config success.");

        hal_config_set_string(KV_WIFI_SSID, wifi_info->ssid);
        hal_config_set_string(KV_WIFI_PWD, wifi_info->password);
        hal_config_set_string(KV_WIFI_CC, wifi_info->cc);
        hal_config_set_string(KV_EZLOUCD_DOMIAN, wifi_info->domain);
        hal_config_set_string(KV_EZLOUCD_TOKEN, wifi_info->token);

        ezos_sem_post(g_sem_prov);
        break;
    }
    case EZCONN_STATE_CONNECTING_ROUTE:
    {
        ezlog_w(TAG_AP, "connecting route.");
        break;
    }
    case EZCONN_STATE_CONNECT_FAILED:
    {
        ezlog_w(TAG_AP, "connect failed.");
        ezos_delay_ms(1000);
        break;
    }
    case EZCONN_STATE_WIFI_CONFIG_TIMEOUT:
    {
        ezlog_w(TAG_AP, "wifi config timeout.");
        ezos_sem_post(g_sem_prov);
        break;
    }
    default:
    {
        break;
    }
    }
}