#include "ezos_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_event_loop.h"
#include "esp_wifi.h"
#include "ezlog.h"

#include "string.h"
static const char *TAG_WIFI = "T_WIFI";

static EventGroupHandle_t g_wifi_event_group = NULL;
static ezos_wifi_state_e g_wifi_connect_state = EZOS_WIFI_STATE_NOT_CONNECT;

static ez_bool_t g_wifi_init = false;

static ez_bool_t g_wifi_scan_start = false;
const int g_WIFI_SCAN_DONE_BIT = BIT0;

static void wifi_disconnect_reason(uint8_t reason)
{
    switch (reason)
    {
    case WIFI_REASON_AUTH_FAIL:
    case WIFI_REASON_AUTH_EXPIRE:
        ezlog_i(TAG_WIFI, "WIFI_REASON_PWD_ERROR(%d)", reason);
        g_wifi_connect_state = EZOS_WIFI_STATE_PASSWORD_ERROR;
        break;
    case WIFI_REASON_NO_AP_FOUND:
        ezlog_i(TAG_WIFI, "WIFI_REASON_NO_AP_FOUND(%d)", reason);
        g_wifi_connect_state = EZOS_WIFI_STATE_NO_AP_FOUND;
        break;
    case WIFI_REASON_ASSOC_FAIL:
        ezlog_i(TAG_WIFI, "WIFI_REASON_ASSOC_FAIL(%d)", reason);
        g_wifi_connect_state = EZOS_WIFI_STATE_UNKNOW;
        break;

    default:
        ezlog_e(TAG_WIFI, "unknow reason(%d)", reason);

        break;
    }
    return;
}

static esp_err_t wifi_event_handler(void *ctx, system_event_t *event)
{
    system_event_info_t *info = &event->event_info;

    ezlog_d(TAG_WIFI, "enter!!event->event_id=%d!!!", event->event_id);

    switch (event->event_id)
    {
    case SYSTEM_EVENT_AP_START:
        ezlog_i(TAG_WIFI, "AP_START SUCCESS!!!!!");
        break;
    case SYSTEM_EVENT_AP_STOP:
        ezlog_i(TAG_WIFI, "AP_STOP SUCCESS!!!!!");
        break;
    case SYSTEM_EVENT_SCAN_DONE:
        ezlog_i(TAG_WIFI, "SYSTEM_EVENT_SCAN_DONE!!!!!");
        xEventGroupSetBits(g_wifi_event_group, g_WIFI_SCAN_DONE_BIT);
        break;
    case SYSTEM_EVENT_STA_START:
        ezlog_i(TAG_WIFI, "SYSTEM_EVENT_STA_START!!!!!");
        break;
    case SYSTEM_EVENT_STA_STOP:
        ezlog_i(TAG_WIFI, "SYSTEM_EVENT_STA_STOP!!!!!");
        g_wifi_connect_state = EZOS_WIFI_STATE_NOT_CONNECT;
        break;
    case SYSTEM_EVENT_STA_CONNECTED:
        ezlog_i(TAG_WIFI, "SYSTEM_EVENT_STA_CONNECTED!!!!!");
        g_wifi_connect_state = EZOS_WIFI_STATE_CONNECT_SUCCESS;
        break;
    case SYSTEM_EVENT_STA_GOT_IP:
        ezlog_i(TAG_WIFI, "got ip:%s",
                 ip4addr_ntoa(&event->event_info.got_ip.ip_info.ip));
        break;
    case SYSTEM_EVENT_AP_STACONNECTED: /**< a station connected to ESP8266 soft-AP */
        ezlog_w(TAG_WIFI, "AP_STA_CONNECTED");
        ezlog_i(TAG_WIFI, "station:" MACSTR " join, AID=%d",
                 MAC2STR(event->event_info.sta_connected.mac),
                 event->event_info.sta_connected.aid);
        break;
    case SYSTEM_EVENT_AP_STAIPASSIGNED: /**< ESP8266 soft-AP assign an IP to a connected station */
        ezlog_w(TAG_WIFI, "AP_STA_IPASSIGNED");
        g_wifi_connect_state = EZOS_WIFI_STATE_STA_CONNECTED;
        break;
    case SYSTEM_EVENT_AP_STADISCONNECTED: /**< a station disconnected from ESP8266 soft-AP */
        ezlog_w(TAG_WIFI, "AP_STA_DISCONNECTED");
        ezlog_i(TAG_WIFI, "station:" MACSTR "leave, AID=%d",
                 MAC2STR(event->event_info.sta_disconnected.mac),
                 event->event_info.sta_disconnected.aid);
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        ezlog_w(TAG_WIFI, "SYSTEM_EVENT_STA_DISCONNECTED");
        wifi_disconnect_reason(info->disconnected.reason);
        break;

    default:
        ezlog_e(TAG_WIFI, "unknow event id(%d)", event->event_id);
        break;
    }
    return ESP_OK;
}

