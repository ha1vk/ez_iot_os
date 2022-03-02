#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ez_iot_core_def.h"
#include "ez_iot_core.h"
#include "ez_iot_base.h"
#include "ez_iot_tsl.h"
#include "cJSON.h"
#include "register_server.h"
#include "esp_wifi.h"
#include "internal/esp_wifi_internal.h"
#include "ezhal_wifi.h"
#include "ezlog.h"
#include "kv_imp.h"

#ifndef HEAP_DEBUG
#include "bulb_protocol_tsl_parse.h"
#endif


#include "config_type.h"
#include "config_implement.h"
#include "product_config.h"
#include "dev_init.h"
#include "ezcloud_ota.h"
#include "dev_info.h"
#ifdef SUPPORT_ESP_LWIP_NTP
#include "lwip/apps/sntp.h"
extern char g_time_zone[8];//tsl 需要用
#endif

#include "net_ctrl_tcp.h"

static int g_event_id = -1;
static int g_last_err = 0;
extern int8_t Ezviz_Wifi_Get_Rssi(int8_t* rssi);
char bind_token[64] = {0};

int8_t Ezviz_Wifi_Get_Rssi(int8_t *rssi)
{
    int8_t ret = -1;

    if (EZOS_WIFI_STATE_CONNECT_SUCCESS == ezhal_get_wifi_state())
    {
#ifdef _FREE_RTOS_
        int8_t rssi_temp = 0;
        rssi_temp = esp_wifi_get_ap_rssi();
        *rssi = rssi_temp;
#elif _FREE_RTOS_32_ || _FREE_RTOS_32C3_
        wifi_ap_record_t wifi_param = {0};
        esp_wifi_sta_get_ap_info(&wifi_param);
        *rssi = wifi_param.rssi;
#endif

        //ezlog_i(TAG_WIFI, "Get Rssi success! rssi = %d", *rssi);

        ret = 0;
    }
    else
    {
        //ezlog_i(TAG_WIFI, "Wifi Disconnected, Rssi can't get.");
    }

    return ret;
}

int report_wifi_info(ez_tsl_value_t *value_out)
{
    wifi_t wifi_info = {0};
    memset(&wifi_info, 0, sizeof(wifi_t));
    cJSON *wifi_info_json = NULL;
    char *wifi_info_str = NULL;
    int8_t rssi = 0;
    char rssiStr[32] = {0};
    int rv = -1;

    int ssid_len = sizeof(wifi_info.ssid);
    int password_len = sizeof(wifi_info.ssid);
    int cc_len = sizeof(wifi_info.ssid);
    int ip_len = sizeof(wifi_info.ssid);
    int mask_len = sizeof(wifi_info.ssid);
    int gateway_len = sizeof(wifi_info.ssid);
    do
    {
		char dev_serial[72] = {0};
        sprintf(dev_serial, "%s:%s", get_dev_productKey(), get_dev_deviceName());

        char dev_firmwareversion[64] = {0};
        mk_soft_version(dev_firmwareversion);
        
        if(0 != config_get_value(WIFI_SSID,&wifi_info.ssid,&ssid_len))
        {
            ezlog_e(TAG_APP, "config_read WIFI_SSID error!");
            break;
        }
        ezlog_e(TAG_APP, "config_read WIFI_SSID is:%s",wifi_info.ssid);
        if(0 != config_get_value(WIFI_PASSWORD,&wifi_info.password,&password_len))
        {
            ezlog_e(TAG_APP, "config_read WIFI_PASSWORD error!");
            break;
        }
        if(0 != config_get_value(WIFI_CC,&wifi_info.cc,&cc_len))
        {
            ezlog_e(TAG_APP, "config_read WIFI_CC error!");
            break;
        }
        if(0 != config_get_value(WIFI_IP,&wifi_info.ip,&ip_len))
        {
            ezlog_e(TAG_APP, "config_read WIFI_IP error!");
            break;
        }
        ezlog_e(TAG_APP, "config_read WIFI_IP is:%s",wifi_info.ip);
        if(0 != config_get_value(WIFI_MASK,&wifi_info.mask,&mask_len))
        {
            ezlog_e(TAG_APP, "config_read WIFI_MASK error!");
            break;
        }
        if(0 != config_get_value(WIFI_GATEWAY,&wifi_info.gateway,&gateway_len))
        {
            ezlog_e(TAG_APP, "config_read IFI_GATEWAY error!");
            break;
        }

        if (NULL == (wifi_info_json = cJSON_CreateObject()))
        {
            ezlog_e(TAG_APP, "report_wifi_info cJSON_CreateObject!!");
            break;
        }

        Ezviz_Wifi_Get_Rssi(&rssi);
        snprintf(rssiStr, sizeof(rssiStr), "%d", rssi);
        ezlog_e(TAG_APP, "get rssi is:%d",rssi);
        cJSON_AddStringToObject(wifi_info_json, "type", "wireless");
        cJSON_AddStringToObject(wifi_info_json, "address", wifi_info.ip);
        cJSON_AddStringToObject(wifi_info_json, "mask", wifi_info.mask);
        cJSON_AddStringToObject(wifi_info_json, "gateway", wifi_info.gateway);
        cJSON_AddStringToObject(wifi_info_json, "signal", rssiStr);
        cJSON_AddStringToObject(wifi_info_json, "ssid", wifi_info.ssid);

        if (NULL == (wifi_info_str = cJSON_PrintUnformatted(wifi_info_json)))
        {
            ezlog_e(TAG_APP, "malloc error!!");
            cJSON_Delete(wifi_info_json);
            break;
        }
        if(NULL!=value_out)
        {
            value_out->size = strlen(wifi_info_str);
            value_out->type = EZ_TSL_DATA_TYPE_OBJECT;
            memcpy(value_out->value, wifi_info_str,value_out->size);
            rv=0;
        }
        else
        {
            ez_tsl_value_t tsl_value = {0};
            tsl_value.size = strlen(wifi_info_str);
            tsl_value.type = EZ_TSL_DATA_TYPE_OBJECT;
            tsl_value.value = (void *)wifi_info_str;

            ez_tsl_key_t key_info = {0};
            key_info.domain = (uint8_t *)"WifiStatus";
            key_info.key = (uint8_t *)"NetStatus";


            ez_tsl_rsc_t rsc_info = {.res_type = NULL, .local_index = NULL};

            
        // rv = ez_iot_tsl_property_report(&key_info, &dev_info, &tsl_value);

            rv = ez_iot_tsl_property_report(dev_serial, &rsc_info, &key_info, &tsl_value);
        }
		if (0 != rv)
        {
            ezlog_e(TAG_APP, "prop report wifi_info failed.");       
        }


    } while (0);

    if (wifi_info_str)
        free(wifi_info_str);

    if (wifi_info_json)
        cJSON_Delete(wifi_info_json);

    return rv;
}

