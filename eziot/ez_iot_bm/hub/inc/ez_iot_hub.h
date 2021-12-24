/**
 * @file ez_iot_hub.h
 * @author zhangdi29 (zhangdi29@ezvizlife.com)
 * @brief Hub类设备管理
 * @version 0.1
 * @date 2021-11-25
 * 
 * @copyright Copyright (c) 2020
 * 
 */

#ifndef _EZ_IOT_HUB_H_
#define _EZ_IOT_HUB_H_

#include <ezos.h>
#include "ez_iot_base.h"
#define HUB_MODULE_ERRNO_BASE 0x00030000

#ifdef __cplusplus
extern "C"
{
#endif

    typedef enum
    {
        EZ_HUB_ERR_SUCC = 0x00,                             ///< Success
        EZ_HUB_ERR_NOT_INIT = HUB_MODULE_ERRNO_BASE + 0x01, ///< The module is not initialized
        EZ_HUB_ERR_PARAM_INVALID      = HUB_MODULE_ERRNO_BASE + 0x02, ///< The input parameters is illegal, it may be that some parameters can not be null or out of range
        EZ_HUB_ERR_INTERNAL           = HUB_MODULE_ERRNO_BASE + 0x03, ///< Unknown error
        EZ_HUB_ERR_STORAGE            = HUB_MODULE_ERRNO_BASE + 0x04, ///< flash operation has failed.
        EZ_HUB_ERR_SUBDEV_NOT_FOUND   = HUB_MODULE_ERRNO_BASE + 0x05, ///< Can not found subdev
        EZ_HUB_ERR_ENUM_END           = HUB_MODULE_ERRNO_BASE + 0x06, ///< End of enumeration, no more data
        EZ_HUB_ERR_MEMORY             = HUB_MODULE_ERRNO_BASE + 0x07, ///< Out of memory
        EZ_HUB_ERR_OUT_OF_RANGE       = HUB_MODULE_ERRNO_BASE + 0x08, ///< Sub count out of range
        EZ_HUB_ERR_SUBDEV_EXISTED     = HUB_MODULE_ERRNO_BASE + 0x09, ///< The sub device already exists
    } ez_hub_err_e;

    typedef struct
    {
        ez_int8_t auth_mode;            ///< 认证模式：0-SAP设备, 1-licence设备
        ez_char_t subdev_type[32 + 1];  ///< 子设备型号(licence设备为productKey)
        ez_char_t subdev_sn[16 + 1];    ///< 子设备序列号(licence设备为deviceName)
        ez_char_t subdev_vcode[32 + 1]; ///< 子设备验证码(对应licence认证中deviceLicense)
        ez_char_t subdev_ver[32 + 1];   ///< 子设备固件版本号
        ez_char_t subdev_uuid[16 + 1];  ///< 子设备局域部ID，用于直连或者mesh网络通讯，一般为mac地址
        ez_int8_t sta;                  ///< 在线状态：0-不在线，1-在线
    } ez_subdev_info_t;

    typedef enum
    {
        EZ_EVENT_SUBDEV_ADD_SUCC, ///< 设备添加成功，msg data: subdev_sn
        EZ_EVENT_SUBDEV_ADD_FAIL, ///< 设备添加失败（认证不通过），msg data: \c ez_subdev_info_t
    } ez_subdev_event_e;

    typedef struct
    {
        /* 接收来自本地的事件 */
        ez_int32_t (*recv_event)(ez_subdev_event_e event_type, ez_void_t *data, ez_int32_t len);
    } ez_hub_callbacks_t;

    /**
     * @brief Hub模块初始化函数
     * 
     * @param info 
     * @return ez_hub_err_e 
     */
    EZOS_API ez_err_t ez_iot_hub_init(ez_hub_callbacks_t *phub_cbs);

    /**
     * @brief 添加子设备
     * 
     * @param info 子设备信息
     * @returnez_hub_err_e 
     */
    EZOS_API ez_err_t ez_iot_hub_add(const ez_subdev_info_t *subdev_info);

    /**
     * @brief 删除子设备
     * 
     * @param subdev_sn 子设备序列号
     * @return ez_hub_err_e 
     */
    EZOS_API ez_err_t ez_iot_hub_del(const ez_char_t *subdev_sn);

    /**
     * @brief 更新子设备版本号，常见于升级完成后
     * 
     * @param info 子设备信息
     * @return ez_hub_err_e 
     */
    EZOS_API ez_err_t ez_iot_hub_ver_update(const ez_char_t *subdev_sn, const ez_char_t *subdev_ver);

    /**
     * @brief 更新子设备联网状态
     * 
     * @param online false不在线，true在线
     * @return ez_hub_err_e 
     */
    EZOS_API ez_err_t ez_iot_hub_status_update(const ez_char_t *subdev_sn, ez_bool_t online);

    /**
     * @brief 根据序列号查询子设备信息
     * 
     * @param subdev_sn 子设备序列号
     * @param subdev_info 子设备信息，不能为空
     * @return ez_hub_err_e 
     */
    EZOS_API ez_err_t ez_iot_hub_subdev_query(const ez_char_t *subdev_sn, ez_subdev_info_t *subdev_info);

    /**
     * @brief 枚举所有子设备信息
     * 
     * @param subdev_info 子设备信息，不能为空;subdev_sn未空串标识获取首个子设备
     * @return ez_hub_err_e 
     */
    EZOS_API ez_err_t ez_iot_hub_subdev_next(ez_subdev_info_t *subdev_info);

    /**
     * @brief 根据子设备序列号查询uuid
     * 
     * @param subdev_sn 子设备序列号
     * @param subdev_uuid 子设备uuid
     * @param buf_len 接收缓冲区大小
     * @return ez_hub_err_e 
     */
    EZOS_API ez_err_t ez_iot_hub_sn2uuid(const ez_char_t *subdev_sn, ez_char_t *subdev_uuid, ez_int32_t buf_len);

    /**
     * @brief 根据子设备uuid查询序列号
     * 
     * @param subdev_uuid 子设备uuid
     * @param subdev_sn 子设备序列号
     * @param buf_len 接收缓冲区大小
     * @return ez_hub_err_e 
     */
    EZOS_API ez_err_t ez_iot_hub_uuid2sn(const ez_char_t *subdev_uuid, ez_char_t *subdev_sn, ez_int32_t buf_len);

    /**
     * @brief 清空所有子设备，常见于网关重置
     * 
     * @return ez_hub_err_e 
     */
    EZOS_API ez_err_t ez_iot_hub_clean(void);

    /**
     * @brief 反初始化
     * 
     * @return ez_hub_err_e 
     */
    EZOS_API ez_err_t ez_iot_hub_deinit(void);

#ifdef __cplusplus
}
#endif

#endif