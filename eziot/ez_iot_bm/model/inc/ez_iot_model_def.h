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
#ifndef H_EZ_IOT_MODEL_DEF_H_
#define H_EZ_IOT_MODEL_DEF_H_

#include <ezos.h>

#define EZ_ERR_CODE_LEN        32
#define EZ_ERR_MSG_LEN         256
#define EZ_RES_ID_LEN          64
#define EZ_RES_TYPE_LEN        64
#define EZ_MSG_TYPE_LEN        64
#define EZ_COMMON_LEN          64
#define EZ_EXT_MSG_LEN         128
#define EZ_SUB_SERIAL_LEN      72
/**
 * \brief model接口错误码
 */
typedef enum {
    EZ_CODE_SUCESS                = 0,    ///< 成功
    EZ_CODE_FAILED                = 1001, ///< 通用错误
    EZ_CODE_NOT_INITED            = 1002, ///< 未初始化
    EZ_CODE_INVALID_PARAM         = 1003, ///< 参数无效
    EZ_CODE_CALLBACK_FUN_NULL     = 1004, ///< 回调函数未实现
    EZ_CODE_MODEL_REG_FAIL        = 1005, ///< model模块注册失败
    EZ_CODE_MODEL_DOMAIN_REGED    = 1006, ///< 领域已注册
    EZ_CODE_MODEL_DOMAIN_REG_ERR  = 1007, ///< 领域注册错误
    EZ_CODE_MODEL_MALLOC_ERR      = 1008, ///< malloc失败
	EZ_CODE_JSON_ERR              = 1009, ///< JSON 转换失败
    EZ_CODE_JSON_CREATE_ERR       = 1010, ///< JSON对象创建失败
	EZ_CODE_JSON_PRINT_ERR        = 1011, ///< JSON格式化错误
	EZ_CODE_KERNEL_SEND_ERR       = 1012, ///< 消息发送失败
    EZ_CODE_CB_REGED              = 1013, ///< 回调已注册
}EZ_ERR_CODE_E;

/**
 * \brief   ez_prime 通道消息类型
 * \warning 
 * \note    
 */
typedef  enum
{
    ez_event,     ///< 事件消息
    ez_attribute, ///< 属性消息
    ez_service    ///< 操作消息
} ez_model_type_t; 

/**
 * \brief   ez_prime 通用消息信息
 * \warning 
 * \note    
 */

typedef struct
{   
    ez_model_type_t type;                       ///< "event" "attribute" "service"
    char resource_id[EZ_RES_ID_LEN];          ///< 设备资源id
    char resource_type[EZ_RES_TYPE_LEN];      ///< 设备资源类型
    char domain[EZ_COMMON_LEN];               ///< domain
    char identifier[EZ_COMMON_LEN];           ///< identifier /业务标识
    char subserial[EZ_SUB_SERIAL_LEN];        ///< 子设备序列号
}ez_basic_info_t;  

/**
 * \brief   消息属性结构体
 * \warning 
 * \note      
 */
typedef struct
{
    unsigned int msg_seq;           ///< 消息的 seq
    int msg_qos;                    ///< 消息的 Qos类型 0 / 1
    char msg_type[EZ_MSG_TYPE_LEN]; ///< 消息类型 set/set_reply / operate/operate_reply / query/query_reply 等
}ez_msg_attr_t;

/**
 * \brief   消息json类型
 * \warning 内部根据传入的类型,将传入内容转成对应的json类型
 * \note      
 */
typedef  enum
{
    model_data_type_bool,
    model_data_type_int,
    model_data_type_double,
    model_data_type_string,
    model_data_type_array,
    model_data_type_object,
    model_data_type_null
} ez_json_type_t;   

/**
 * \brief  协议消息体
 * \note        
 */
typedef struct
{
    ez_json_type_t type;
    int length;
    union
    {
        ez_bool_t   value_bool;
        int    value_int;
        double value_double;
        void*  value;      /* 复杂类型的数据 */
    };
}ez_model_msg_t;

/**
 * \brief   消息处理状态结构体
 * \warning 
 * \note        
 */
typedef struct
{
    int   status;                      ///< 状态码
    char  err_code[EZ_ERR_CODE_LEN];   ///< 错误码 
    char  err_msg[EZ_ERR_MSG_LEN];     ///< 错误码描述信息     
}ez_err_info_t;


/**
 * \brief 领域注册结构体,向model模块注册领域信息,便于按领域进行路由消息
 * \warning 请勿在消息回调里等待SDK的其他消息
 * \note   
 */
typedef struct
{
    char domain[EZ_COMMON_LEN];           
    int (*das_req_router)(ez_basic_info_t* basic_info, char* msg_type, ez_model_msg_t* buf, unsigned int msg_seq);
    int (*das_reply_router)(ez_basic_info_t* basic_info, char* msg_type, ez_err_info_t* status, ez_model_msg_t* buf, unsigned int msg_seq);
} ez_domain_reg_t;

/** 
 *  \brief	    默认回调函数
 *  \method		ez_model_default_cb_t
 *  \param[out]  basic_info
 *  \param[out]  msg_type
 *  \param[out]  buf
 *  \param[out]  len
 *  \param[out]  msg_seq
 *  \return     void
 */
typedef struct
{
    void (*ez_model_default_cb_t)(ez_basic_info_t* basic_info, char* msg_type, void* buf, int len, unsigned int msg_seq);
}ez_model_default_cb_t;


#endif