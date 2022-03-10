#include <string.h>
#include "ezlog.h"

#include "ezconn.h"
#include "ap_distribution.h"

#include "config_type.h"
#include "register_server.h"

#include "config_implement.h"
#include "product_config.h"
#include "pt_light_mode.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_wifi.h"

#include "dev_init.h"
#include "dev_info.h"

extern char bind_token[64];
bool g_need_ap = false;
bool g_ap_exit = true;     //配网模式下，内部会有两次获取到ip，需要在第二次获取到ip时进行联网
bool g_if_need_ap = false; //检测是否需要进入ap 模式
bool g_apstamode_flag = false;

static void wd_sucess_proc(void *param)
{
    ezconn_wifi_info_t wifi_info = {0};
    memcpy(&wifi_info, param, sizeof(ezconn_wifi_info_t));

    strncpy(bind_token, wifi_info.token, sizeof(bind_token) - 1);

    int ret = 0;
    do
    {
        ret |= config_set_value(K_WIFI_SSID, wifi_info.ssid, strlen(wifi_info.ssid));
        ret |= config_set_value(K_WIFI_PASSWORD, wifi_info.password, strlen(wifi_info.password));
        ret |= config_set_value(K_WIFI_CC, wifi_info.cc, strlen(wifi_info.cc));

        ret |= config_set_value(K_DOMAIN, wifi_info.domain, strlen(wifi_info.domain));
        if (0 != strlen(wifi_info.device_id))
        {
            ret |= config_set_value(K_DEVICE_ID, wifi_info.device_id, strlen(wifi_info.device_id));
        }

        if (0 != ret)
        {
            ezlog_e(TAG_APP, "set wifi config failed.");
            break;
        }
    } while (false);

    if (0 != ret)
    {
        return;
    }

    ezconn_ap_stop();

    g_ap_exit = true;

    /*此时需要将配网信息写入到flash中*/

    /*ap模式关闭后，底层连接也断开，此处时重新连接*/
    // ezconn_wifi_init();
    ezconn_wifi_config(EZCONN_WIFI_MODE_STA);
    ezconn_sta_start(wifi_info.ssid, wifi_info.password);

    vTaskDelete(NULL);
    return;
}

static void wifi_ap_distribution_cb(ezconn_state_e err_code, ezconn_wifi_info_t *wifi_info)
{
    switch (err_code)
    {
    case EZCONN_STATE_APP_CONNECTED:
        ezlog_w(TAG_AP, "app connected.");
        pt_light_set_mode(MODE_AP_CLIENT_CONN);
        break;
    case EZCONN_STATE_SUCC:
        ezlog_w(TAG_AP, "wifi config success.");
        pt_light_set_mode(MODE_AP_CONN_SUCC);
        ezlog_i(TAG_AP, "ssid: %s", wifi_info->ssid);
        ezlog_i(TAG_AP, "password: %s", wifi_info->password);
        ezlog_i(TAG_AP, "token: %s", wifi_info->token);
        ezlog_i(TAG_AP, "domain: %s", wifi_info->domain);

        ezos_delay_ms(1000);
        pt_light_deinit();
        ez_thread_t wd_sucess_handle;
        g_apstamode_flag = false;
        if (0 != ezos_thread_create(&wd_sucess_handle, "wd_sucess_proc", wd_sucess_proc, (void *)wifi_info, 3 * 1024, 3))
        {
            ezlog_e(TAG_AP, "create wd_success thread failed.");
        }
        break;
    case EZCONN_STATE_CONNECTING_ROUTE:
        ezlog_w(TAG_AP, "connecting route.");
        pt_light_set_mode(MODE_AP_CONN_ROUTE);
        break;
    case EZCONN_STATE_CONNECT_FAILED:
        ezlog_w(TAG_AP, "connect failed.");
        ezos_delay_ms(1000);
        pt_light_deinit();
        break;
    case EZCONN_STATE_WIFI_CONFIG_TIMEOUT:
        g_apstamode_flag = false;
        ezlog_w(TAG_AP, "wifi config timeout.");
        pt_light_set_mode(MODE_AP_TIMEOUT);
        ezos_delay_ms(1000);
        pt_light_deinit();
        break;
    default:
        break;
    }
}

static void ez_cloud_access(void)
{
    static bool first = true;

    if (!first)
    {
        return;
    }

    if (0 != register_server())
    {
        return;
    }

    first = false;
}

