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

#ifndef H_EZDEV_SDK_API_STRUCT_H_
#define H_EZDEV_SDK_API_STRUCT_H_

#include "base_typedef.h"
#include "ezdev_sdk_kernel_error.h"

#define version_max_len               32     ///< 版本长度
#define ezdev_sdk_extend_name_len     32     ///< 扩展模块名字长度
#define ezdev_sdk_ip_max_len          64     ///< ip最长长度
#define ezdev_sdk_timezone_max_len    32     ///< timezone最长长度
#define ezdev_sdk_sessionkey_len      16     ///< 设备会话秘钥长度，默认是16个字节
#define ezdev_sdk_devid_len           32     ///< 设备唯一标识长度  默认是32字节的字符串
#define ezdev_sdk_masterkey_len       16     ///< 设备mastekey长度 ,默认是16个字节
#define ezdev_sdk_verify_code_maxlen  48     ///< 设备验证码支持最大长度(对应licence认证中product_key)
#define ezdev_sdk_resource_id_len     64
#define ezdev_sdk_resource_type_len   64
#define ezdev_sdk_max_serial_len      72
#define ezdev_sdk_msg_type_len        64
#define ezdev_sdk_module_name_len     16
#define ezdev_sdk_ext_msg_len        128
#define ezdev_sdk_method_len         128


#if !defined(ezdev_sdk_name_len)
#define ezdev_sdk_name_len 64 ///< 设备SDK 一些命名的长度
#endif


/**
 * \brief 通知消息
 */
typedef enum
{
	device_online,				                  ///<	设备上线	            context == sdk_sessionkey_context
	device_offline,				                  ///<	设备下线	            context == sdk_offline_context
	device_switch,			                      ///<	海外设备注册上线通知	 context == sdk_switchover_context
	invaild_authcode,	                          ///<	验证码不合规且没有绑定用户,交给上层APP处理	event_context == null
	fast_reg_online,		                      ///<	设备快速上线            event_context == sdk_sessionkey_context	
	runtime_cb,			                          ///<  运行时回调              evnet_context == sdk_runtime_err_context 
	reconnect_success,                            ///<  重连成功通知回调        evnet_context == NULL 
	heartbeat_interval_changed,                   ///<  心跳改变通知回调        evnet_context == int  
}ez_event_e;

/**
 * \brief 日志级别
 */
typedef enum
{
	log_error,				///<	错误
	log_warn,				///<	警告
	log_info,				///<	信息
	log_debug,				///<	调试
	log_trace				///<	轨迹
}log_level_e;

/**
 * \brief 存取数据类型
 */
typedef enum
{
	key_devid,				       ///<	设备唯一标识,首次设备上线后会分配,一定要写入flash永久固化,以保证在设备的寿命终止前该数据不丢失
	key_masterkey,			       ///<	设备masterkey 首次设备上线后会分配,尽量写入flash,目前有效期三个月
	key_count				       ///<	此枚举上限
}ez_key_type_e;

/**
 * \brief 存取数据类型
 */
typedef enum
{
	data_secretkey,			    ///<	验证码不合规设备重新申请的secretkey，一定要固化
	type_count				///<	此枚举上限
}ez_data_type_e;

/** 
 *  \brief		定义事件通知函数
 *  \method		ez_event_notice
 *  \param[in] 	event_id				消息类型
 *  \param[in] 	context					消息相关联的数据
 */
typedef void (*ez_event_notice)(ez_event_e event_id, void * context);


/** 
 *  \brief		定义加载关键信息的函数
 *  \method		ez_key_value_load
 *  \param[in] 	valuetype				数据类型
 *  \param[in] 	keyvalue				数据存放地址
 *  \param[in] 	keyvalue_maxsize		数据存放最大值
 */
typedef void (*ez_key_value_load)(ez_key_type_e valuetype, unsigned char* keyvalue, EZDEV_SDK_UINT32 keyvalue_maxsize);

