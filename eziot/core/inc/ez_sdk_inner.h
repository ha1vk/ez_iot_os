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

#ifndef H_EZDEV_SDK_KERNEL_INNER_H_
#define H_EZDEV_SDK_KERNEL_INNER_H_

#include "ez_sdk_api_struct.h"
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
	 *  \brief		设置SDK主版本号(只能由于SDK设置，如果裸接微内核的话，不要调用) 在调用ezdev_sdk_kernel_start之前调用
	 *  \method		ezdev_sdk_kernel_set_sdk_main_version
	 *  \param[in] 	char szMainVersion[ezdev_sdk_extend_name_len]
	 *  \return 	EZOS_API ez_sdk_error
	 */
	EZOS_API ez_sdk_error ez_sdk_set_sdk_main_version(char szMainVersion[version_max_len]);

#ifdef __cplusplus
}
#endif

#endif