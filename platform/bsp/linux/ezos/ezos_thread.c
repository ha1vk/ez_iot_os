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
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/prctl.h>
#include <pthread.h>

#include "ezos_thread.h"

static pthread_mutex_t *g_create_thread_mutex = NULL;

struct targ
{
    char name[64];
    pthread_t *pthread;
    void (*fn)(void *);
    void *arg;
};

static void *sdk_thread_fun(void *aArg)
{
    struct targ *hd = (struct targ *)aArg;
    if (hd == NULL)
    {
        return NULL;
    }

    prctl(PR_SET_NAME, hd->name);
    printf("start thread:%s\n", hd->name);
    if (hd->fn)
        hd->fn(hd->arg);

    free(hd);
    hd = NULL;
    printf("exit thread:%s\n", hd->name);
    return NULL;
}

EZOS_API ez_err_t EZOS_CALL ezos_thread_create(ez_thread_t *const handle, const ez_char_t *name, ez_thread_func_t thread_fun,
                                               const ez_void_t *param, ez_uint32_t stack_size, ez_uint32_t priority)
{
    // pthread_t *pt;
    // pthread_attr_t attr;
    // struct targ *targ = NULL;

    // if (g_create_thread_mutex == NULL)
    // {
    //     g_create_thread_mutex = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
    //     pthread_mutex_init(g_create_thread_mutex, NULL);
    // }

    // pthread_mutex_lock(g_create_thread_mutex);
    // if (taskParam == NULL)
    // {
    //     printf("taskParam NULL\n");
    //     pthread_mutex_unlock(g_create_thread_mutex);
    //     return NULL;
    // }

    // if (taskParam->task_fun == NULL || strlen(taskParam->task_name) == 0)
    // {
    //     printf("taskParam error\n");
    //     pthread_mutex_unlock(g_create_thread_mutex);
    //     return NULL;
    // }
    // pthread_attr_init(&attr);
    // if ((taskParam->stackSize > THREAD_STACKSIZE_MIN) && (taskParam->stackSize <= THREAD_STACKSIZE_MAX))
    // {
    //     printf("thread stacksize=%d\n", taskParam->stackSize);
    //     if (0 != pthread_attr_setstacksize(&attr, taskParam->stackSize))
    //     {
    //         printf("setstacksize error\n");
    //         pthread_attr_destroy(&attr);
    //         pthread_mutex_unlock(g_create_thread_mutex);
    //         return NULL;
    //     }
    // }

    // targ = malloc(sizeof(struct targ));
    // if (targ == NULL)
    // {
    //     pthread_mutex_unlock(g_create_thread_mutex);
    //     printf("malloc targ error\n");
    //     return NULL;
    // }
    // pt = (pthread_t *)malloc(sizeof(pthread_t));
    // if (pt == NULL)
    // {
    //     free(targ);
    //     pthread_mutex_unlock(g_create_thread_mutex);
    //     printf("malloc pthread_t error\n");
    //     return NULL;
    // }
    // memset(targ, 0, sizeof(struct targ));
    // memcpy(targ->name, taskParam->task_name, strlen(taskParam->task_name));
    // targ->fn = taskParam->task_fun;
    // targ->arg = taskParam->task_arg;
    // targ->pthread = pt;
    // if (pthread_create(pt, &attr, sdk_thread_fun, (void *)targ) != 0)
    // {
    //     free(pt);
    //     free(targ);
    //     pthread_attr_destroy(&attr);
    //     pthread_mutex_unlock(g_create_thread_mutex);
    //     printf("pthread_create error\n");
    //     return NULL;
    // }

    // pthread_attr_destroy(&attr);
    // pthread_mutex_unlock(g_create_thread_mutex);
    // return pt;
    return 0;
}

EZOS_API int EZOS_CALL ezos_thread_detach(ez_thread_t handle)
{
    pthread_t *pt = (pthread_t *)handle;
    if (pt == NULL)
    {
        return -1;
    }

    pthread_detach(*pt);
    free(pt);
    pt = NULL;
    return 0;
}

EZOS_API int EZOS_CALL ezos_thread_destroy(ez_thread_t handle)
{
    pthread_t *pt = (pthread_t *)handle;
    if (pt == NULL)
    {
        return -1;
    }
    pthread_join(*pt, NULL);
    free(pt);
    pt = NULL;

    return 0;
}

EZOS_API ez_int32_t EZOS_CALL ezos_thread_self()
{
    return pthread_self();
}

EZOS_API ez_mutex_t EZOS_CALL ezos_mutex_create(void)
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

EZOS_API int EZOS_CALL ezos_mutex_destory(ez_mutex_t mutex)
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

EZOS_API int EZOS_CALL ezos_mutex_lock(ez_mutex_t mutex)
{
    pthread_mutex_t *mtx = (pthread_mutex_t *)mutex;
    if (mtx == NULL)
    {
        return -1;
    }

    pthread_mutex_lock(mtx);
    return 0;
}

EZOS_API int EZOS_CALL ezos_mutex_unlock(ez_mutex_t mutex)
{
    pthread_mutex_t *mtx = (pthread_mutex_t *)mutex;
    if (mtx == NULL)
    {
        return -1;
    }

    pthread_mutex_unlock(mtx);
    return 0;
}
