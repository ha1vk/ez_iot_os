#ifndef H_TIME_PLATFORM_WRAPPER_H_
#define H_TIME_PLATFORM_WRAPPER_H_

//#include <stdint.h> /* for size_t */
#include "FreeRTOS.h"
#include "task.h"

#include "ezdev_sdk_kernel_struct.h"
/**
* \brief   free rtos й╣ож
*/
/*
typedef struct 
{
	portTickType xTicksToWait;
	xTimeOutType xTimeOut;
}freertos_time;
*/
typedef struct 
{
	uint32_t end_time;
}freertos_time;
#endif