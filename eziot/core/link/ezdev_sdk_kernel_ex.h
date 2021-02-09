/**
 *  \file      
 *  \filename  ezdev_sdk_kernel_ex.h  
 *  \filepath  Y:\makedir\devsdk_v2.0.0\microkernel\src\ezdev_sdk_kernel_ex.h
 *  \copyright HangZhou Hikvision System Technology Co.,Ltd. All Right Reserved.
 *  \brief     微内核额外的一些接口
 *  \author    panlong
 *  \date      2017/6/1
 */

/**
* @addtogroup micro_kernel
*
* @section 微内核额外接口
* 提供一些原本从职责上不属于微内核的功能 
*
*  微内核获取STUN服务器信息接口
* (see \c ezdev_sdk_kernel_get_stun()).
* 
*/

#ifndef H_EZDEV_SDK_KERNEL_EX_H_
#define H_EZDEV_SDK_KERNEL_EX_H_

#include "ezdev_sdk_kernel_struct.h"
#include "ezdev_sdk_kernel_error.h"

#ifdef __cplusplus
extern "C"
{
#endif

/** 
 *  \brief		获取STUN服务器信息接口
 *  \method		ezdev_sdk_kernel_get_stun
 *  \param[in] 	stun_info * ptr_stun
 *  \return 	EZDEV_SDK_KERNEL_API ezdev_sdk_kernel_error
 */
EZDEV_SDK_KERNEL_API ezdev_sdk_kernel_error ezdev_sdk_kernel_get_stun(stun_info* ptr_stun, EZDEV_SDK_BOOL bforce_refresh);

/** 
 *  \brief		设置心跳时间
 *  \method		ezdev_sdk_kernel_set_keepalive_interval
 *  \param[in] 	EZDEV_SDK_UINT16 internal   设置心跳间隔时间
 *  \param[in]  EZDEV_SDK_UINT16 timeout_s  等待服务器响应的超时时间，0表示不等待
 *  \return 	如果不等待响应，信令送进队列返回成功，反之等待平台响应错误码，如果超时，返回等待信令超时。
 */
EZDEV_SDK_KERNEL_API ezdev_sdk_kernel_error ezdev_sdk_kernel_set_keepalive_interval(EZDEV_SDK_UINT16 internal, EZDEV_SDK_UINT16 timeout_s);

#ifdef __cplusplus
}
#endif

#endif