/** 
 *  \brief		定义存储关键信息的函数
 *  \method		ez_key_value_save
 *  \param[in] 	valuetype				数据类型
 *  \param[in] 	keyvalue				传入数据地址
 *  \param[in] 	keyvalue_size			传入数据长度
 *  \return 	成功返回0 失败返回非0
 */
typedef EZDEV_SDK_INT32 (*ez_key_value_save)(ez_key_type_e valuetype, unsigned char* keyvalue, EZDEV_SDK_UINT32 keyvalue_size);

/** 
 *  \brief			定义加载关键信息的函数
 *  \method			ez_data_load
 *  \param[in] 		valuetype				数据类型
 *  \param[in] 		keyvalue				数据存放地址
 *  \param[in\out] 	*keyvalue_maxsize		数据缓冲区最大值,返回数据大小
 *  \return 		成功返回0，失败返回非0
 */
typedef EZDEV_SDK_INT32 (*ez_data_load)(ez_data_type_e valuetype, unsigned char* data, EZDEV_SDK_UINT32 *data_maxsize);

/** 
 *  \brief		定义存储关键信息的函数
 *  \method		ezDevSDK_curing_data_save
 *  \param[in]	valuetype				数据类型
 *  \param[in]	keyvalue				传入数据地址
 *  \param[in] 	keyvalue_size			传入数据长度
 *  \return 	成功返回0，失败返回非0
 */
typedef EZDEV_SDK_INT32 (*ez_data_save)(ez_data_type_e valuetype, unsigned char* data, EZDEV_SDK_UINT32 data_maxsize);

/**
 * 低功耗设备快速上线.
 */
typedef struct
{
	EZDEV_SDK_INT8   bLightreg;							///< 指定是否快速上线,0(否),1是wifi快速重连，2是RF快速重连
	EZDEV_SDK_UINT16 das_port;							///< das端口
	EZDEV_SDK_UINT16 das_udp_port;						///< das udp端口
	int              das_socket;						///< 上次下线保修的socket,可以不指定
	char             das_address[64];				    ///< das IP地址
	char             das_domain[64];				    ///< das 域名
	char             das_serverid[64];				    ///< das serverid
	unsigned char    session_key[16];                   ///< das session key
}ez_das_info_t;

/**
 * 初始化配置结构体.
 * \note
 * - 如果bUser为!0，需要实现存取回调函数并在初始化时候传入
 * - 启动微内核前devinfo的文件应存在且有效
 * - 启动微内核前dev_id和dev_masterkey可以只提供路径，在设备上线后得到数据会根据此路径写文件
 */
typedef struct
{
	EZDEV_SDK_UINT8     bUser;					        ///< 是否开启用户回调, 0 dev_id&masterkey通过文件方式存取 !0 通过回调的方式存取
	ez_key_value_load	value_load;			            ///< 读信息的函数
	ez_key_value_save   value_save;			            ///< 写信息的函数
	ez_data_load	    data_load;		                ///< 固化数据的读函数,必须实现
	ez_data_save 	    data_save;		                ///< 固化数据的写函数,必须实现
	char                devinfo_path[128];				///< devinfo文件路径
	char                dev_id[128];					///< dev_id文件路径
	char                dev_masterkey[128];				///< masterkey文件路径
	ez_das_info_t*      pdas_info;					    ///< 低功耗设备快速上线,需要提供das信息,如果不需要默认为NULL
}ez_config_t;

/**
 * 日志和上下线等消息通知回调
 */
typedef struct
{
	ez_event_notice event_notice;			            ///< 事件通知回调
}ez_notice_t;

/**
 * 初始化信息结构体
 */
typedef struct
{
	ez_notice_t	   notice;						         ///< 消息通知相关
	ez_config_t    config;						         ///< 配置相关
}ez_init_info_t;

typedef struct
{
	char           host[64];						     ///< 平台域名
	unsigned int   port;						         ///< 端口
}ez_server_info_t;

/**
 * @addtogroup micro_kernel
 *  \{
 */

