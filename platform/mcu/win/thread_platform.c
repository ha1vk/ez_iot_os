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
 *******************************************************************************/
#include <Windows.h>
#include <stdlib.h>
#include <string.h>
#include <process.h>
#include "thread_interface.h"
#include "io_interface.h"

struct targ
{
    char name[64];
    void (*fn)(void *);
    void *arg;
};

static unsigned int __stdcall sdk_thread_fun(void *aArg)
{
	struct targ *hd = (struct targ *)aArg;
	if ((hd != NULL) && (hd->fn != NULL))
    {
        ez_printf("start thread:%s\n", hd->name);
        hd->fn(hd->arg);
	}

	return 0;
}

EZ_OS_API_EXTERN ez_thread_t EZ_OS_API_CALL ez_thread_create(ez_task_init_parm *taskParam)
{
	struct targ *targ = NULL;
	unsigned int threadID = 0;
	HANDLE handle = NULL;

    do 
    {
        if (taskParam == NULL)
        {
            ez_printf("taskParam is null!\n");
            break;
        }

        if ((taskParam->task_fun == NULL) || (strlen(taskParam->task_name) == 0))
        {
            ez_printf("taskParam error\n");
            break;
        }

        targ = (struct targ *)malloc(sizeof(struct targ));
        if (targ == NULL)
        {
            break;
        }

        memset(targ, 0, sizeof(struct targ));
        memcpy(targ->name, taskParam->task_name, strlen(taskParam->task_name));
        targ->fn = taskParam->task_fun;
        targ->arg = taskParam->task_arg;

        uintptr_t ret = _beginthreadex(NULL, 0, sdk_thread_fun, targ, 0, &threadID);
        if (0 != ret)
        {
            handle = (HANDLE)ret;
        }
    } while (0);

    if (NULL == handle)
    {
        free(targ);
    }

	return handle;
}

EZ_OS_API_EXTERN int EZ_OS_API_CALL ez_thread_detach(ez_thread_t handle)
{
	HANDLE hd = (HANDLE)handle;
	if (hd == NULL)
	{
		return -1;
	}

	CloseHandle(hd);
	hd = NULL;

	return 0;
}

EZ_OS_API_EXTERN int EZ_OS_API_CALL ez_thread_destroy(ez_thread_t handle)
{
	HANDLE hd = (HANDLE)handle;
	if (hd == NULL)
    {
		return -1;
	}

    if (WAIT_OBJECT_0 == WaitForSingleObject(hd, INFINITE))
    {
        CloseHandle(hd);
        hd = NULL;
    }

    return 0;
}

EZ_OS_API_EXTERN unsigned int EZ_OS_API_CALL ez_thread_self()
{
	return (int)GetCurrentThreadId();
}

EZ_OS_API_EXTERN ez_mutex_t EZ_OS_API_CALL ez_mutex_create(void)
{
	CRITICAL_SECTION* mtx = NULL;
	mtx = (CRITICAL_SECTION*)malloc(sizeof(CRITICAL_SECTION));
	if (mtx == NULL){
		return NULL;
	}
	InitializeCriticalSection(mtx);
	return (ez_mutex_t)mtx;
}

EZ_OS_API_EXTERN int EZ_OS_API_CALL ez_mutex_destory(ez_mutex_t mutex)
{
	CRITICAL_SECTION* mtx = (CRITICAL_SECTION*)mutex;
	if (mtx == NULL){
		return -1;
	}

	DeleteCriticalSection(mtx);
	free(mtx);
	mtx = NULL;
	return 0;
}

EZ_OS_API_EXTERN int EZ_OS_API_CALL ez_mutex_lock(ez_mutex_t mutex)
{
	CRITICAL_SECTION* mtx = (CRITICAL_SECTION*)mutex;
	if (mtx == NULL){
		return -1;
	}

	EnterCriticalSection(mtx);
	return 0;
}

EZ_OS_API_EXTERN int EZ_OS_API_CALL ez_mutex_unlock(ez_mutex_t mutex)
{
	CRITICAL_SECTION* mtx = (CRITICAL_SECTION*)mutex;
	if (mtx == NULL){
		return -1;
	}

	LeaveCriticalSection(mtx);
	return 0;
}