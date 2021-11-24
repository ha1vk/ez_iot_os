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
 * 2021-11-04     xurongjun    first version 
 *******************************************************************************/

#ifndef _EZ_IOT_DEF_H_
#define _EZ_IOT_DEF_H_

#include <ezos.h>

#define BASE_ERROR 0x00000000
#define NET_ERROR 0x00000100
#define LBS_ERROR 0x00010000
#define SECRETKEY_ERROR 0x00010100
#define DAS_ERROR 0x00020000

#define version_max_len 32              ///< 版本长度
#define ezdev_sdk_extend_name_len 32    ///< 扩展模块名字长度
#define ezdev_sdk_ip_max_len 64         ///< ip最长长度
#define ezdev_sdk_timezone_max_len 32   ///< timezone最长长度
#define ezdev_sdk_sessionkey_len 16     ///< 设备会话秘钥长度，默认是16个字节
#define ezdev_sdk_devid_len 32          ///< 设备唯一标识长度  默认是32字节的字符串
#define ezdev_sdk_masterkey_len 16      ///< 设备mastekey长度 ,默认是16个字节
#define ezdev_sdk_verify_code_maxlen 48 ///< 设备验证码支持最大长度(对应licence认证中product_key)
#define ezdev_sdk_domain_id_len 64
#define ezdev_sdk_resource_id_len 64
#define ezdev_sdk_resource_type_len 64
#define ezdev_sdk_identifier_len 64
#define ezdev_sdk_max_serial_len 72
#define ezdev_sdk_msg_type_len 64
#define ezdev_sdk_module_name_len 16
#define ezdev_sdk_ext_msg_len 128
#define ezdev_sdk_method_len 128
#define ezdev_sdk_name_len 64
#define ezdev_sdk_business_type_len 64

typedef enum
{
    QOS_T0, ///< 最多一次送到
    QOS_T1, ///< 至少一次送到
    QOS_T2  ///< 准确一次送达
} ez_kernel_qos_e;

typedef struct
{
    ez_uint16_t das_udp_port;
    ez_char_t das_ip[ezdev_sdk_ip_max_len];
    ez_char_t lbs_ip[ezdev_sdk_ip_max_len];
    ez_uchar_t session_key[ezdev_sdk_sessionkey_len];
    ez_uchar_t lbs_domain[ezdev_sdk_ip_max_len];
} ez_kernel_switchover_context_t;

typedef struct
{
    ez_uint16_t das_port;                             ///< das服务器的TCP端口号
    ez_uint16_t das_udp_port;                         ///< das服务器UDP端口号
    ez_char_t das_ip[ezdev_sdk_ip_max_len];           ///< das服务器IP
    ez_char_t lbs_ip[ezdev_sdk_ip_max_len];           ///< lbs服务器IP
    ez_uchar_t session_key[ezdev_sdk_sessionkey_len]; ///< 会话秘钥
    ez_int32_t das_socket;                            ///< das的socket
    ez_char_t das_domain[ezdev_sdk_ip_max_len];       ///< das的域名
    ez_char_t das_serverid[ezdev_sdk_name_len];       ///< das的serverid
} ez_kernel_sessionkey_context_t;

typedef struct
{
    ez_uint32_t last_error;                 ///< 错误码
    ez_char_t das_ip[ezdev_sdk_ip_max_len]; ///< das服务器IP
    ez_char_t lbs_ip[ezdev_sdk_ip_max_len]; ///< lbs服务器IP
} ez_kernel_offline_context_t;

typedef struct
{
    ez_err_t last_error;                              ///< 错误码
    ez_uint32_t msg_seq;                              ///< 消息
    ez_char_t module_name[ezdev_sdk_module_name_len]; ///< 模块标识
} ez_kernel_publish_ack_t;

typedef enum
{
    SDK_KERNEL_EVENT_ONLINE,      ///< event_context(ez_kernel_sessionkey_context_t)    设备上线
    SDK_KERNEL_EVENT_BREAK,       ///< event_context(ez_kernel_offline_context_t)       设备离线
    SDK_KERNEL_EVENT_SWITCHOVER,  ///< event_context(ez_kernel_switchover_context_t)    平台地址发送切换
    SDK_KERNEL_EVENT_RECONNECT,   ///< event_context(NULL)                              重连成功事件回调
    SDK_KERNEL_EVENT_PUBLISH_ACK, ///< event_context(ez_kernel_publish_ack_t)            上行消息响应
} ez_kernel_event_e;

typedef struct
{
    ez_kernel_event_e event_type;
    ez_void_t *event_context;
} ez_kernel_event_t;

typedef void (*sdk_kernel_event_notice)(ez_kernel_event_t *ptr_event);

typedef struct
{
    ez_uint32_t msg_seq;                    ///< 消息seq值
    ez_void_t *buf;                         ///< 消息缓冲区
    ez_uint32_t buf_len;                    ///< 消息长度
    ez_int32_t msg_domain_id;               ///< 扩展模块ID（领域ID）
    ez_int32_t msg_command_id;              ///< 指令ID
    ez_char_t command_ver[version_max_len]; ///< 指令版本
} ez_kernel_submsg_t;

typedef struct
{
    ez_uint16_t domain_id;                                                                             ///< domain_id 对应到业务领域id
    ez_void_t (*ezdev_sdk_kernel_extend_start)(ez_void_t *pUser);                                      ///< 微内核启动
    ez_void_t (*ezdev_sdk_kernel_extend_stop)(ez_void_t *pUser);                                       ///< 微内核停止
    ez_void_t (*ezdev_sdk_kernel_extend_data_route)(ez_kernel_submsg_t *ptr_submsg, ez_void_t *pUser); ///< 数据路由（领域回调函数）
    ez_void_t (*ezdev_sdk_kernel_extend_event)(ez_kernel_event_t *ptr_event, ez_void_t *pUser);        ///< 事件回调（启动、停止、上线、下线等事件）
    ez_void_t **pUser;                                                                                 ///< 用户指针（一般为NULL 也可以用于携带用户数据指针）
    ez_char_t extend_module_name[ezdev_sdk_extend_name_len];                                           ///< 模块名字
    ez_char_t extend_module_version[version_max_len];                                                  ///< 模块版本号
} ez_kernel_extend_t;

