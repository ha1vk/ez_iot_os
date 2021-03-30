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

#ifndef H_EZ_SDK_API_H_ 
#define H_EZ_SDK_API_H_

#include "base_typedef.h"

#if defined (_WIN32) || defined(_WIN64)

#ifdef EZ_SDK_API_EXPORTS
#define EZ_SDK_API	__declspec(dllexport)
#else
#define EZ_SDK_API __declspec(dllimport)
#endif

#define EZ_IOT_CALLBACK __stdcall
#else
#define EZ_SDK_API 
#define EZ_IOT_CALLBACK 
#endif


/*! \addtogroup Boot_Module SDKBoot模块
 * SDKBoot模块，向上层应用提供初始化、反初始化、启动、停止微内核等功能接口，承接时间、线程、网络等和平台关联性大的功能实现
 *  \{
 */

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
 *  \brief		定义日志打印回调
 *  \method		ez_log_notice
 *  \param[in] 	level					日志等级
 *  \param[in] 	sdk_error				SDK错误码
 *  \param[in] 	othercode				其他的错误码
 *  \param[in] 	buf						消息存放地址
 */
typedef void (*ez_log_notice)(log_level_e level, EZDEV_SDK_INT32 sdk_error, EZDEV_SDK_INT32 othercode, const char *buf);

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
	ez_log_notice   log_notice;				            ///< 日志回调
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
	char*          host;						         ///< 平台域名
	unsigned int   port;						         ///< 端口
}ez_server_info_t;

#ifdef __cplusplus
extern "C"
{
#endif

/** 
 *  \brief		Boot模块初始化接口
 *  \method		ez_init
 *  \param[in] 	pserver_info	    平台服务器信息
 *  \param[in] 	pinit			    初始化信息
 *  \param[in]  reg_mode			注册模式
 *  \details
 *	1---正常设备(平台默认值)
 *	2---wifi托管低功耗设备(现已有,表示电池设备当前托管状态)
 *	3---RF托管低功耗设备(表示电池设备当前托管状态)
 *	4---RF管理(表示支持RF托管,由Base设备上报)
 *  \return 	ezdev_sdk_kernel_succ、ezdev_sdk_kernel_params_invalid、ezdev_sdk_kernel_json_invalid、ezdev_sdk_kernel_json_format、 \n
 *				ezdev_sdk_kernel_value_load、ezdev_sdk_kernel_invald_call、ezdev_sdk_kernel_memory、ezdev_sdk_kernel_internal
 *	\see		错误码 ： ezdev_sdk_kernel_error
 */
EZ_SDK_API  EZDEV_SDK_INT32 ez_sdk_init(const ez_server_info_t* pserver_info, const ez_init_info_t* pinit, const EZDEV_SDK_UINT32 reg_mode);

/** 
 *  \brief		Boot模块启动接口（开启整个SDK，调用完接口后，SDK进入工作模式）
 *  \method		ez_sdk_start
 *  \return 	成功返回0 失败详见错误码
 *  \return 	ezdev_sdk_kernel_succ、ezdev_sdk_kernel_invald_call
 *	\see		错误码 ： ezdev_sdk_kernel_error
 */
EZ_SDK_API  EZDEV_SDK_INT32 ez_sdk_start();

/** 
 *  \brief		Boot模块停止接口
 *  \method		ez_sdk_stop
 *  \return 	成功返回0 失败详见错误码
 *  \return 	ezdev_sdk_kernel_succ、ezdev_sdk_kernel_invald_call
 *	\see		错误码 ： ezdev_sdk_kernel_error
 */
EZ_SDK_API  EZDEV_SDK_INT32 ez_sdk_stop();
/** 
 *  \brief		Boot模块反初始化接口
 *  \method		ezDevSDK_Fini
 *  \return 	成功返回0 失败详见错误码
 *  \return 	ezdev_sdk_kernel_succ、ezdev_sdk_kernel_invald_call
 *	\see		错误码 ： ezdev_sdk_kernel_error
 */
EZ_SDK_API  EZDEV_SDK_INT32 ez_sdk_deinit();


#ifdef __cplusplus
}
#endif
/*! \} */

#endif //H_EZDEVSDK_BOOT_H_