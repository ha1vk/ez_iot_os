/*******************************************************************************
 * Copyright © 2017-2021 Ezviz Inc.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *******************************************************************************/

#ifndef H_THREAD_INTERFACE_H_
#define H_THREAD_INTERFACE_H_

#include "thread_platform_wrapper.h"

typedef void * ez_thread_t;
typedef void * ez_mutex_t;

typedef struct{
	void (* task_fun)(void *task_arg);	//线程入口函数
	void *task_arg;						//入口函数的传入参数
	unsigned char priority;				//线程的优先级
	unsigned int stackSize;				//程堆栈的大小
	char task_name[64];					//线程名称
	unsigned int tick;					//线程的时间片大小
}ez_task_init_parm;

/** 
 *  \brief		创建线程
 *  \method		ez_thread_create
 *  \param[in] 	taskParam 线程上下文
 *  \return 	成功返回线程句柄 失败返回NULL
 */
ez_thread_t ez_thread_create(ez_task_init_parm *taskParam);

/** 
 *  \brief		线程销毁
 *  \method		ez_thread_destroy
 *  \param[in] 	handle 线程上下文
 *  \return 	成功返回0 失败返回-1
 */
int ez_thread_destroy(ez_thread_t handle);


ez_mutex_t ez_mutex_create(void);

int ez_mutex_destory(ez_mutex_t mutex);

int ez_mutex_lock(ez_mutex_t mutex);

int ez_mutex_unlock(ez_mutex_t mutex);

int ez_delay_ms(unsigned int time_ms);
//typedef struct thread_handle_platform thread_handle;

#endif