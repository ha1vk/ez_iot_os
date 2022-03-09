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

#ifndef H_SDK_KERNEL_DEF_H_
#define H_SDK_KERNEL_DEF_H_

#include <ezos.h>
#include "ez_iot_core_def.h"

#define EZDEV_SDK_TRUE 1
#define EZDEV_SDK_FALSE 0
#define EZDEV_SDK_UNUSED(var) (void)var;

#define EZDEV_SDK_AUTH_GROUP_SIZE 64
#define ezdev_sdk_devserial_maxlen 72
#define ezdev_sdk_recv_topic_len 128
#define ezdev_sdk_identificationcode_max_len 256

typedef ez_kernel_submsg_t ezdev_sdk_kernel_submsg;
typedef ez_kernel_submsg_v3_t ezdev_sdk_kernel_submsg_v3;
typedef ez_kernel_pubmsg_t ezdev_sdk_kernel_pubmsg;
typedef ez_kernel_pubmsg_v3_t ezdev_sdk_kernel_pubmsg_v3;
typedef ez_kernel_extend_t ezdev_sdk_kernel_extend;
typedef ez_kernel_extend_v3_t ezdev_sdk_kernel_extend_v3;
typedef ez_kernel_event_t ezdev_sdk_kernel_event;
typedef ez_kernel_event_e sdk_kernel_event_type;
typedef ez_kernel_switchover_context_t sdk_switchover_context;
typedef ez_err_t ez_sdk_error;

typedef ez_void_t *EZDEV_SDK_PTR;
typedef ez_int8_t EZDEV_SDK_INT8;
typedef ez_int16_t EZDEV_SDK_INT16;
typedef ez_int32_t EZDEV_SDK_INT32;
typedef ez_int64_t EZDEV_SDK_INT64;
typedef ez_uint8_t EZDEV_SDK_UINT8;
typedef ez_uint16_t EZDEV_SDK_UINT16;
typedef ez_uint32_t EZDEV_SDK_UINT32;
typedef ez_uint64_t EZDEV_SDK_UINT64;
typedef EZDEV_SDK_INT8 EZDEV_SDK_BOOL;

typedef enum
{
    sdk_no_risk_control = 0, ///<	不风控
    sdk_risk_control = 1,    ///<	风控
} sdk_risk_control_flag;

/**
* \brief   标记SDK入口状态
*/
typedef enum
{
    sdk_entrance_normal,     ///<	SDK正常入口
    sdk_entrance_switchover, ///<	SDK入口切换
} sdk_entrance_state;

/**
* \brief   标记SDK状态
*/
typedef enum
{
    sdk_idle0 = 0, ///<	刚创建未初始化
    sdk_idle,      ///<	空闲		初始化状态
    sdk_start,     ///<	sdk处于启动
    sdk_stop,      ///<	sdk处于关闭
} sdk_state;

/**
* \brief   标记SDK与cloud连接状态
*/
typedef enum
{
    sdk_cnt_unredirect = 0, ///<	未重定向状态		需要到LBS去做重定向
    sdk_cnt_redirected = 1, ///<	重定向完成			需要到DAS上注册
    sdk_cnt_das_reged = 2,  ///<	完成注册			需要接收和发送指令
    sdk_cnt_das_break = 4,  ///<	与DAS处于中断状态	需要重连
} sdk_cloud_cnt_state;

typedef enum
{
    sdk_dev_auth_sap = 0,    ///<	SAP认证
    sdk_dev_auth_license = 1 ///<	license认证
} sdk_dev_auth_mode;

typedef enum
{
    sdk_v3_unreg = 0, ///<	V3协议未使用
    sdk_v3_reged,
} sdk_v3_reg_status;

typedef enum
{
    sdk_dev_auth_protocol_none = 0, ///<	无认证
    sdk_dev_auth_protocol_ecdh = 1, ///<	ECDH认证
    sdk_dev_auth_protocol_end
} sdk_dev_auth_protocol_type;

/**
* \brief   与LBS交互通用协议体
*/
typedef struct
{
    unsigned char pro_form_version;      ///<	交互协议形式版本号
    unsigned char pro_type_low_version;  ///<	交互协议类型版本号(低)
    unsigned char pro_type_high_version; ///<	交互协议类型版本号(高)
} lbs_common;

typedef struct
{
    char server_name[ezdev_sdk_name_len]; ///<	lbs 域名
    char server_ip[ezdev_sdk_name_len];   ///<	lbs Ip
    EZDEV_SDK_INT16 server_port;          ///<	lbs port
} ezdev_server_info;

typedef struct
{
    EZDEV_SDK_UINT16 das_port;
    EZDEV_SDK_UINT16 das_udp_port;
    char das_address[ezdev_sdk_ip_max_len];
    char das_domain[ezdev_sdk_ip_max_len];
    char das_serverid[ezdev_sdk_name_len];
} das_info;

/**
 * \brief   设备基本信息
 */
typedef struct
{
    EZDEV_SDK_UINT16 dev_access_mode;                                          ///< 设备接入模式  0-普通（2.0）   1-HUB（2.0）
    sdk_dev_auth_mode dev_auth_mode;                                           ///< 认证模式：0 SAP认证   1 licence认证
    EZDEV_SDK_UINT16 dev_status;                                               ///< 设备工作状态  1：正常工作模式  5：待机(或睡眠)工作模式
    char dev_subserial[ezdev_sdk_devserial_maxlen];                            ///< 设备短序列号(对应licence认证中device_id)
    char dev_verification_code[ezdev_sdk_verify_code_maxlen];                  ///< 设备验证码(对应licence认证中licence)
    char dev_serial[ezdev_sdk_devserial_maxlen];                               ///< 设备长序列号
    char dev_firmwareversion[ezdev_sdk_name_len];                              ///< 设备固件版本号
    char dev_type[ezdev_sdk_name_len];                                         ///< 设备型号
    char dev_typedisplay[ezdev_sdk_name_len];                                  ///< 设备显示型号
    char dev_mac[ezdev_sdk_name_len];                                          ///< 设备网上物理地址
    char dev_firmwareidentificationcode[ezdev_sdk_identificationcode_max_len]; ///< 设备固件识别码
} dev_basic_info;

