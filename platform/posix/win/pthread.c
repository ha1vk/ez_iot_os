
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
 * Contributors:
 *    shenhongyin - initial API and implementation and/or initial documentation
 *******************************************************************************/
#include <Windows.h>
#include "pthread.h"

static unsigned int __stdcall sdk_thread_fun( void * arg)
{
	pthread_t* hd = (pthread_t*)arg;
	if (hd == NULL)
	{
		return 0;
	}
	hd->task(hd->arg);
	return 0;
}

 int pthread_create(pthread_t *handle, const  pthread_attr_t *attr, VOIDPTR (CALLBACK *fun)(VOIDPTR), void*arg)
{
    unsigned int threadID = 0;
    
    handle->arg = arg;
    handle->task = fun;
	handle->thread_hd = (HANDLE)_beginthreadex(NULL, attr->stack_size, sdk_thread_fun, handle, 0, &threadID);
	if(handle->thread_hd == NULL)
	{
		return -1;
	}
	return 0;
}

 int pthread_detach(pthread_t handle)
{

    if (handle.thread_hd == NULL)
	{
		return -1;
	}
	 
	CloseHandle(handle.thread_hd);
	handle.thread_hd = NULL;
    
    return 0;
}


 int pthread_join(pthread_t handle, void **retval)
{

    if (handle.thread_hd == NULL)
	{
		return -1;
	}
	
	WaitForSingleObject(handle.thread_hd, INFINITE);
	CloseHandle(handle.thread_hd);
	handle.thread_hd = NULL;
    return 0;
}

 int pthread_mutex_destroy(pthread_mutex_t* mutex)
 {
     if(!mutex||NULL == mutex->lock)
     {
         return -1;
     }
     CloseHandle(mutex->lock);
     return 0;
 }

 int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *restrict_attr)
{
   
	if (mutex == NULL)
	{
		return -1;
	}
    mutex->lock = CreateMutex(NULL, FALSE, NULL);
    if(NULL == mutex->lock)
    {
        return -1;
    }

	return 0;
}


 int pthread_mutex_lock(pthread_mutex_t *mutex)
{
	 if (mutex == NULL||NULL == mutex->lock)
	{
		return -1;
	}
	WaitForSingleObject(mutex->lock, INFINITE);
	return 0;
}

 int pthread_mutex_unlock(pthread_mutex_t *mutex)
{
    if (mutex == NULL||NULL == mutex->lock)
	{
		return -1;
	}
	ReleaseMutex(mutex->lock);
	return 0;
}


