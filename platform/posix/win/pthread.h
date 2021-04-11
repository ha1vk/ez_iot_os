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
#ifndef _H_PTHREAD_H_
#define _H_PTHREAD_H_

#include <process.h>
#include <windows.h>


typedef void*		VOIDPTR;

typedef struct 
{
	void* mutex_attr;
}pthread_mutexattr_t;

typedef struct 
{
	HANDLE lock;
}pthread_mutex_t;

typedef struct 
{
	int stack_size;
}pthread_attr_t;

typedef struct 
{
	HANDLE thread_hd;
	void* arg;
	void* (CALLBACK *task)(void*user_data);
}pthread_t;


#ifdef __cplusplus
extern "C" {
#endif 

 int pthread_create(pthread_t* handle, const pthread_attr_t* attr, VOIDPTR (CALLBACK *StartAddress)(VOIDPTR), void* arg );

 int pthread_join(pthread_t handle, void **retval);

 int pthread_detach(pthread_t handle);

 int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *restrict_attr);

 int pthread_mutex_destroy(pthread_mutex_t* mutex);

 int pthread_mutex_lock(pthread_mutex_t *mutex);

 int pthread_mutex_unlock(pthread_mutex_t *mutex);



#ifdef __cplusplus
}
#endif 

#endif//_H_PTHREAD_H_



