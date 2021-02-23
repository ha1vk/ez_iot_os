#ifndef H_TIME_PLATFORM_WRAPPER_H_
#define H_TIME_PLATFORM_WRAPPER_H_

#include "esp_common.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

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
	uint32 end_time;
}freertos_time;
#endif