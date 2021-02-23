#ifndef H_THREAD_PLATFORM_SWAP_H_
#define H_THREAD_PLATFORM_SWAP_H_

#include "esp_common.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

/**
 * \brief   free rtos й╣ож
 */

typedef struct  
{
	xSemaphoreHandle lock;
}sdk_mutex_platform;

typedef struct thread_handle_platform
{
	xTaskHandle thread_hd; ;
	void* thread_arg;
	void (*task_do)(void * user_data);
	char thread_name[16];
}thread_handle;

#endif