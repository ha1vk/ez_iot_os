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
 * 2021-10-11     XuRongjun    first version
 *******************************************************************************/

#ifndef H_EZOS_TIME_H_
#define H_EZOS_TIME_H_

#if (defined(_WIN32) || defined(_WIN64))
#if defined(EZOS_API_EXPORTS)
#define EZOS_API __declspec(dllexport)
#else
#define EZOS_API __declspec(dllimport)
#endif
#define EZOS_CALL __stdcall
#elif defined(__linux__)
#define EZOS_API
#define EZOS_CALL
#else
#define EZOS_API
#define EZOS_CALL
#endif

#if defined(_WIN32) || defined(_WIN64)
#include <Windows.h>
#include <time.h>
#include <windows.h>
#include <WinSock2.h>
#include <winsock.h>
#define ez_localtime(timep, result) localtime_s(result, timep)
#else
#include <time.h>
#include <sys/time.h>
#define ez_localtime(timep, result) localtime_r(timep, result)
#endif

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct
    {
        time_t tv_sec;  //秒
        time_t tv_nsec; //纳秒
    } ezos_timespec_t;

    /**
     * @brief This function will get current time from operating system startup
     * 
     * @param clock if clock non-NULL, the current time will be filled.
     * @return return 0 for success, or -1 for failure 
     */
    EZOS_API int EZOS_CALL ezos_time_get_clock(ezos_timespec_t *clock);

    /**
     * @brief This functions can get the time as well as a timezone.
     * 
     * @param tv The tv argument is a struct timeval.
     * @return return 0 for success, or -1 for failure 
     */
    EZOS_API int EZOS_CALL ezos_time_gettimeofday(struct timeval *tv);

    /**
     * @brief returns the time as the number of seconds since the Epoch, 1970-01-01 00:00:00 +0000 (UTC).
     * 
     * @return On success, the value of time in seconds since the Epoch is returned. 
     *         On error, ((time_t) -1) is returned, and errno is set to indicate the error. 
     */
    EZOS_API time_t EZOS_CALL ezos_get_time_stamp(time_t *__timer);

    /**
     * @brief Return the `struct tm' representation of *TIMER in local time.
     * 
     * @param timep 
     * @param result return the address of the structure pointed to by result. 
     * @return void
     */
    EZOS_API void EZOS_CALL ezos_localtime(const time_t *timep, struct tm *result);

    /**
     * @brief This function suspends execution of the calling thread for (at least) microseconds.
     * 
     * @param time_ms suspends time, as millisecond.
     * @return returns 0 for success, or -1 for failure.
     */
    EZOS_API void EZOS_CALL ezos_delay_ms(unsigned int time_ms);

#ifdef __cplusplus
}
#endif

#endif//H_EZOS_TIME_H_
