#ifndef _EZVIZ_CONFIG_IMPLEMENT_H_
#ifdef __cplusplus
extern "C"
{
#endif
#include "ezos_def.h"
#define _EZVIZ_CONFIG_IMPLEMENT_H_

    typedef enum
    {

        K_PARAM_VER,
		K_AP_ENABLE,    // ap_enable
        K_FIRST_BOOT,   // first_boot
        K_AP_TIME_OUT,  // ap_time_out
        K_POWER_ON_NUM, // power_on_num

        K_WIFI_SSID,
        K_WIFI_PASSWORD,
        K_WIFI_CC,
        K_WIFI_IP,
        K_WIFI_MASK,
        K_WIFI_GATEWAY,

        K_BOOT_LABEL,
        K_OTA_CODE,

        K_LOG_LEVEL,
        K_LOG_REC_START,

        K_PT_PARA1,
        K_PT_PARA2,
        K_PT_AGE_TIME,

        K_DAYLIGHT,
        K_NTP_SERVER,
        K_TIMEZONE,
        K_DAYLIGHT_STR,

        K_DOMAIN,
        K_DEVICE_ID,
        K_USER_ID,
        K_MASTERKEY,

        K_DEV_LIST,

        K_POWERSWITCH,
        K_COLORTEMPERATURE,
        K_BRIGHTNESS,
        K_COUNTDOWNCFG,
        K_LIGHTSWITCHPLAN,
        K_WORKMODE,
        K_CUSTOMSCENECFG,
        K_COUNTDOWN,
        K_COLORRGB,
        K_MUSICRHYTHM,
        K_BIORHYTHM,
        K_WAKEUP,
        K_HELPSLEEP,
        K_TIMEZONECOMPOSE,
        K_WIFISTATUS,
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

    /**
     * @brief       set the log print level 
     * 
     * @return      
     */
    void set_log_lvl(int loglvl);

    int config_reset_factory();
	/* 日志记录到flash ，指定级别类的串口打印记录到设备*/
	void set_logRec_start(int bLogRecStart);

    int config_init(void);
	
#ifdef __cplusplus
}
#endif
#endif /* _EZVIZ_CONFIG_IMPLEMENT_H_ */