ez_int32_t ezos_wifi_init()
{
    ezlog_d(TAG_WIFI, "%s", __FUNCTION__);
    if (g_wifi_init)
    {
        ezlog_w(TAG_WIFI, "wifi inited.");
        return 0;
    }
    g_wifi_event_group = xEventGroupCreate();
    tcpip_adapter_init();
    wifi_init_config_t wifi_config = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&wifi_config);
    esp_event_loop_init(wifi_event_handler, NULL);
    g_wifi_init = true;
    return 0;
}

ez_int32_t ezos_sta_connect(ez_int8_t *ssid, ez_int8_t *password)
{
    wifi_config_t wifi_config;
    esp_err_t ret = ESP_OK;

    ezlog_d(TAG_WIFI, "%s", __FUNCTION__);
    
    ret = esp_wifi_set_mode(WIFI_MODE_STA);
    if (ESP_OK != ret)
    {
        ezlog_e(TAG_WIFI, "wifi config sta mode failed.");
        return -1;
    }

    g_wifi_connect_state = EZOS_WIFI_STATE_NOT_CONNECT; //init connect state
    memset(&wifi_config, 0, sizeof(wifi_config));

    if ((ssid == NULL) || (strlen((char *)(ssid)) == 0))
    {
        ezlog_e(TAG_WIFI, "%s wifi config error, please check ssid!!!", __FUNCTION__);
        return -1;
    }

    if (strlen((char *)(ssid)) > 32)
    {
        ezlog_e(TAG_WIFI, "%s wifi config error, ssid is too long!!!!", __FUNCTION__);
        return -1;
    }
    else
    {
        memcpy(wifi_config.sta.ssid, ssid, 32);
    }

    if (NULL == password)
    {
        wifi_config.sta.threshold.authmode = EZOS_WIFI_OPEN;
        ezlog_i(TAG_WIFI, "connect to ap SSID:%.32s password: NULL", wifi_config.sta.ssid);
    }
    else
    {
        if (strlen((char *)(password)) > 64)
        {
            ezlog_e(TAG_WIFI, "%s wifi config error, pwd is too long!!!!", __FUNCTION__);
            return -1;
        }
        //wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA_WPA2_PSK;
        memcpy(wifi_config.ap.password, password, 64);
        ezlog_i(TAG_WIFI, "connect to ap SSID:%.32s password:%s", wifi_config.sta.ssid, wifi_config.sta.password);
    }

    wifi_config.sta.listen_interval = 10;
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    esp_wifi_set_ps(WIFI_PS_MIN_MODEM);

    ret = esp_wifi_connect();
    if (ESP_OK != ret)
    {
        switch (ret)
        {
        case ESP_ERR_WIFI_NOT_INIT:
            ezlog_i(TAG_WIFI, "WiFi is not initialized by Ezviz_Wifi_Init! ret = %d", ret);
            break;
        case ESP_ERR_WIFI_CONN:
            ezlog_i(TAG_WIFI, "WiFi internal error, station or soft-AP control block wrong! ret = %d", ret);
            break;
        case ESP_ERR_WIFI_SSID:
            ezlog_i(TAG_WIFI, "SSID of AP which station connects is invalid! ret = %d", ret);
            break;
        default:
            ezlog_i(TAG_WIFI, "unknown connect state ret = %d!", ret);
            break;
        }
        g_wifi_connect_state = EZOS_WIFI_STATE_UNKNOW;
    }
    return 0;
}

ez_int32_t ezos_sta_stop()
{
    ezlog_d(TAG_WIFI, "%s", __FUNCTION__);
    g_wifi_connect_state = EZOS_WIFI_STATE_NOT_CONNECT;
    esp_wifi_disconnect();
    return 0;
}

ez_int32_t ezos_ap_start(ez_int8_t *ssid, ez_int8_t *password, ez_uint8_t auth_mode, ez_uint8_t channel)
{
    wifi_config_t wifi_config;
    int ssid_len = 0, pwd_len = 0;

    ezlog_d(TAG_WIFI, "%s", __FUNCTION__);

    memset(&wifi_config, 0, sizeof(wifi_config));

    if ((ssid == NULL) || (strlen((char *)(ssid)) == 0))
    {
        ezlog_e(TAG_WIFI, "%s wifi config  error, please check ssid!!!", __FUNCTION__);
        return -1;
    }

    if (NULL == password && 0 != auth_mode)
    {
        ezlog_e(TAG_WIFI, "%s wifi config error, please set open mode or set pwd!!!", __FUNCTION__);
        return -1;
    }

    ssid_len = strlen((char *)(ssid));

    if (ssid_len > 32)
    {
        ezlog_e(TAG_WIFI, "%s wifi config error, ssid is too long!!!!", __FUNCTION__);
        return -1;
    }

    if (0 != auth_mode)
    {
        pwd_len = strlen((char *)(password));
        if (pwd_len > 64)
        {
            ezlog_e(TAG_WIFI, "%s wifi config error, pwd is too long!!!!", __FUNCTION__);
            return -1;
        }

        memcpy(wifi_config.ap.password, password, 64);
    }

    memcpy(wifi_config.ap.ssid, ssid, 32);
    wifi_config.ap.ssid_len = ssid_len;
    wifi_config.ap.max_connection = 4;
    wifi_config.ap.authmode = auth_mode;
    wifi_config.ap.channel = channel;

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config));

    ezlog_i(TAG_WIFI, "wifi_init_softap finished.SSID:%s password:%s", wifi_config.sta.ssid, wifi_config.sta.password);

    ESP_ERROR_CHECK(esp_wifi_start());

    return 0;
}

