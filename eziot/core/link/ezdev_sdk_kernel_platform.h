#ifndef H_EZDEV_SDK_KERNEL_PLATFORM_H_
#define H_EZDEV_SDK_KERNEL_PLATFORM_H_

#define EZDEV_SDK_KERNEL_PLATFORM_INTERFACE	\
	extern ezdev_sdk_mutex ezdev_sdk_kernel_platform_thread_mutex_create();	\
	extern void  ezdev_sdk_kernel_platform_thread_mutex_destroy(ezdev_sdk_mutex ptr_mutex); \
	extern int ezdev_sdk_kernel_platform_thread_mutex_lock(ezdev_sdk_mutex ptr_mutex); \
	extern int ezdev_sdk_kernel_platform_thread_mutex_unlock(ezdev_sdk_mutex ptr_mutex);
#endif