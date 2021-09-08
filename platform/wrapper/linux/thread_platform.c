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
#include <sys/prctl.h>
#include <pthread.h>

#include "thread_interface.h"

static void *sdk_thread_fun(void *aArg)
{
	ez_task_init_parm *hd = (ez_task_init_parm *)aArg;
	if (hd == NULL){
		return NULL;
	}

	prctl(PR_SET_NAME, hd->task_name);
	hd->task_fun(hd->task_arg);

	return NULL;
}

ez_thread_t ez_thread_create(ez_task_init_parm *taskParam)
{
	pthread_t *hd;
	pthread_attr_t attr;

	if (taskParam == NULL){
		return NULL;
	}
	
    pthread_attr_init(&attr);  
	if(0 != pthread_attr_setstacksize(&attr, taskParam->stackSize))
    {
	    pthread_attr_destroy(&attr);
		return NULL;            
    }    

	hd = (pthread_t *)malloc(sizeof(pthread_t));
	if(hd == NULL){
		return NULL;
	}
	if (pthread_create(hd, &attr, sdk_thread_fun, (void *)taskParam) != 0)
	{
		free(hd);
		return NULL;
	}

	return hd;
}

int ez_thread_destroy(ez_thread_t handle)
{
	pthread_t *hd;
	if (handle == NULL){
		return -1;
	}
	hd = (pthread_t *)handle;
	void *pres = NULL;
	pthread_join(*hd, &pres);
	return 0;
}

ez_mutex_t ez_mutex_create(void)
{
	pthread_mutex_t *mtx = NULL;
	mtx = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
	if (mtx == NULL){
		return NULL;
	}

	pthread_mutex_init(mtx, NULL);

	return (ez_mutex_t)mtx;
}

int ez_mutex_destory(ez_mutex_t mutex)
{
	pthread_mutex_t *mtx = (pthread_mutex_t *)mutex;
	if (mtx == NULL){
		return -1;
	}

	pthread_mutex_destroy(mtx);
	free(mtx);
	mtx = NULL;
	return 0;
}

int ez_mutex_lock(ez_mutex_t mutex)
{
	pthread_mutex_t *mtx = (pthread_mutex_t *)mutex;
	if (mtx == NULL){
		return -1;
	}

	pthread_mutex_lock(mtx);
	return 0;
}

int ez_mutex_unlock(ez_mutex_t mutex)
{
	pthread_mutex_t *mtx = (pthread_mutex_t *)mutex;
	if (mtx == NULL){
		return -1;
	}

	pthread_mutex_unlock(mtx);
	return 0;
}

int ez_delay_ms(unsigned int time_ms)
{
	usleep((int)(time_ms * 1000));
}