enum QOS_T
{
    QOS_T0, ///< 最多一次送到
    QOS_T1, ///< 至少一次送到
    QOS_T2  ///< 准确一次送达
};

/**
 * \brief 往服务器发送的消息结构体
 */
typedef struct
{
    EZDEV_SDK_UINT8 msg_response;        ///< 0:非响应消息 1:响应消息
    enum QOS_T msg_qos;                  ///< 消息QOS类型
    EZDEV_SDK_UINT32 msg_seq;            ///< 消息seq值
    unsigned char *msg_body;             ///< 消息缓冲区
    EZDEV_SDK_UINT32 msg_body_len;       ///< 消息长度，最大不能超过16K
    EZDEV_SDK_UINT32 msg_domain_id;      ///< 领域模块ID
    EZDEV_SDK_UINT32 msg_command_id;     ///< 指令ID
    EZDEV_SDK_PTR    externel_ctx;       ///< 外部自定义数据，在消息发送回执中带回
    EZDEV_SDK_UINT32 externel_ctx_len;   ///< 外部自定义数据长度
    char command_ver[version_max_len];   ///< 指令版本
} ezdev_sdk_kernel_pubmsg;

/**
 * \brief 往服务器发送的消息结构体 3.0协议
 */
typedef struct
{
    EZDEV_SDK_UINT8 msg_response;                    ///< 0:非响应消息 1:响应消息
    enum QOS_T msg_qos;                              ///< 消息QOS类型
    EZDEV_SDK_UINT32 msg_seq;                        ///< 消息seq值
    EZDEV_SDK_UINT32 msg_body_len;                   ///< 消息长度
    unsigned char *msg_body;                         ///< 消息缓冲区
    char resource_id[ezdev_sdk_resource_id_len];     ///< 设备资源id
    char resource_type[ezdev_sdk_resource_type_len]; ///< 设备资源类型
    char module[ezdev_sdk_module_name_len];          ///< 用户和萤石云服务约定的模块标识,例如 "model"  "ota" "basic" "storage"等,
    char method[ezdev_sdk_method_len];               ///< 通道下的方法类型,例如 "event"  "attribute" "service" "inform"  "upload/result/"等,
    char msg_type[ezdev_sdk_msg_type_len];           ///< 消息类型"report" / "query" / "set_reply" / "operate_reply"等
    char sub_serial[ezdev_sdk_max_serial_len];       ///< 子设备序列号
    char ext_msg[ezdev_sdk_ext_msg_len];             ///< 扩展内容，例如"model"中的 "domainid/identifier"字段
}  ezdev_sdk_kernel_pubmsg_v3;


/**
 * \brief 订阅的消息
 */
typedef struct
{
    EZDEV_SDK_UINT32 msg_seq;          ///< 消息seq值
    void *buf;                         ///< 消息缓冲区
    EZDEV_SDK_UINT32 buf_len;          ///< 消息长度
    EZDEV_SDK_INT32 msg_domain_id;     ///< 扩展模块ID（领域ID）
    EZDEV_SDK_INT32 msg_command_id;    ///< 指令ID
    char command_ver[version_max_len]; ///< 指令版本 
} ezdev_sdk_kernel_submsg;

/**
 * \brief 订阅的消息 v3.0 协议
 */
typedef struct
{
    void *buf;                                       ///< 消息缓冲区
    EZDEV_SDK_UINT32 msg_seq;                        ///< 消息seq值
    EZDEV_SDK_UINT32 buf_len;                        ///< 消息长度
    char resource_id[ezdev_sdk_resource_id_len];     ///< 设备资源id
    char resource_type[ezdev_sdk_resource_type_len]; ///< 设备资源类型
    char module[ezdev_sdk_module_name_len];          ///< 用户和萤石云服务约定的模块标识,例如 "model"  "ota" "basic" "storage"等,
    char method[ezdev_sdk_method_len];               ///< 通道下的方法类型,例如 "event"  "attribute" "service" "inform"  "upload/result"等,
    char msg_type[ezdev_sdk_msg_type_len];           ///< 消息类型"report" / "query" / "set_reply" / "operate_reply"等
    char sub_serial[ezdev_sdk_max_serial_len];       ///< 子设备序列号
    char ext_msg[ezdev_sdk_ext_msg_len];             ///< 扩展字段,"model"模块中的 "domainid/identifier"内容
} ezdev_sdk_kernel_submsg_v3;

