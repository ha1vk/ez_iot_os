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

#include "ezdev_sdk_kernel_struct.h"
#include "time_module.h"
#include "file_interface.h"
#include "io_interface.h"
#include "mem_interface.h"
#include "network_interface.h"
#include "thread_interface.h"
#include "time_interface.h"

#define TIMESPEC_THOUSAND	1000
#define TIMESPEC_MILLION	1000000
#define TIMESPEC_BILLION	1000000000

# define Platform_Timespec_Add(a, b, result)						      \
	do {									      \
		(result)->tv_sec = (a)->tv_sec + (b)->tv_sec;			      \
		(result)->tv_nsec = (a)->tv_nsec + (b)->tv_nsec;			      \
		if ((result)->tv_nsec >= TIMESPEC_BILLION)					      \
		{									      \
			++(result)->tv_sec;						      \
			(result)->tv_nsec -= TIMESPEC_BILLION;					      \
		}									      \
	} while (0)
# define Platform_Timespec_Sub(a, b, result)						      \
	do {									      \
		(result)->tv_sec = (a)->tv_sec - (b)->tv_sec;			      \
		(result)->tv_nsec = (a)->tv_nsec - (b)->tv_nsec;			      \
		if ((result)->tv_nsec < 0) {					      \
			--(result)->tv_sec;						      \
			(result)->tv_nsec += TIMESPEC_BILLION;					      \
		}									      \
	} while (0)

ez_timespec* Platform_TimerCreater()
{
	ez_timespec* time = NULL;
	time = (ez_timespec*)ez_malloc(sizeof(ez_timespec));
	if (time == NULL){
		return NULL;
	}

	time->tv_nsec = 0;
	time->tv_sec = 0;
	return time;
}

char Platform_TimeIsExpired_Bydiff(ez_timespec *sdktime, EZDEV_SDK_UINT32 time_ms)
{
	ez_timespec now, res;
	if (sdktime == NULL){
		return (char)1;
	}

	ez_get_clock_time(&now);
	Platform_Timespec_Sub(&now, sdktime, &res);
	if (res.tv_sec < 0){
		return (char)0;
	}
	else if (res.tv_sec == 0){
		if ((res.tv_nsec/TIMESPEC_MILLION) > time_ms){
			return (char)1;
		}
		else{
			return (char)0;
		}
	}
	else{
		if ( (res.tv_sec*TIMESPEC_THOUSAND + res.tv_nsec/TIMESPEC_MILLION) > time_ms ){
			return (char)1;
		}
		else{
			return (char)0;
		}
	}
}

char Platform_TimerIsExpired(ez_timespec *sdktime)
{
	ez_timespec now, res;
	if (sdktime == NULL){
		return (char)1;
	}

	ez_get_clock_time(&now);
	Platform_Timespec_Sub(sdktime, &now, &res);		
	return res.tv_sec < 0 || (res.tv_sec == 0 && res.tv_nsec <= 0);
}


void Platform_TimerCountdownMS(ez_timespec *sdktime, unsigned int time_ms)
{
	ez_timespec now;
	ez_timespec interval = {time_ms / TIMESPEC_THOUSAND, (time_ms % TIMESPEC_THOUSAND) * TIMESPEC_MILLION};
	if (sdktime == NULL){
		return;
	}

	ez_get_clock_time(&now);
	Platform_Timespec_Add(&now, &interval, sdktime);
}


void Platform_TimerCountdown(ez_timespec *sdktime, unsigned int timeout)
{
	ez_timespec now;
	ez_timespec interval = {timeout, 0};
	if (sdktime == NULL){
		return;
	}

	ez_get_clock_time(&now);
	Platform_Timespec_Add(&now, &interval, sdktime);
}

EZDEV_SDK_UINT32 Platform_TimerLeftMS(ez_timespec *sdktime)
{
	ez_timespec now, res;
	if (sdktime == NULL){
		return 0;
	}

	ez_get_clock_time(&now);
	Platform_Timespec_Sub(sdktime, &now, &res);
	//ez_printf("left %d ms\n", (res.tv_sec < 0) ? 0 : res.tv_sec * 1000 + res.tv_usec / 1000);
	return (res.tv_sec < 0) ? 0 : res.tv_sec * TIMESPEC_THOUSAND + res.tv_nsec / TIMESPEC_MILLION;
}

void Platform_TimeDestroy(ez_timespec *sdktime)
{
	if (sdktime == NULL){
		return;
	}
	
	ez_free(sdktime);
	sdktime = NULL;
}