ez_int32_t ezos_ap_stop()
{
    wifi_mode_t wifi_mode = WIFI_MODE_NULL;
    esp_err_t ret = ESP_OK;

    esp_wifi_get_mode(&wifi_mode);
    switch (wifi_mode)
    {
    case WIFI_MODE_AP:
        ezlog_d(TAG_WIFI, "stop ap mode.");
        esp_wifi_disconnect();
        break;
    case WIFI_MODE_APSTA:
        ezlog_d(TAG_WIFI, "stop apsta mode.");
        esp_wifi_disconnect();
        esp_wifi_deauth_sta(0);
        break;
    default:
        ezlog_w(TAG_WIFI, "not ap or apsta mode, nothing to do");
        break;
    }
    return ret;
}

ez_int32_t ezos_wifi_deinit()
{
    esp_wifi_stop();
    esp_wifi_deinit();
    if (g_wifi_event_group) 
    {
        vEventGroupDelete(g_wifi_event_group);
        g_wifi_event_group = NULL;
    }
    return 0;
}

ez_uint8_t ezos_sta_get_scan_list(ez_uint8_t max_ap_num, ezos_wifi_list_t *ap_list)
{
    if (g_wifi_scan_start)
    {
        ezlog_w(TAG_WIFI, "wifi scan already started.");
        return 0;
    }
    g_wifi_scan_start = true;
    static wifi_scan_config_t scanConf =
        {
            .ssid = NULL,
            .bssid = NULL,
            .channel = 0,
            .show_hidden = true,
        };

    ezlog_d(TAG_WIFI, "%s enter!!", __FUNCTION__);

    if (max_ap_num == 0 || ap_list == NULL)
    {
        ezlog_e(TAG_WIFI, "!%s parameter erroe!", __FUNCTION__);
        g_wifi_scan_start = false;
        return 0;
    }
    wifi_ap_record_t *tmp_list = (wifi_ap_record_t *)malloc(max_ap_num * sizeof(wifi_ap_record_t));
    if (NULL == tmp_list)
    {
        ezlog_e(TAG_WIFI, "malloc tmp_list failed.");
        return -1;
    }
    

    ESP_ERROR_CHECK(esp_wifi_scan_start(&scanConf, true));
    xEventGroupWaitBits(g_wifi_event_group, g_WIFI_SCAN_DONE_BIT, 0, 1, portMAX_DELAY);
    ezlog_i(TAG_WIFI, "WIFI scan done");
    xEventGroupClearBits(g_wifi_event_group, g_WIFI_SCAN_DONE_BIT);
    
    ez_uint16_t tmp_num = max_ap_num;
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&tmp_num, tmp_list));
    
    for (size_t i = 0; i < tmp_num; i++)
    {
        ezos_wifi_list_t *ap_info = ap_list + i;
        wifi_ap_record_t *ap_scan_info = tmp_list + i;
        ap_info->authmode = ap_scan_info->authmode;
        ap_info->rssi = ap_scan_info->rssi;
        ap_info->channel = ap_scan_info->primary;
        memcpy(ap_info->bssid, ap_scan_info->bssid, sizeof(ap_info->bssid));
        strncpy(ap_info->ssid, ap_scan_info->ssid, sizeof(ap_info->ssid) - 1);
    }
    free(tmp_list);

    g_wifi_scan_start = false;
    return tmp_num;
}


ez_int32_t ezos_get_rssi(ez_int8_t *rssi)
{
    int8_t ret = -1;

    if (EZOS_WIFI_STATE_CONNECT_SUCCESS == g_wifi_connect_state)
    {
        wifi_ap_record_t wifi_param = {0};
        esp_wifi_sta_get_ap_info(&wifi_param);
        *rssi = wifi_param.rssi;

        ezlog_i(TAG_WIFI, "Get Rssi success! rssi = %d", *rssi);

        ret = 0;
    }
    else
    {
        ezlog_i(TAG_WIFI, "Wifi Disconnected, Rssi can't get.");
    }

    return ret;
}

ezos_wifi_state_e ezos_get_state()
{
    return g_wifi_connect_state;
}
