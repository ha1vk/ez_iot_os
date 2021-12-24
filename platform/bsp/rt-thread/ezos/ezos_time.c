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
#include <sys/time.h>
#include <unistd.h>
#include <rtthread.h>

#define TIMESPEC_THOUSAND 1000
#define TIMESPEC_MILLION 1000000

static rt_tick_t tick_get_millisecond(void)
{
#if 1000 % RT_TICK_PER_SECOND == 0u
    return rt_tick_get() * (1000u / RT_TICK_PER_SECOND);
#else
    #warning "rt-thread cannot provide a correct 1ms-based tick any longer,\
    please redefine this function in another file by using a high-precision hard-timer."
    return 0;
#endif
}

EZOS_API ez_err_t ezos_get_clock(ezos_timespec_t *clock)
{
    rt_tick_t ms = tick_get_millisecond();
    clock->tv_sec = ms / TIMESPEC_THOUSAND;
    clock->tv_nsec = ms % TIMESPEC_THOUSAND * TIMESPEC_MILLION;

    return 0;
}

EZOS_API int ezos_gettimeofday(struct ezos_timeval *tv, struct ezos_timezone *tz)
{
    return gettimeofday((struct timeval *)tv, (struct timezone *)tz);
}

EZOS_API ezos_time_t ezos_time(ezos_time_t *__timer)
{
    return (ezos_time_t)time((time_t *)__timer);
}

EZOS_API struct ezos_tm * ezos_localtime(const ezos_time_t *timep, struct ezos_tm *result)
{
    return (struct ezos_tm *)localtime_r((time_t *)timep, (struct tm *)result);;
}

EZOS_API void ezos_delay_ms(ez_ulong_t time_ms)
{
    rt_thread_mdelay(time_ms);
}
