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
 *******************************************************************************/

#ifndef H_EZDEV_SDK_KERNEL_STRUCT_H_
#define H_EZDEV_SDK_KERNEL_STRUCT_H_

#include "ez_iot_core_def.h"
#include "sdk_kernel_def.h"
#include "mkernel_internal_error.h"

/**
 * 日志级别信息.
 */
typedef enum
{
    sdk_log_error,
    sdk_log_warn,
    sdk_log_info,
    sdk_log_debug,
    sdk_log_trace
} sdk_log_level;

/**
 * 关键信息类型
 */
typedef enum
{
    sdk_keyvalue_devid,     ///< 设备唯一标识  首次设备上线后会分配 一定要写入flash
    sdk_keyvalue_masterkey, ///< 设备masterkey 首次设备上线后会分配 尽量写入flash
    sdk_keyvalue_count      ///< 枚举上限 用来判定越界
} sdk_keyvalue_type;

/**
 * 关键信息类型
 */
typedef enum
{
    sdk_curingdata_secretkey, ///< 验证码不合规设备重新申请的secretkey，一定要固化
    sdk_curingdata_count      ///< 枚举上限，用来判定越界
} sdk_curingdata_type;

/**
 * 低功耗设备快速上线.
 */
typedef struct
{
    EZDEV_SDK_INT8 bLightreg;                            ///< 指定是否快速上线,0(否),1是wifi快速重连，2是RF快速重连
    EZDEV_SDK_UINT16 das_port;                           ///< das端口
    EZDEV_SDK_UINT16 das_udp_port;                       ///< das udp端口
    int das_socket;                                      ///< 上次下线保活的socket,可以不指定
    char das_address[ezdev_sdk_ip_max_len];              ///< das IP地址
    char das_domain[ezdev_sdk_ip_max_len];               ///< das 域名
    char das_serverid[ezdev_sdk_name_len];               ///< das serverid
    unsigned char session_key[ezdev_sdk_sessionkey_len]; ///< das session key
} kernel_das_info;

/**
 * \brief 通用领域注册专用接口  领域ID是固定的
 */
typedef struct
{
    EZDEV_SDK_UINT16 domain_id; ///<	extendID 对应到业务领域
    EZDEV_SDK_INT8(*ezdev_sdk_kernel_common_module_data_handle)
    (ez_kernel_submsg_t *ptr_submsg, EZDEV_SDK_PTR pUser);
    EZDEV_SDK_PTR pUser;
} ezdev_sdk_kernel_common_module;

/**
 * \}
 */

/* STUN信息 */
typedef struct
{
    EZDEV_SDK_UINT16 stun_interval;
    EZDEV_SDK_UINT16 stun1_port;
    EZDEV_SDK_UINT16 stun2_port;
    char stun1_address[ezdev_sdk_ip_max_len];
    char stun1_domain[ezdev_sdk_ip_max_len];
    char stun2_address[ezdev_sdk_ip_max_len];
    char stun2_domain[ezdev_sdk_ip_max_len];
} stun_info;

typedef struct
{
    char host[ezdev_sdk_ip_max_len];
    unsigned int port;
} server_info_t;

typedef struct
{
    char dev_id[32 + 1];
    char dev_masterkey[16 + 1];
} ezdev_sdk_auth_info_t;

typedef struct
{
    char *pdev_info;
    server_info_t server;
    kernel_das_info *pdas_info;
    EZDEV_SDK_UINT32 buf_size;
} sdk_config_t;

typedef enum
{
    TAG_ACCESS,     ///< 设备接入萤石云错误    err_ctx == NULL
    TAG_MSG_ACK,    ///< 设备信令发送回执      err_ctx == sdk_send_msg_ack_context
    TAG_MSG_ACK_V3, ///< 设备信令发送回执      err_ctx == sdk_send_msg_ack_context_v3
} err_tag_e;

/** 
 *  \details	err_tag==TAG_ACCESS，	err_code为ezdev_sdk_kernel_succ、ezdev_sdk_kernel_params_invalid、ezdev_sdk_kernel_buffer_too_small、\n
 *										ezdev_sdk_kernel_internal、ezdev_sdk_kernel_value_load、ezdev_sdk_kernel_value_save、ezdev_sdk_kernel_memory、\n
 *										NET_ERROR、LBS_ERROR、SECRETKEY_ERROR、DAS_ERROR
 *
 *	\details	err_tag==TAG_MSG_ACK，	err_code为ezdev_sdk_kernel_succ、ezdev_sdk_kernel_net_transmit、ezdev_sdk_kernel_extend_no_find、\n
 *										ezdev_sdk_kernel_force_domain_risk、ezdev_sdk_kernel_force_cmd_risk
 */
typedef struct
{
    err_tag_e err_tag;     ///< 错误码类型
    ez_err_t err_code;     ///< 错误码
    EZDEV_SDK_PTR err_ctx; ///< 错误码上线文
} sdk_runtime_err_context;

typedef struct
{
    EZDEV_SDK_UINT32 msg_domain_id;    ///< 领域模块ID
    EZDEV_SDK_UINT32 msg_command_id;   ///< 指令ID
    EZDEV_SDK_UINT32 msg_seq;          ///< 消息seq值
    ez_kernel_qos_e msg_qos;           ///< 消息QOS类型
    EZDEV_SDK_PTR externel_ctx;        ///< 外部自定义数据，由信令发送接口传进
    EZDEV_SDK_UINT32 externel_ctx_len; ///< 外部自定义数据长度
} sdk_send_msg_ack_context;

typedef struct
{
    ez_kernel_qos_e msg_qos;                         ///< 消息QOS类型
    EZDEV_SDK_UINT32 msg_seq;                        ///< 消息seq值
    char resource_id[ezdev_sdk_resource_id_len];     ///< 设备资源id
    char resource_type[ezdev_sdk_resource_type_len]; ///< 设备资源类型
    char module[ezdev_sdk_module_name_len];          ///< 用户和萤石云服务约定的模块标识,例如 "model"  "ota" "basic" "storage"等,
    char method[ezdev_sdk_method_len];               ///< 例如 "event"  "attribute" "service" "shadow" "inform" "upload/result"等,
    char msg_type[ezdev_sdk_msg_type_len];           ///< 消息类型"report" / "query" / "set_reply" / "operate_reply"等
    char sub_serial[ezdev_sdk_max_serial_len];       ///< 子设备序列号,如果非hub模式, 这个参数不需要填写
    char ext_msg[ezdev_sdk_ext_msg_len];             ///< 扩展内容，例如"model"中的 "domainid/identifier"字段
} sdk_send_msg_ack_context_v3;

#endif //H_EZDEV_SDK_KERNEL_STRUCT_H_
