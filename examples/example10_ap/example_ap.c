/**
 * @file ut_ap.c
 * @author chentengfei(chentengfei5@ezvizlife.com)
 * @brief ceshi
 * @version 0.1
 * @date 2021-01-28
 *
 * @copyright Copyright (c) 2021
 *
 */

#include <stdlib.h>
#include <string.h>
#include "ut_config.h"
#include "ez_iot_core.h"
#include "ez_iot_log.h"
#include "ez_iot_ap.h"
#include "ez_iot_errno.h"
#include "hal_thread.h"

const char *TAG_UT_AP = "UT_AP";

static void wifi_cb(ez_iot_ap_wifi_info_t *wifi_info)
{
/** ���ϲ㴦�������ݴ��������ϲ�ҵ���� */
/** 
    example code:

    switch (wifi_info->err_code)
    {
    case ez_errno_ap_app_connected:
        ez_log_w(TAG_UT_AP, "app connected.");
        break;
    case ez_errno_succ:
        ez_log_w(TAG_UT_AP, "wifi config success.");
        ez_log_i(TAG_UT_AP, "ssid: %s", wifi_info->ssid);
        ez_log_i(TAG_UT_AP, "password: %s", wifi_info->password);
        ez_log_i(TAG_UT_AP, "token: %s", wifi_info->token);
        ez_log_i(TAG_UT_AP, "domain: %s", wifi_info->domain);
        break;
    case ez_errno_ap_connecting_route:
        ez_log_w(TAG_UT_AP, "connecting route.");
        break;
    case ez_errno_ap_connect_failed:
        ez_log_w(TAG_UT_AP, "connect failed.");
        break;
    case ez_errno_ap_wifi_config_timeout:
        ez_log_w(TAG_UT_AP, "wifi config timeout.");
        ez_iot_ap_finit();
        break;
    default:
        break;
    }
*/
}

/**
 * ����ap����
 * ap�ȵ�������
 * ֧��ap_sta����
 */
void example_ap_init_open_apsta()
{
    ez_iot_ap_dev_info_t dev_info = {0};
    strncpy(dev_info.ap_ssid, "EZVIZ_AP_11112", sizeof(dev_info.ap_ssid) - 1);
    dev_info.auth_mode = 0;
    strncpy(dev_info.dev_serial, "88888888", sizeof(dev_info.dev_serial) - 1);
    strncpy(dev_info.dev_type, "EZ_001", sizeof(dev_info.dev_type) - 1);
    strncpy(dev_info.dev_version, "V1.0.0 build 210302", sizeof(dev_info.dev_version) - 1);
    ez_iot_wifi_init();
    ez_iot_ap_init(&dev_info, wifi_cb, 5, true);
    return;
}

/**
 * ����ap����
 * ap�ȵ�������
 * ֧��ap_sta����
 */
void example_ap_init_auth_apsta()
{
    ez_iot_ap_dev_info_t dev_info = {0};
    strncpy(dev_info.ap_ssid, "EZVIZ_AP_11112", sizeof(dev_info.ap_ssid) - 1);
    strncpy(dev_info.ap_password, "12345678", sizeof(dev_info.ap_password) - 1);
    dev_info.auth_mode = 4;
    strncpy(dev_info.dev_serial, "88888888", sizeof(dev_info.dev_serial) - 1);
    strncpy(dev_info.dev_type, "EZ_001", sizeof(dev_info.dev_type) - 1);
    strncpy(dev_info.dev_version, "V1.0.0 build 210302", sizeof(dev_info.dev_version) - 1);
    ez_iot_wifi_init();
    ez_iot_ap_init(&dev_info, wifi_cb, 5, true);
    return;
}

void example_ap_init()
{
    example_ap_init_open_apsta();

    hal_thread_sleep(10 * 60 * 1000);

    example_ap_init_auth_apsta();
}
