#ifndef _EZVIZ_CONFIG_H_
#ifdef __cplusplus
extern "C"
{
#endif
#define _EZVIZ_CONFIG_H_

#include "stdint.h"
#include <time.h>

    typedef struct
    {
        int enable;       //手机app重置网络0:无；1:重置网络，需要重新ap配网
        int first_boot;   //首次上电 0：非首次上电 1：首次上电
        int timeout;      //配网窗口期时间单位分钟，=0时，用配置文件中的时间，
        int power_on_num; //电源开关几次
    } net_share_ap_t;

    typedef struct
    {
        char ssid[33];     // 设备连接的wifi ssid
        char password[65]; // 设备连接的wifi password
        char cc[4];       //保留位，保持4字节对齐
        char ip[16];       // Ip地址
        char mask[16];     // 子网掩码
        char gateway[16];   // 网关
    } wifi_t;

    typedef struct
    {
        char domain[128];   // 设备注册平台地址;
        char device_id[32]; // 设备uuid，可选配置
        char user_id[64];   // 设备所属用户的id，设备绑定用户后保存
        char masterkey[16]; // 设备的masterkey，由首次上线时候生成或者按平台策略更新
    } server_t;
    
    typedef struct
    {
        char host[64];           // 服务器域名
        char daylightstring[64]; //夏令时格式化字符串
        char timezone[16];       // 设备时区
        int8_t daylight;         // 夏令时
        char res[3];
    } time_zone_t;

#ifdef __cplusplus
}
#endif
#endif /* _EZVIZ_CONFIG_H_ */
