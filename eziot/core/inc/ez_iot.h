/*******************************************************************************
 * Copyright © 2017-2021 Ezviz Inc.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 * 
 * Contributors:
 * XuRongjun (xurongjun@ezvizlife.com)
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-11-04     xurongjun    first version 
 *******************************************************************************/

#ifndef _EZ_IOT_H_
#define _EZ_IOT_H_

#include <ezos.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct
    {
        ez_char_t host[64]; ///< 平台域名
        ez_uint16_t port;   ///< 端口
    } ez_server_info_t;

    typedef struct
    {
        ez_int8_t auth_mode;                 ///< 认证模式：0 SAP认证   1 licence认证
        ez_char_t dev_type[64];              ///< 设备型号(对应licence认证中productKey)
        ez_char_t dev_subserial[16];         ///< 设备短序列号(对应licence认证中productKey:deviceName)
        ez_char_t dev_verification_code[16]; ///< 设备验证码(对应licence认证中deviceLicense)
        ez_char_t dev_firmwareversion[64];   ///< 设备固件版本号
        ez_char_t dev_typedisplay[64];       ///< 设备显示型号
        ez_char_t dev_mac[32];               ///< 设备网上物理地址
    } ez_dev_info_t;

    typedef enum
    {
        EZ_EVENT_ONLINE,        ///< 设备上线
        EZ_EVENT_OFFLINE,       ///< 设备离线
        EZ_EVENT_DISCONNECT,    ///< 设备网络异常
        EZ_EVENT_RECONNECT,     ///< 设备网络重连成功
        EZ_EVENT_DEVID_UPDATE,  ///< 设备DEVID更新，需要外部保存至FLASH
        EZ_EVENT_SERVER_UPDATE, ///< 设备上线的域名更新，下次上线需要用新的域名，需要外部保存至FLASH
    } ez_event_e;

    typedef enum
    {
        EZ_CMD_DEVID_SET,  ///< 设置设备DEVID，arg(ez_byte_t [32])
        EZ_CMD_KVIMPL_SET, ///< 设置KV 实现函数，arg(ez_kv_func_t*)
    } ez_cmd_e;

    typedef int32_t (*ez_event_notice)(ez_event_e event_type, ez_void_t *data, ez_int32_t len);

    /**
     * @brief 
     * 
     * @param psrv_info 
     * @param pdev_info 
     * @param pfunc 
     * @return 参考 ez_err_e 
     */
    EZOS_API ez_err_t EZOS_CALL ez_iot_init(const ez_server_info_t *psrv_info, const ez_dev_info_t *pdev_info, const ez_event_notice pfunc);

    /**
     * @brief 开始接入云端
     * 
     * @return 参考 ez_err_e
     */
    EZOS_API ez_err_t EZOS_CALL ez_iot_start(ez_void_t);

    /**
     * @brief 停止接入云端
     * 
     * @return 参考 ez_err_e
     */
    EZOS_API ez_err_t EZOS_CALL ez_iot_stop(ez_void_t);

    /**
     * @brief 
     * 
     * @return 参考 ez_err_e
     */
    EZOS_API ez_void_t EZOS_CALL ez_iot_deinit(ez_void_t);

    /**
     * @brief 
     * 
     * @param cmd 
     * @param arg 
     * @return 参考 ez_err_e
     */
    EZOS_API ez_err_t EZOS_CALL ez_iot_attr_ctrl(ez_cmd_e cmd, ez_void_t *arg);

#ifdef __cplusplus
}
#endif

#endif