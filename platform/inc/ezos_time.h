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

    struct ezos_timeval
    {
        ezos_time_t tv_sec;       /* Seconds.  */
        ezos_suseconds_t tv_usec; /* Microseconds.  */
    } ezos_timeval_t;

    struct ezos_tm
    {
        ez_int32_t tm_sec;
        ez_int32_t tm_min;
        ez_int32_t tm_hour;
        ez_int32_t tm_mday;
        ez_int32_t tm_mon;
        ez_int32_t tm_year;
        ez_int32_t tm_wday;
        ez_int32_t tm_yday;
        ez_int32_t tm_isdst;
    };

    /**
     * @brief This function will get current time from operating system startup
     * 
     * @param clock if clock non-NULL, the current time will be filled.
     * @return return 0 for success, or -1 for failure 
     */
    EZOS_API ez_err_t ezos_get_clock(ezos_timespec_t *clock);

    /**
     * @brief This functions can get the time as well as a timezone.
     * 
     * @param tv The tv argument is a struct timeval.
     * @return return 0 for success, or -1 for failure 
     */
    EZOS_API ez_err_t ezos_gettimeofday(struct ezos_timeval *tv);

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
    EZOS_API struct ezos_tm *EZOS_CALL ezos_localtime(const ezos_time_t *timep, struct ezos_tm *result);

    /**
     * @brief This function suspends execution of the calling thread for (at least) microseconds.
     * 
     * @param time_ms suspends time, as millisecond.
     * @return ez_void_t.
     */
    EZOS_API ez_void_t ezos_delay_ms(ez_ulong_t msecs);

#ifdef __cplusplus
}
#endif

#endif
