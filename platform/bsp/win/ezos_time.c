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
 * 
 * Brief:
 * Time related interface declaration
 * 
 * Change Logs:
 * Date           Author       Notes
 * 2021-10-27     zoujinwei    first version
 *******************************************************************************/

#include "ezos_time.h"
#include <time.h>
#include <stdint.h>
#include <windef.h>
#include <Windows.h>
#include <stdio.h>

const uint64_t usecs_per_msec = 1000;       ///> 毫秒单位
const uint64_t nsecs_per_usec = 1000;       ///> 微妙单位
const uint64_t usecs_per_sec  = 1000000;    ///> 秒单位

EZOS_API int ez_get_clock_time(ez_timespec *clock)
{
	SYSTEMTIME wtm;
	GetLocalTime(&wtm);

	clock->tv_sec = time(NULL);
	clock->tv_nsec = wtm.wMilliseconds * 1000000;
	return (0);
    //LARGE_INTEGER ticks_per_second;
    //if (0 == QueryPerformanceFrequency(&ticks_per_second))
    //{
    //    return -1;
    //}

    //LARGE_INTEGER tick;
    //if (0 == QueryPerformanceCounter(&tick))
    //{
    //    return -1;
    //}

    //const double ticks_div_sec  = (double) (ticks_per_second.QuadPart) / usecs_per_sec;
    //clock->tv_sec = (uint64_t)(tick.QuadPart / ticks_div_sec);

    //const double ticks_div_msec = (double)(ticks_per_second.QuadPart) / usecs_per_msec;
    //clock->tv_nsec = (uint64_t) (tick.QuadPart / ticks_div_msec);

    //return 0;
}

EZOS_API int ez_gettimeofday(struct timeval *tv)
{
	SYSTEMTIME wtm;
	GetLocalTime(&wtm);

	tv->tv_sec = time(NULL);
	tv->tv_usec = wtm.wMilliseconds * 1000;
	return (0);
	//return 0;//gettimeofday(tv, NULL);
}

EZOS_API time_t ez_get_time_stamp(time_t *__timer)
{
	return time(__timer);
}

EZOS_API int ez_set_time_stamp(time_t *__timer)
{
	return 0;
}


EZOS_API void ezos_delay_ms(unsigned int time_ms)
{
	Sleep(time_ms);
}