#ifndef H_THREAD_INTERFACE_H_
#define H_THREAD_INTERFACE_H_

#include "thread_platform_wrapper.h"

typedef struct  sdk_mutex_platform sdk_mutex;

int sdk_thread_mutex_init(sdk_mutex* mutex);
int sdk_thread_mutex_fini(sdk_mutex* mutex);
int sdk_thread_mutex_lock(sdk_mutex* mutex);
int sdk_thread_mutex_unlock(sdk_mutex* mutex);


typedef struct thread_handle_platform thread_handle;

int sdk_thread_create(thread_handle* handle);
int sdk_thread_destroy(thread_handle* handle);

void sdk_thread_sleep(unsigned int time_ms);

#endif