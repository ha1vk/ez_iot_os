

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
 *
 * Contributors:
 * xurongjun (xurongjun@ezvizlife.com)
 * 
 * Change Logs:
 * Date           Author       Notes
 * 2021-11-15     xurongjun    first version
*******************************************************************************/

#ifndef _EZOS_THREAD_H_
#define _EZOS_THREAD_H_

#include <ezos_def.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef void *ez_thread_t;
    typedef void *ez_mutex_t;
    typedef void (*ez_thread_func_t)(void *param);

    /**
     * @brief This function will create a thread object
     * 
     * @param handle if non-null, return the thread handle, if null, marks the thread as detached, the resources whill released back to the system automatically
     * @param name the name of thread, which shall be unique
     * @param thread_fun the function of thread
     * @param param thread parameter of thread function
     * @param stack_size the size of thread stack
     * @param priority the size of thread stack
     * @return 0 for success, -1 for failure 
     */
    EZOS_API int ezos_thread_create(ez_thread_t *const handle, const char *name, ez_thread_func_t thread_fun,
                                    const void *param, unsigned int stack_size, unsigned int priority);

    /**
     * @brief The function waits for the thread specified by handle to terminate.  If that thread has
     *        already terminated, returns immediately. 
     * 
     * @param handle thread handle
     * @return 0 for success, -1 for failure 
     */
    EZOS_API int ezos_thread_destroy(ez_thread_t handle);

    /** 
     * @brief  Get the unique ID of the calling thread
     * 
     * @return returning the calling thread's ID
     */
    EZOS_API int ezos_thread_self(void);

    /**
     * @brief 
     * 
     * @return non-null for success, null for failure
     */
    EZOS_API ez_mutex_t ezos_mutex_create(void);

    /**
     * @brief 
     * 
     * @param mutex 
     * @return 0 for success, -1 for failure
     */
    EZOS_API int ezos_mutex_destroy(ez_mutex_t mutex);

    /**
     * @brief 
     * 
     * @param mutex 
     * @return 0 for success, -1 for failure
     */
    EZOS_API int ezos_mutex_lock(ez_mutex_t mutex);

    /**
     * @brief 
     * 
     * @param mutex 
     * @return 0 for success, -1 for failure
     */
    EZOS_API int ezos_mutex_unlock(ez_mutex_t mutex);

#ifdef __cplusplus
}
#endif

#endif //H_EZOS_THREAD_H_