#include "time_platform_wrapper.h"
#include "esp_common.h"
#include "base_typedef.h"

ezdev_sdk_time Platform_TimerCreater()
{
	freertos_time* freertostime = NULL;
	freertostime = (freertos_time*)malloc(sizeof(freertos_time));
	if (freertostime == NULL)
	{
		return NULL;
	}

//	freertostime->xTicksToWait = 0;
//	memset(&freertostime->xTimeOut, '\0', sizeof(freertostime->xTimeOut));
	freertostime->end_time = system_get_time();
	return freertostime;
}

char Platform_TimeIsExpired_Bydiff(ezdev_sdk_time sdktime, EZDEV_SDK_UINT32 time_ms)
{
	freertos_time* freertostime = (freertos_time*)sdktime;
	if (freertostime == NULL)
	{
		return (char)1;
	}


	/*
	if (xTaskCheckForTimeOut(&freertostime->xTimeOut, &freertostime->xTicksToWait) == pdTRUE)
	{
		os_printf("Platform_TimeIsExpired_Bydiff 222222222222222:%x, time_ms:%d \n", freertostime, time_ms);
		vTaskSetTimeOutState(&freertostime->xTimeOut);
		freertostime->xTicksToWait = time_ms / portTICK_RATE_MS;
		
		os_printf("Platform_TimeIsExpired_Bydiff 444444444444444:%x, time_ms:%d \n", freertostime, time_ms);
		return (char)1;
	}
	else
	{
		os_printf("Platform_TimeIsExpired_Bydiff 333333333333333:%x, time_ms:%d \n", freertostime, time_ms);
		return (char)0;
	}
	*/
	uint32 cur_time = system_get_time();
	
	
	if(cur_time - freertostime->end_time >= (time_ms*1000))
	{
		return (char)1;
	}
	else
	{
		return (char)0;
	}
}

char Platform_TimerIsExpired(ezdev_sdk_time sdktime)
{
	freertos_time* freertostime = (freertos_time*)sdktime;
	if (freertostime == NULL)
	{
		return (char)1;
	}
	uint32 cur_time = system_get_time();
//	os_printf("Platform_TimerIsExpired cur_time:%d end_time:%d \n", cur_time, freertostime->end_time);
	if (cur_time >= freertostime->end_time)
	{
		return (char)1;
	}
	else
	{
		return (char)0;
	}
	/*
	if (xTaskCheckForTimeOut(&freertostime->xTimeOut, &freertostime->xTicksToWait) == pdTRUE)
	{
		return (char)1;
	}
	else
	{
		return (char)0;
	}
	*/
}


void Platform_TimerCountdownMS(ezdev_sdk_time sdktime, unsigned int timeout)
{
	freertos_time* freertostime = (freertos_time*)sdktime;
	if (freertostime == NULL)
	{
		return;
	}
	uint32 cur_time = system_get_time();
//	os_printf("Platform_TimerCountdownMS cur_time:%d timeout:%d \n", cur_time, timeout);
	freertostime->end_time = cur_time + timeout*1000;
	//freertostime->xTicksToWait = timeout/portTICK_RATE_MS;
	//vTaskSetTimeOutState(&freertostime->xTimeOut);
}


void Platform_TimerCountdown(ezdev_sdk_time sdktime, unsigned int timeout)
{
	Platform_TimerCountdownMS(sdktime, timeout*1000);
}


EZDEV_SDK_UINT32 Platform_TimerLeftMS(ezdev_sdk_time sdktime)
{
	freertos_time* freertostime = (freertos_time*)sdktime;
	if (freertostime == NULL)
	{
		return 0;
	}
//	xTaskCheckForTimeOut(&freertostime->xTimeOut, &freertostime->xTicksToWait); /* updates xTicksToWait to the number left */
//	return (freertostime->xTicksToWait < 0) ? 0 : (freertostime->xTicksToWait * portTICK_RATE_MS);
	uint32 cur_time = system_get_time();
	
	if (freertostime->end_time - cur_time < 0)
	{
		return 0;
	}
	else
	{
		return (freertostime->end_time - cur_time)/1000;
	}
}

void Platform_TimeDestroy(ezdev_sdk_time sdktime)
{
	freertos_time* freertostime = (freertos_time*)sdktime;
	if (freertostime == NULL)
	{
		return;
	}
	
	free(freertostime);
	freertostime = NULL;
}