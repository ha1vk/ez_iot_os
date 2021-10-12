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
#include "ezdev_sdk_kernel_error.h"
#include "ez_sdk_api_struct.h"

#if (defined(_WIN32) || defined(_WIN64))
#if !define(EZOS_API)
#if defined(EZ_OS_API_EXPORTS)
#define EZOS_API __declspec(dllexport)
#else
#define EZOS_API __declspec(dllimport)
#endif
#if !define(EZOS_CALL)
#define EZOS_CALL __stdcall
#endif
#endif
#else
#define EZOS_API
#define EZOS_CALL
#endif

#ifdef __cplusplus
extern "C"
{
#endif

    /** 
     * \brief       Boot模块初始化接口
     * \param[in]   pserver_info    平台服务器信息
     * \param[in]   pinit           初始化信息
     * \param[in]   reg_mode         注册模式
     * \details
     * 1---正常设备(平台默认值)
     * 2---wifi托管低功耗设备(现已有,表示电池设备当前托管状态)
     * 3---RF托管低功耗设备(表示电池设备当前托管状态)
     * 4---RF管理(表示支持RF托管,由Base设备上报)
     * \return  ezdev_sdk_kernel_succ、ezdev_sdk_kernel_params_invalid、ezdev_sdk_kernel_json_invalid、ezdev_sdk_kernel_json_format、 \n
     * ezdev_sdk_kernel_value_load、ezdev_sdk_kernel_invald_call、ezdev_sdk_kernel_memory、ezdev_sdk_kernel_internal
     * \see     错误码 ： ezdev_sdk_kernel_error
     */
    EZOS_API EZDEV_SDK_INT32 EZOS_CALL ez_sdk_init(const ez_server_info_t *pserver_info, const ez_init_info_t *pinit, const EZDEV_SDK_UINT32 reg_mode);

    /** 
     *  \brief      Boot模块启动接口（开启整个SDK，调用完接口后，SDK进入工作模式）
     *  \return     成功返回0 失败详见错误码
     *  \return     ezdev_sdk_kernel_succ、ezdev_sdk_kernel_invald_call
     *  \see        错误码 ： ezdev_sdk_kernel_error
     */
    EZOS_API EZDEV_SDK_INT32 EZOS_CALL ez_sdk_start();

    /** 
     *  \brief      Boot模块停止接口
     *  \return     成功返回0 失败详见错误码
     *  \return     ezdev_sdk_kernel_succ、ezdev_sdk_kernel_invald_call
     *  \see        错误码 ： ezdev_sdk_kernel_error
     */
    EZOS_API EZDEV_SDK_INT32 EZOS_CALL ez_sdk_stop();

    /** 
     *  \brief      Boot模块反初始化接口
     *  \return     成功返回0 失败详见错误码
     *  \return     ezdev_sdk_kernel_succ、ezdev_sdk_kernel_invald_call
     *  \see        错误码 ： ezdev_sdk_kernel_error
     */
    EZOS_API EZDEV_SDK_INT32 EZOS_CALL ez_sdk_deinit();

    /** 
     *  \brief      微内核领域模块加载接口（非线程安全）
     *  \param[in]  external_extend 领域模块信息
     *  \return     ezdev_sdk_kernel_succ、ezdev_sdk_kernel_invald_call、ezdev_sdk_kernel_params_invalid、ezdev_sdk_kernel_extend_existed、\n
     *              ezdev_sdk_kernel_extend_full
     */
    EZOS_API ezdev_sdk_kernel_error EZOS_CALL ezdev_sdk_kernel_extend_load(const ezdev_sdk_kernel_extend *external_extend);

    /** 
     *  \brief      扩展模块加载接口（非线程安全）
     *  \param[in]  external_extend 扩展模块信息
     *  \return     ezdev_sdk_kernel_succ、ezdev_sdk_kernel_invald_call、ezdev_sdk_kernel_params_invalid、ezdev_sdk_kernel_extend_existed、\n
     *              ezdev_sdk_kernel_extend_full
     */
    EZOS_API ezdev_sdk_kernel_error EZOS_CALL ezdev_sdk_kernel_extend_load_v3(const ezdev_sdk_kernel_extend_v3 *extend_info);

    /** 
     *  \brief      微内核数据发送接口（线程安全）
     *  \note   只负责发消息，不管消息内容, 非阻塞式调用
     *  \param[in]  pubmsg  消息内容
     *  \return     ezdev_sdk_kernel_succ、ezdev_sdk_kernel_invald_call、ezdev_sdk_kernel_params_invalid、ezdev_sdk_kernel_data_len_range、\n
     *              ezdev_sdk_kernel_memory、ezdev_sdk_kernel_queue_full、ezdev_sdk_kernel_extend_no_find、ezdev_sdk_kernel_force_domain_risk、\n
     *              ezdev_sdk_kernel_force_cmd_risk
     */
    EZOS_API ezdev_sdk_kernel_error EZOS_CALL ezdev_sdk_kernel_send(ezdev_sdk_kernel_pubmsg *pubmsg);

    /** 
     *  \brief      微内核数据发送接口 v3协议（线程安全）
     *  \note       只负责发消息，不管消息内容, 非阻塞式调用
     *  \param[in]  pubmsg  消息内容
     *  \return     ezdev_sdk_kernel_succ、ezdev_sdk_kernel_invald_call、ezdev_sdk_kernel_params_invalid、ezdev_sdk_kernel_data_len_range、\n
     *              ezdev_sdk_kernel_memory、ezdev_sdk_kernel_queue_full、ezdev_sdk_kernel_extend_no_find、ezdev_sdk_kernel_force_domain_risk、\n
     *              ezdev_sdk_kernel_force_cmd_risk
     */
    EZOS_API ezdev_sdk_kernel_error EZOS_CALL ezdev_sdk_kernel_send_v3(ezdev_sdk_kernel_pubmsg_v3 *pubmsg_v3);

    /** 
     *  \brief      设置socket参数
     *  \note       设置socket参数，需要在ezdev_sdk_kernel_start前调用
     *  \param[in]  optname 操作类型, 1 绑定到某张网卡 3 设备接入链路断开重连
     *  \param[in]  optval 操作参数
     *  \param[in]  optlen 操作参数长度
     *  \return     ezdev_sdk_kernel_succ、ezdev_sdk_kernel_params_invalid、ezdev_sdk_kernel_buffer_too_small
     */
    EZOS_API ezdev_sdk_kernel_error EZOS_CALL ezdev_sdk_kernel_set_net_option(int optname, const void *optval, int optlen);

    /** 
     *  \brief      萤石设备接入SDK 微内核 设备信息获取接口
     *  \method     ezdev_sdk_kernel_getdevinfo_bykey
     *  \param[in]  key 查找键值
     *  \return     成功返回value值指针 失败返回"invalidkey"
     */
    EZOS_API const char* EZOS_CALL ezdev_sdk_kernel_getdevinfo_bykey(const char *key);

    /** 
     *  \brief          获取微内核的版本号，二次调用
     *  \param[out]     pbuf 微内核版本
     *  \param[inout]   pbuflen 如果pbuf为空，返回待拷贝数据长度，否则返回真实拷贝长度
     *  \return         ezdev_sdk_kernel_succ、ezdev_sdk_kernel_params_invalid、ezdev_sdk_kernel_buffer_too_small
     */
    EZOS_API ezdev_sdk_kernel_error EZOS_CALL ezdev_sdk_kernel_get_sdk_version(char *pbuf, int *pbuflen);

    /** 
     *  \brief          获取LBS、DAS服务器信息接口，二次调用
     *  \param[out]     ptr_server_info 服务器信息数组
     *  \param[inout]   ptr_count 如果ptr_server_info为空，返回待拷贝数据的数量，否则返回真实拷贝数量
     *  \return         ezdev_sdk_kernel_succ、ezdev_sdk_kernel_invald_call、ezdev_sdk_kernel_params_invalid、ezdev_sdk_kernel_buffer_too_small
     */
    EZOS_API ezdev_sdk_kernel_error EZOS_CALL ezdev_sdk_kernel_get_server_info(server_info_s *ptr_server_info, int *ptr_count);

    /** 
     *  \brief          show_key功能接口
     *  \param[out]     show_key信息数组
     *  \return         ezdev_sdk_kernel_succ、ezdev_sdk_kernel_invald_call、ezdev_sdk_kernel_params_invalid、ezdev_sdk_kernel_buffer_too_small
     */
    EZOS_API ezdev_sdk_kernel_error EZOS_CALL ezdev_sdk_kernel_show_key_info(showkey_info *ptr_showkey_info);

#ifdef __cplusplus
}
#endif
/*! \} */

#endif //H_EZDEVSDK_BOOT_H_