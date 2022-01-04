#include "errno.h"
#include "utest.h"
#include "ezconn.h"
#include "ezos_def.h"
#include "ezlog.h"
#include <string.h>

void ut_ap_test();
static void eziot_ut_ap(void)
{
    UTEST_UNIT_RUN(ut_ap_test);
}
UTEST_TC_EXPORT(eziot_ut_ap, "eziot.ut_ap", NULL, NULL, CONFIG_EZIOT_UNIT_TEST_CASE_TIEMOUT_SECONDS);

const char *TAG_DEMO = "T_DEMO";

static void wifi_cb(ezconn_state_e err_code, ezconn_wifi_info_t *wifi_info)
{
    switch (err_code)
    {
    case EZCONN_STATE_APP_CONNECTED:
        ezlog_w(TAG_DEMO, "app connected.");
        break;
    case EZCONN_STATE_SUCC:
        ezlog_w(TAG_DEMO, "wifi config success.");
        ezlog_i(TAG_DEMO, "ssid: %s", wifi_info->ssid);
        ezlog_i(TAG_DEMO, "password: %s", wifi_info->password);
        ezlog_i(TAG_DEMO, "token: %s", wifi_info->token);
        ezlog_i(TAG_DEMO, "domain: %s", wifi_info->domain);
        ezconn_ap_stop();
        break;
    case EZCONN_STATE_CONNECTING_ROUTE:
        ezlog_w(TAG_DEMO, "connecting route.");
        break;
    case EZCONN_STATE_CONNECT_FAILED:
        ezlog_w(TAG_DEMO, "connect failed.");
        break;
    case EZCONN_STATE_WIFI_CONFIG_TIMEOUT:
        ezlog_w(TAG_DEMO, "wifi config timeout.");
        ezconn_ap_stop();
        break; 
    default:
        break;
    }
}

void example_ap_init_open_ap()
{
    ezconn_dev_info_t dev_info = {0};
    ezconn_ap_info_t ap_info = {0};

    strncpy((char *)ap_info.ap_ssid, "EZVIZ_AP_11112", sizeof(ap_info.ap_ssid) - 1);
    ap_info.auth_mode = 0;
    ap_info.channel = 1;
    ap_info.ap_timeout = 5;
    ap_info.apsta_coexist = ez_false;

    strncpy((char *)dev_info.dev_serial, "88888888", sizeof(dev_info.dev_serial) - 1);
    strncpy((char *)dev_info.dev_type, "EZ_001", sizeof(dev_info.dev_type) - 1);
    strncpy((char *)dev_info.dev_version, "V1.0.0 build 210302", sizeof(dev_info.dev_version) - 1);
    ezconn_wifi_init();
    ezconn_wifi_config(EZCONN_WIFI_MODE_AP);
    ezconn_ap_start(&ap_info, &dev_info, wifi_cb);
    ezos_delay_ms(5000);
    ezconn_ap_stop();

    ezconn_wifi_deinit();

    return;
}

void example_ap_init_open_apsta()
{
    ezconn_dev_info_t dev_info = {0};
    ezconn_ap_info_t ap_info = {0};


    strncpy((char *)ap_info.ap_ssid, "EZVIZ_AP_11112", sizeof(ap_info.ap_ssid) - 1);
    ap_info.auth_mode = 0;
    ap_info.channel = 1;
    ap_info.ap_timeout = 5;
    ap_info.apsta_coexist = ez_true;

    strncpy((char *)dev_info.dev_serial, "88888888", sizeof(dev_info.dev_serial) - 1);
    strncpy((char *)dev_info.dev_type, "EZ_001", sizeof(dev_info.dev_type) - 1);
    strncpy((char *)dev_info.dev_version, "V1.0.0 build 210302", sizeof(dev_info.dev_version) - 1);
    uassert_int_equal(EZCONN_SUCC, ezconn_wifi_init());
    uassert_int_equal(EZCONN_SUCC, ezconn_wifi_config(EZCONN_WIFI_MODE_APSTA));
    uassert_int_equal(EZCONN_SUCC, ezconn_ap_start(&ap_info, &dev_info, wifi_cb));
    ezos_delay_ms(5000);
    uassert_int_equal(EZCONN_SUCC, ezconn_ap_stop());

    uassert_int_equal(EZCONN_SUCC, ezconn_wifi_deinit());

    return;
}


void example_ap_init_auth_apsta()
{
    ezconn_dev_info_t dev_info = {0};
    ezconn_ap_info_t ap_info = {0};

    strncpy((char *)ap_info.ap_ssid, "EZVIZ_AP_11112", sizeof(ap_info.ap_ssid) - 1);
    strncpy((char *)ap_info.ap_pwd, "12345678", sizeof(ap_info.ap_pwd) - 1);
    ap_info.auth_mode = 4;
    ap_info.channel = 1;
    ap_info.ap_timeout = 5;
    ap_info.apsta_coexist = ez_true;

    strncpy((char *)dev_info.dev_serial, "88888888", sizeof(dev_info.dev_serial) - 1);
    strncpy((char *)dev_info.dev_type, "EZ_001", sizeof(dev_info.dev_type) - 1);
    strncpy((char *)dev_info.dev_version, "V1.0.0 build 210302", sizeof(dev_info.dev_version) - 1);
    ezconn_wifi_init();

    ezconn_ap_start(&ap_info, &dev_info, wifi_cb);

    ezos_delay_ms(5000);
    ezconn_ap_stop();

    ezconn_wifi_deinit();

    return;
}

void ut_ap_test()
{
    ezlog_init();
    ezlog_start();
    ezlog_filter_lvl(EZ_ELOG_LVL_VERBOSE);
    ezlog_e(TAG_DEMO, "ap demo start");

    //tcpip_adapter_init();
    //test_bind();
    //test_ezos_bind();
    example_ap_init_open_apsta();
    return;
}