//判断ip地址是否发生变化，0:未变化；1:变化
int ip_change(char *new_ip, char *old_ip)
{
    if (0 == strncmp(new_ip, old_ip, 16))
    {
        ezlog_i(TAG_APP, "ip is same!");
        return 0;
    }

    return 1;
}

static void ip_info_update(tcpip_adapter_ip_info_t *ip_info)
{
    wifi_t wifi_info = {0};
    char *address = NULL;
    char *mask = NULL;
    char *gateWay = NULL;
    int ret = 0;
    address = ip4addr_ntoa(&ip_info->ip);
    if (0 == ip_change(address, wifi_info.ip))
    {
        return;
    }
    strncpy(wifi_info.ip, address, sizeof(wifi_info.ip) - 1);

    mask = ip4addr_ntoa(&ip_info->netmask);
    strncpy(wifi_info.mask, mask, sizeof(wifi_info.mask) - 1);

    gateWay = ip4addr_ntoa(&ip_info->gw);
    strncpy(wifi_info.gateway, gateWay, sizeof(wifi_info.gateway) - 1);
    ezlog_e(TAG_APP, "ip is:%s", wifi_info.ip);
    ret |= config_set_value(K_WIFI_IP, wifi_info.ip, sizeof(wifi_info.ip));
    ret |= config_set_value(K_WIFI_MASK, wifi_info.mask, sizeof(wifi_info.mask));
    ret |= config_set_value(K_WIFI_GATEWAY, wifi_info.gateway, sizeof(wifi_info.gateway));
    ezlog_e(TAG_APP, "ip set value.");
    if (0 != ret)
    {
        ezlog_e(TAG_APP, "set wifi config failed.");
    }
}

void wifi_status_cb(void *wifi_status)
{
    tcpip_adapter_ip_info_t *ip_info = (tcpip_adapter_ip_info_t *)wifi_status;
    /*todo ip地址更新到flash*/
    ip_info_update(ip_info);
    if (g_ap_exit)
    {
        ez_cloud_access();
    }
}

void power_on_num_clear_task()
{
    //vTaskDelay(3300 / portTICK_PERIOD_MS); //3次总共10s，因此3.3s后擦除flash中的计数
    vTaskDelay(10000 / portTICK_PERIOD_MS); //10s 后再清空，避免进入配网之后又误操作按了一次，无法进入配网

    int power_on_num = 0;
    int ret = config_set_value(K_POWER_ON_NUM, &power_on_num, sizeof(power_on_num));
    if (0 != ret)
    {
        ezlog_e(TAG_APP, "set value failed. key: POWER_ON_NUM");
    }

    vTaskDelete(NULL);
}

//判断是否需要配网
//return 0：如果非首次上电以及上次配网超时，则不配网；1：需要配网
//设备有3种情况下需要配网：1、首次上电；2、电源开关开关开；3、手机APP重置
void need_ap_config(void)
{
    //1、首次上电
    int first_boot = 1;
    int len = sizeof(first_boot);

    int ret = config_get_value(K_FIRST_BOOT, &first_boot, &len);
    if (0 != ret)
    {
        ezlog_e(TAG_APP, "get valued failed. key: FIRST_BOOT");
        return;
    }

    if (1 == first_boot)
    {
        first_boot = 0;
        if (0 != config_set_value(K_FIRST_BOOT, &first_boot, sizeof(first_boot)))
        {
            ezlog_e(TAG_APP, "need_ap_config 111 config_write error!\n");
        }

        ezlog_i(TAG_APP, "first boot , need ap!\n");
        g_if_need_ap = true;
        return;
    }

    //2、电源开关开关开
    int power_on_num = 0;
    len = sizeof(power_on_num);
    if (0 != config_get_value(K_POWER_ON_NUM, &power_on_num, &len))
    {
        ezlog_e(TAG_APP, "get value failed.");
        return;
    }

    char reset_switch_times = get_product_reset_switch_times();
    int reset_time_upper = get_reset_time_upper();
    if (power_on_num >= reset_switch_times && power_on_num <= reset_time_upper)
    {
        ezlog_i(TAG_APP, "on-off need ap config!");
        g_if_need_ap = true;
        return;
    }

    //3、手机APP重置
    int enable = 0;
    len = sizeof(enable);
    ret = config_get_value(K_AP_ENABLE, &enable, &len);
    if (0 != ret)
    {
        ezlog_e(TAG_APP, "get valued failed. key: AP_ENABLE");
        return;
    }

    if (1 == enable)
    {
        enable = 0;

        ezlog_i(TAG_APP, "app reset need ap config!");

        if (0 != config_set_value(K_AP_ENABLE, &enable, sizeof(enable)))
        {
            ezlog_e(TAG_APP, "set value failed!");
        }
        g_if_need_ap = true;
        return;
    }
    return;
}

