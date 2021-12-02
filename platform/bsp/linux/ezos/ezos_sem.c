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
 * Time related interface declaration
 * 
 * Change Logs:
 * Date           Author       Notes
 * 2021-10-27     zoujinwei    first version
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
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    unsigned short count;
    unsigned short maxCount;
} sem_data_t;

ez_sem_t ezos_sem_create(unsigned short count, unsigned short maxCount)
{
    sem_data_t *psem_data = (sem_data_t *)malloc(sizeof(sem_data_t));
    if (psem_data == NULL)
    {
        return NULL;
    }

    pthread_condattr_t m_condAttr;
    memset(&m_condAttr, 0, sizeof(pthread_condattr_t));
    pthread_condattr_setclock(&m_condAttr, CLOCK_MONOTONIC);

    pthread_mutex_init(&(psem_data->mutex), NULL);
    pthread_cond_init(&(psem_data->cond), &m_condAttr);
    psem_data->count = count;
    psem_data->maxCount = maxCount;

    return (ez_sem_t *)psem_data;
}

int ezos_sem_destroy(ez_sem_t sem)
{
    sem_data_t *psem_data = (sem_data_t *)sem;
    if (psem_data == NULL)
    {
        return -1;
    }

    pthread_mutex_destroy(&psem_data->mutex);
    pthread_cond_destroy(&psem_data->cond);
    free(psem_data);

    return 0;
}

int ezos_sem_wait(ez_sem_t sem, int timewait_ms)
{
    int ret = -1;
    struct timespec tv = {0};

    sem_data_t *psem_data = (sem_data_t *)sem;
    if (psem_data == NULL)
    {
        return -1;
    }

    if (0 != clock_gettime(CLOCK_MONOTONIC, &tv))
    {
        return -1;
    }

    pthread_mutex_lock(&psem_data->mutex);
    if (psem_data->count > 0)
    {
        psem_data->count--;
    }

    tv.tv_sec += timewait_ms / 1000;
    tv.tv_nsec += timewait_ms % 1000 * 1000000;
    if (ETIMEDOUT != pthread_cond_timedwait(&psem_data->cond, &psem_data->mutex, &tv))
    {
        ret = 0;
    }

    pthread_mutex_unlock(&psem_data->mutex);

    return ret;
}

int ezos_sem_post(ez_sem_t sem)
{
    sem_data_t *psem_data = (sem_data_t *)sem;
    if (psem_data == NULL)
    {
        return -1;
    }

    pthread_mutex_lock(&psem_data->mutex);
    if (psem_data->count == psem_data->maxCount)
    {
        pthread_mutex_unlock(&psem_data->mutex);
        return -1;
    }

    psem_data->count++;
    pthread_cond_signal(&psem_data->cond);
    pthread_mutex_unlock(&psem_data->mutex);

    return 0;
}
