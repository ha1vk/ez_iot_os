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
 * xurongjun (xurongjun@ezvizlife.com)
*******************************************************************************/

#ifndef _EZOS_TIME_H_
#define _EZOS_TIME_H_

#include <ezos_def.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef long int ezos_time_t;
    typedef long int ezos_suseconds_t;

    typedef struct
    {
        ezos_time_t tv_sec;  /* Seconds.  */
        ezos_time_t tv_nsec; /* nanosecond.  */
    } ezos_timespec_t;

    typedef struct ezos_timeval
    {
        ezos_time_t tv_sec;       /* Seconds.  */
        ezos_suseconds_t tv_usec; /* Microseconds.  */
    }ezos_timeval_t;

    typedef struct ezos_tm
    {
        int tm_sec;
        int tm_min;
        int tm_hour;
        int tm_mday;
        int tm_mon;
        int tm_year;
        int tm_wday;
        int tm_yday;
        int tm_isdst;
        long int tm_gmtoff;
        char *tm_zone;
    }ezos_tm_t;

    typedef struct ezos_timezone
    {
        int tz_minuteswest; /* minutes west of Greenwich */
        int tz_dsttime;     /* type of DST correction */
    }ezos_timezone_t;

    /**
     * @brief This function will get current time from operating system startup
     * 
     * @param clock if clock non-NULL, the current time will be filled.
     * @return return 0 for success, or -1 for failure 
     */
    EZOS_API int ezos_get_clock(ezos_timespec_t *clock);

    /**
     * @brief This functions can get the time as well as a timezone.
     * 
     * @param tv The tv argument is a struct timeval.
     * @return return 0 for success, or -1 for failure 
     */
    EZOS_API int ezos_gettimeofday(struct ezos_timeval *tv, struct ezos_timezone *tz);

    /**
     * @brief Get current timestamp
     * 
     * @param t put current time in *TIMER if TIMER is not NULL
     * @return Return the current time
     */
    EZOS_API ezos_time_t ezos_time(ezos_time_t *t);

    /**
     * @brief Return the `struct tm' representation of *TIMER in local time.
     * 
     * @param timep 
     * @param result return the address of the structure pointed to by result. 
     * @return ezos_tm*
     */
    EZOS_API struct ezos_tm *ezos_localtime(const ezos_time_t *timep, struct ezos_tm *result);

    /**
     * @brief This function suspends execution of the calling thread for (at least) microseconds.
     * 
     * @param time_ms suspends time, as millisecond.
     * @return void.
     */
    EZOS_API void ezos_delay_ms(ez_ulong_t msecs);

    /**
     * @brief set time zone
     * 
     * @param timezone 
     * @return void 
     */
    EZOS_API void ezos_tzset(const ez_char_t *timezone);

#ifdef __cplusplus
}
#endif

#endif
