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
 * chentengfei (chentengfei5@ezvizlife.com)
  *******************************************************************************/

/*此文件解析物模型传上来的协议，分action，属性，属性上报，此文件针对单芯片设备设计为统一格式文件.
物模型的功能点在此处以action_$(KEY)_set，property_$(KEY)_set,property_$(key)_get,event_$(KEY)_up命名

需要寻找到具体的功能点透出去

*/
#include <string.h>
#include "ez_iot_tsl.h"

#define DOMAIN_LEN_MAX 32

#define IDENTIFIER_LEN_MAX 32

#define KEY_LEN_MAX 384

typedef struct
{
    char *identify;       //功能点
    char *domain;           //领域标识
    char *res_type;      //资源标识 resourceCategory                        
    char *index;        //资源的通道
    int (*func_set)(ez_tsl_value_t *value_out);
    int (*func_up)(ez_tsl_value_t *value_return);
} property_cmd_t;

typedef struct
{
    char *identify;
    int (*func_set)(ez_tsl_value_t *data, ez_tsl_value_t *retrun_value);
} action_cmd_t;


int user_property_report(char *key);


ez_int32_t tsl_notice(ez_tsl_event_e event_type, ez_void_t *data, ez_int32_t len);

/**
 * @brief 物模型操作回调函数
 * @param[in] sn :sdk 回调出来资源通道信息，灯类可以不处理。
 * @param[in] rsc_info :sdk 回调出来资源通道信息，灯类可以不处理。
 * @param[in] key_info :sdk 回调出来领域功能点信息，必须处理。
 * @param[in] value_in :sdk 回调出来功能点具体值信息，必须处理
 * @param[out] value_out :函数处理后返回给sdk 的值，可以不处理。无值返回时，sdk action 底层会补充一个0或者-1 code 给平台
 * @return :成功SUCCESS/失败返回:ERROR
 * @note: 
 */
ez_int32_t tsl_things_action2dev(const ez_int8_t *sn, const ez_tsl_rsc_t *rsc_info, const ez_tsl_key_t *key_info,

                                 const ez_tsl_value_t *value_in, ez_tsl_value_t *value_out);
/**
 * @brief 物模型属性同步回调函数
 * @param[in] sn :sdk 回调出来资源通道信息，灯类可以不处理。
 * @param[in] rsc_info :sdk 回调出来资源通道信息，灯类可以不处理。
 * @param[in] key_info :sdk 回调出来领域功能点信息，必须处理。
 * @param[in] value_in :sdk 回调出来功能点具体值信息，必须处理
 * @param[out] value_out :函数处理后的属性值返回给sdk，必须处理
 * @return :成功SUCCESS/失败返回:ERROR
 * @note: SDK 24小时会强制执行同步设备的属性给平台，会遍历所有功能点回调此函数
 */
ez_int32_t tsl_things_property2cloud(const ez_int8_t *sn, const ez_tsl_rsc_t *rsc_info, const ez_tsl_key_t *key_info, ez_tsl_value_t *value_out);

/**
 * @brief 物模型属性同步回调函数
 * @param[in] sn :sdk 回调出来资源通道信息，灯类可以不处理。
 * @param[in] rsc_info :sdk 回调出来资源通道信息，灯类可以不处理。
 * @param[in] key_info :sdk 回调出来领域功能点信息，必须处理。
 * @param[in] value_out :sdk 回调出来领域功能点具体值信息，必须处理
 * @return :成功SUCCESS/失败返回:ERROR
 * @note: SDK 24小时会强制执行同步设备的属性给平台，会遍历所有功能点回调此函数
 */
ez_int32_t tsl_things_property2dev(const ez_int8_t *sn, const ez_tsl_rsc_t *rsc_info, const ez_tsl_key_t *key_info, ez_tsl_value_t *value_out);
