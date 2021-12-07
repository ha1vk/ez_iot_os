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
 * XuRongjun (xurongjun@ezvizlife.com) - TSL(Thing Specification Language) user interface declaration 
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-11-15     xurongjun    first version 
 *******************************************************************************/

#ifndef _EZ_IOT_TSL_H_
#define _EZ_IOT_TSL_H_

#include <ezos.h>

#define TSL_MODULE_ERRNO_BASE 0x00040000

#ifdef __cplusplus
extern "C"
{
#endif

    typedef enum
    {
        EZ_TSL_ERR_SUCC = 0x00,                                      ///< Success
        EZ_TSL_ERR_NOT_INIT = TSL_MODULE_ERRNO_BASE + 0x01,          ///< The tsl module is not initialized
        EZ_TSL_ERR_NOT_READY = TSL_MODULE_ERRNO_BASE + 0x02,         ///< The sdk core module is not started
        EZ_TSL_ERR_PARAM_INVALID = TSL_MODULE_ERRNO_BASE + 0x03,     ///< The input parameters is illegal, it may be that some parameters can not be null or out of range
        EZ_TSL_ERR_GENERAL = TSL_MODULE_ERRNO_BASE + 0x04,           ///< Unknown error
        EZ_TSL_ERR_MEMORY = TSL_MODULE_ERRNO_BASE + 0x05,            ///< Out of memory
        EZ_TSL_ERR_DEV_NOT_FOUND = TSL_MODULE_ERRNO_BASE + 0x06,     ///< Can't find the device, need to register first
        EZ_TSL_ERR_RSCTYPE_NOT_FOUND = TSL_MODULE_ERRNO_BASE + 0x07, ///< The rsc_type is illegal, is not defined in the profile
        EZ_TSL_ERR_INDEX_NOT_FOUND = TSL_MODULE_ERRNO_BASE + 0x08,   ///< The local index is illegal, is not defined in the profile
        EZ_TSL_ERR_DOMAIN_NOT_FOUND = TSL_MODULE_ERRNO_BASE + 0x09,  ///< The domain is illegal, is not defined in the profile
        EZ_TSL_ERR_KEY_NOT_FOUND = TSL_MODULE_ERRNO_BASE + 0x0a,     ///< The Key is illegal, is not defined in the profile
        EZ_TSL_ERR_VALUE_TYPE = TSL_MODULE_ERRNO_BASE + 0x0b,        ///< The type of the value does not match the definition
        EZ_TSL_ERR_VALUE_ILLEGAL = TSL_MODULE_ERRNO_BASE + 0x0c,     ///< The value out of the defined range
        EZ_TSL_ERR_PROFILE_LOADING = TSL_MODULE_ERRNO_BASE + 0x0d,   ///< The device profile is loading or profile illegal
        EZ_TSL_ERR_STORAGE = TSL_MODULE_ERRNO_BASE + 0x0e,           ///< An error occurred when flash I/O
    } ez_tsl_err_e;

    typedef enum
    {
        EZ_TSL_DATA_TYPE_BOOL,
        EZ_TSL_DATA_TYPE_INT,
        EZ_TSL_DATA_TYPE_DOUBLE,
        EZ_TSL_DATA_TYPE_STRING,
        EZ_TSL_DATA_TYPE_ARRAY,
        EZ_TSL_DATA_TYPE_OBJECT,
        EZ_TSL_DATA_TYPE_NULL,

        EZ_TSL_DATA_TYPE_MAX,
    } ez_tsl_data_type_e;

    typedef struct
    {
        ez_char_t *domain; ///< 功能点所属于的领域。如key=switch，其在视频领域表示关闭摄像头，照明领域表示关闭灯光。
        ez_char_t *key;    ///< 功能点键值
    } ez_tsl_key_t;

    typedef struct
    {
        ez_tsl_data_type_e type;
        ez_int_t size;
        union
        {
            ez_bool_t value_bool;
            ez_int_t value_int;
            ez_int64_t value_double;
            ez_void_t *value; /* 复杂类型的数据 */
        };
    } ez_tsl_value_t;

    typedef struct
    {
        ez_char_t *key;
        ez_tsl_value_t value;
    } ez_tsl_param_t;

    typedef struct
    {
        ez_char_t *dev_subserial;       ///< 序列号, 必填, max 72字节
        ez_char_t *dev_type;            ///< 型号, 必填, max 64字节
        ez_char_t *dev_firmwareversion; ///< 版本号, 必填, max 64字节
    } ez_tsl_devinfo_t;

    typedef struct
    {
        ez_char_t *res_type;    ///< 通道类型（按键通道类型或者报警通道类型）
        ez_char_t *local_index; ///< 通道号
    } ez_tsl_rsc_t;

    typedef enum
    {
        EZ_EVENT_PROPERTY_FULL_REPORT, ///< 开始全量上报属性, data(NULL)
    } ez_tsl_event_e;

    typedef struct
    {

        ez_int32_t (*ez_tsl_notice)(ez_tsl_event_e event_type, ez_void_t *data, ez_int32_t len);

        /**
        * @brief 平台下发操作
        * 
        * @param sn 序列号
        * @param rsc_info 通道信息
        * @param key_info 操作功能点信息
        * @param value_in 操作入参
        * @param value_out 操作出参，所申请内存由内部释放
        * @return ez_int32_t 0表示成功，-1表示失败
        */
        ez_int32_t (*action2dev)(const ez_char_t *sn, const ez_tsl_rsc_t *rsc_info, const ez_tsl_key_t *key_info,
                                 const ez_tsl_value_t *value_in, ez_tsl_value_t *value_out);

        /**
        * @brief 获取属性向平台上报
        * 
        * @param sn 序列号
        * @param rsc_info 通道信息
        * @param key_info 操作功能点信息
        * @param value_out 属性数据
        * @return ez_int32_t 0表示成功，-1表示失败
        */
        ez_int32_t (*property2cloud)(const ez_char_t *sn, const ez_tsl_rsc_t *rsc_info, const ez_tsl_key_t *key_info, ez_tsl_value_t *value_out);

        /**
        * @brief 平台属性下发
        * 
        * @param sn 序列号
        * @param rsc_info 通道信息
        * @param key_info 操作功能点信息
        * @param value 属性数据
        * @return ez_int32_t 0表示成功，-1表示失败
        */
        ez_int32_t (*property2dev)(const ez_char_t *sn, const ez_tsl_rsc_t *rsc_info, const ez_tsl_key_t *key_info, const ez_tsl_value_t *value);

    } ez_tsl_callbacks_t;

    /**
    * @brief 模块初始化
    * 
    * @param pdata_cbs 
    * @return ez_tsl_err_e 
    */
    EZOS_API ez_err_t ez_iot_tsl_init(ez_tsl_callbacks_t *ptsl_cbs);

    /**
     * @brief 设备注册
     * 
     * @param pevinfo 设备信息; NULL默认当前设备，子设备不能为NULL
     * @param profile 物模型描述文件; NULL默认从网络自动下载
     * @return ez_tsl_err_e 
     */
    EZOS_API ez_err_t ez_iot_tsl_reg(ez_tsl_devinfo_t *pdevinfo, ez_char_t *profile);

    /**
    * @brief 向平台上报一条属性
    * 
    * @param sn 设备序列号
    * @param rsc_info 通道信息
    * @param key_info 操作功能点信息
    * @param value 属性数据
    * @return ez_tsl_err_e 
    */
    EZOS_API ez_err_t ez_iot_tsl_property_report(const ez_char_t *sn, const ez_tsl_rsc_t *rsc_info, const ez_tsl_key_t *key_info, const ez_tsl_value_t *value);

    /**
    * @brief 向平台上报一个事件
    * 
    * @param sn 设备序列号
    * @param rsc_info 通道信息
    * @param key_info 功能点信息
    * @param value 事件数据，一般为根据物模型组好的json报文
    * @return ez_tsl_err_e 
    */
    EZOS_API ez_err_t ez_iot_tsl_event_report(const ez_char_t *sn, const ez_tsl_rsc_t *rsc_info, const ez_tsl_key_t *key_info, const ez_tsl_value_t *value);

    /**
     * @brief 设备反注册
     * 
     * @param pevinfo 设备信息; NULL默认当前设备，子设备不能为NULL
     * @return ez_tsl_err_e 
     */
    EZOS_API ez_err_t ez_iot_tsl_unreg(ez_char_t *dev_subserial);

    /**
    * @brief tsl反初始化
    * 
    * @return ez_tsl_err_e 
    */
    EZOS_API ez_err_t ez_iot_tsl_deinit(ez_void_t);

#ifdef __cplusplus
}
#endif

#endif //_EZ_IOT_TSL_ENGINE_H_