#ifdef HEAP_DEBUG
ez_int32_t tsl_notice(ez_tsl_event_e event_type, ez_void_t *data, ez_int32_t len)
{
    return  0;
}

ez_int32_t tsl_things_action2dev(const ez_int8_t *sn, const ez_tsl_rsc_t *rsc_info, const ez_tsl_key_t *key_info,

                                 const ez_tsl_value_t *value_in, ez_tsl_value_t *value_out)
{
printf("\n to_do DEBUG in line (%d) and function (%s)): \n ",__LINE__, __func__);

    return  0;
}

ez_int32_t tsl_things_property2cloud(const ez_int8_t *sn, const ez_tsl_rsc_t *rsc_info, const ez_tsl_key_t *key_info, ez_tsl_value_t *value_out)
{
printf("\n to_do DEBUG in line (%d) and function (%s)): \n ",__LINE__, __func__);

    return  0;
}

ez_int32_t tsl_things_property2dev(const ez_int8_t *sn, const ez_tsl_rsc_t *rsc_info, const ez_tsl_key_t *key_info, ez_tsl_value_t *value_out)
{
    printf("\n to_do DEBUG in line (%d) and function (%s)): \n ",__LINE__, __func__);
    return  0;
}
#endif

#define BULB_EZIOT_TEST_DEV_TYPE "G1FC47D8"
//线上序列号
#define BULB_EZIOT_TEST_DEV_SERIAL_NUMBER "P20151771"
#define BULB_EZIOT_TEST_DEV_VERIFICATION_CODE "LGYmzx"

#define BULB_EZIOT_TEST_CLOUD_HOST "devcn.eziot.com"
#define BULB_EZIOT_TEST_CLOUD_PORT 8666

static ez_kv_func_t g_kv_func = {
    .ezos_kv_init = kv_init,
    .ezos_kv_raw_set = kv_raw_set,
    .ezos_kv_raw_get = kv_raw_get,
    .ezos_kv_del = kv_del,
    .ezos_kv_del_by_prefix = kv_del_by_prefix,
    .ezos_kv_print = kv_print,
    .ezos_kv_deinit = kv_deinit,
};

