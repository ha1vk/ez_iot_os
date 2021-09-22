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
#include "sem_interface.h"
#include <stdio.h>
#include <stdlib.h>


EZ_OS_API_EXTERN ez_sem_t EZ_OS_API_CALL ez_sem_create(void)
{
    HANDLE  semaphore = NULL;
    semaphore = CreateSemaphore(NULL, 0, 1, NULL);
    if(semaphore == NULL)
    {
		printf("semaphore create error\n");
        return NULL;
    }

    return (ez_sem_t)semaphore;
}

EZ_OS_API_EXTERN int EZ_OS_API_CALL ez_sem_destory(ez_sem_t sem)
{
    HANDLE  semaphore = (HANDLE )sem;

	if (semaphore == NULL){
		return -1;
	}
    
    CloseHandle(semaphore);
    semaphore = NULL;
    return 0;
}

EZ_OS_API_EXTERN int EZ_OS_API_CALL ez_sem_wait(ez_sem_t sem, int timewait_ms)
{
    HANDLE  semaphore = (HANDLE)sem;
	if (semaphore == NULL)
	{
		return -1;
	}

    DWORD dwMilliseconds = (timewait_ms > 0) ? timewait_ms : INFINITE;
    return (WAIT_OBJECT_0 == WaitForSingleObject(semaphore, dwMilliseconds)) ? 0 : -1;
}

EZ_OS_API_EXTERN int EZ_OS_API_CALL ez_sem_post(ez_sem_t sem)
{
    HANDLE semaphore = (HANDLE)sem;
    if (semaphore == NULL)
    {
        return -1;
    }

    return ReleaseSemaphore(semaphore, 1, NULL) ? 0 : -1;
}