/**
 * \brief 事件类型
 */
typedef enum
{
    sdk_kernel_event_online,                    ///< event_context == sdk_sessionkey_context     设备上线
    sdk_kernel_event_break,                     ///< event_context == sdk_offline_context        设备离线
    sdk_kernel_event_switchover,                ///< event_context == sdk_switchover_context     平台地址发送切换
    sdk_kernel_event_invaild_authcode,          ///< event_context == null                       验证码不合规且没有绑定用户，交给上层APP处理
    sdk_kernel_event_fast_reg_online,           ///< event_context == sdk_sessionkey_context     设备快速上线
    sdk_kernel_event_runtime_err,               ///< evnet_context == sdk_runtime_err_context    设备sdk运行时错误信息
    sdk_kernel_event_reconnect_success,         ///< evnet_context == NULL                       重连成功事件回调  
    sdk_kernel_event_heartbeat_interval_changed ///< event_context == int                        das心跳发生改变事件回调
} sdk_kernel_event_type;

/**
 * \brief 事件结构体
 */
typedef struct
{
    sdk_kernel_event_type event_type;
    void *event_context;
} ezdev_sdk_kernel_event;

typedef struct
{
    EZDEV_SDK_UINT16 das_port;                           ///< das服务器的TCP端口号
    EZDEV_SDK_UINT16 das_udp_port;                       ///< das服务器UDP端口号
    char das_ip[ezdev_sdk_ip_max_len];                   ///< das服务器IP
    char lbs_ip[ezdev_sdk_ip_max_len];                   ///< lbs服务器IP
    unsigned char session_key[ezdev_sdk_sessionkey_len]; ///< 会话秘钥
    int  das_socket;                                      ///< das的socket
    char das_domain[ezdev_sdk_ip_max_len];               ///< das的域名
    char das_serverid[ezdev_sdk_name_len];               ///< das的serverid
} sdk_sessionkey_context;

typedef struct
{
    EZDEV_SDK_UINT32 last_error;       ///< 错误码
    char das_ip[ezdev_sdk_ip_max_len]; ///< das服务器IP
    char lbs_ip[ezdev_sdk_ip_max_len]; ///< lbs服务器IP
} sdk_offline_context;

typedef struct
{
    EZDEV_SDK_UINT16 das_udp_port;
    char das_ip[ezdev_sdk_ip_max_len];
    char lbs_ip[ezdev_sdk_ip_max_len];
    unsigned char session_key[ezdev_sdk_sessionkey_len];
    unsigned char lbs_domain[ezdev_sdk_ip_max_len];
} sdk_switchover_context;

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
    err_tag_e err_tag;               ///< 错误码类型
    ezdev_sdk_kernel_error err_code; ///< 错误码
    EZDEV_SDK_PTR err_ctx;           ///< 错误码上线文
} sdk_runtime_err_context;

typedef struct
{
    EZDEV_SDK_UINT32 msg_domain_id;    ///< 领域模块ID
    EZDEV_SDK_UINT32 msg_command_id;   ///< 指令ID
    EZDEV_SDK_UINT32 msg_seq;          ///< 消息seq值
    enum QOS_T msg_qos;                ///< 消息QOS类型
    EZDEV_SDK_PTR externel_ctx;        ///< 外部自定义数据，由信令发送接口传进
    EZDEV_SDK_UINT32 externel_ctx_len; ///< 外部自定义数据长度
} sdk_send_msg_ack_context;

