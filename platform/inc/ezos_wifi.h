#ifndef EZOS_WIFI_H
#define EZOS_WIFI_H

#include "ezos_def.h"

ez_int32_t ezos_wifi_init();

ez_int32_t ezos_sta_connect(ez_int8_t *ssid, ez_int8_t *password);

ez_int32_t ezos_sta_stop();

ez_int32_t ezos_ap_start(ez_int8_t *ssid, ez_int8_t *password, ez_uint8_t auth_mode, ez_uint8_t channel);

ez_int32_t ezos_ap_stop();

ez_int32_t ezos_wifi_deinit();

typedef enum
{
    EZOS_WIFI_OPEN,
    EZOS_WIFI_WEP,
    EZOS_WIFI_WPA_PERSONAL,
    EZOS_WIFI_WPA2_PERSONAL,
    EZOS_WIFI_WPA_WPA2_PERSONAL,
    EZOS_WIFI_WPA2_ENTERPRISE,
} ezos_wifi_auth_mode_e;

typedef struct
{
    ezos_wifi_auth_mode_e authmode;
    ez_int8_t rssi;
    ez_uint8_t channel;
    ez_int8_t bssid[6];
    ez_int8_t ssid[33];
    ez_int8_t res[2];
} ezos_wifi_list_t;

/*sta或ap+sta模式下扫描wifi列表*/
/**
 *  @brief  station模式或ap+station模式下扫描wifi列表
 *  @param  max_ap_num：本次扫描获取的ap列表最大size
 *  @param  ap_list：数组，其size为max_ap_num，结构体为wifi_info_list_t
 *  @info   wifi列表，按照rssi从大到小的顺序排列
 *
 *  @return 实际扫描到的wifi个数
 */
ez_uint8_t ezos_sta_get_scan_list(ez_uint8_t max_ap_num, ezos_wifi_list_t *ap_list);


ez_int32_t ezos_get_rssi(ez_int8_t *rssi);

typedef enum
{
    EZOS_WIFI_STATE_STA_CONNECTED   = 100,  // ap模式下，有客户端连接上

    EZOS_WIFI_STATE_NOT_CONNECT     = 102,  // 设备未连接路由器
    EZOS_WIFI_STATE_CONNECT_SUCCESS = 104,  // 设备连接路由器成功
    EZOS_WIFI_STATE_PASSWORD_ERROR  = 106,  // 密码错误
    EZOS_WIFI_STATE_NO_AP_FOUND     = 201,  // app设置路由器ssid未找到
    EZOS_WIFI_STATE_UNKNOW          = 202,  // 未知错误
} ezos_wifi_state_e;

ezos_wifi_state_e ezos_get_state();

#endif