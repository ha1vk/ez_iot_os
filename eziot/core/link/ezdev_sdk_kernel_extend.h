/**
*  \file      
*  \filename  ezdev_sdk_kernel_extend.h  
*  \filepath  E:\workdir\小项目\ezDevSDK_v2.0.0\microkernel\src\ezdev_sdk_kernel_extend.h
*  \copyright HangZhou Hikvision System Technology Co.,Ltd. All Right Reserved.
*  \brief     设备SDK微内核，支持模块扩展的头文件
*  \author    panlong
*  \date      2017/3/2
*/
#ifndef H_EZDEV_SDK_KERNEL_EXTEND_H_
#define H_EZDEV_SDK_KERNEL_EXTEND_H_




#define EZDEV_SDK_KERNEL_EXTEND_INTERFACE	\
	extern void extend_init(sdk_kernel_event_notice kernel_event_notice_cb); \
	extern void extend_fini(void); \
	extern mkernel_internal_error extend_load(const ezdev_sdk_kernel_extend* external_extend); \
	extern mkernel_internal_error extend_load_v3(const ezdev_sdk_kernel_extend_v3* external_extend_v3); \
	extern mkernel_internal_error extend_yield(ezdev_sdk_kernel* sdk_kernel);\
	extern ezdev_sdk_kernel_domain_info* extend_get(EZDEV_SDK_UINT32 domain_id);  \
	extern ezdev_sdk_kernel_domain_info_v3* extend_get_by_extend_id(const char* module); \
	extern mkernel_internal_error extend_serialize_sdk_version(bscJSON * pJsonRoot);  \
    extern mkernel_internal_error clear_queue_pubmsg_exchange(); \
	extern mkernel_internal_error clear_queue_submsg(); \
	extern mkernel_internal_error clear_queue_pubmsg_exchange_v3(); \
	extern mkernel_internal_error clear_queue_submsg_v3(); \

	
#endif //H_EZDEV_SDK_KERNEL_EXTEND_H_