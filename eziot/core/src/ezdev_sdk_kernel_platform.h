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

#ifndef H_EZDEV_SDK_KERNEL_PLATFORM_H_
#define H_EZDEV_SDK_KERNEL_PLATFORM_H_

#define EZDEV_SDK_KERNEL_PLATFORM_INTERFACE	\
	extern ez_mutex_t ezdev_sdk_kernel_platform_thread_mutex_create();	\
	extern void  ezdev_sdk_kernel_platform_thread_mutex_destroy(ez_mutex_t ptr_mutex); \
	extern int ezdev_sdk_kernel_platform_thread_mutex_lock(ez_mutex_t ptr_mutex); \
	extern int ezdev_sdk_kernel_platform_thread_mutex_unlock(ez_mutex_t ptr_mutex);
#endif