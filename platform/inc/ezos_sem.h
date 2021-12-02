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
* Brief:
* 
* 
* Change Logs:
* Date           Author       Notes
* 2021-11-15     xurongjun    first vision
*******************************************************************************/

#ifndef H_EZOS_SEM_H_
#define H_EZOS_SEM_H_

#include <ezos_def.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef void *ez_sem_t;

    /**
     * @brief create the Semaphore
     * 
     * @param count Initial count 
     * @param maxCount Max count
     * @return  !NULL for success, or NULL for failure
     */
    EZOS_API ez_sem_t ezos_sem_create(unsigned short count, unsigned short maxCount);

    /**
     * @brief Delete the semaphore
     * 
     * @param sem semaphore operation handle
     * @return 0 for success, or -1 for failure
     */
    EZOS_API int ezos_sem_destroy(ez_sem_t sem);

    /**
     * @brief wait for semaphore
     * 
     * @param sem semaphore operation handle
     * @param timewait_ms waitting time
     * @return  0 for success, or -1 for failure
     */
    EZOS_API int ezos_sem_wait(ez_sem_t sem, int timewait_ms);

    /**
     * @brief post semaphore
     * 
     * @param sem semaphore operation handle
     * @return 0 for success, or -1 for failure
     */
    EZOS_API int ezos_sem_post(ez_sem_t sem);

#ifdef __cplusplus
}
#endif

#endif //H_EZOS_SEM_H_
