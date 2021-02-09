/**
*  \file	  lbs_transport.h  
*  \filepath  E:\workdir\小项目\ezDevSDK_v2.0.0\microkernel\src\lbs_transport.h
*  \copyright HangZhou Hikvision System Technology Co.,Ltd. All Right Reserved.
*  \brief     处理和LBS 直接的所有网络交互
*  \author    panlong
*  \date      2017/3/3 
*/
#ifndef H_LBS_TRANSPORT_H_
#define H_LBS_TRANSPORT_H_

#define LBS_TRANSPORT_INTERFACE	 \
	extern ezdev_sdk_kernel_error lbs_redirect(ezdev_sdk_kernel* sdk_kernel); \
	extern ezdev_sdk_kernel_error lbs_redirect_with_auth(ezdev_sdk_kernel* sdk_kernel, EZDEV_SDK_UINT8 nUpper); \
	extern ezdev_sdk_kernel_error lbs_redirect_createdevid_with_auth(ezdev_sdk_kernel* sdk_kernel, EZDEV_SDK_UINT8 nUpper); \
	extern ezdev_sdk_kernel_error lbs_getstun(ezdev_sdk_kernel* sdk_kernel, stun_info* ptr_stun);\
	extern ezdev_sdk_kernel_error cnt_state_lbs_apply_serectkey(ezdev_sdk_kernel* sdk_kernel, EZDEV_SDK_UINT16 *interval, EZDEV_SDK_UINT32 *duration); \
// ezdev_sdk_kernel_error lbs_connect(ezdev_sdk_kernel* sdk_kernel, const char* lbs_name, EZDEV_SDK_INT16 lbs_port);
// 
// ezdev_sdk_kernel_error lbs_redirect_with_auth();


#endif //H_LBS_TRANSPORT_H_