int ap_config_checkupdate(void)
{
    int rv = 0;
    char reset_switch_times = get_product_reset_switch_times();
    int reset_time_upper = get_reset_time_upper();

    do
    {
        int power_on_num = 0;
        int len = sizeof(power_on_num);
        if (0 != config_get_value(K_POWER_ON_NUM, &power_on_num, &len))
        {
            ezlog_e(TAG_APP, "read ap cfg");
            break;
        }

        ezlog_i(TAG_APP, "power_on_num[%d]", power_on_num);
        power_on_num++;

        if (0 != config_set_value(K_POWER_ON_NUM, &power_on_num, sizeof(power_on_num)))
        {
            ezlog_e(TAG_APP, "set value failed.");
            break;
        }

        if (power_on_num >= reset_switch_times && power_on_num <= reset_time_upper)
        {
            rv = 1;
        }
        ez_thread_t power_on_thread_handle;
        rv = ezos_thread_create(&power_on_thread_handle, "power_on_num_clear_task", power_on_num_clear_task, NULL, 2 * 1024, 5);
        if (rv < 0)
        {
            ezlog_e(TAG_SHD, "power_on_num_clear_task thread create error");
        }

    } while (0);

    need_ap_config();

    return rv;
}

int ap_distribution_check(void)
{
    return g_if_need_ap;
}

/* 设置ap模式为开启状态，设备重启会生效一次*/
ez_bool_t ap_distribution_set(ez_bool_t enable, ez_int_t timeout)
{
    g_need_ap = enable;

    int ret = 0;

    do
    {
        int ap_enable = enable ? 1 : 0;
        int ap_timeout = timeout;

        int len = sizeof(int);

        ret = config_set_value(K_AP_ENABLE, &ap_enable, len);
        ret |= config_set_value(K_AP_TIME_OUT, &ap_timeout, len);
    } while (false);

    if (0 != ret)
    {
        return false;
    }
    return true;
}

void wifi_connect_do(void)
{
    /* 从设备flash 获取ssid 以及密码信息*/
    char ssid[33] = {0};
    int len = sizeof(ssid);

    if (0 != config_get_value(K_WIFI_SSID, &ssid, &len))
    {
        return;
    }

    if (0 == strlen(ssid))
    {
        ezlog_e(TAG_APP, "ssid len range!");
        return;
    }

    char password[65] = {0};
    len = sizeof(password);
    if (0 != config_get_value(K_WIFI_PASSWORD, &password, &len))
    {
        return;
    }

    ezconn_wifi_config(EZCONN_WIFI_MODE_STA);
    ezconn_sta_start(ssid, password);
}

void ap_distribution_do()
{
    ezconn_dev_info_t dev_info = {0};
    ezconn_ap_info_t ap_info = {0};
    char ssid[33] = {0};
    g_apstamode_flag = true;

    device_t *device_info = get_product_device_config();
    if (NULL == device_info)
    {
        ezlog_e(TAG_APP, "ap_config get_product_device_config error!");
        return false;
    }

    if (0 == get_dev_auth_mode())
    {
        sprintf(ssid, "%s_%s", device_info->ap_prefix, get_dev_deviceName());
    }
    else
    {
        sprintf(ssid, "%s_%s", device_info->ap_prefix, get_dev_subserial() + strlen(get_dev_subserial()) - 9);
    }

    if (ssid[32] != '\0')
    {
        ssid[32] = '\0';
    }

    strncpy((char *)ap_info.ap_ssid, ssid, sizeof(ap_info.ap_ssid) - 1);
    ap_info.auth_mode = 0;
    ap_info.channel = 1;
    ap_info.ap_timeout = 5;
    ap_info.apsta_coexist = ez_true;

    strncpy((char *)dev_info.dev_serial, get_dev_subserial(), sizeof(dev_info.dev_type));
    strncpy((char *)dev_info.dev_type, get_dev_productKey(), sizeof(dev_info.dev_type));

    char dev_version[64];
    mk_soft_version(dev_version);
    memcpy(dev_info.dev_version, dev_version, sizeof(dev_info.dev_version));

    ezconn_wifi_config(EZCONN_WIFI_MODE_APSTA);
    ezconn_ap_start(&ap_info, &dev_info, wifi_ap_distribution_cb);
    pt_light_set_mode(MODE_AP_START);
    g_ap_exit = false;

    return;
}
