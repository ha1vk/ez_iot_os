#include "config_implement.h"
#include <stdio.h>
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include <stdint.h>
#include <stdlib.h>
#include "config_type.h"
#include "ezlog.h"

#include "product_config.h"
#include "bulb_business.h"


//#include "fdb_def.h"
#include "kv_imp.h"

#define TAG_CONFIG "T_CONF"
#define KV_LENGTH_MAX 2048

typedef enum
{
    TYPE_NUM,
    TYPE_STRING,
} bulb_value_type_e;

typedef struct
{
    bulb_key_e key_e;
    char *key_v;
    bulb_value_type_e value_type;
    int bulb_default_value_num;
    char *bulb_default_value_str;
} bulb_kv_t;


static const bulb_kv_t g_bulb_kv_tab[] = {
    // ap key
    {K_PARAM_VER, "param_ver", TYPE_STRING, 0, "V1.0.0 build 200101"},
    {K_AP_ENABLE, "ap_enable", TYPE_NUM, 0, ""},
    {K_FIRST_BOOT, "first_boot", TYPE_NUM, 1, ""},
    {K_AP_TIME_OUT, "ap_time_out", TYPE_NUM, 0, ""},
    {K_POWER_ON_NUM, "power_on_num", TYPE_NUM, 0, ""},

    {K_WIFI_SSID, "wifi_ssid", TYPE_STRING, 0, ""},
    {K_WIFI_PASSWORD, "wifi_pwd", TYPE_STRING, 0, ""},
    {K_WIFI_CC, "wifi_cc", TYPE_STRING, 0, ""},
    {K_WIFI_IP, "wifi_ip", TYPE_STRING, 0, ""},
    {K_WIFI_MASK, "wifi_mask", TYPE_STRING, 0, ""},
    {K_WIFI_GATEWAY, "wifi_gateway", TYPE_STRING, 0, ""},

    {K_BOOT_LABEL, "boot_lable", TYPE_STRING, 0, ""},
    {K_OTA_CODE, "ota_code", TYPE_NUM, REBOOT_NORMAL, ""},

    {K_LOG_LEVEL, "loglvl", TYPE_NUM, EZ_ELOG_LVL_WARN, ""},
    {K_LOG_REC_START, "LogRecStart", TYPE_NUM, 1, ""},

    {K_PT_PARA1, "para1", TYPE_NUM, 0, ""},
    {K_PT_PARA2, "para2", TYPE_NUM, 0, ""},
    {K_PT_AGE_TIME, "age_time", TYPE_NUM, 0, ""},

    {K_DAYLIGHT, "daylight", TYPE_NUM, 0, ""},
    {K_NTP_SERVER, "ntp_server", TYPE_STRING, 0, "time.ys7.com"},
    {K_TIMEZONE, "time_zone", TYPE_STRING, 0, "UTC+08:00"},
    {K_DAYLIGHT_STR, "daylight_string", TYPE_STRING, 0, ""},

    {K_DOMAIN, "domain", TYPE_STRING, 0, ""},
    {K_DEVICE_ID, "device_id", TYPE_STRING, 0, ""},
    {K_USER_ID, "user_id", TYPE_STRING, 0, ""},
    {K_MASTERKEY, "masterkey", TYPE_STRING, 0, ""},

    {K_DEV_LIST, "dev_list", TYPE_STRING, 0, ""},

    // light things key
    {K_POWERSWITCH, "PowerSwitch", TYPE_NUM, 1, ""},
    {K_COLORTEMPERATURE, "ColorTemperature", TYPE_NUM, 2700, ""},
    {K_BRIGHTNESS, "Brightness", TYPE_NUM, 100, ""},
    {K_COUNTDOWNCFG, "CountdownCfg", TYPE_STRING, 0, "{}"},
    {K_LIGHTSWITCHPLAN, "LightSwitchPlan", TYPE_STRING, 0, "{}"},

    {K_WORKMODE, "WrokMode", TYPE_STRING, 0, "white"},
    {K_CUSTOMSCENECFG, "CustomSceneCfg", TYPE_STRING, 0, "{}"},
    {K_COUNTDOWN, "Countdown", TYPE_STRING, 0, "{}"},
    {K_COLORRGB, "ColorRgb", TYPE_STRING, 0, "#FF0000"},
    {K_MUSICRHYTHM, "MusicRhythm", TYPE_STRING, 0, "{}"},
    {K_BIORHYTHM, "Biorhythm", TYPE_STRING, 0, "{}"},
    {K_WAKEUP, "WakeUp", TYPE_STRING, 0, "{}"},
    {K_HELPSLEEP, "HelpSleep", TYPE_STRING, 0, "{}"},
    {K_TIMEZONECOMPOSE, "TimeZoneCompose", TYPE_STRING, 0, "{\"timeFormat\":\"0\",\"timeZone\": \"UTC+08:00\",\"tzCode\": 42,\"daylightSavingTime\": 1,\"offsetTime\": 0,\"startMonth\": 0,\"startWeekIndex\": \"\",\"startWeekDay\": \"\",\"startTime\": \"08:00:00\",\"endMonth\": 0,\"endWeekIndex\": \"\",\"endWeekDay\": \"\",\"endTime\": \"08:00:00\"}"},
    {K_WIFISTATUS, "WifiStatus", TYPE_STRING, 0, "{}"},

};

