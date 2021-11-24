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

#include <Windows.h>
#include <stdlib.h>
#include <string.h>
#include <process.h>
#include "ezos_thread.h"
#include "ezos_io.h"

struct targ
{
    char name[64];
    void (EZ_OS_API_CALL *fn)(void *);
    void *arg;
};

static unsigned int sdk_thread_fun(void *aArg)
{
	struct targ *hd = (struct targ *)aArg;
	if ((hd != NULL) && (hd->fn != NULL))
    {
        ezos_printf("start thread:%s\n", hd->name);
        hd->fn(hd->arg);
	}

	return 0;
}

EZOS_API ez_thread_t ezos_thread_create(ez_task_init_parm *taskParam)
{
	struct targ *targ = NULL;
	unsigned int threadID = 0;
	HANDLE handle = NULL;

    do 
    {
        if (taskParam == NULL)
        {
            ezos_printf("taskParam is null!\n");
            break;
        }

        if ((taskParam->task_fun == NULL) || (strlen(taskParam->task_name) == 0))
        {
            ezos_printf("taskParam error\n");
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

EZOS_API int ezos_thread_detach(ez_thread_t handle)
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

EZOS_API int ezos_thread_destroy(ez_thread_t handle)
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

EZOS_API unsigned int ezos_thread_self()
{
	return (int)GetCurrentThreadId();
}

EZOS_API ez_mutex_t ezos_mutex_create(void)
{
	CRITICAL_SECTION* mtx = NULL;
	mtx = (CRITICAL_SECTION*)malloc(sizeof(CRITICAL_SECTION));
	if (mtx == NULL){
		return NULL;
	}
	InitializeCriticalSection(mtx);
	return (ez_mutex_t)mtx;
}

EZOS_API int ezos_mutex_destroy(ez_mutex_t mutex)
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

EZOS_API int ezos_mutex_lock(ez_mutex_t mutex)
{
	CRITICAL_SECTION* mtx = (CRITICAL_SECTION*)mutex;
	if (mtx == NULL){
		return -1;
	}

	EnterCriticalSection(mtx);
	return 0;
}

EZOS_API int ezos_mutex_unlock(ez_mutex_t mutex)
{
	CRITICAL_SECTION* mtx = (CRITICAL_SECTION*)mutex;
	if (mtx == NULL){
		return -1;
	}

	LeaveCriticalSection(mtx);
	return 0;
}