/**
* \brief   领域信息
*/
typedef struct
{
    sdk_risk_control_flag domain_risk;                               ///<	领域是否被风控
    EZDEV_SDK_UINT32 cmd_risk_array[CONFIG_EZIOT_CORE_RISK_CONTROL_CMD_MAX]; ///<	领域内被风控的指令
    ez_kernel_extend_t kernel_extend;                                ///<	SDK注册进来的领域扩展
} ezdev_sdk_kernel_domain_info;

/**
* \brief   领域信息
*/
typedef struct
{
    ez_kernel_extend_v3_t kernel_extend; ///<	SDK注册进来的领域扩展
} ezdev_sdk_kernel_domain_info_v3;

typedef struct
{
    EZDEV_SDK_UINT8 lbs_redirect_times;  ///<	记录lbs连续重定向的次数
    EZDEV_SDK_UINT8 das_retry_times;     ///<	记录das连续重连的次数
    sdk_v3_reg_status v3_reg_status;     ///<	V3协议是否使用

    sdk_entrance_state entr_state;   ///<	sdk入口状态
    sdk_state my_state;              ///<	sdk状态
    sdk_cloud_cnt_state cnt_state;   ///<	连接状态
    ezos_timespec_t cnt_state_timer; ///<	重连相关的定时器

    char dev_subserial[ezdev_sdk_devserial_maxlen];
    unsigned char master_key[ezdev_sdk_masterkey_len];
    unsigned char dev_id[ezdev_sdk_devid_len];
    unsigned char session_key[ezdev_sdk_sessionkey_len];

    EZDEV_SDK_UINT32 das_keepalive_interval; ///<	DAS心跳时间间隔
    dev_basic_info dev_info;                 ///<	设备基础信息

    ezdev_server_info server_info; ///<	lbs服务信息
    das_info redirect_das_info;    ///<	lbs重定向过来的das信息

    char subscribe_topic[ezdev_sdk_recv_topic_len]; ///<	设备向平台订阅的主题

    char szMainVersion[version_max_len]; ///<	SDK主版本号

    EZDEV_SDK_UINT8 reg_mode;          ///<	设备注册模式
    sdk_risk_control_flag access_risk; ///<	接入风控标识

    EZDEV_SDK_UINT8 dev_cur_auth_type;
    EZDEV_SDK_UINT8 dev_def_auth_type;
    EZDEV_SDK_UINT8 dev_auth_type_count;
    EZDEV_SDK_UINT8 dev_last_auth_type;
    EZDEV_SDK_UINT8 dev_auth_type_group[EZDEV_SDK_AUTH_GROUP_SIZE];
    EZDEV_SDK_INT32(*key_value_save)
    (EZDEV_SDK_UINT32 valuetype, unsigned char *keyvalue, EZDEV_SDK_INT32 keyvalue_size);
} ezdev_sdk_kernel;

/**
 * \brief   设备发布消息的消息存储载体
 */
typedef struct
{
    EZDEV_SDK_UINT16 max_send_count; ///<	最大发布次数，send后--
    ez_kernel_pubmsg_t msg_conntext; ///<	发布的消息内容
} ezdev_sdk_kernel_pubmsg_exchange;

/**
 * \brief   设备发布消息的消息存储载体 3.0协议
 */
typedef struct
{
    EZDEV_SDK_UINT16 max_send_count;       ///<	最大发布次数，send后--
    ez_kernel_pubmsg_v3_t msg_conntext_v3; ///<	发布的消息内容
} ezdev_sdk_kernel_pubmsg_exchange_v3;

typedef enum
{
    extend_cb_start, ///<	ez_kernel_extend_t_start
    extend_cb_stop,  ///<	ez_kernel_extend_t_stop
    extend_cb_event  ///<	ez_kernel_extend_t_event
} extend_cb_type;

/**
* \brief   kernel内部使用的扩展异步回调消息
*/
typedef struct
{
    extend_cb_type cb_type;
    ez_kernel_event_t cb_event;
} ezdev_sdk_kernel_inner_cb_notic;

ezdev_sdk_kernel *get_ezdev_sdk_kernel();

#define FUNC_IN() elog_output(EZ_ELOG_LVL_VERBOSE, TAG_CORE, "", __FUNCTION__, __LINE__, " in")
#define FUNC_OUT() elog_output(EZ_ELOG_LVL_VERBOSE, TAG_CORE, "", __FUNCTION__, __LINE__, " out")

#define CHECK_COND_DONE(cond, errcode)                                   \
    if ((cond))                                                          \
    {                                                                    \
        ezlog_e(TAG_CORE, "cond done:0x%x,errcode:0x%x", cond, errcode); \
        rv = (errcode);                                                  \
        goto done;                                                       \
    }

#define CHECK_RV_DONE(errcode)                      \
    if (0 != errcode)                               \
    {                                               \
        ezlog_e(TAG_CORE, "errcode:0x%x", errcode); \
        rv = (errcode);                             \
        goto done;                                  \
    }

#endif