typedef struct
{
    enum QOS_T msg_qos;                              ///< 消息QOS类型
    EZDEV_SDK_UINT32 msg_seq;                        ///< 消息seq值
    char resource_id[ezdev_sdk_resource_id_len];     ///< 设备资源id
    char resource_type[ezdev_sdk_resource_type_len]; ///< 设备资源类型
    char module[ezdev_sdk_module_name_len];          ///< 用户和萤石云服务约定的模块标识,例如 "model"  "ota" "basic" "storage"等,
    char method[ezdev_sdk_method_len];               ///< 例如 "event"  "attribute" "service" "shadow" "inform" "upload/result"等,
    char msg_type[ezdev_sdk_msg_type_len];           ///< 消息类型"report" / "query" / "set_reply" / "operate_reply"等
    char sub_serial[ezdev_sdk_max_serial_len];       ///< 子设备序列号,如果非hub模式, 这个参数不需要填写
    char ext_msg[ezdev_sdk_ext_msg_len];             ///< 扩展内容，例如"model"中的 "domainid/identifier"字段
} sdk_send_msg_ack_context_v3;


/**
 * \brief 领域注册信息
 */
typedef struct
{
    EZDEV_SDK_UINT16 domain_id;                                                                           ///< domain_id 对应到业务领域id
    void (*ezdev_sdk_kernel_extend_start)(EZDEV_SDK_PTR pUser);                                           ///< 微内核启动
    void (*ezdev_sdk_kernel_extend_stop)(EZDEV_SDK_PTR pUser);                                            ///< 微内核停止
    void (*ezdev_sdk_kernel_extend_data_route)(ezdev_sdk_kernel_submsg *ptr_submsg, EZDEV_SDK_PTR pUser); ///< 数据路由（领域回调函数）
    void (*ezdev_sdk_kernel_extend_event)(ezdev_sdk_kernel_event *ptr_event, EZDEV_SDK_PTR pUser);        ///< 事件回调（启动、停止、上线、下线等事件）
    EZDEV_SDK_PTR *pUser;                                                                                 ///< 用户指针（一般为NULL 也可以用于携带用户数据指针）
    char extend_module_name[ezdev_sdk_extend_name_len];                                                   ///< 模块名字
    char extend_module_version[version_max_len];                                                          ///< 模块版本号
} ezdev_sdk_kernel_extend;

/**
 * \brief 扩展模块注册信息(V3协议)
 */
typedef struct
{
    char module[ezdev_sdk_module_name_len];                                          ///< 用户和萤石云约定的模块标识,例如"model" "ota" "ota" "storage"等
    void (*ezdev_sdk_kernel_data_route)(ezdev_sdk_kernel_submsg_v3 *ptr_submsg);     ///< 数据路由（按照用户注册的model_type路由）
    void (*ezdev_sdk_kernel_event_route)(ezdev_sdk_kernel_event *ptr_event);     
} ezdev_sdk_kernel_extend_v3;




typedef struct
{
    char lbs_domain[ezdev_sdk_ip_max_len];      ///< lbs服务器域名
    char lbs_ip[ezdev_sdk_ip_max_len];          ///< lbs服务器IP
    EZDEV_SDK_UINT16 lbs_port;                  ///< lbs服务器的TCP端口号
    char das_domain[ezdev_sdk_ip_max_len];      ///< das服务器域名
    char das_ip[ezdev_sdk_ip_max_len];          ///< das服务器IP
    EZDEV_SDK_UINT16 das_port;                  ///< das服务器的TCP端口号
    EZDEV_SDK_UINT16 das_udp_port;              ///< das服务器UDP端口号
    int das_socket;                             ///< das的sock
    char session_key[ezdev_sdk_sessionkey_len]; ///< 会话秘钥
} server_info_s;

typedef struct
{
    unsigned char master_key[ezdev_sdk_masterkey_len + 1];
    unsigned char dev_id[ezdev_sdk_devid_len + 1];
    unsigned char dev_verification_code[ezdev_sdk_verify_code_maxlen + 1];
} showkey_info;


#endif //H_EZDEV_SDK_KERNEL_STRUCT_H_
