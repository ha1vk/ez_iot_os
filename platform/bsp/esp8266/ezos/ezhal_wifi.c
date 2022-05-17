#include "ezhal_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_event_loop.h"
#include "esp_wifi.h"
#include "esp_err.h"
#include "string.h"

static const char *TAG_WIFI = "T_WIFI";

static int g_wifi_start_flag = 0;
static int g_wifi_disconnect_flag = 0;

static EventGroupHandle_t g_wifi_event_group = NULL;
static ezhal_wifi_state_e g_wifi_connect_state = EZOS_WIFI_STATE_NOT_CONNECT;

static ez_bool_t g_wifi_init = false;

static ez_bool_t g_wifi_scan_start = false;
const int g_WIFI_SCAN_DONE_BIT = BIT0;

static void wifi_disconnect_reason(uint8_t reason)
{
    switch (reason)
    {
    case WIFI_REASON_AUTH_FAIL:
    case WIFI_REASON_AUTH_EXPIRE:
        g_wifi_connect_state = EZOS_WIFI_STATE_PASSWORD_ERROR;
        break;
    case WIFI_REASON_NO_AP_FOUND:
        g_wifi_connect_state = EZOS_WIFI_STATE_NO_AP_FOUND;
        break;
    case WIFI_REASON_ASSOC_FAIL:
        g_wifi_connect_state = EZOS_WIFI_STATE_UNKNOW;
        break;

    default:
        break;
    }
    return;
}

static esp_err_t wifi_event_handler(void *ctx, system_event_t *event)
{
    system_event_info_t *info = &event->event_info;

    switch (event->event_id)
    {
    case SYSTEM_EVENT_AP_START:
        break;
    case SYSTEM_EVENT_AP_STOP:
        break;
    case SYSTEM_EVENT_SCAN_DONE:
        xEventGroupSetBits(g_wifi_event_group, g_WIFI_SCAN_DONE_BIT);
        break;
    case SYSTEM_EVENT_STA_START:
        break;
    case SYSTEM_EVENT_STA_STOP:
        g_wifi_connect_state = EZOS_WIFI_STATE_NOT_CONNECT;
        break;
    case SYSTEM_EVENT_STA_CONNECTED:
        g_wifi_connect_state = EZOS_WIFI_STATE_CONNECT_SUCCESS;
        break;
    case SYSTEM_EVENT_STA_GOT_IP:
        break;
    case SYSTEM_EVENT_AP_STACONNECTED: /**< a station connected to ESP8266 soft-AP */
        break;
    case SYSTEM_EVENT_AP_STAIPASSIGNED: /**< ESP8266 soft-AP assign an IP to a connected station */
        g_wifi_connect_state = EZOS_WIFI_STATE_STA_CONNECTED;
        break;
    case SYSTEM_EVENT_AP_STADISCONNECTED: /**< a station disconnected from ESP8266 soft-AP */
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        wifi_disconnect_reason(info->disconnected.reason);

        if (0 == g_wifi_disconnect_flag)
        {
            esp_wifi_connect();
        }
        break;

    default:
        break;
    }
    return ESP_OK;
}

int ezhal_wifi_init()
{
    if (g_wifi_init)
    {
        return 0;
    }

    g_wifi_event_group = xEventGroupCreate();
    tcpip_adapter_init();
    ESP_ERROR_CHECK(esp_event_loop_init(wifi_event_handler, NULL));
    wifi_init_config_t wifi_config = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&wifi_config));
    g_wifi_init = true;
    return 0;
}

int ezhal_wifi_config(ezhal_wifi_mode_e wifi_mode)
{
    switch (wifi_mode)
    {
    case EZOS_WIFI_MODE_AP:
        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
        break;
    case EZOS_WIFI_MODE_STA:
        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
        break;
    case EZOS_WIFI_MODE_APSTA:
        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
        break;
    default:
        break;
    }

    return 0;
}

int ezhal_sta_connect(char *ssid, char *password)
{
    wifi_config_t wifi_config;
    esp_err_t ret = ESP_OK;

    g_wifi_disconnect_flag = 0;
    wifi_mode_t wifi_mode = WIFI_MODE_NULL;
    esp_wifi_get_mode(&wifi_mode);
    if (WIFI_MODE_STA != wifi_mode && WIFI_MODE_APSTA != wifi_mode)
    {
        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    }

    g_wifi_connect_state = EZOS_WIFI_STATE_NOT_CONNECT; //init connect state
    memset(&wifi_config, 0, sizeof(wifi_config));

    if ((ssid == NULL) || (strlen((char *)(ssid)) == 0))
    {
        return -1;
    }

    if (strlen((char *)(ssid)) > 32)
    {
        return -1;
    }
    else
    {
        memcpy(wifi_config.sta.ssid, ssid, 32);
    }

    if (NULL == password)
    {
        wifi_config.sta.threshold.authmode = EZOS_WIFI_OPEN;
    }
    else
    {
        if (strlen((char *)(password)) > 64)
        {
            return -1;
        }

        memcpy(wifi_config.ap.password, password, 64);
    }

    wifi_config.sta.listen_interval = 10;
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    esp_wifi_set_ps(WIFI_PS_MIN_MODEM);
    g_wifi_start_flag = 1;

    ret = esp_wifi_connect();
    if (ESP_OK != ret)
    {
        switch (ret)
        {
        case ESP_ERR_WIFI_NOT_INIT:
            break;
        case ESP_ERR_WIFI_CONN:
            break;
        case ESP_ERR_WIFI_SSID:
            break;
        default:
            break;
        }
        g_wifi_connect_state = EZOS_WIFI_STATE_UNKNOW;
    }
    return 0;
}