#ifdef SUPPORT_ESP_LWIP_NTP
int8_t ez_iot_correct_time(const char *ntp_server, const char *time_zone_info, int daylight, const char *daylight_string)
{
    int8_t rv = -1;
    char zone[64] = {0};
    int i = 0;

    do
    {
        if (NULL == ntp_server || NULL == time_zone_info)
        {
            ezlog_e(TAG_TIME, "sntp param error.");
            break;
        }
		//strcpy(g_time_zone, time_zone_info);
		
		strcpy(g_time_zone, time_zone_info+3);// UTC+08:00，这里需要去除UTC 这个字段,给事件上报时传时间。
		ezlog_w(TAG_TIME, "ntp_server:%s, time_zone_info:%s, daylight:%d, daylight_string:%s", ntp_server, time_zone_info, daylight, daylight_string);

		if (0 != strlen(ntp_server))
		{       
            sntp_setservername(0, ntp_server);
            sntp_init();
        }

        for (i = 0; i < strlen(time_zone_info); i++)
        {
            if (time_zone_info[i] == '+')
            {
                sprintf(zone, "CST-%s", &time_zone_info[i + 1]);
                break;
            }
            if (time_zone_info[i] == '-')
            {
                sprintf(zone, "CST+%s", &time_zone_info[i + 1]);
                break;
            }
        }

        /* 若没有找到 时区格式，默认北京时区*/
        if (i == strlen(time_zone_info))
        {
            sprintf(zone, "%s", "CST-8");
        }

        if (0 != daylight)
        {
            ezlog_i(TAG_TIME, "daylight_string:%s.", daylight_string);
            strcat(zone, daylight_string);
        }

        ezlog_i(TAG_TIME, "zone string: %s", zone);
        if (0 != setenv("TZ", zone, 1))
        {
            ezlog_e(TAG_TIME, "Set env failed.");
            break;
        }

        tzset();

        rv = 0;
    } while (0);

    return rv;
}
#endif


void tsl_access()
{

    ez_tsl_callbacks_t tsl_things_cbs = {tsl_notice,tsl_things_action2dev, tsl_things_property2cloud, tsl_things_property2dev};
    //http://resource.eziot.com/group2/M00/00/76/CtwQFmIY8xCAMBQSAAAEdyoaQek67.json
    ez_char_t *profile_bulbV3 = "{\"version\":\"V1.0.0 build 201231\",\"resources\":[{\"identifier\":\"global\",\"resourceCategory\":\"global\",\"localIndex\":[\"0\"],\"localIndexRule\":{},\"dynamic\":false,\"global\":true,\"domains\":[{\"identifier\":\"LightCtrl\",\"props\":[{\"identifier\":\"Brightness\",\"access\":\"rw\"},{\"identifier\":\"ColorTemperature\",\"access\":\"rw\"},{\"identifier\":\"CountdownCfg\",\"access\":\"rw\"},{\"identifier\":\"LightSwitchPlan\",\"access\":\"rw\"}],\"actions\":[{\"identifier\":\"GetCountdown\",\"direction\":\"Plt2Dev\"}],\"events\":[]},{\"identifier\":\"PowerMgr\",\"props\":[{\"identifier\":\"PowerSwitch\",\"access\":\"rw\"}],\"actions\":[],\"events\":[]},{\"identifier\":\"RGBLightCtrl\",\"props\":[{\"identifier\":\"WorkMode\",\"access\":\"rw\"},{\"identifier\":\"MusicRhythm\",\"access\":\"rw\"},{\"identifier\":\"ColorRgb\",\"access\":\"rw\"},{\"identifier\":\"Biorhythm\",\"access\":\"rw\"},{\"identifier\":\"HelpSleep\",\"access\":\"rw\"},{\"identifier\":\"WakeUp\",\"access\":\"rw\"},{\"identifier\":\"CustomSceneCfg\",\"access\":\"rwu\"}],\"actions\":[],\"events\":[]},{\"identifier\":\"TimeMgr\",\"props\":[{\"identifier\":\"TimeZoneCompose\",\"access\":\"rw\"}],\"actions\":[],\"events\":[]},{\"identifier\":\"WifiStatus\",\"props\":[{\"identifier\":\"NetStatus\",\"access\":\"r\"}],\"actions\":[],\"events\":[]}]}]}";

    if (EZ_CORE_ERR_SUCC != ez_iot_tsl_init(&tsl_things_cbs))
    {
        ezlog_w(TAG_APP, "ez_iot_tsl_init  error");
        return;
    }

    if (EZ_CORE_ERR_SUCC != ez_iot_tsl_reg(NULL, profile_bulbV3))
    {
        ezlog_w(TAG_APP, "ez_iot_tsl_reg  error");
        return;
    }
    
}

