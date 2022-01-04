#include "nvs_flash.h"
#include "errno.h"

#include "ezconn.h"
#include "ezos_def.h"
#include "ezlog.h"
#include <string.h>

#include "lwip/sockets.h"
#include "tcpip_adapter.h"

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
    ezconn_wifi_init();
    ezconn_wifi_config(EZCONN_WIFI_MODE_APSTA);
    ezconn_ap_start(&ap_info, &dev_info, wifi_cb);
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

    return;
}

void test_bind()
{
    int fd = socket(PF_INET, SOCK_STREAM, 0);
    if (fd < 0)
    {
        ezlog_e(TAG_DEMO, "socket error. errno: %d", errno);
        return;
    }
    struct sockaddr_in srv_addr = {
.sin_family = AF_INET,
.sin_addr = {
.s_addr = htonl(INADDR_ANY)
},
.sin_port = htons(80)
    };

    int ret = bind(fd, (struct sockaddr *)&srv_addr, sizeof(srv_addr));
    if (ret < 0)
    {
        ezlog_e(TAG_DEMO, "bind error. errno: %d", errno);
        close(fd);
        return;
    }
    ezlog_e(TAG_DEMO, "bind success");
    close(fd);
    return;
}

void test_ezos_bind()
{
    int fd = ezos_socket(EZ_PF_INET, EZ_SOCK_STREAM, 0);
    if (fd < 0)
    {
        ezlog_e(TAG_DEMO, "ezos socket error. errno: %d", errno);
        return;
    }
    struct ez_sockaddr_in srv_addr = {
.sin_family = EZ_AF_INET,
.sin_addr = {
.s_addr = htonl(EZ_INADDR_ANY)
},
.sin_port = htons(80)
    };


    int ret = ezos_bind(fd, (struct ez_sockaddr *)&srv_addr, sizeof(srv_addr));
    if (ret < 0)
    {
        ezlog_e(TAG_DEMO, "ezos bind error. errno: %d", errno);
        ezos_closesocket(fd);
        return;
    }

    ezlog_e(TAG_DEMO, "ezos bind success");
    ezos_closesocket(fd);
    return;
}

void app_main()
{
    ezlog_init();
    ezlog_start();
    ezlog_filter_lvl(EZ_ELOG_LVL_VERBOSE);
    ezlog_e(TAG_DEMO, "ap demo start");
    nvs_flash_init();

    //tcpip_adapter_init();
    //test_bind();
    //test_ezos_bind();
    example_ap_init_open_apsta();
}