int ezhal_sta_stop()
{
    g_wifi_connect_state = EZOS_WIFI_STATE_NOT_CONNECT;
    g_wifi_disconnect_flag = 1;
    esp_wifi_disconnect();
    return 0;
}

int ezhal_ap_start(char *ssid, char *password, unsigned char auth_mode, unsigned char channel)
{
    wifi_config_t wifi_config;
    int ssid_len = 0, pwd_len = 0;

    wifi_mode_t wifi_mode = WIFI_MODE_NULL;
    esp_wifi_get_mode(&wifi_mode);
    if (WIFI_MODE_AP != wifi_mode && WIFI_MODE_APSTA != wifi_mode)
    {
        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    }

    memset(&wifi_config, 0, sizeof(wifi_config));

    if ((ssid == NULL) || (strlen((char *)(ssid)) == 0))
    {
        return -1;
    }

    if (NULL == password && 0 != auth_mode)
    {
        return -1;
    }

    ssid_len = strlen((char *)(ssid));

    if (ssid_len > 32)
    {
        return -1;
    }

    if (0 != auth_mode)
    {
        pwd_len = strlen((char *)(password));
        if (pwd_len > 64)
        {
            return -1;
        }

        memcpy(wifi_config.ap.password, password, 64);
    }

    memcpy(wifi_config.ap.ssid, ssid, 32);
    wifi_config.ap.ssid_len = ssid_len;
    wifi_config.ap.max_connection = 4;
    wifi_config.ap.authmode = auth_mode;
    wifi_config.ap.channel = channel;

    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config));

    ESP_ERROR_CHECK(esp_wifi_start());
    g_wifi_start_flag = 1;

    return 0;
}

int ezhal_ap_stop()
{
    wifi_mode_t wifi_mode = WIFI_MODE_NULL;
    esp_err_t ret = ESP_OK;
    
    g_wifi_disconnect_flag = 1;
    esp_wifi_get_mode(&wifi_mode);

    switch (wifi_mode)
    {
    case WIFI_MODE_AP:
        esp_wifi_disconnect();
        break;
    case WIFI_MODE_APSTA:
        esp_wifi_disconnect();
        esp_wifi_deauth_sta(0);
        break;
    default:
        break;
    }

    return ret;
}

int ezhal_wifi_deinit()
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

unsigned char ezhal_sta_get_scan_list(unsigned char max_ap_num, ezhal_wifi_list_t *ap_list)
{
    if (g_wifi_scan_start)
    {
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

    if (max_ap_num == 0 || ap_list == NULL)
    {
        g_wifi_scan_start = false;
        return 0;
    }

    if (g_wifi_start_flag != 1)
    {
        ESP_ERROR_CHECK(esp_wifi_start());
    }

    wifi_ap_record_t *tmp_list = (wifi_ap_record_t *)malloc(max_ap_num * sizeof(wifi_ap_record_t));
    if (NULL == tmp_list)
    {
        return -1;
    }

    ESP_ERROR_CHECK(esp_wifi_scan_start(&scanConf, true));
    xEventGroupWaitBits(g_wifi_event_group, g_WIFI_SCAN_DONE_BIT, 0, 1, portMAX_DELAY);
    xEventGroupClearBits(g_wifi_event_group, g_WIFI_SCAN_DONE_BIT);

    ez_uint16_t tmp_num = max_ap_num;
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&tmp_num, tmp_list));

    for (size_t i = 0; i < tmp_num; i++)
    {
        ezhal_wifi_list_t *ap_info = ap_list + i;
        wifi_ap_record_t *ap_scan_info = tmp_list + i;
        ap_info->authmode = ap_scan_info->authmode;
        ap_info->rssi = ap_scan_info->rssi;
        ap_info->channel = ap_scan_info->primary;
        memcpy(ap_info->bssid, ap_scan_info->bssid, sizeof(ap_info->bssid));
        strncpy(ap_info->ssid, (const char *)ap_scan_info->ssid, sizeof(ap_info->ssid) - 1);
    }
    free(tmp_list);

    g_wifi_scan_start = false;
    return tmp_num;
}

int ezhal_get_rssi(char *rssi)
{
    int8_t ret = -1;

    if (EZOS_WIFI_STATE_CONNECT_SUCCESS == g_wifi_connect_state)
    {
        wifi_ap_record_t wifi_param = {0};
        esp_wifi_sta_get_ap_info(&wifi_param);
        *rssi = wifi_param.rssi;

        ret = 0;
    }
    else
    {
    }

    return ret;
}

ezhal_wifi_state_e ezhal_get_wifi_state()
{
    return g_wifi_connect_state;
}

void ezhal_set_country_code(char *CountryCode)
{
    //TODO 待适配
}
