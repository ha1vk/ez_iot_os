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
#include <unistd.h>
#include "FreeRTOS.h"
#include "task.h"
#include "time.h"

EZOS_API ez_err_t ezos_get_clock(ezos_timespec_t *clock)
{
    return clock_gettime(CLOCK_MONOTONIC, (struct timespec *)clock);
}

//EZOS_API int ezos_gettimeofday(struct ezos_timeval *tv, struct ezos_timezone *tz)
//{
//    return gettimeofday((struct timeval *)tv, (struct timezone *)tz);
//}

EZOS_API ezos_time_t ezos_time(ezos_time_t *__timer)
{
    return (ezos_time_t)time((time_t *)__timer);
}

EZOS_API struct ezos_tm *ezos_localtime(const ezos_time_t *timep, struct ezos_tm *result)
{
    return (struct ezos_tm *)localtime_r((time_t *)timep, (struct tm *)result);
}

EZOS_API void ezos_delay_ms(ez_ulong_t time_ms)
{
    portTickType xTicksToWait = time_ms / portTICK_RATE_MS;

    if (0 == xTicksToWait)
        xTicksToWait = 1;

    vTaskDelay(xTicksToWait);
}

EZOS_API void ezos_tzset(const ez_char_t *timezone)
{
    setenv("TZ", timezone, 1);
    tzset();
}