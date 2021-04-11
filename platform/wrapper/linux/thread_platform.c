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
#include <unistd.h>
#include <sys/prctl.h>

#include "thread_platform_wrapper.h"
#include "ezdev_sdk_kernel_struct.h"

static void *sdk_thread_fun(void *aArg)
{
	thread_handle *hd = (thread_handle *)aArg;
	if (hd == NULL)
	{
		return NULL;
	}

	prctl(PR_SET_NAME, hd->thread_name);
	hd->task_do(hd->thread_arg);

	return NULL;
}

ezdev_sdk_mutex sdk_platform_thread_mutex_create()
{
	sdk_mutex_platform *ptr_mutex_platform = NULL;
	ptr_mutex_platform = (sdk_mutex_platform *)malloc(sizeof(sdk_mutex_platform));
	if (ptr_mutex_platform == NULL)
	{
		return NULL;
	}

	pthread_mutex_init(&ptr_mutex_platform->lock, NULL);

	return (ezdev_sdk_mutex)ptr_mutex_platform;
}

void sdk_platform_thread_mutex_destroy(ezdev_sdk_mutex ptr_mutex)
{
	sdk_mutex_platform *ptr_mutex_platform = (sdk_mutex_platform *)ptr_mutex;
	if (ptr_mutex_platform == NULL)
	{
		return;
	}

	pthread_mutex_destroy(&ptr_mutex_platform->lock);

	free(ptr_mutex_platform);
	ptr_mutex_platform = NULL;
}

int sdk_platform_thread_mutex_lock(ezdev_sdk_mutex ptr_mutex)
{
	sdk_mutex_platform *ptr_mutex_platform = (sdk_mutex_platform *)ptr_mutex;
	if (ptr_mutex_platform == NULL)
	{
		return -1;
	}

	pthread_mutex_lock(&ptr_mutex_platform->lock);
	return 0;
}

int sdk_platform_thread_mutex_unlock(ezdev_sdk_mutex ptr_mutex)
{
	sdk_mutex_platform *ptr_mutex_platform = (sdk_mutex_platform *)ptr_mutex;
	if (ptr_mutex_platform == NULL)
	{
		return -1;
	}

	pthread_mutex_unlock(&ptr_mutex_platform->lock);
	return 0;
}

int sdk_thread_create(thread_handle *handle)
{
	if (handle == NULL)
	{
		return -1;
	}
	if (pthread_create(&handle->thread_hd, NULL, sdk_thread_fun, (void *)handle) != 0)
	{
		return -1;
	}
	return 0;
}

int sdk_thread_destroy(thread_handle *handle)
{
	if (handle == NULL)
	{
		return -1;
	}
	if (handle->thread_hd != 0)
	{
		void *pres = NULL;
		pthread_join(handle->thread_hd, &pres);
	}
	return 0;
}

void sdk_thread_sleep(unsigned int time_ms)
{
	usleep((int)(time_ms * 1000));
}