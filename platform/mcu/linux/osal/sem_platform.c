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

#include "sem_interface.h"
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>



ez_sem_t ez_sem_create(void)
{
    sem_t *semaphore = NULL;
    semaphore = (sem_t *)malloc(sizeof(sem_t));
    if(semaphore == NULL){
        return NULL;
    }
    if(sem_init(semaphore, 0, 0)){
        free(semaphore);
        return NULL;
    }

    return (ez_sem_t)semaphore;
}

int ez_sem_destory(ez_sem_t sem)
{
    sem_t *semaphore = (sem_t *)sem;

	if (semaphore == NULL){
		return -1;
	}
    if(sem_destroy(semaphore)){
        free(semaphore);
        semaphore = NULL;
        return -1;
    }
    free(semaphore);
    semaphore = NULL;
    return 0;
}

int ez_sem_wait(ez_sem_t sem, int timewait_ms)
{
    struct timespec time;
    sem_t *semaphore = (sem_t *)sem;
	if (semaphore == NULL){
		return -1;
	}
    if(timewait_ms == -1)
        return sem_wait(semaphore);

    clock_gettime(CLOCK_MONOTONIC, &time);
    time.tv_sec += timewait_ms/1000;
    time.tv_nsec += ((timewait_ms%1000) *1000000);
    return sem_timedwait(semaphore, &time);
}

int ez_sem_post(ez_sem_t sem)
{
    sem_t *semaphore = (sem_t *)sem;
	if (semaphore == NULL){
		return -1;
	}

    return sem_post(semaphore);
}


