#include "string.h"
#include "stdio.h"

#include "ezhal_wifi.h"
#include "ezos_def.h"

static ezhal_wifi_state_e g_wifi_connect_state = EZOS_WIFI_STATE_NOT_CONNECT;

static ez_bool_t g_wifi_init = ez_false;

int ezhal_wifi_init()
{
    printf("%s\n", __FUNCTION__);
    if (g_wifi_init)
    {
        printf("wifi inited.\n");
        return 0;
    }
    g_wifi_init = ez_true;
    return 0;
}

int ezhal_wifi_config(ezhal_wifi_mode_e wifi_mode)
{
    switch (wifi_mode)
    {
    case EZOS_WIFI_MODE_AP:
        printf("wifi ap mode\n");
        break;
    case EZOS_WIFI_MODE_STA:
        printf("wifi sta mode\n");
        break;
    case EZOS_WIFI_MODE_APSTA:
        printf("wifi ap sta mode\n");
        break;
    default:
        break;
    }
    return 0;
}

int ezhal_sta_connect(char *ssid, char *password)
{
    printf("%s\n", __FUNCTION__);

    g_wifi_connect_state = EZOS_WIFI_STATE_NOT_CONNECT; //init connect state

    if ((ssid == NULL) || (strlen((char *)(ssid)) == 0))
    {
        printf("%s wifi config error, please check ssid!!!\n", __FUNCTION__);
        return -1;
    }

    if (strlen((char *)(ssid)) > 32)
    {
        printf("%s wifi config error, ssid is too long!!!!\n", __FUNCTION__);
        return -1;
    }

    if (NULL == password)
    {
        printf("connect to ap SSID:%.32s password: NULL\n", ssid);
    }
    else
    {
        if (strlen((char *)(password)) > 64)
        {
            printf("%s wifi config error, pwd is too long!!!!\n", __FUNCTION__);
            return -1;
        }
        printf("connect to ap SSID:%.32s password:%s\n", ssid, password);
    }

    g_wifi_connect_state = EZOS_WIFI_STATE_UNKNOW;
    return 0;
}

int ezhal_sta_stop()
{
    printf("%s\n", __FUNCTION__);
    g_wifi_connect_state = EZOS_WIFI_STATE_NOT_CONNECT;
    return 0;
}

int ezhal_ap_start(char *ssid, char *password, unsigned char auth_mode, unsigned char channel)
{
    int ssid_len = 0, pwd_len = 0;

    printf("%s\n", __FUNCTION__);

    if ((ssid == NULL) || (strlen((char *)(ssid)) == 0))
    {
        printf("%s wifi config  error, please check ssid!!!\n", __FUNCTION__);
        return -1;
    }

    if (NULL == password && 0 != auth_mode)
    {
        printf("%s wifi config error, please set open mode or set pwd!!!\n", __FUNCTION__);
        return -1;
    }

    ssid_len = strlen((char *)(ssid));

    if (ssid_len > 32)
    {
        printf("%s wifi config error, ssid is too long!!!!\n", __FUNCTION__);
        return -1;
    }

    if (0 != auth_mode)
    {
        pwd_len = strlen((char *)(password));
        if (pwd_len > 64)
        {
            printf("%s wifi config error, pwd is too long!!!!\n", __FUNCTION__);
            return -1;
        }

    }

    printf("wifi_init_softap finished.SSID:%s password:%s\n", ssid, password);

    return 0;
}

int ezhal_ap_stop()
{
    printf("ap stop\n");

    return 0;
}

int ezhal_wifi_deinit()
{
    printf("ap stop\n");

    return 0;
}

unsigned char ezhal_sta_get_scan_list(unsigned char max_ap_num, ezhal_wifi_list_t *ap_list)
{
    printf("%s enter!!\n", __FUNCTION__);

    if (max_ap_num == 0 || ap_list == NULL)
    {
        printf("!%s parameter error!\n", __FUNCTION__);
        return 0;
    }
    
    for (int i = 0; i < 4; i++)
    {
        ezhal_wifi_list_t *ap_info = ap_list + i;
        ap_info->authmode = i;
        ap_info->rssi = 10 * i;
        ap_info->channel = i;
        char ssid[32] = {0};
        sprintf(ssid, "%s_%d", "test_ssid", i);
        strncpy(ap_info->ssid, ssid, sizeof(ap_info->ssid) - 1);
    }

    return 4;
}


int ezhal_get_rssi(char *rssi)
{
    *rssi = -35;

    return 0;
}

ezhal_wifi_state_e ezhal_get_wifi_state()
{
    return g_wifi_connect_state;
}
