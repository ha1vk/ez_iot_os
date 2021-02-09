/**
*  \file      
*  \filename  ezdev_sdk_kernel_risk_control.h  
*  \filepath  y:\makedir\devsdk_v2.0.0\microkernel\src\ezdev_sdk_kernel_risk_control.h
*  \copyright HangZhou Hikvision System Technology Co.,Ltd. All Right Reserved.
*  \brief     ·ç¿Ø¹ÜÀí
*  \author    panlong
*  \date      2017/4/26
*/
#ifndef H_EZDEV_SDK_KERNEL_RISK_CONTROL_H_
#define H_EZDEV_SDK_KERNEL_RISK_CONTROL_H_

#define EZDEV_SDK_KERNEL_RISK_CONTROL_INTERFACE		\
	extern void add_access_risk_control(ezdev_sdk_kernel* sdk_kernel);\
	extern void add_domain_risk_control(ezdev_sdk_kernel* sdk_kernel, EZDEV_SDK_UINT32 domain_id);\
	extern void add_cmd_risk_control(ezdev_sdk_kernel* sdk_kernel, EZDEV_SDK_UINT32 domain_id, EZDEV_SDK_UINT32 cmd_id);\
	extern char check_access_risk_control(ezdev_sdk_kernel* sdk_kernel);\
	extern char check_cmd_risk_control(ezdev_sdk_kernel* sdk_kernel, EZDEV_SDK_UINT32 domain_id, EZDEV_SDK_UINT32 cmd_id);

#endif