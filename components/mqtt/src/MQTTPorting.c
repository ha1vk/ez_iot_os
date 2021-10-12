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
 * Contributors:
 * XuRongjun (xurongjun@ezvizlife.com) - Mqtt adaptation interface implementation, convert to ezosal
 *******************************************************************************/

#include "MQTTPorting.h"

#define TIMESPEC_THOUSAND 1000
#define TIMESPEC_MILLION 1000000
#define TIMESPEC_BILLION 1000000000

#define Platform_Timespec_Add(a, b, result)              \
    do                                                   \
    {                                                    \
        (result)->tv_sec = (a)->tv_sec + (b)->tv_sec;    \
        (result)->tv_nsec = (a)->tv_nsec + (b)->tv_nsec; \
        if ((result)->tv_nsec >= TIMESPEC_BILLION)       \
        {                                                \
            ++(result)->tv_sec;                          \
            (result)->tv_nsec -= TIMESPEC_BILLION;       \
        }                                                \
    } while (0)

#define Platform_Timespec_Sub(a, b, result)              \
    do                                                   \
    {                                                    \
        (result)->tv_sec = (a)->tv_sec - (b)->tv_sec;    \
        (result)->tv_nsec = (a)->tv_nsec - (b)->tv_nsec; \
        if ((result)->tv_nsec < 0)                       \
        {                                                \
            --(result)->tv_sec;                          \
            (result)->tv_nsec += TIMESPEC_BILLION;       \
        }                                                \
    } while (0)

void TimerInit(Timer *assign_timer)
{
    assign_timer->end_time.tv_sec = 0;
    assign_timer->end_time.tv_nsec = 0;
}

char TimerIsExpired(Timer *assign_timer)
{
    osal_timespec_t now, res;
    if (NULL == assign_timer)
    {
        return (char)1;
    }

    osal_time_get_clock(&now);
    Platform_Timespec_Sub(&assign_timer->end_time, &now, &res);
    return res.tv_sec < 0 || (res.tv_sec == 0 && res.tv_nsec <= 0);
}

void TimerCountdownMS(Timer *assign_timer, unsigned int time_count)
{
    osal_timespec_t now;
    osal_timespec_t interval = {time_count / TIMESPEC_THOUSAND, (time_count % TIMESPEC_THOUSAND) * TIMESPEC_MILLION};
    if (NULL == assign_timer)
    {
        return;
    }

    osal_time_get_clock(&now);
    Platform_Timespec_Add(&now, &interval, &assign_timer->end_time);
}

void TimerCountdown(Timer *assign_timer, unsigned int time_count)
{
    osal_timespec_t now;
    osal_timespec_t interval = {time_count, 0};
    if (NULL == assign_timer)
    {
        return;
    }

    osal_time_get_clock(&now);
    Platform_Timespec_Add(&now, &interval, &assign_timer->end_time);
}

int TimerLeftMS(Timer *assign_timer)
{
    osal_timespec_t now, res;
    if (NULL == assign_timer)
    {
        return 0;
    }

    osal_time_get_clock(&now);
    Platform_Timespec_Sub(&assign_timer->end_time, &now, &res);
    return (res.tv_sec < 0) ? 0 : res.tv_sec * TIMESPEC_THOUSAND + res.tv_nsec / TIMESPEC_MILLION;
}