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
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "ez_sdk_log.h"
#include "osal_thread.h"

#define 	FREERTOS_DEFAULT_STACKSIZE		2048

struct targ {
	TaskHandle_t handle;
    void (*fn)(void *);
    void *arg;
};

static void sdk_thread_fun(void *aArg)
{
	struct targ *hd = (struct targ *)aArg;
	if (hd == NULL){
		return NULL;
	}

	if(hd->fn)
		hd->fn(hd->arg);
	if(hd->handle)
		vTaskDelete(hd->handle);
	free(hd);
	return NULL;
}

ez_thread_t ez_thread_create(ez_task_init_parm *taskParam)
{
	struct targ *targ = NULL;
	size_t stackSize;
	int priority;

	if (taskParam == NULL){
		ez_log_e(TAG_SDK,"taskParam NULL\n");
		return NULL;
	}
	if (taskParam->task_fun == NULL || strlen(taskParam->task_name)==0){
		ez_log_e(TAG_SDK,"taskParam error\n");
		return NULL;
	}

	targ = ez_malloc(sizeof(struct targ));
	if(targ == NULL){
		return NULL;
	}
	targ->fn = taskParam->task_fun;
	targ->arg = taskParam->task_arg;
	stackSize = taskParam->stackSize;
	if((stackSize > THREAD_STACKSIZE_MIN) && (stackSize <= THREAD_STACKSIZE_MAX)){
		stackSize = FREERTOS_DEFAULT_STACKSIZE;
	}
	priority = taskParam->priority;
	if(priority <= 0 || priority >= 256){
		priority = 5;
	}
	ez_log_i(TAG_SDK,"create thread:%s\n", taskParam->name);
	if(!xTaskCreate(sdk_thread_fun, taskParam->task_name, taskParam->stackSize, targ, taskParam->priority, &targ->handle)){
		return NULL;
	}

	return targ->handle;
}

int ez_thread_destroy(ez_thread_t handle)
{
    return 0;
}

ez_mutex_t ez_mutex_create(void)
{
	SemaphoreHandle_t mtx = NULL;

    mtx = xSemaphoreCreateMutex();
    if(mtx){
        return NULL;
    }

    return (ez_mutex_t)mtx;
}

int ez_mutex_destory(ez_mutex_t mutex)
{
	SemaphoreHandle_t mtx = (SemaphoreHandle_t *)mutex;

	if (mtx == NULL){
		return -1;
	}
    if(!vSemaphoreDelete(mtx)){
        mtx = NULL;
        return -1;
    }
    mtx = NULL;
    return 0;
}

int ez_mutex_lock(ez_mutex_t mutex)
{
	SemaphoreHandle_t mtx = (SemaphoreHandle_t )mutex;
	if (mtx == NULL){
		return -1;
	}

    return xSemaphoreTake(mtx, portMAX_DELAY) ? 0:-1;
}

int ez_mutex_unlock(ez_mutex_t mutex)
{
	SemaphoreHandle_t mtx = (SemaphoreHandle_t )mutex;
	if (mtx == NULL){
		return -1;
	}

    return xSemaphoreGive(mtx, 0) ? 0:-1;
}
