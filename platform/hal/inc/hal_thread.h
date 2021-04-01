
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
* Contributors:
 *    shenhongyin - initial API and implementation and/or initial documentation
 *******************************************************************************/
#ifndef _HAL_THREAD_H_
#define _HAL_THREAD_H_
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif


typedef void (*hal_thread_fun_t)(void *user_data);

/**
 * @brief 
 * 
 * @param handle 
 * @return 
 */
void *hal_thread_create(int8_t *thread_name, hal_thread_fun_t thread_fun, int32_t stack_size, int32_t priority, void *arg);

/**
 * @brief 
 * 
 * @param handle 
 * @return int 
 */
 int hal_thread_destroy(void *handle);

/**
 * @brief 
 * 
 * @param handle 
 * @return int 
 */
 int hal_thread_detach(void *handle);

/**
 * @brief 
 * 
 * @return void* 
 */
void *hal_thread_mutex_create();

/**
 * @brief 
 * 
 * @param ptr_mutex 
 */
void hal_thread_mutex_destroy(void *ptr_mutex);

/**
 * @brief 
 * 
 * @param ptr_mutex 
 * @return int 
 */
int hal_thread_mutex_lock(void *ptr_mutex);

/**
 * @brief 
 * 
 * @param ptr_mutex 
 * @return int 
 */
int hal_thread_mutex_unlock(void *ptr_mutex);

/**
 * @brief 
 * 
 * @param time_ms 
 */
void hal_thread_sleep(unsigned int time_ms);

#ifdef __cplusplus
}
#endif

#endif