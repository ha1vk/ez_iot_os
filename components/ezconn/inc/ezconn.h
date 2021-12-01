/**
 * @file ezconn.h
 * @author chentengfei (chentengfei5@ezvizlife.com)
 * @brief 
 * @version 0.1
 * @date 2019-11-06
 * 
 * @copyright HangZhou Ezviz Co.,Ltd. All Right Reserved.
 * 
 */

#ifndef EZ_CONN_H
#define EZ_CONN_H

#include "ezos_def.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define TAG_AP  "T_AP"

#define EZCONN_ERRNO_BASE   0x00010000

#define MIN_TIME_OUT 1      // 配网最小窗口期，单位min
#define MAX_TIME_OUT 60     // 配网最大窗口期，单位min
#define DEFAULT_TIME_OUT 15 // 默认配网窗口期，15min

    typedef enum
    {
        EZCONN_FAILED = -1,             // general errno
        EZCONN_SUCC = 0,                // success
        EZCONN_ERRNO_INVALID_PARAM      = EZCONN_ERRNO_BASE + 0x01,  // invalid param
        EZCONN_ERRNO_INTERNAL           = EZCONN_ERRNO_BASE + 0x02,  // internal error
        EZCONN_ERRNO_AP_START_FAILED    = EZCONN_ERRNO_BASE + 0x03,  // ap start failed
        EZCONN_ERRNO_HTTPD_START_FAILED = EZCONN_ERRNO_BASE + 0x04,  // httpd start failed
        EZCONN_ERRNO_AP_ALREADY_INIT    = EZCONN_ERRNO_BASE + 0x05,  // ap already inited
        EZCONN_ERRNO_GET_LIST_FAILED    = EZCONN_ERRNO_BASE + 0x06,  // get list failed
        EZCONN_ERRNO_NO_MEMORY          = EZCONN_ERRNO_BASE + 0x07,  // no memory
        EZCONN_ERRNO_JSON_PARSE         = EZCONN_ERRNO_BASE + 0x08,  // json parse failed
        EZCONN_ERRNO_PARAM_ERROR        = EZCONN_ERRNO_BASE + 0x09,  // wifi config req context param error
    } ezconn_errno_e;

    // 设备信息，用于ap初始化
    typedef struct
    {
        ez_int8_t ap_ssid[32 + 1];      // ap ssid
        ez_int8_t ap_pwd[64 + 1];       // ap password, 不需要密码则传空
        ez_int8_t auth_mode;            // ap auth mode, 0 for no ap_password 
        ez_uint8_t channel;             // ap channel

        ez_int8_t ap_timeout;           // 配网超时时间，单位min
        ez_bool_t apsta_coexist;        // 是否支持ap station模式共存
        ez_int8_t res[2];
    } ezconn_ap_info_t;

    typedef struct 
    {
        ez_int8_t dev_serial[72];        // 设备序列号
        ez_int8_t dev_type[64];          // 设备类型
        ez_int8_t dev_version[64];       // 设备版本号
    } ezconn_dev_info_t;

    typedef enum
    {
        EZCONN_STATE_SUCC = 0,             ///< ap config succ
        EZCONN_STATE_APP_CONNECTED ,       ///< app connected
        EZCONN_STATE_CONNECTING_ROUTE ,    ///< connecting
        EZCONN_STATE_CONNECT_FAILED,       ///< connect failed
        EZCONN_STATE_WIFI_CONFIG_TIMEOUT,  ///< connect timeout
    } ezconn_state_e;

    // 配网过程获取到的信息
    typedef struct
    {
        ez_int8_t ssid[32+1];        // 设备连接的wifi ssid
        ez_int8_t password[64+1];    // 设备连接的wifi password
        ez_int8_t res[2];

        ez_int8_t cc[4];             // 国家码cc
        ez_int8_t token[128];        // app端校验的token
    
        ez_int8_t domain[128];       // 设备注册平台地址;
        ez_int8_t device_id[32];     // 设备uuid，可选配置
    } ezconn_wifi_info_t;

    /**
     *  @brief  给上层wifi信息的回调函数
     *  @param  err_code: 配网错误码, see ezconn_errno_e
     *  @param  wifi_info: 给应用层的数据
     *  @info   回调中先判断err_code，若err_code为EZCONN_SUCC，则可以从参数wifi_info中拿到ssid和密码
     *          否则，均为配网失败
     *  @warn   回调中不能处理过多的业务，否则可能会导致栈溢出
     */
    typedef void (*wifi_info_cb)(ezconn_state_e err_code, ezconn_wifi_info_t *wifi_info);

    /**
     *  @brief      wifi功能初始化，需要在ez_iot_ap_init和ez_iot_wifi_init调用之前
     *  @return     成功：EZCONN_SUCC，失败：other
     */
    ez_err_t ezconn_wifi_init();

    /**
     *  @brief      station模式下连接路由器
     *  @return     成功：EZCONN_SUCC，失败：other
     */
    ez_err_t ezconn_sta_start(ez_int8_t *ssid, ez_int8_t *password);
    
    /**
     *  @brief      station模式停止
     *  @return     成功：EZCONN_SUCC，失败：other
     */
    ez_err_t ezconn_sta_stop();

    /**
     *  @fn         AP模块初始化，包括初始化ap以及启动httpserver
     *  @param[in]  dev_info：设备信息
     *  @param[in]  cb：回调函数，用于获取wifi设置状态
     *  @param[in]  timeout：wifi配置超时时间，范围1-60，单位min，默认15min
     *  @param[in]  support_apsta：是否支持ap、sta共存模式
     *  @return     成功：EZCONN_SUCC，失败：other
     */
    ez_err_t ezconn_ap_start(ezconn_ap_info_t *ap_info, ezconn_dev_info_t *dev_info, wifi_info_cb cb);

    /**
     *  @fn         AP模块反初始化，包括停掉httpserver和ap模块
     *  @return     成功：EZCONN_SUCC，失败：other
     */
    ez_err_t ezconn_ap_stop();

    /**
     * @brief       wifi功能反初始化 
     * 
     * @return      成功：EZCONN_SUCC ，失败：other
     */
    ez_err_t ezconn_wifi_deinit();

#ifdef __cplusplus
}
#endif

#endif //EZ_CONN_H
