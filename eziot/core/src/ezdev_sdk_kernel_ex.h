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

#ifndef H_EZDEV_SDK_KERNEL_EX_H_
#define H_EZDEV_SDK_KERNEL_EX_H_

#include "ezdev_sdk_kernel_struct.h"
#include "ez_sdk_error.h"
#include "ezos_file.h"
#include "ezos_io.h"
#include "ezos_mem.h"
#include "ezos_network.h"
#include "ezos_thread.h"
#include "ezos_time.h"

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
 *  \brief		获取STUN服务器信息接口
 *  \method		ezdev_sdk_kernel_get_stun
 *  \param[in] 	stun_info * ptr_stun
 *  \return 	EZOS_API ez_sdk_error
 */
EZOS_API ez_sdk_error EZOS_CALL ezdev_sdk_kernel_get_stun(stun_info* ptr_stun, EZDEV_SDK_BOOL bforce_refresh);

/** 
 *  \brief		设置心跳时间
 *  \method		ezdev_sdk_kernel_set_keepalive_interval
 *  \param[in] 	EZDEV_SDK_UINT16 internal   设置心跳间隔时间
 *  \param[in]  EZDEV_SDK_UINT16 timeout_s  等待服务器响应的超时时间，0表示不等待
 *  \return 	如果不等待响应，信令送进队列返回成功，反之等待平台响应错误码，如果超时，返回等待信令超时。
 */
EZOS_API ez_sdk_error  EZOS_CALL ezdev_sdk_kernel_set_keepalive_interval(EZDEV_SDK_UINT16 internal, EZDEV_SDK_UINT16 timeout_s);

#ifdef __cplusplus
}
#endif

#endif