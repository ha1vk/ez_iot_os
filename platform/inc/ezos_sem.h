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
     * @brief 
     * 
     * @return
     */
    EZOS_API ez_sem_t ez_sem_create(void);

    /**
     * @brief 
     * 
     * @param sem 
     * @return  
     */
    EZOS_API int ez_sem_destory(ez_sem_t sem);

    /**
     * @brief 
     * 
     * @param sem 
     * @param timewait_ms 
     * @return  
     */
    EZOS_API int ez_sem_wait(ez_sem_t sem, int timewait_ms);

    /**
     * @brief 
     * 
     * @param sem 
     * @return  
     */
    EZOS_API int ez_sem_post(ez_sem_t sem);

#ifdef __cplusplus
}
#endif

#endif //H_EZOS_SEM_H_
