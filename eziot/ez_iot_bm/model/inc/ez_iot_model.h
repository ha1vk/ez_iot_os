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
* Contributors:
 *    shenhongyin - initial API and implementation and/or initial documentation
 *******************************************************************************/
#ifndef EZ_IOT_MODEL_H_
#define EZ_IOT_MODEL_H_

#include "ez_iot_model_def.h"
#include <ezos.h>

#ifdef __cplusplus
extern "C"
{
#endif

/** 
 *  \brief	向model模块注册领域,用于das消息的路由
 *  \method		ez_iot_model_domain_reg
 *  \param[out] 
 *  \return  当前封装版本号，非空
 */
    EZOS_API int ez_iot_model_domain_reg(const ez_domain_reg_t *domain_reg);

/** 
 *  \brief	   领域反注册
 *  \method	   ez_iot_model_domain_dereg
 *  \param[in] domain ,注册的domian标识
 *  \return  当前封装版本号，非空
 */
    EZOS_API int ez_iot_model_domain_dereg(const char *domain);

/** 
 *  \brief		设备回复das下发的请求
 *  \method		ez_iot_model_reply_to_das
 *  \param[in]  basic_info 通用消息参数
 *  \param[in]  msg 用户定义响应消息协议内容, json类型
 *  \param[in]  status 消息处理结果
 *  \param[in]  msg_attr 发送消息属性
 *  \return  0成功 非0 失败
 */
    EZOS_API int ez_iot_model_reply_to_das(ez_basic_info_t *basic_info, ez_model_msg_t *msg, ez_err_info_t *status, ez_msg_attr_t *msg_attr);

/** 
 *  \brief	发送消息接口,可以通过该接口实现事件/属性的上报/操作查询等
 *  \method		ez_iot_model_send_msg
 *  \param[in]  basic_info 通用消息参数
 *  \param[in]  msg 用户定义协议内容, json类型
 *  \param[in]  msg_attr     发送消息属性
 *  \return  0成功 非0 失败
 */
    EZOS_API int ez_iot_model_send_msg(ez_basic_info_t *basic_info, ez_model_msg_t *msg, ez_msg_attr_t *msg_attr);

/** 
 *  \brief	发送消息给das,kuzhong
 *  \method		ez_iot_model_send_to_platform
 *  \param[in]  basic_info 通用消息参数
 *  \param[in]  msg 具体协议内容,必须是可解析为json的字符串
 *  \param[in]  msg_len 消息长度
 *  \param[in]  msg_response 请求或者响应消息标识 请求消息填 0  响应消息填 1
 *  \param[in]  msg_attr     发送消息属性
 *  \return  0成功 非0 失败
 */
    EZOS_API int ez_iot_model_send_to_platform(ez_basic_info_t *basic_info, const char *msg, unsigned int msg_len, int msg_response, ez_msg_attr_t *msg_attr);
    /** 
 *  \brief	获取当前封装版本号
 *  \method		ez_iot_model_get_current_version
 *  \param[out] 
 *  \return  当前封装版本号,未初始化时返回NULL
 */
    EZOS_API const char * ez_iot_model_get_current_version();

/** 
 *  \brief	设置默认回调函数,用于未注册的领域消息的接收
 *  \method		ez_iot_model_reg_default_cb
 *  \param[in]  ez_data_router消息回调
 *  \return  0成功 非0 失败
 */
    EZOS_API int ez_iot_model_reg_default_cb(ez_model_default_cb_t *ez_data_router);

/** 
 *  \brief	    清除默认回调信息及模块反初始化
 *  \method		ez_iot_model_dereg_default_cb
 *  \param[in]  ez_data_router
 *  \return  0成功 
 */
    EZOS_API int ez_iot_model_dereg_default_cb(ez_model_default_cb_t *ez_data_router);

#ifdef __cplusplus
}
#endif

#endif //