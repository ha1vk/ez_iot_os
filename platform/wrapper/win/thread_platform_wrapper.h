#ifndef H_THREAD_PLATFORM_SWAP_H_
#define H_THREAD_PLATFORM_SWAP_H_

#include <Windows.h>

typedef struct 
{
	CRITICAL_SECTION lock;
}sdk_mutex_platform;

typedef struct thread_handle_platform
{
	HANDLE thread_hd;
	void* thread_arg;
	unsigned int (*task_do)(void * user_data);
	char thread_name[16];
}thread_handle;

#endif