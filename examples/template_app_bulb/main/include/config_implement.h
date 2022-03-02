#ifndef _EZVIZ_CONFIG_IMPLEMENT_H_
#ifdef __cplusplus
extern "C"
{
#endif
#include "ezos_def.h"
#define _EZVIZ_CONFIG_IMPLEMENT_H_

    typedef enum
    {
        AP_ENABLE,    // ap_enable
        FIRST_BOOT,   // first_boot
        AP_TIME_OUT,  // ap_time_out
        POWER_ON_NUM, // power_on_num

        WIFI_SSID,
        WIFI_PASSWORD,
        WIFI_CC,
        WIFI_IP,
        WIFI_MASK,
        WIFI_GATEWAY,

        BOOT_LABEL,
        OTA_CODE,

        LOG_LEVEL,
        LOG_REC_START,

        PT_PARA1,
        PT_PARA2,

        DAYLIGHT,
        NTP_SERVER,
        TIMEZONE,
        DATLIGHT_STR,

        DOMAIN,
        DEVICE_ID,
        USER_ID,
        MASTERKEY,

        DEV_LIST,

        SWITCH,
        COLORTEMPERATURE,
        BRIGHTNESS,
        COUNTDOWNCFG,
        LIGHTSWITCHPLAN,
        WORKMODE,
        CUSTOMSCENECFG,
        COUNTDOWN,
        COLORRGB,
        MUSICRHYTHM,
        BIORHYTHM,
        WAKEUP,
        HELPSLEEP,
        TIMEMGR,
        WIFISTATUS,
    } bulb_key_e;

    /**
     * @brief       set value to kv 
     * 
     * @param[in]   key_e: see bulb_key_e 
     * @param[in]   len: length of data to save 
     * @param[out]  value: value of data to save 
     * @return      0 for success, other for failed 
     */
    int config_set_value(bulb_key_e key_e, void *value, int len);

    /**
     * @brief       get value from kv
     *
     * @info        the function get length when value is equal to NULL
     * @param[in]   key_e: see bulb_key_e
     * @param[out]  len: length of data to save
     * @param[out]  value: value of data to save
     * @return      0 for success, other for failed
     */
    int config_get_value(bulb_key_e key_e, void *value, ez_size_t *len);

    /**
     * @brief       device reset factory setting 
     * 
     * @return      0 for success, other for failed 
     */
    int config_reset_factory(void);

    /**
     * @brief       reset wifi config
     * 
     * @return      0 for success, other for failed 
     */
    int config_reset_wifi(void);

    /**
     * @brief       reset time_zone config
     * 
     * @return      0 for success, other for failed 
     */
    int config_reset_time_zone(void);

    /**
     * @brief       reset time_zone string config
     * 
     * @return      0 for success, other for failed 
     */
    int config_reset_time_zone_str(void);

    /**
     * @brief       print all kv in kv_tab 
     * 
     * @return      0 for success, other for failed 
     */
    int config_print();

    void set_log_lvl(int loglvl);

	/* 日志记录到flash ，指定级别类的串口打印记录到设备*/
	void set_logRec_start(int bLogRecStart);
	
#ifdef __cplusplus
}
#endif
#endif /* _EZVIZ_CONFIG_IMPLEMENT_H_ */
