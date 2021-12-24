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
#include <stdio.h>
#include <stdlib.h>
#include <rtthread.h>
#include <errno.h>

typedef struct
{
    rt_sem_t sem;
} sem_data_t;

ez_sem_t ezos_sem_create(unsigned short count, unsigned short maxCount)
{
    char name[RT_NAME_MAX];
    static rt_uint16_t psem_number = 0;

    sem_data_t *psem_data = (sem_data_t *)malloc(sizeof(sem_data_t));
    if (psem_data == NULL)
    {
        return NULL;
    }

    rt_snprintf(name, sizeof(name), "psem%02d", psem_number++);
    psem_data->sem = rt_sem_create(name, count, RT_IPC_FLAG_FIFO);

    return (ez_sem_t *)psem_data;
}

int ezos_sem_destroy(ez_sem_t sem)
{
    sem_data_t *psem_data = (sem_data_t *)sem;
    if (psem_data == NULL)
    {
        return -1;
    }

    rt_sem_delete(psem_data->sem);
    free(psem_data);

    return 0;
}

int ezos_sem_wait(ez_sem_t sem, int timewait_ms)
{
    int ret = -1;
    sem_data_t *psem_data = (sem_data_t *)sem;
    if (psem_data == NULL)
    {
        return -1;
    }

    rt_int32_t tick = rt_tick_from_millisecond(timewait_ms);
    if (RT_EOK != rt_sem_take(psem_data->sem, tick))
    {
        return -1;
    }

    return ret;
}

int ezos_sem_post(ez_sem_t sem)
{
    sem_data_t *psem_data = (sem_data_t *)sem;
    if (psem_data == NULL)
    {
        return -1;
    }

    rt_sem_release(psem_data->sem);

    return 0;
}
