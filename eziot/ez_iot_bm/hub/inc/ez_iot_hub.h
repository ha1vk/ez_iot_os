/**
 * @file ez_iot_hub.h
 * @author xurongjun (xurongjun@ezvizlife.com)
 * @brief Hub类设备管理
 * @version 0.1
 * @date 2020-08-06
 * 
 * @copyright Copyright (c) 2020
 * 
 */

#ifndef _EZ_IOT_HUB_H_
#define _EZ_IOT_HUB_H_

#include <ezos.h>

#define HUB_MODULE_ERRNO_BASE 0x00030000

#ifdef __cplusplus
extern "C"
{
#endif

    typedef enum
    {
        EZ_HUB_ERR_SUCC = 0x00,                             ///< Success
        EZ_HUB_ERR_NOT_INIT = HUB_MODULE_ERRNO_BASE + 0x01, ///< The module is not initialized
    } ez_hub_err_e;

    typedef struct
    {
        int8_t auth_mode;            ///< 认证模式：0-SAP设备, 1-licence设备
        int8_t subdev_type[32 + 1];  ///< 子设备型号(licence设备为productKey)
        int8_t subdev_sn[16 + 1];    ///< 子设备序列号(licence设备为deviceName)
        int8_t subdev_vcode[32 + 1]; ///< 子设备验证码(对应licence认证中deviceLicense)
        int8_t subdev_ver[32 + 1];   ///< 子设备固件版本号
        int8_t subdev_uuid[16 + 1];  ///< 子设备局域部ID，用于直连或者mesh网络通讯，一般为mac地址
        int8_t sta;                  ///< 在线状态：0-不在线，1-在线
    } ez_subdev_info_t;

    typedef enum
    {
        EZ_EVENT_SUBDEV_ADD_SUCC, ///< 设备添加成功，msg data: subdev_sn
        EZ_EVENT_SUBDEV_ADD_FAIL, ///< 设备添加失败（认证不通过），msg data: \c ez_subdev_info_t
    } ez_subdev_event_e;

    typedef struct
    {
        /* 接收来自本地的事件 */
        int32_t (*recv_event)(ez_subdev_event_e event_type, void *data, int len);
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
    EZOS_API ez_err_t ez_iot_hub_add(const hub_subdev_info_t *subdev_info);

    /**
     * @brief 删除子设备
     * 
     * @param subdev_sn 子设备序列号
     * @return ez_hub_err_e 
     */
    EZOS_API ez_err_t ez_iot_hub_del(const int8_t *subdev_sn);

    /**
     * @brief 更新子设备版本号，常见于升级完成后
     * 
     * @param info 子设备信息
     * @return ez_hub_err_e 
     */
    EZOS_API ez_err_t ez_iot_hub_ver_update(const int8_t *subdev_sn, const int8_t *subdev_ver);

    /**
     * @brief 更新子设备联网状态
     * 
     * @param online false不在线，true在线
     * @return ez_hub_err_e 
     */
    EZOS_API ez_err_t ez_iot_hub_status_update(const int8_t *subdev_sn, bool online);

    /**
     * @brief 根据序列号查询子设备信息
     * 
     * @param subdev_sn 子设备序列号
     * @param subdev_info 子设备信息，不能为空
     * @return ez_hub_err_e 
     */
    EZOS_API ez_err_t ez_iot_hub_subdev_query(const int8_t *subdev_sn, hub_subdev_info_t *subdev_info);

    /**
     * @brief 枚举所有子设备信息
     * 
     * @param subdev_info 子设备信息，不能为空;subdev_sn未空串标识获取首个子设备
     * @return ez_hub_err_e 
     */
    EZOS_API ez_err_t ez_iot_hub_subdev_next(hub_subdev_info_t *subdev_info);

    /**
     * @brief 根据子设备序列号查询uuid
     * 
     * @param subdev_sn 子设备序列号
     * @param subdev_uuid 子设备uuid
     * @param buf_len 接收缓冲区大小
     * @return ez_hub_err_e 
     */
    EZOS_API ez_err_t ez_iot_hub_sn2uuid(const int8_t *subdev_sn, int8_t *subdev_uuid, int32_t buf_len);

    /**
     * @brief 根据子设备uuid查询序列号
     * 
     * @param subdev_uuid 子设备uuid
     * @param subdev_sn 子设备序列号
     * @param buf_len 接收缓冲区大小
     * @return ez_hub_err_e 
     */
    EZOS_API ez_err_t ez_iot_hub_uuid2sn(const int8_t *subdev_uuid, int8_t *subdev_sn, int32_t buf_len);

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