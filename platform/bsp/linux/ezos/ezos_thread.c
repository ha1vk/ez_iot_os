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
 * 2021-11-24     xurongjun    Redeclare the threaded user interface and implement it
 *******************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/prctl.h>
#include <pthread.h>

#include "ezos_thread.h"
#include "ezos_sem.h"

typedef void *(*task_func)(void *user_data);

#define THREAD_STATE_JOINABLE 0
#define THREAD_STATE_DETACHED 1

typedef struct
{
    pthread_t handle;
    char name[16];
    int state;
    ez_thread_func_t thread_fun;
    void *user_param;
} thread_data_t;

static void *sdk_thread_fun(void *aArg)
{
    thread_data_t *pthd = (thread_data_t *)aArg;
    prctl(PR_SET_NAME, pthd->name);

    pthd->thread_fun(pthd->user_param);

    if (THREAD_STATE_DETACHED == pthd->state)
    {
        free(pthd);
    }

    return NULL;
}

EZOS_API int ezos_thread_create(ez_thread_t *const handle, const char *name, ez_thread_func_t thread_fun,
                                const void *param, unsigned int stack_size, unsigned int priority)
{
    pthread_attr_t thread_attr;
    struct sched_param thread_sched_param;

    thread_data_t *pthd = (thread_data_t *)malloc(sizeof(thread_data_t));
    if (NULL == pthd)
    {
        return -1;
    }

    memset(pthd, 0, sizeof(thread_data_t));
    pthd->thread_fun = thread_fun;
    pthd->user_param = (void *)param;
    strncpy(pthd->name, (char *)name, sizeof(pthd->name) - 1);
    pthread_attr_init(&thread_attr);

    if (NULL != handle)
    {
        pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_JOINABLE);
        pthd->state = THREAD_STATE_JOINABLE;
    }
    else
    {
        pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_DETACHED);
        pthd->state = THREAD_STATE_JOINABLE;
    }

    pthread_attr_setstacksize(&thread_attr, stack_size);
    pthread_attr_setschedpolicy(&thread_attr, SCHED_RR);
    thread_sched_param.sched_priority = priority;
    pthread_attr_setschedparam(&thread_attr, &thread_sched_param);

    if (pthread_create(&pthd->handle, &thread_attr, sdk_thread_fun, pthd) != 0)
    {
        pthread_attr_destroy(&thread_attr);
        free(pthd);
        return -1;
    }

    if (NULL != handle)
    {
        *handle = (ez_thread_t)pthd;
    }

    pthread_attr_destroy(&thread_attr);

    return 0;
}

EZOS_API int ezos_thread_destroy(ez_thread_t handle)
{
    thread_data_t *pthd = (thread_data_t *)handle;
    if (NULL == pthd)
    {
        return -1;
    }

    pthread_join(pthd->handle, NULL);
    free(pthd);

    return 0;
}

EZOS_API ez_int32_t ezos_thread_self()
{
    return pthread_self();
}

EZOS_API ez_mutex_t ezos_mutex_create(void)
{
    pthread_mutex_t *mtx = NULL;
    mtx = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
    if (mtx == NULL)
    {
        return NULL;
    }

    pthread_mutex_init(mtx, NULL);

    return (ez_mutex_t)mtx;
}

EZOS_API int ezos_mutex_destroy(ez_mutex_t mutex)
{
    pthread_mutex_t *mtx = (pthread_mutex_t *)mutex;
    if (mtx == NULL)
    {
        return -1;
    }

    pthread_mutex_destroy(mtx);
    free(mtx);
    mtx = NULL;
    return 0;
}

EZOS_API int ezos_mutex_lock(ez_mutex_t mutex)
{
    pthread_mutex_t *mtx = (pthread_mutex_t *)mutex;
    if (mtx == NULL)
    {
        return -1;
    }

    pthread_mutex_lock(mtx);
    return 0;
}

EZOS_API int ezos_mutex_unlock(ez_mutex_t mutex)
{
    pthread_mutex_t *mtx = (pthread_mutex_t *)mutex;
    if (mtx == NULL)
    {
        return -1;
    }

    pthread_mutex_unlock(mtx);
    return 0;
}
