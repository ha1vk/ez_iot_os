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

#include "base_typedef.h"
#include "ezdev_sdk_kernel_error.h"
#include "mkernel_internal_error.h"
#include "ez_sdk_api_struct.h"
#include "file_interface.h"
#include "io_interface.h"
#include "mem_interface.h"
#include "network_interface.h"
#include "thread_interface.h"
#include "time_interface.h"


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
    sdk_keyvalue_coapinfo,  ///< coap信息,
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
    EZDEV_SDK_INT8   bLightreg;							   ///< 指定是否快速上线,0(否),1是wifi快速重连，2是RF快速重连
    EZDEV_SDK_UINT16 das_port;                             ///< das端口
    EZDEV_SDK_UINT16 das_udp_port;                         ///< das udp端口
    int              das_socket;                           ///< 上次下线保活的socket,可以不指定
    char             das_address[ezdev_sdk_ip_max_len];    ///< das IP地址
    char             das_domain[ezdev_sdk_ip_max_len];     ///< das 域名
    char             das_serverid[ezdev_sdk_name_len];     ///< das serverid
    unsigned char    session_key[ezdev_sdk_sessionkey_len];///< das session key
} kernel_das_info;


/**
 * \brief 通用领域注册专用接口  领域ID是固定的
 */
typedef struct
{
	EZDEV_SDK_UINT16 domain_id; ///<	extendID 对应到业务领域
	EZDEV_SDK_INT8(*ezdev_sdk_kernel_common_module_data_handle)(ezdev_sdk_kernel_submsg *ptr_submsg, EZDEV_SDK_PTR pUser); 
	EZDEV_SDK_PTR pUser;
} ezdev_sdk_kernel_common_module;

/**
 * \brief 微内核内部使用的跨平台接口.
 */
typedef struct
{
    int (*net_work_create)(char *nic_name);
    mkernel_internal_error (*net_work_connect)(int socket_fd, const char *server_ip, EZDEV_SDK_INT32 server_port, EZDEV_SDK_INT32 timeout_ms, char szRealIp[ezdev_sdk_ip_max_len]);
    mkernel_internal_error (*net_work_read)(int socket_fd, unsigned char *read_buf, EZDEV_SDK_INT32 read_buf_maxsize, EZDEV_SDK_INT32 read_timeout_ms);
    mkernel_internal_error (*net_work_write)(int socket_fd, unsigned char *write_buf, EZDEV_SDK_INT32 write_buf_size, EZDEV_SDK_INT32 write_timeout_ms, EZDEV_SDK_INT32 *real_write_buf_size);
    void (*net_work_disconnect)(int socket_fd);

	ez_timespec* (*time_creator)(void);
	char (*time_isexpired_bydiff)(ez_timespec *sdktime, EZDEV_SDK_UINT32 time_ms);
	char (*time_isexpired)(ez_timespec *sdktime);
	void (*time_countdownms)(ez_timespec *sdktime, EZDEV_SDK_UINT32 time_ms);
	void (*time_countdown)(ez_timespec *sdktime, EZDEV_SDK_UINT32 time_count);
	void (*time_destroy)(ez_timespec *sdktime);
	EZDEV_SDK_UINT32 (*time_leftms)(ez_timespec *sdktime);
    void (*time_sleep)(unsigned int time_ms);

	void (*key_value_load)(sdk_keyvalue_type valuetype, unsigned char* keyvalue, EZDEV_SDK_INT32 keyvalue_maxsize);						///<	读信息的函数，必须处理secretkey的读操作
	EZDEV_SDK_INT32 (*key_value_save)(sdk_keyvalue_type valuetype, unsigned char* keyvalue, EZDEV_SDK_INT32 keyvalue_size);				///<	写信息的函数，必须处理secretkey的写操作
	EZDEV_SDK_INT32 (*curing_data_load)(sdk_curingdata_type valuetype, unsigned char* keyvalue, EZDEV_SDK_INT32 *keyvalue_maxsize);		///<    读信息的函数，必须处理secretkey的读操作
	EZDEV_SDK_INT32 (*curing_data_save)(sdk_curingdata_type valuetype, unsigned char* keyvalue, EZDEV_SDK_INT32 keyvalue_size);			///<	写信息的函数，必须处理secretkey的写操作

	
	ez_mutex_t (*thread_mutex_create)();
	int (*thread_mutex_destroy)(ez_mutex_t ptr_mutex);
	int (*thread_mutex_lock)(ez_mutex_t ptr_mutex);
	int (*thread_mutex_unlock)(ez_mutex_t ptr_mutex);
} ezdev_sdk_kernel_platform_handle;

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
    char         host[ezdev_sdk_ip_max_len];
    unsigned int port;
} server_info_t;

typedef struct
{
    char*             pdev_info;
    server_info_t     server;
    kernel_das_info*  pdas_info;   
    EZDEV_SDK_UINT32  buf_size;        
} sdk_config_t;


typedef void (*sdk_kernel_event_notice)(ezdev_sdk_kernel_event *ptr_event);
#endif //H_EZDEV_SDK_KERNEL_STRUCT_H_
