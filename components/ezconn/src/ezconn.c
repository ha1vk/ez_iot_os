#include "ezconn.h"
#include "ezlog.h"
#include "ezhal_wifi.h"
#include "ezconn_adapter.h"

ez_err_t ezconn_wifi_init()
{
    ezlog_w(TAG_AP, "wifi init");
    return ezhal_wifi_init();
}

ez_err_t ezconn_wifi_config(ezconn_wifi_mode_e wifi_mode)
{
    ezlog_w(TAG_AP, "wifi config");
    return ezhal_wifi_config(wifi_mode);
}

ez_err_t ezconn_sta_start(ez_char_t *ssid, ez_char_t *password)
{
    ezlog_w(TAG_AP, "station start");
    return ezhal_sta_connect(ssid, password);
}

ez_err_t ezconn_sta_stop()
{
    ezlog_w(TAG_AP, "station stop");
    return ezhal_sta_stop();
}

ez_err_t ezconn_ap_start(ezconn_ap_info_t *ap_info, ezconn_dev_info_t *dev_info, wifi_info_cb cb)
{
    ezlog_w(TAG_AP, "ap start");

    if (NULL == ap_info || NULL == dev_info || NULL == cb)
    {
        ezlog_e(TAG_AP, "%s param error.", __FUNCTION__);
        return EZCONN_ERRNO_INVALID_PARAM;
    }

    return ezconn_adatper_init(ap_info, dev_info, cb);

   
}
ez_err_t ezconn_ap_stop()
{
    ezlog_w(TAG_AP, "ap stop");
    ezconn_adapter_deinit();
    ezhal_ap_stop();
    return 0;
}

ez_err_t ezconn_wifi_deinit()
{
    ezlog_w(TAG_AP, "wifi deinit");
    return ezhal_wifi_deinit();
}
