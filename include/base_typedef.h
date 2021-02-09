/**
 * \file		base_typedef.h
 *
 * \brief		设备SDK 跨平台基础类型定义
 *
 * \copyright	HangZhou Hikvision System Technology Co.,Ltd. All Right Reserved.
 *
 * \author		panlong
 *
 * \date		2017/3/2
 */

#ifndef H_BASE_TYPEDEF_H_  
#define H_BASE_TYPEDEF_H_

#include <stdio.h>

typedef void*				EZDEV_SDK_PTR;

#define EZDEV_SDK_UNUSED(var)	(void)var;		

#if defined (_WIN32) || defined(_WIN64)
#include <WinSock2.h>
#include <Windows.h>
#include <malloc.h>
typedef signed char         EZDEV_SDK_INT8;			
typedef signed short        EZDEV_SDK_INT16;	
typedef signed int          EZDEV_SDK_INT32;	
typedef __int64             EZDEV_SDK_INT64;	
typedef unsigned char       EZDEV_SDK_UINT8;	
typedef unsigned short      EZDEV_SDK_UINT16;	
typedef unsigned int        EZDEV_SDK_UINT32;	
typedef unsigned __int64    EZDEV_SDK_UINT64;	

#define  snprintf _snprintf

#elif defined (__STDC__)
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#if defined(__STDC_VERSION__) && __STDC_VERSION__>=199901L
/* C99 */
#include <stdint.h>
typedef int8_t				EZDEV_SDK_INT8;
typedef int16_t				EZDEV_SDK_INT16;
typedef int32_t				EZDEV_SDK_INT32;
typedef int64_t				EZDEV_SDK_INT64;
typedef uint8_t				EZDEV_SDK_UINT8;
typedef uint16_t			EZDEV_SDK_UINT16;
typedef uint32_t			EZDEV_SDK_UINT32;
typedef uint64_t			EZDEV_SDK_UINT64;
#elif defined(__STDC_VERSION__) &&__STDC_VERSION__>=199409L
/* C89 */
/* other no stdc */
typedef signed char         EZDEV_SDK_INT8;
typedef signed short        EZDEV_SDK_INT16;
typedef signed int          EZDEV_SDK_INT32;
typedef signed long long             EZDEV_SDK_INT64;
typedef unsigned char       EZDEV_SDK_UINT8;
typedef unsigned short      EZDEV_SDK_UINT16;
typedef unsigned int        EZDEV_SDK_UINT32;
typedef unsigned long long    EZDEV_SDK_UINT64;

#else
 /* C89 but not Amendment 1*/
typedef signed char         EZDEV_SDK_INT8;
typedef signed short        EZDEV_SDK_INT16;
typedef signed int          EZDEV_SDK_INT32;
typedef signed long long    EZDEV_SDK_INT64;
typedef unsigned char       EZDEV_SDK_UINT8;
typedef unsigned short      EZDEV_SDK_UINT16;
typedef unsigned int        EZDEV_SDK_UINT32;
typedef unsigned long long    EZDEV_SDK_UINT64;

#endif

#else
/* other no stdc */
typedef signed char         EZDEV_SDK_INT8;
typedef signed short        EZDEV_SDK_INT16;
typedef signed int          EZDEV_SDK_INT32;
typedef signed long long             EZDEV_SDK_INT64;
typedef unsigned char       EZDEV_SDK_UINT8;
typedef unsigned short      EZDEV_SDK_UINT16;
typedef unsigned int        EZDEV_SDK_UINT32;
typedef unsigned long long    EZDEV_SDK_UINT64;
#endif //_WIN32


typedef EZDEV_SDK_INT8		EZDEV_SDK_BOOL;
#define EZDEV_SDK_TRUE		1
#define	EZDEV_SDK_FALSE		0

#endif //H_EZDEV_SDK_KERNEL_TYPEDEF_H_