static ez_int32_t ez_event_notice_func(ez_event_e event_type, ez_void_t *data, ez_int32_t len)
{
    ezlog_w(TAG_APP, "receive event from sdk: event_type[%d]", event_type);
    ///< 通过ap配网或者蓝牙配网从app获取token
    //ez_char_t *dev_token = "11fc0ed37d874d1f8fc58016495db19a";

    switch (event_type)
    {
    case EZ_EVENT_ONLINE:
        ezlog_w(TAG_APP, "dev online");
        //ezos_strncpy(bind_token,"3c8c1db4f4284103afa635a8f54747ab",sizeof(bind_token));
        if (EZ_CORE_ERR_SUCC != ez_iot_base_bind_near(bind_token))
        {
            ezlog_w(TAG_APP, "bind request send error");
        }
        
        music_server_start();
        
        break;

    case EZ_EVENT_OFFLINE:
        ezlog_w(TAG_APP, "dev offline");

        break;
    case EZ_EVENT_DISCONNECT:
        break;
    case EZ_EVENT_RECONNECT:
        break;
    case EZ_EVENT_DEVID_UPDATE:
        ezlog_w(TAG_APP, "because first regist to ezcloud, you recv event devid update");
        /*you should write the devid to your flash to permanent save */
        break;

    case EZ_EVENT_SERVER_UPDATE:
    {
        ezlog_w(TAG_APP, "recv event svrname update,usually it is not happend");

        /* 遇到服务器迁移时，服务器会会发出重定向的请求，这里需要记录服务器域名，以及发起重新绑定 */
        ezlog_w(TAG_APP, "new domain is %s \n", (char *)data);

        if (EZ_CORE_ERR_SUCC != ez_iot_base_bind_near(bind_token))
        {
            ezlog_w(TAG_APP, "bind request send error");
        }
        break;
    }

    default:
        ezlog_w(TAG_APP, "unknow event type. %d", event_type);
        break;
    }

    return 0;
}

static ez_int32_t ez_base_notice_func(ez_base_event_e event_type, ez_void_t *data, ez_int32_t len)
{
    ez_bind_info_t *bind_info;
    ez_bind_challenge_t *bind_chanllenge;
    ezlog_d(TAG_APP, "receive notice message from sdk: event_type[%d]", event_type);

    switch (event_type)
    {
    case EZ_EVENT_BINDING:
    {
        ezlog_w(TAG_APP, "dev bound");
        bind_info = (ez_bind_info_t *)data;
        if (0 != ezos_strlen(bind_info->user_id))
        {
            /*
	            user_id 为客户端绑定时生成的一个对应id，可以用这个id 做一些业务逻辑，比如
	            一台设备被另一个用户抢占式绑定后，之前绑定保存下来的user_id会与之前的user_id 不一致，
	            此时可以恢复下设备上的默认参数比如清除定时参数
	            简单应用可以暂时不考虑处理
				*/
        }
    }
    break;
    case EZ_EVENT_UNBINDING:
    {
        ezlog_w(TAG_APP, "dev unbound");
    }
    break;
    case EZ_EVENT_BINDING_CHALLENGE:
    {
        ezlog_w(TAG_APP, "binding process......receive EZ_EVENT_BINDING_CHALLENGE ");
        //物理按键绑定
    }
    case EZ_EVENT_NTP_INFO:
    {
        
        ez_base_ntp_info_t *p_ntp_info = (ez_base_ntp_info_t *)data;

        
        time_zone_t time_zone_cfg = {0};
        #if 0 //需要保存到flash 中
        if (0 == config_read(CONFIG_TIME_ZONE, &time_zone_cfg) &&
            0 != strncmp(time_zone_cfg.host, ntp_info_t->host, sizeof(time_zone_cfg.host)))
        {
            ez_log_w(TAG_APP, "ntp server update");
            strcpy(time_zone_cfg.host, ntp_info_t->host);
            config_write(CONFIG_TIME_ZONE, &time_zone_cfg);
        }
        #endif
        
		#ifdef SUPPORT_ESP_LWIP_NTP
        ez_iot_correct_time(p_ntp_info->host, time_zone_cfg.timezone, time_zone_cfg.daylight, time_zone_cfg.daylightstring);
		#endif
		
        break;
    }
    break;
    default:
        break;
    }

    return 0;
}

