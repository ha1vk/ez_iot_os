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
 * Brief:
 * 
 * 
 * Change Logs:
 * Date           Author       Notes
 * 2021-11-25    zhangdi29     
 *******************************************************************************/
#ifndef _HUB_FUNC_H_
#define _HUB_FUNC_H_

#include "ez_iot_hub.h"
    
#define S2J_JSON_SET_int_ELEMENT(to_json, from_struct, _element) \
    cJSON_AddNumberToObject(to_json, #_element, (from_struct)->_element);

#define S2J_JSON_SET_string_ELEMENT(to_json, from_struct, _element) \
    cJSON_AddStringToObject(to_json, #_element, (from_struct)->_element);


#define S2J_JSON_SET_BASIC_ELEMENT(to_json, from_struct, type, _element) \
    S2J_JSON_SET_##type##_ELEMENT(to_json, from_struct, _element)

/* Set basic type element for JSON object */
#define s2j_json_set_basic_element(to_json, from_struct, type, element) \
    S2J_JSON_SET_BASIC_ELEMENT(to_json, from_struct, type, element)

typedef struct
{
    EZ_INT8 authm;         ///< 认证模式：0-SAP设备, 1-licence设备
    EZ_INT8 type[32 + 1];  ///< 子设备型号(licence设备为productKey)
    EZ_INT8 sn[16 + 1];    ///< 子设备序列号(licence设备为deviceName)
    EZ_INT8 vcode[32 + 1]; ///< 子设备验证码(对应licence认证中deviceLicense)
    EZ_INT8 ver[32 + 1];   ///< 子设备固件版本号
    EZ_INT8 uuid[16 + 1];  ///< 子设备局域网唯一标识
    EZ_INT8 sta;           ///< 在线状态
    EZ_INT8 access;        ///< 认证状态：0-未认证，1-已认证
} hub_subdev_info_internal_t;

/**
 * @brief 初始化Hub子设备管理模块
 * 
 * @param phub_cbs 子设备添加回调
 * @return EZ_INT ez_errno_succ、ez_errno_hub_internal
 */
EZ_INT hub_func_init(const ez_hub_callbacks_t *phub_cbs);

/**
 * @brief 反初始化Hub子设备管理模块
 * 
 */
void hub_func_deinit();

/**
 * @brief 新增一个子设备
 * 
 * @param subdev_info 子设备信息
 * @return ez_err_t ez_errno_succ、ez_errno_hub_storage、ez_errno_hub_memory、ez_errno_hub_subdev_existed、ez_errno_hub_out_of_range
 */
ez_err_t hub_add_do(const hub_subdev_info_internal_t *subdev_info);

/**
 * @brief 删除一个子设备
 * 
 * @param subdev_sn 子设备序列号
 * @return ez_err_t ez_errno_succ、ez_errno_hub_storage、ez_errno_hub_memory、ez_errno_hub_subdev_not_found
 */
ez_err_t hub_del_do(const EZ_INT8 *subdev_sn);

/**
 * @brief 更新子设备版本号
 * 
 * @param subdev_sn 子设备序列号
 * @param subdev_ver 新的版本号
 * @return ez_err_t ez_errno_succ、ez_errno_hub_storage、ez_errno_hub_memory、ez_errno_hub_subdev_not_found
 */
ez_err_t hub_ver_update_do(const EZ_INT8 *subdev_sn, const EZ_INT8 *subdev_ver);

/**
 * @brief 更新子设备状态
 * 
 * @param subdev_sn 子设备序列号
 * @param online 子设备在线状态
 * @return ez_err_t ez_errno_succ、ez_errno_hub_storage、ez_errno_hub_memory、ez_errno_hub_subdev_not_found
 */
ez_err_t hub_status_update_do(const EZ_INT8 *subdev_sn, EZ_BOOL online);

/**
 * @brief 查询子设备信息
 * 
 * @param subdev_sn 子设备序列号
 * @param subdev_info 子设备信息
 * @return ez_err_t ez_errno_succ、ez_errno_hub_storage、ez_errno_hub_memory、ez_errno_hub_subdev_not_found
 */
ez_err_t hub_subdev_query(const EZ_INT8 *subdev_sn, hub_subdev_info_internal_t *subdev_info);

/**
 * @brief 枚举子设备
 * 
 * @param subdev_info 子设备信息
 * @return ez_err_t ez_errno_succ、ez_errno_hub_storage、ez_errno_hub_memory、ez_errno_hub_subdev_not_found、ez_errno_hub_enum_end
 */
ez_err_t hub_subdev_next(hub_subdev_info_internal_t *subdev_info);

/**
 * @brief 清空子设备
 * 
 * @return ez_err_t ez_errno_succ、ez_errno_hub_storage
 */
ez_err_t hub_clean_do(void);

/**
 * @brief 上报子设备关联关系
 * 
 * @return EZ_INT 
 */
EZ_INT hub_subdev_list_report(void);

/**
 * @brief 更新子设备在线状态
 * 
 * @return EZ_INT 
 */
EZ_INT hub_subdev_sta_report(void);

/**
 * @brief 认证子设备
 * 
 * @param subdev_info 需要认证的子设备信息
 * @return EZ_INT 信令发送结果
 */
EZ_INT hub_subdev_auth_do(void *subdev_info);

/**
 * @brief 子设备认证结果
 * 
 * @param buf 协议报文
 * @param len 协议报文长度
 */
void hub_subdev_auth_done(void *buf, EZ_INT len);

/**
 * @brief 注册物模型，模块初始化时调用
 * 
 * @return ez_err_t 
 */
ez_err_t hub_tsl_reg_all(void);

/**
 * @brief 注销物模型，模块反初始化时调用
 * 
 * @return ez_err_t 
 */
ez_err_t hub_tsl_unreg_all(void);

#endif