typedef struct
{
    ez_uint8_t msg_response;                ///< 0:非响应消息 1:响应消息
    ez_kernel_qos_e msg_qos;                ///< 消息QOS类型
    ez_uint32_t msg_seq;                    ///< 消息seq值
    ez_char_t *msg_body;                    ///< 消息缓冲区
    ez_uint32_t msg_body_len;               ///< 消息长度，最大不能超过16K
    ez_uint32_t msg_domain_id;              ///< 领域模块ID
    ez_uint32_t msg_command_id;             ///< 指令ID
    ez_void_t *externel_ctx;                ///< 外部自定义数据，在消息发送回执中带回
    ez_uint32_t externel_ctx_len;           ///< 外部自定义数据长度
    ez_char_t command_ver[version_max_len]; ///< 指令版本
} ez_kernel_pubmsg_t;

typedef struct
{
    ez_void_t *buf;                                       ///< 消息缓冲区
    ez_uint32_t msg_seq;                                  ///< 消息seq值
    ez_uint32_t buf_len;                                  ///< 消息长度
    ez_char_t resource_id[ezdev_sdk_resource_id_len];     ///< 设备资源id
    ez_char_t resource_type[ezdev_sdk_resource_type_len]; ///< 设备资源类型
    ez_char_t module[ezdev_sdk_module_name_len];          ///< 用户和萤石云服务约定的模块标识,例如 "model"  "ota" "basic" "storage"等,
    ez_char_t method[ezdev_sdk_method_len];               ///< 通道下的方法类型,例如 "event"  "attribute" "service" "inform"  "upload/result"等,
    ez_char_t msg_type[ezdev_sdk_msg_type_len];           ///< 消息类型"report" / "query" / "set_reply" / "operate_reply"等
    ez_char_t sub_serial[ezdev_sdk_max_serial_len];       ///< 子设备序列号
    ez_char_t ext_msg[ezdev_sdk_ext_msg_len];             ///< 扩展字段,"model"模块中的 "domainid/identifier"内容
} ez_kernel_submsg_v3_t;

typedef struct
{
    ez_char_t module[ezdev_sdk_module_name_len];                          ///< 用户和萤石云约定的模块标识,例如"model" "ota" "ota" "storage"等
    ez_void_t (*ez_kernel_data_route)(ez_kernel_submsg_v3_t *ptr_submsg); ///< 数据路由（按照用户注册的model_type路由）
    ez_void_t (*ez_kernel_event_route)(ez_kernel_event_t *ptr_event);
} ez_kernel_extend_v3_t;

typedef struct
{
    ez_uint8_t msg_response;                              ///< 0:非响应消息 1:响应消息
    ez_kernel_qos_e msg_qos;                              ///< 消息QOS类型
    ez_uint32_t msg_seq;                                  ///< 消息seq值
    ez_uint32_t msg_body_len;                             ///< 消息长度
    ez_char_t *msg_body;                                  ///< 消息缓冲区
    ez_char_t resource_id[ezdev_sdk_resource_id_len];     ///< 设备资源id
    ez_char_t resource_type[ezdev_sdk_resource_type_len]; ///< 设备资源类型
    ez_char_t module[ezdev_sdk_module_name_len];          ///< 用户和萤石云服务约定的模块标识,例如 "model"  "ota" "basic" "storage"等,
    ez_char_t method[ezdev_sdk_method_len];               ///< 通道下的方法类型,例如 "event"  "attribute" "service" "inform"  "upload/result/"等,
    ez_char_t msg_type[ezdev_sdk_msg_type_len];           ///< 消息类型"report" / "query" / "set_reply" / "operate_reply"等
    ez_char_t sub_serial[ezdev_sdk_max_serial_len];       ///< 子设备序列号
    ez_char_t ext_msg[ezdev_sdk_ext_msg_len];             ///< 扩展内容，例如"model"中的 "domainid/identifier"字段
} ez_kernel_pubmsg_v3_t;

typedef enum ez_err
{
    EZ_CORE_ERR_SUCC = 0x00,          ///< Success
    EZ_CORE_ERR_NOT_INIT = 0x01,      ///< The sdk core module is not initialized
    EZ_CORE_ERR_NOT_READY = 0x02,     ///< The sdk core module is not started
    EZ_CORE_ERR_PARAM_INVALID = 0x03, ///< The input parameters is illegal, it may be that some parameters can not be null or out of range
    EZ_CORE_ERR_GENERAL = 0x04,       ///< Unknown error
    EZ_CORE_ERR_MEMORY = 0x05,        ///< Out of memory
    EZ_CORE_ERR_STORAGE = 0x06,       ///< An error occurred when flash I/O
    EZ_CORE_ERR_DEVID = 0x07,         ///< Devid MUST be set before the sdk init
    EZ_CORE_ERR_OUT_RANGE = 0x08,     ///< The param is out of the valid range
    EZ_CORE_ERR_NO_EXTEND = 0x09,     ///< The expansion module is not registered
    EZ_CORE_ERR_RISK_CRTL = 0x0a,     ///< The device has been blacklisted
    EZ_CORE_ERR_NET = 0x0b,           ///<
    EZ_CORE_ERR_AUTH = 0x0c,          ///<
} ez_core_err_e;

#endif