void online_access()
{
    ez_dev_info_t dev_info = {0};
    ez_server_info_t lbs_addr = {BULB_EZIOT_TEST_CLOUD_HOST, BULB_EZIOT_TEST_CLOUD_PORT};

    int ret = 0;
    int len = 0;
    char domain[128] = {0};
    char device_id[32] = {0};
    do
    {
        char ssid[33] = {0};
        len = sizeof(ssid);

        ret = config_get_value(WIFI_SSID, ssid, &len);
        if (0 != ret)
        {
            ezlog_e(TAG_APP, "get ssid failed.");
            break;
        }

        len = sizeof(domain);

        ret = config_get_value(DOMAIN, domain, &len);
        if (0 != ret)
        {
            ezlog_e(TAG_APP, "get domain failed.");
            break;
        }

        if (0 == strlen(ssid) || 0 == strlen(domain))
        {
            ezlog_w(TAG_APP, "invalid lbs / ssid");
			
            ezos_strncpy(domain, "devcn.eziot.com", sizeof(lbs_addr.host) - 1);

            break;
        }


        len = sizeof(device_id);

        ret = config_get_value(DEVICE_ID, device_id, &len);
        if (0 != ret)
        {
            ezlog_e(TAG_APP, "get device_id failed.");
            break;
        }

    } while (ez_false);

    if (0 != ret)
    {
        ezlog_e(TAG_APP, "get config failed.");
        return;
    }

    /*First regist to ezcloud,devid is null, otherwise you shuld get the devid from your flash storage */
    if (EZ_CORE_ERR_SUCC != ez_iot_core_ctrl(EZ_CMD_DEVID_SET, (ez_void_t *)device_id))
    {
        ezlog_w(TAG_APP, "set devid error");
        return;
    }

    if (EZ_CORE_ERR_SUCC != ez_iot_core_ctrl(EZ_CMD_KVIMPL_SET, (ez_void_t *)&g_kv_func))
    {
        ezlog_w(TAG_APP, "set flash kv function error");
        return;
    }

    char dev_firmwareversion[64] = {0};
    mk_soft_version(dev_firmwareversion);

    /*you can get the device info from you flash storage or other method*/
    ezos_strncpy((char*)dev_info.dev_typedisplay, get_user_defined_type(), sizeof(dev_info.dev_typedisplay)- 1);
    ezos_strncpy((char*)dev_info.dev_firmwareversion, dev_firmwareversion, sizeof(dev_info.dev_firmwareversion) - 1);
    ezos_strncpy((char*)lbs_addr.host, domain, sizeof(lbs_addr.host) - 1);

#if 0
    dev_info.auth_mode = get_dev_auth_mode();
    ezos_strncpy(dev_info.dev_type, BULB_EZIOT_TEST_DEV_TYPE, sizeof(dev_info.dev_type) - 1);
    ezos_strncpy(dev_info.dev_subserial, BULB_EZIOT_TEST_DEV_SERIAL_NUMBER, sizeof(dev_info.dev_subserial) - 1);
    ezos_strncpy(dev_info.dev_verification_code, BULB_EZIOT_TEST_DEV_VERIFICATION_CODE, sizeof(dev_info.dev_verification_code) - 1);
#else
    dev_info.auth_mode = get_dev_auth_mode();
    ezos_strncpy((char*)dev_info.dev_type, get_dev_productKey(), sizeof(dev_info.dev_type) - 1);
    ezos_snprintf((char*)dev_info.dev_subserial, sizeof(dev_info.dev_subserial),"%s:%s", get_dev_productKey(), get_dev_deviceName());

    ezos_strncpy((char*)dev_info.dev_verification_code, get_dev_License(), sizeof(dev_info.dev_verification_code) - 1);
#endif
    /*you can get the lbs addres from the ap distribution or from the flash storage*/
    if (EZ_CORE_ERR_SUCC != ez_iot_core_init(&lbs_addr, &dev_info, ez_event_notice_func))
    {
        ezlog_w(TAG_APP, "ez sdk init error");
        return;
    }

    if (EZ_CORE_ERR_SUCC != ez_iot_base_init(ez_base_notice_func))
    {
    }

    if (EZ_CORE_ERR_SUCC != ez_iot_core_start())
    {
        ezlog_w(TAG_APP, "ez_sdk start error");
        return;
    }
}

int register_server(void)
{
    online_access();
    tsl_access();
    #ifdef HAL_ESP
    ez_ota_init();
    ez_ota_start();
    #endif
    return 0;
}

