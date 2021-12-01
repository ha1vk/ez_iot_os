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
 * 2021-11-15     xurongjun    first version 
 *******************************************************************************/

#ifndef _EZ_IOT_OTA_H_
#define _EZ_IOT_OTA_H_

#include <ezos.h>

#define OTA_MODULE_ERRNO_BASE 0x00020000

#ifdef __cplusplus
extern "C"
{
#endif

    typedef enum
    {
        OTA_CODE_NONE = 0,                     ///< 没有错误
        OTA_CODE_FILE_SIZE = 0X00500032,       ///< 下文件大小和查询的信息不匹配
        OTA_CODE_FILE_SIZE_RANGE = 0X00500033, ///< 下文件大小超过分区大小
        OTA_CODE_DNS = 0X00500034,             ///< 下载地址域名解析失败
        OTA_CODE_DOWNLOAD = 0X00500035,        ///< 下载出错（超过最大重试次数）
        OTA_CODE_DIGEST = 0X00500036,          ///< 底层接口调用失败
        OTA_CODE_SIGN = 0X00500037,            ///< 签名校验失败
        OTA_CODE_DAMAGE = 0X00500038,          ///< 升级包损坏
        OTA_CODE_MEM = 0X00500039,             ///< 内存不足
        OTA_CODE_BURN = 0X0050003A,            ///< 烧录固件出错
        OTA_CODE_GENNERAL = 0X0050003B,        ///< 未知错误
        OTA_CODE_CANCEL = 0X0050003C,          ///< 主动停止
        OTA_CODE_RECOVERY = 0X0050003D,        ///< 升级失败，通过备份系统或最小系统恢复
        OTA_CODE_UART_ERR = 0X0050003E,        ///< 外挂设备升级失败
        OTA_CODE_MAX = 0X0050004F
    } ez_ota_errcode_t;

    typedef enum
    {
        EZ_OTA_ERR_SUCC = 0x00,                             ///< Success
        EZ_OTA_ERR_NOT_INIT = OTA_MODULE_ERRNO_BASE + 0x01, ///< The module is not initialized
        EZ_OTA_ERR_PARAM_INVALID = OTA_MODULE_ERRNO_BASE + 0x02,     ///< param invalid
        EZ_OTA_ERR_SEND_MSG_ERR = OTA_MODULE_ERRNO_BASE + 0x03,      ///< send msg error
        EZ_OTA_ERR_JSON_CREATE_ERR = OTA_MODULE_ERRNO_BASE + 0x04,      ///< json create error
        EZ_OTA_ERR_JSON_FORAMT_ERR = OTA_MODULE_ERRNO_BASE + 0x05,      ///< json format error
        EZ_OTA_ERR_REGISTER_ERR = OTA_MODULE_ERRNO_BASE + 0x06,      ///< register error
        EZ_OTA_ERR_DOWNLOAD_ALREADY = OTA_MODULE_ERRNO_BASE + 0x07,      ///< download_already
        EZ_OTA_ERR_MEM_ERROR = OTA_MODULE_ERRNO_BASE + 0x08,      ///< malloc error
    } ez_ota_err_t;

    /**
     * \brief   服务下发的ota消息通知回调
     * \warning 不能阻塞
     * \note    
     */
    typedef enum
    {
        START_UPGRADE, ///< 通知设备执行升级,收到该消息后,设备开始执行升级动作 data: ez_ota_upgrade_info_t*,
    } ez_ota_event_t;

    /**
     * @brief 升级过程状态变化，设备只有在ota_state_upgradeable、ota_state_succ、ota_state_fail三种状态下,
     *        云端才能判断设备是否可升级。
     */
    typedef enum
    {
        OTA_STATE_STARTING = 1,           ///< 开始升级
        OTA_STATE_DOWNLOADING = 2,        ///< 正在下载
        OTA_STATE_DOWNLOAD_COMPLETED = 3, ///< 下载完成
        OTA_STATE_BURNING = 4,            ///< 正在烧录
        OTA_STATE_BURNING_COMPLETED = 5,  ///< 烧录完成
        OTA_STATE_REBOOTING = 6,          ///< 正在重启
    } ez_ota_status_t;

    typedef enum
    {
        OTA_PROGRESS_MIN = 1,

        OTA_PROGRESS_MAX = 100
    } ez_ota_progress_t;

    typedef struct
    {
        ez_int8_t url[270];       ///< 包下载路径,不带http/https前缀。设备根据自己的能力选择下载通道。
        ez_int8_t fw_ver_dst[64]; ///< 差分包对应的原升级包版本号，只有这个版本的固件才能通过差分包升级至fw_ver版本的固件
        ez_int8_t degist[32 + 1]; ///< 升级包的摘要值,包含'\0'
        ez_int32_t size;          ///< 软件包大小，单位Byte
    } ez_ota_file_diff_t;

    typedef struct
    {
        ez_int8_t mod_name[56];     ///< 模块名称,iot 互联模组：PID,e.g41X9TLMJB9GOA2ABOXKLEP, 透传模组对应的mcu 模组为PID_MCU
        ez_int8_t url[270];         ///< 包下载路径,不带http/https前缀，iot互联模组url：ezviz-ota.cn-sh2.ufileos.com/e7135e21d0be26c483ce90e1c09a131f
        ez_int8_t fw_ver[32];       ///< 升级包版本号 e.g V1.1.5 build 210602
        ez_int8_t degist[32 + 1];   ///< 升级包的摘要值,包含'\0'
        ez_int32_t size;            ///< 软件包大小，单位Byte
        ez_ota_file_diff_t *pdiffs; ///< 该版本对应的的差分包
    } ez_ota_file_info_t;

    /**
    * \brief   设备升级模块信息
    */
    typedef struct
    {
        ez_int8_t *mod_name; ///<  模块名称, max 256字节
        ez_int8_t *fw_ver;   ///< 模块固件版本号, max 64字节
    } ez_ota_module_t;

    /**
    * \brief   设备升级模块信息列表
    */
    typedef struct
    {
        ez_int8_t num;          ///< 升级模块数量，最多支持16组
        ez_ota_module_t *plist; ///< 升级模块信息列表
    } ez_ota_modules_t;

    typedef enum
    {
        RESULT_SUC,    ///<下载成功
        RESULT_FAILED, ///<下载失败
    } ez_ota_cb_result_e;

    typedef struct
    {
        ez_int32_t retry_max;           ///< 下载最大重试次数
        ez_int32_t file_num;            ///< 升级信息数量
        ez_int32_t interval;            ///< 上报升级进度间隔
        ez_ota_file_info_t *pota_files; ///< 升级信息
    } ez_ota_upgrade_info_t;

    typedef struct
    {
        ez_int8_t url[286];    ///< 升级包的url信息,带http/https前缀，270+前缀长度
        ez_int8_t degist[33];  ///< 升级包摘要
        ez_int32_t block_size; ///< 接收缓冲区大小\单次接收大小
        ez_int32_t total_size; ///< 升级包的总大小
        ez_int32_t timeout_s;  ///< 下载超时时间
        ez_int32_t retry_max;  ///< 下载最大重试次数
    } ez_ota_download_info_t;

    /** 
    * @brief 接收服务下发的升级消息回调,例：通知设备升级消息
    */
    typedef struct
    {
        ez_int8_t dev_serial[32]; ///< 子设备序列号
    } ez_ota_res_t;

    /** 
    * @brief 接收服务下发的升级消息回调,例：通知设备升级消息
    * @param pres  用于标识子设备,非空，长度为 0 表示主设备,长度非0 表示子设备
    * @param event 消息类型,参考ota_event_e
    * @param data  对应ota_event_e类型中的数据结构体
    * @param len  sizeof(struct)
    * return 处理出错返回对应的错误码
    */
    typedef struct
    {
        ez_int32_t (*ota_recv_msg)(ez_ota_res_t *pres, ez_ota_event_t event, ez_void_t *data, ez_int32_t len);
    } ez_ota_msg_cb_t;

    /**
    * \brief	ota初始化参数
    */
    typedef struct
    {
        ez_ota_msg_cb_t cb; ///< 回调函数
    } ez_ota_init_t;

    /**
     * @brief 升级包下载数据回调
     * @param total_len :package total size
     * @param offset    :offset of current download package
     * @param data &len :current download package data & len 
     * @param remain_len:the size left to process in next cb.
     * @param user_data :user input data
     * @return ez_int32_t 
     */
    typedef ez_int32_t (*get_file_cb)(ez_int32_t total_len, ez_int32_t offset, ez_void_t *data, ez_int32_t len, ez_void_t *user_data);
    /**
     * @brief 升级包下载过程通知回调
     * @param result  下载结果,0表示成功 非0表示失败
     * @param user_data: 用户自定义协议
     * @return ez_int32_t 
     */
    typedef ez_void_t (*notify_cb)(ez_ota_cb_result_e result, ez_void_t *user_data);

    /**
     * @brief 升级组件初始化
     * 
     * @param pdata_cbs 
     * @return ez_ota_err_t 
     */
    EZOS_API ez_err_t ez_iot_ota_init(ez_ota_init_t *pota_init);

    /**
     * @brief 上报升级模块信息,同步接口，请勿在SDK的任何消息回调里调用该接口
     * @param pres 设备信息，NULL标识当前设备，!NULL标识子设备
     * @param pmodules 设备模块信息列表,可以有多个模块，
     * @param  timeout_ms   指定超时时间及Qos
     * @return ez_ota_err_t 
     */
    EZOS_API ez_err_t ez_iot_ota_modules_report(const ez_ota_res_t *pres, const ez_ota_modules_t *pmodules, ez_int32_t timeout_ms);

    /**
     * @brief 通知升级服务，设备待升级，设备开机上线上报一次即可。
     * 
     * @param pres 设备信息，NULL标识当前设备，!NULL标识子设备
     * @return ez_ota_err_t 
     */
    EZOS_API ez_err_t ez_iot_ota_status_ready(const ez_ota_res_t *pres, ez_int8_t *pmodule);

    /**
     * @brief 升级成功信息上报
     * 
     * @param pres 设备信息，NULL标识当前设备，!NULL标识子设备
     * @return ez_ota_err_t 
     */
    EZOS_API ez_err_t ez_iot_ota_status_succ(const ez_ota_res_t *pres, ez_int8_t *pmodule);

    /**
     * @brief 升级失败信息上报
     * 
     * @param pres 设备信息，NULL标识当前设备，!NULL标识子设备
     * @param pmodule 固件模块名称
     * @param perr_msg 升级失败错误码
     * @param code 升级失败错误码
     * @return ez_ota_err_t 
     */
    EZOS_API ez_err_t ez_iot_ota_status_fail(const ez_ota_res_t *pres, ez_int8_t *pmodule, ez_int8_t *perr_msg, ez_ota_errcode_t code);

    /**
     * @brief 上报设备升级进度,请按照服务下发间隔时间上报,
     * @param pres 设备信息（设备序列号），NULL标识当前设备，!NULL标识子设备
     * @param pmodule 模块信息，NULL标识当前设备，!NULL标识子设备
     * @param status 设备升级状态
     * @param progress 升级进度 1-100
     * @return ez_ota_err_t 
     * 
     * note:app点击升级后,需要在10s时间内上报一次进度信息,否则app端可能会直接提示升级成功，正常可报stating
     * 
     */
    EZOS_API ez_err_t ez_iot_ota_progress_report(const ez_ota_res_t *pres, ez_int8_t *pmodule, ez_ota_status_t status, ez_int16_t progress);

    /**
     * @brief 下载升级包
     * @param input_info 升级信息
     * @param get_file   下载数据回调
     * @param notify      下载过程信息通知，包括超时,c
     * @return ez_ota_err_t 
     */
    EZOS_API ez_err_t ez_iot_ota_download(ez_ota_download_info_t *input_info, get_file_cb file_cb, notify_cb notify, ez_void_t *user_data);

    /**
     * @brief 
     * 
     * @return ez_ota_err_t 
     */
    EZOS_API ez_err_t ez_iot_ota_deinit();

#ifdef __cplusplus
}
#endif

#endif
