#include "ezdev_sdk_kernel_inner.h"
#include "sdk_kernel_def.h"

extern ezdev_sdk_kernel g_ezdev_sdk_kernel;

EZDEV_SDK_KERNEL_API ezdev_sdk_kernel_error ezdev_sdk_kernel_set_sdk_main_version( char szMainVersion[version_max_len] )
{
	strncpy(g_ezdev_sdk_kernel.szMainVersion, szMainVersion, version_max_len - 1);
	return ezdev_sdk_kernel_succ;
}