int config_set_value(bulb_key_e key_e, void *value, int len)
{
    int ret = 0;

    ret = kv_raw_set(g_bulb_kv_tab[key_e].key_v, value, len);
    if (0 != ret)
    {
        ezlog_e(TAG_CONFIG, "set value failed: %s", g_bulb_kv_tab[key_e].key_v);
    }

    return ret;
}

int config_get_value(bulb_key_e key_e, void *value, ez_size_t *len)
{
    int ret = 0;

    ret = kv_raw_get(g_bulb_kv_tab[key_e].key_v, NULL, len);
    if (0 != ret)
    {
        ezlog_e(TAG_CONFIG, "get length failed: %s, len: %d", g_bulb_kv_tab[key_e].key_v, *len);
        return -1;
    }

    if (NULL == value)
    {
        return 0;
    }
    
    ret = kv_raw_get(g_bulb_kv_tab[key_e].key_v, value, len);
    if (0 != ret)
    {
        ezlog_e(TAG_CONFIG, "get value failed: %s", g_bulb_kv_tab[key_e].key_v);
    }

    return ret;
}

int config_reset_factory()
{
    int ret = 0;

    int tab_size = sizeof(g_bulb_kv_tab) / sizeof(bulb_kv_t);

    for (int i = 0; i < tab_size; i++)
    {
        if (TYPE_NUM == g_bulb_kv_tab[i].value_type)
        {
            ret = kv_raw_set(g_bulb_kv_tab[i].key_v, &g_bulb_kv_tab[i].bulb_default_value_num, sizeof(int));
        }
        else if (TYPE_STRING == g_bulb_kv_tab[i].value_type)
        {
            ret = kv_raw_set(g_bulb_kv_tab[i].key_v, g_bulb_kv_tab[i].bulb_default_value_str, strlen(g_bulb_kv_tab[i].bulb_default_value_str));
        }

        if (0 != ret)
        {
            ezlog_e(TAG_CONFIG, "reset config failed. key: %s", g_bulb_kv_tab[i].key_v);
            break;
        }
    }

    product_config_t *p_config = get_product_config();
    int default_cct = p_config->param.function.default_cct;
    config_set_value(K_COLORTEMPERATURE, &default_cct, sizeof(default_cct));

    return ret;
}

