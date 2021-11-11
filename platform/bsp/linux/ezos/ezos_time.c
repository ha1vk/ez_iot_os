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

EZOS_API ez_err_t EZOS_CALL ezos_get_clock(ezos_timespec_t *clock)
{
    return clock_gettime(CLOCK_MONOTONIC, (struct timespec *)clock);
}

EZOS_API ez_err_t EZOS_CALL ezos_gettimeofday(struct ezos_timeval *tv)
{
    return gettimeofday(tv, NULL);
}

EZOS_API time_t EZOS_CALL ezos_time(ezos_time_t *__timer)
{
    return time(__timer);
}

EZOS_API struct ezos_tm *EZOS_CALL ezos_localtime(const ezos_time_t *timep, struct ezos_tm *result)
{
    //TODO
    return result;
}

EZOS_API void EZOS_CALL ezos_delay_ms(ez_ulong_t time_ms)
{
    usleep((int)(time_ms * 1000));
}
