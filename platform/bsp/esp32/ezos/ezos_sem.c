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
 * 2021-12-31     zhangdi29    first version
 *******************************************************************************/

#include "ezos_sem.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <string.h>
#include <time.h>
#include <errno.h>


typedef struct
{
    xSemaphoreHandle sem;
} sem_data_t;


ez_sem_t ezos_sem_create(unsigned short count, unsigned short maxCount)
{
    sem_data_t *psem_data = (sem_data_t *)malloc(sizeof(sem_data_t));
    if (psem_data == NULL)
    {
        return NULL;
    }

    psem_data->sem = xSemaphoreCreateCounting(maxCount, count);
    
    if (psem_data->sem == NULL)
    {
        free(psem_data);
        psem_data = NULL;
    }

    return (ez_sem_t *)psem_data;
}

int ezos_sem_destroy(ez_sem_t sem)
{
    sem_data_t *psem_data = (sem_data_t *)sem;
    if (psem_data == NULL)
    {
        return -1;
    }

    vSemaphoreDelete(psem_data->sem);
    free(psem_data);

    psem_data = NULL;

    return 0;
}

int ezos_sem_wait(ez_sem_t sem, int timewait_ms)
{
    sem_data_t *psem_data = (sem_data_t *)sem;
    if (psem_data == NULL)
    {
        return -1;
    }
    

    if (xSemaphoreTake(psem_data->sem, timewait_ms / portTICK_RATE_MS) == pdTRUE)
    {
        return 0;
    }

    return -1;
}


int ezos_sem_post(ez_sem_t sem)
{
    sem_data_t *psem_data = (sem_data_t *)sem;
    if (psem_data == NULL)
    {
        return -1;
    }

    return xSemaphoreGive(psem_data->sem);
}


