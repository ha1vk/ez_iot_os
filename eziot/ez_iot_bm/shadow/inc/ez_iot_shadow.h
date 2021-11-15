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

#ifndef _EZ_IOT_SHADOW_H_
#define _EZ_IOT_SHADOW_H_

#include <ezos.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct
    {
        const ez_void_t *pres;    ///< 目标资源(shadow V3)，状态给到哪个设备的哪个通道（无子设备和单通道忽略）。参见ezDevSDK_Shadow_Res_t
        const ez_int8_t *pdomain; ///< 领域id(shadow V3)
        const ez_int8_t *pkey;    ///< 属性id(shadow V3)
    } ez_shadow_business2dev_param_t;

    typedef struct
    {
        ez_uint32_t ver;          ///< 版本号，用于shadow的同步
        const ez_void_t *pres;    ///< 目标资源(shadow V3)，从哪个设备的哪个通道获取状态（无子设备和单通道忽略）。参见ezDevSDK_Shadow_Res_t
        const ez_int8_t *pdomain; ///< 领域id(shadow V3)
        const ez_int8_t *pkey;    ///< 属性id(shadow V3)
    } ez_shadow_business2cloud_param_t;

    typedef enum
    {
        SHD_DATA_TYPE_BOOL,
        SHD_DATA_TYPE_INT,
        SHD_DATA_TYPE_DOUBLE,
        SHD_DATA_TYPE_STRING,
        SHD_DATA_TYPE_ARRAY,
        SHD_DATA_TYPE_OBJECT,

        EZ_SHD_DATA_TYPE_MAX,
    } ez_shadow_data_type_e;

    typedef struct
    {
        ez_shadow_data_type_e type;

        ez_uint32_t length; ///< 状态值长度
        union
        {
            ez_bool_t value_bool;
            ez_int32_t value_int;
            ez_uint64_t value_double;
            ez_void_t *value; /* 复杂类型的数据 */
        };
    } ez_shadow_value_t;

    typedef struct
    {
        ez_int8_t key[64];     ///< shadow业务的key
        ez_uint32_t ver;       ///< shadow业务的版本号
        ez_uint32_t seq;       ///< 状态发送后的seq
        ez_int8_t need_report; ///< 是否需要上报标志

        /**
        * @brief 平台的shadow业务同步给设备
        * @param[in] pjson_value 业务报文
        * @param[in] ppram 保留入参
        * @return ez_uint32_t  成功0，失败-1 
        */
        ez_uint32_t (*business2dev)(const ez_shadow_value_t *pvalue, ez_shadow_business2dev_param_t *ppram);

        /**
        * @brief 设备的shadow业务同步给平台
        * @param[out] ppjson_value 组织上报的的业务报文
        * @param[in] ppram 组织报文的入参
        * @return ez_uint32_t  成功0，失败-1 
        */
        ez_uint32_t (*business2cloud)(ez_shadow_value_t *pvalue, ez_shadow_business2cloud_param_t *ppram);

    } ez_shadow_business_t;

    typedef struct
    {
        ez_int8_t reset;                ///< 设备重置，清空所有缓存状态
        ez_int16_t num;                 ///< 注册业务的数量
        ez_shadow_business_t *business; ///< 注册的业务
    } ez_shadow_module_t;

    /**
    * @brief 描述某个资源。设备的某种资源类型集合的某个通道
    * 
    */
    typedef struct
    {
        ez_int8_t dev_serial[72]; ///< 设备序列号
        ez_int8_t res_type[64];   ///< 通道类型（某种类型通道的集合，视频通道集合或者报警通道集合）
        ez_int16_t local_index;   ///< 通道号（例如报警通道0或者报警通道1）
    } ez_shadow_res_t;

    /**
    * @brief 初始化shadow模块
    * 
    * @return 
    */
    EZOS_API ez_err_t ez_iot_shadow_init(ez_void_t);

    /**
    * @brief 向shadow注册一个领域模块
    * 
    * @param pres 资源（设备/子设备/通道）
    * @param domain_id 领域以及对应的key
    * @param module 领域key列表
    * @return  
    */
    EZOS_API ez_err_t ez_iot_shadow_register(ez_shadow_res_t *pres, ez_int8_t *domain_id, ez_shadow_module_t *module);

    /**
    * @brief key值发生更变，触发上报
    * 
    * @param pres 资源（设备/子设备/通道）
    * @param domain_id 领域id
    * @param pkey 变更的key值
    * @param pvalue 变更的value
    * @return  
    */
    EZOS_API ez_err_t ez_iot_shadow_push(ez_shadow_res_t *pres, ez_int8_t *domain_id, ez_int8_t *pkey, ez_shadow_value_t *pvalue);

    /**
    * @brief 反初始化shadow模块
    * 
    * @return EZOS_API ez_err_t  
    */
    EZOS_API ez_void_t ez_iot_shadow_deini(ez_void_t);

#ifdef __cplusplus
}
#endif

#endif