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
 * 2021-10-27     zoujinwei    first version
 *******************************************************************************/
#include "ezos_sem.h"

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include <string.h>


typedef struct
{
    xSemaphoreHandle sem;
} sdk_sem_platform;

ez_sem_t ezos_sem_create(unsigned short count, unsigned short maxCount)
{
    sdk_sem_platform *ptr_sem_platform = NULL;
    ptr_sem_platform = (sdk_sem_platform *)malloc(sizeof(sdk_sem_platform));
    if (ptr_sem_platform == NULL)
    {
        return NULL;
    }

    ptr_sem_platform->sem = xSemaphoreCreateBinary();
    if (ptr_sem_platform->sem == NULL)
    {
        free(ptr_sem_platform);
        ptr_sem_platform = NULL;
    }

    return (void *)ptr_sem_platform;
}


int ezos_sem_destroy(ez_sem_t sem)
{
    sdk_sem_platform *ptr_sem_platform = (sdk_sem_platform *)sem;
    if (ptr_sem_platform == NULL)
    {
        return -1;
    }

    vSemaphoreDelete(ptr_sem_platform->sem);
    free(ptr_sem_platform);

    ptr_sem_platform = NULL;
    return 0;
}


int ezos_sem_wait(ez_sem_t sem, int timewait_ms)
{
    sdk_sem_platform *ptr_sem_platform = (sdk_sem_platform *)sem;
    if (ptr_sem_platform == NULL)
    {
        return -1;
    }

    if (xSemaphoreTake(ptr_sem_platform->sem, timewait_ms / portTICK_RATE_MS) == pdTRUE)
    {
        return 0;
    }

    return -1;
}


int ezos_sem_post(ez_sem_t sem)
{
    sdk_sem_platform *ptr_sem_platform = (sdk_sem_platform *)sem;
    if (ptr_sem_platform == NULL)
    {
        return -1;
    }

    return xSemaphoreGive(ptr_sem_platform->sem);
}