int config_reset_wifi()
{
    int ret = 0;

    for (size_t i = K_AP_ENABLE; i <= K_WIFI_GATEWAY; i++)
    {
        if (TYPE_NUM == g_bulb_kv_tab[i].value_type)
        {
            ret = kv_raw_set(g_bulb_kv_tab[i].key_v, &g_bulb_kv_tab[i].bulb_default_value_num, sizeof(int));
        }
        else if (TYPE_STRING == g_bulb_kv_tab[i].value_type)
        {
            ret = kv_raw_set(g_bulb_kv_tab[i].key_v, g_bulb_kv_tab[i].bulb_default_value_str, strlen(g_bulb_kv_tab[i].bulb_default_value_str));
        }

        if (0 != ret)
        {
            ezlog_e(TAG_CONFIG, "reset config failed. key: %s", g_bulb_kv_tab[i].key_v);
            break;
        }
    }

    int enable = 1;
    ret = kv_raw_set(g_bulb_kv_tab[K_AP_ENABLE].key_v, &enable, sizeof(enable));
    if (0 != ret)
    {
        ezlog_e(TAG_CONFIG, "reset config failed. key: %s", g_bulb_kv_tab[K_AP_ENABLE].key_v);
        return ret;
    }

    return ret;
}

int config_reset_time_zone(void)
{
    int ret = 0;
    for (size_t i = K_DAYLIGHT; i <= K_DAYLIGHT_STR; i++)
    {
        if (TYPE_NUM == g_bulb_kv_tab[i].value_type)
        {
            ret = kv_raw_set(g_bulb_kv_tab[i].key_v, &g_bulb_kv_tab[i].bulb_default_value_num, sizeof(int));
        }
        else if (TYPE_STRING == g_bulb_kv_tab[i].value_type)
        {
            ret = kv_raw_set(g_bulb_kv_tab[i].key_v, g_bulb_kv_tab[i].bulb_default_value_str, strlen(g_bulb_kv_tab[i].bulb_default_value_str));
        }

        if (0 != ret)
        {
            ezlog_e(TAG_CONFIG, "reset time_zone failed. key: %s", g_bulb_kv_tab[i].key_v);
            break;
        }
    }
    return ret;
}

int config_reset_time_zone_str(void)
{
    int ret = 0;

    ret = kv_raw_set(g_bulb_kv_tab[K_TIMEZONECOMPOSE].key_v, g_bulb_kv_tab[K_TIMEZONECOMPOSE].bulb_default_value_str, strlen(g_bulb_kv_tab[K_TIMEZONECOMPOSE].bulb_default_value_str));
    if (0 != ret)
    {
        ezlog_e(TAG_CONFIG, "reset time_zone string failed. key: %s", g_bulb_kv_tab[K_TIMEZONECOMPOSE].key_v);
    }

    return ret;
}

int config_print()
{
    int ret = 0;

    int tab_size = sizeof(g_bulb_kv_tab) / sizeof(bulb_kv_t);

    int value = -100;
    ez_size_t len = 0;
    char value_str[64] = {0};
    for (int i = 0; i < tab_size; i++)
    {
        memset(value_str, 0, sizeof(value_str));
        if (TYPE_NUM == g_bulb_kv_tab[i].value_type)
        {
            len = sizeof(value);
            ret = kv_raw_get(g_bulb_kv_tab[i].key_v, &value, &len);
            ezlog_i(TAG_CONFIG, "key: %s, value: %d", g_bulb_kv_tab[i].key_v, value);
        }
        else if (TYPE_STRING == g_bulb_kv_tab[i].value_type)
        {
            len = sizeof(value_str);
            ret = kv_raw_get(g_bulb_kv_tab[i].key_v, value_str, &len);
            ezlog_i(TAG_CONFIG, "key: %s, value: %s", g_bulb_kv_tab[i].key_v, value_str);
        }

        if (0 != ret)
        {
            ezlog_e(TAG_CONFIG, "reset config failed. key: %s", g_bulb_kv_tab[i].key_v);
            break;
        }
    }

    return ret;
}

