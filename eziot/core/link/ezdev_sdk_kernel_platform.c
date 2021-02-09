#include "ezdev_sdk_kernel_platform.h"
#include "sdk_kernel_def.h"

extern ezdev_sdk_kernel g_ezdev_sdk_kernel;

ezdev_sdk_mutex ezdev_sdk_kernel_platform_thread_mutex_create()
{
	return g_ezdev_sdk_kernel.platform_handle.thread_mutex_create();
}

void  ezdev_sdk_kernel_platform_thread_mutex_destroy(ezdev_sdk_mutex ptr_mutex)
{
	g_ezdev_sdk_kernel.platform_handle.thread_mutex_destroy(ptr_mutex);
}

int ezdev_sdk_kernel_platform_thread_mutex_lock(ezdev_sdk_mutex ptr_mutex)
{
	return g_ezdev_sdk_kernel.platform_handle.thread_mutex_lock(ptr_mutex);
}

int ezdev_sdk_kernel_platform_thread_mutex_unlock(ezdev_sdk_mutex ptr_mutex)
{
	return g_ezdev_sdk_kernel.platform_handle.thread_mutex_unlock(ptr_mutex);
}