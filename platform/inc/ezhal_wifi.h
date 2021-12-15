#ifndef EZOS_WIFI_H
#define EZOS_WIFI_H

#include "ezos_def.h"
/**
 * @brief   wifi模块初始化 
 * 
 * @return  0 for success, other for failed
 */
int ezhal_wifi_init();

typedef enum
{
    EZOS_WIFI_MODE_AP, 
    EZOS_WIFI_MODE_STA,
    EZOS_WIFI_MODE_APSTA,
} ezhal_wifi_mode_e;
/**
 * @brief   设置wifi模式，ap模式/station模式/ap+station模式 
 * 
 * @param   wifi_mode ： see ezhal_wifi_mode_e
 * @return  0 for success, other for failed
 */
int ezhal_wifi_config(ezhal_wifi_mode_e wifi_mode);

/**
 * @brief   start station mode, and connect to route specified ssid and password
 * 
 * @param   ssid : ssid of the route to connect
 * @param   password : password of the route to connect
 * @return  0 for success, other for failed 
 * 
 * @warning must invoke after ezhal_wifi_init
 */
int ezhal_sta_connect(char *ssid, char *password);

/**
 * @brief  stop station mode  
 * 
 * @return  0 for success, other for failed 
 */
int ezhal_sta_stop();

/**
 * @brief   start ap mode
 *
 * @param   ssid : ap ssid
 * @param   password : ap password
 * @param   auth_mode : auth mode. see ezhal_wifi_auth_mode_e
 * @param   channel : ap channel
 * @return  0 for success, other for failed
 */
int ezhal_ap_start(char *ssid, char *password, unsigned char auth_mode, unsigned char channel);

/**
 * @brief   stop ap mode, and stop http server 
 * 
 * @return  0 for success, other for failed
 */
int ezhal_ap_stop();

/**
 * @brief   wifi 模块反初始化 
 * 
 * @return  0 for success, other for failed
 */
int ezhal_wifi_deinit();

typedef enum
{
    EZOS_WIFI_OPEN,
    EZOS_WIFI_WEP,
    EZOS_WIFI_WPA_PERSONAL,
    EZOS_WIFI_WPA2_PERSONAL,
    EZOS_WIFI_WPA_WPA2_PERSONAL,
    EZOS_WIFI_WPA2_ENTERPRISE,
} ezhal_wifi_auth_mode_e;

typedef struct
{
    ezhal_wifi_auth_mode_e authmode;
    char rssi;
    unsigned char channel;
    char bssid[6];
    char ssid[33];
    char res[2];
} ezhal_wifi_list_t;

/*sta或ap+sta模式下扫描wifi列表*/
/**
 *  @brief  station模式或ap+station模式下扫描wifi列表
 *  @param  max_ap_num：本次扫描获取的ap列表最大size
 *  @param  ap_list：数组，其size为max_ap_num，结构体为wifi_info_list_t
 *  @info   wifi列表，按照rssi从大到小的顺序排列
 *
 *  @return 实际扫描到的wifi个数
 */
unsigned char ezhal_sta_get_scan_list(unsigned char max_ap_num, ezhal_wifi_list_t *ap_list);

/**
 * @brief       获取当前连接路由器的rssi
 * 
 * @param[in]   rssi : rssi
 * @return      0 for success, other for failed 
 */
int ezhal_get_rssi(char *rssi);

typedef enum
{
    EZOS_WIFI_STATE_STA_CONNECTED   = 100,  // ap模式下，有客户端连接上

    EZOS_WIFI_STATE_NOT_CONNECT     = 102,  // 设备未连接路由器
    EZOS_WIFI_STATE_CONNECT_SUCCESS = 104,  // 设备连接路由器成功
    EZOS_WIFI_STATE_PASSWORD_ERROR  = 106,  // 密码错误
    EZOS_WIFI_STATE_NO_AP_FOUND     = 201,  // app设置路由器ssid未找到
    EZOS_WIFI_STATE_UNKNOW          = 202,  // 未知错误
} ezhal_wifi_state_e;

/**
 * @brief  获取当前配网或wifi连接状态
 * 
 * @return ezhal_wifi_state_e 
 */
ezhal_wifi_state_e ezhal_get_wifi_state();

#endif