int bulb_param_init()
{    
    int ret = 0;
    light_param_t *p_bulb_param = get_light_param();
    size_t len_tmp= KV_LENGTH_MAX;
    int value_tmp=0;
    int i=0;
    int r, g, b;
    
    char *json_string = ezos_malloc(KV_LENGTH_MAX); 
    if(NULL == json_string)
    {
        ezlog_e(TAG_APP, "bulb_param_init error ,no memory. ");
        return -1;
    }
    
    for(i=K_POWERSWITCH;i<K_HELPSLEEP;i++)
    {
        if (TYPE_NUM == g_bulb_kv_tab[i].value_type)
        {
            len_tmp = sizeof(value_tmp);
            ret = kv_raw_get(g_bulb_kv_tab[i].key_v, &value_tmp, &len_tmp);
        }
        else if (TYPE_STRING == g_bulb_kv_tab[i].value_type)
        {
            len_tmp = KV_LENGTH_MAX;
            memset(json_string,0,KV_LENGTH_MAX);
            kv_raw_get(g_bulb_kv_tab[i].key_v, json_string, &len_tmp);
        }

        switch(i)
        {
            case K_POWERSWITCH:                
                p_bulb_param->swit = value_tmp;
                break;
                
            case K_COLORTEMPERATURE:
                p_bulb_param->cct = value_tmp;
                break;
                
            case K_BRIGHTNESS:
                p_bulb_param->brightness = value_tmp;               
                break;   
                
            case K_COUNTDOWNCFG:
                set_light_countdown(json_string);
                break;
                
            case K_LIGHTSWITCHPLAN:
                set_light_plan(json_string);
                break;

            case K_WORKMODE:
                if (0 == strcmp(json_string, "white"))
                {
                    set_light_mode(LIGHT_WHITE);
                }
                else if (0 == strcmp(json_string, "colour"))
                {
                    set_light_mode(LIGHT_COLOR);
                }
                else if (0 == strcmp(json_string, "scene"))
                {
                    set_light_mode(LIGHT_SCENE);
                }
                else 
                {
                    ezlog_e(TAG_APP, "unknow workmode ");
                }

            case K_CUSTOMSCENECFG:
                set_light_scene(json_string);
                break;

            case K_COUNTDOWN:
                set_light_countdown(json_string);
                break;
                
            case K_COLORRGB:                
                sscanf(json_string, "#%2x%2x%2x", &r, &g, &b);
                p_bulb_param->rgb  =r * 256 * 256 + g * 256 + b;;
                break;

            case K_MUSICRHYTHM:
                printf("\n LW_PRINT DEBUG in line (%d) and function (%s)): \n ",__LINE__, __func__);
                break;
                
            case K_BIORHYTHM:
                set_light_biorhythm(json_string);
                break;
                
            case K_WAKEUP:
                set_light_wakeup(json_string);
                break;
            case K_HELPSLEEP: 
                set_light_helpsleep(json_string);
                break;
        }
    }

    free(json_string);
    return 0;
    
}

int config_init(void)
{
    ezlog_i(TAG_APP, "config_init");
	char param_ver[24] = {0};
	char len =24;
	int ret = config_get_value(K_PARAM_VER, &param_ver, &len);

    if (0 == len)
    {
        ezlog_e(TAG_APP, "config param ver is null. set all to factory");
        config_reset_factory();
    }
    else
    {
        ezlog_i("param ver is %s\n",param_ver);
        
    }

    bulb_param_init();
	return 0;
}


#ifdef BULB_VERSION
/**@fn		  
 * @brief		  除开网络，产测参数，服务器以外的信息
 * @param[in]  
 * @param[out] 
 * @return	  
 */
int generate_default_config_user()
{
	#if 0
	write_default_wifi_config();

	write_default_ap_config();

	write_default_server_config();

	write_default_product_test_config();  
	#endif
	
	// write_default_default_light_config();

	// write_default_share_light_config();

	// write_default_shadow_version();

	// write_default_time_zone_string_config();

	// write_default_time_zone_config();

	// write_default_scene_conf_config();

	// write_default_scene_config();

	return 0;
}

/**@fn		  
 * @brief		  除开产测参数，以外的信息,退货换到其他用户手里
 * @param[in]  
 * @param[out] 
 * @return	  
 */
int generate_default_config_user_factory()
{
	
	// write_default_wifi_config();

	// write_default_ap_config();

	// write_default_server_config();

	// #if 0
	// write_default_product_test_config();  
	// #endif
	
	// write_default_default_light_config();

	// write_default_share_light_config();

	// write_default_shadow_version();

	// write_default_time_zone_string_config();

	// write_default_time_zone_config();

	// write_default_scene_conf_config();

	// write_default_scene_config();

	return 0;
}

#endif