#ifndef H_EZDEV_SDK_KERNEL_COMMON_H_
#define H_EZDEV_SDK_KERNEL_COMMON_H_


#define  EZDEV_SDK_KERNEL_COMMON_INTERFACE	\
	extern void common_module_init(void); \
	extern void common_module_fini(void); \
	extern mkernel_internal_error common_module_load(const ezdev_sdk_kernel_common_module* common_module); \
	extern EZDEV_SDK_INT8 common_module_bus_handle(ezdev_sdk_kernel_submsg* ptr_submsg);

#endif