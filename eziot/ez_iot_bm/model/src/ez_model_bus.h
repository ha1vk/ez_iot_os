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
#ifndef H_EZ_MODEL_BUS_H_
#define H_EZ_MODEL_BUS_H_

#include "ez_model_comm.h"
#include "ez_iot_model_def.h"
#include "ez_model_extern.h"

#define EZ_MODEL_RSP 1
#define EZ_MODEL_REQ 0

/**
 * \brief   消息上报参数
 * \warning 
 * \note      
 */
typedef struct
{
    char method[EZ_METHOD_LEN];               ///< "event" / "attribute" / "service"
    char msg_type[EZ_MSG_TYPE_LEN];           ///< "report" / "query" / "set_reply" / "operate_reply"                              
    char resource_id[EZ_RES_ID_LEN];          ///< 设备资源id
    char resource_type[EZ_RES_TYPE_LEN];      ///< 设备资源类型
    char ext_msg[EZ_EXT_MSG_LEN];             ///< 扩展信息
    char sub_serial[EZ_SUB_SERIAL_LEN];       ///< 子设备序列号
} ez_model_info_t;

#ifdef __cplusplus
extern "C"
{
#endif

int ez_model_msg2platform(unsigned char* context, unsigned int context_len, ez_model_info_t* param, unsigned int* msg_seq,int msg_response, int msg_qos);

#ifdef __cplusplus
}
#endif

#endif//
