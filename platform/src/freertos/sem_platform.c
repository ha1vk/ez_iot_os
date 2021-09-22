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

#include "ez_sdk_log.h"
#include "sem_interface.h"
#include "semphr.h"

ez_sem_t ez_sem_create(void)
{
    SemaphoreHandle_t semaphore = NULL;

    semaphore = xSemaphoreCreateBinary();
    if(semaphore){
        return NULL;
    }

    return (ez_sem_t)semaphore;
}

int ez_sem_destory(ez_sem_t sem)
{
    SemaphoreHandle_t semaphore = (SemaphoreHandle_t *)sem;

	if (semaphore == NULL){
		return -1;
	}
    if(!vSemaphoreDelete(semaphore)){
        semaphore = NULL;
        return -1;
    }
    semaphore = NULL;
    return 0;
}

int ez_sem_wait(ez_sem_t sem)
{
    SemaphoreHandle_t semaphore = (SemaphoreHandle_t )sem;
	if (semaphore == NULL){
		return -1;
	}

    return xSemaphoreTake(semaphore, 0) ? 0:-1;
}

int ez_sem_post(ez_sem_t sem)
{
    SemaphoreHandle_t semaphore = (SemaphoreHandle_t)sem;
	if (semaphore == NULL){
		return -1;
	}

    return xSemaphoreGive(semaphore) ? 0:-1;
}


