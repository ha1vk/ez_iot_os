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

#include "osal_time.h"
#include <time.h>
#include <sys/time.h>
#include <unistd.h>

int ez_get_clock_time(osal_timespec_t *clock)
{
    return clock_gettime(CLOCK_MONOTONIC, (struct timespec *)clock);
}

int ez_gettimeofday(struct timeval *tv)
{
    return gettimeofday(tv, NULL);
}

time_t ez_get_time_stamp(time_t *__timer)
{
    return time(__timer);
}

int ez_set_time_stamp(time_t *__timer)
{
    return 0;
}

void ez_delay_ms(unsigned int time_ms)
{
    usleep((int)(time_ms * 1000));
}
