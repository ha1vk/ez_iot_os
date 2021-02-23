/**
 * \file		thread_interface.h
 *
 * \brief		线程相关操作接口，不同平台在同层次目录都有对应的实现
 *
 * \copyright	HangZhou Hikvision System Technology Co.,Ltd. All Right Reserved.
 *
 * \author		xurongjun
 *
 * \date		2018/6/27
 */
#ifndef H_THREAD_INTERFACE_H_
#define H_THREAD_INTERFACE_H_

#include "thread_platform_wrapper.h"


/** 
 *  \brief		创建线程
 *  \method		sdk_thread_create
 *  \param[in] 	handle 线程上下文
 *  \return 	成功返回0 失败返回-1
 */
int sdk_thread_create(thread_handle* handle);

/** 
 *  \brief		线程销毁
 *  \method		sdk_thread_destroy
 *  \param[in] 	handle 线程上下文
 *  \return 	成功返回0 失败返回-1
 */
int sdk_thread_destroy(thread_handle* handle);

/** 
 *  \brief		线程休眠
 *  \method		sdk_thread_sleep
 *  \param[in] 	time_ms 休眠时间（毫秒）
 */
void sdk_thread_sleep(unsigned int time_ms);

//typedef struct thread_handle_platform thread_handle;

#endif