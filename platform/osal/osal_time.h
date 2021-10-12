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

#ifndef H_TIME_INTERFACE_H_
#define H_TIME_INTERFACE_H_

#if (defined(_WIN32) || defined(_WIN64))
#if defined(EZ_OS_API_EXPORTS)
#define EZ_OS_API_EXTERN __declspec(dllexport)
#else
#define EZ_OS_API_EXTERN __declspec(dllimport)
#endif
#define EZ_OS_API_CALL __stdcall
#elif defined(__linux__)
#define EZ_OS_API_EXTERN
#define EZ_OS_API_CALL
#else
#define EZ_OS_API_EXTERN
#define EZ_OS_API_CALL
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
    } osal_timespec_t;

    /**
     * @brief This function will get current time from operating system startup
     * 
     * @param clock if clock non-NULL, the current time will be filled.
     * @return return 0 for success, or -1 for failure 
     */
    EZ_OS_API_EXTERN int EZ_OS_API_CALL osal_time_get_clock(osal_timespec_t *clock);

    /**
     * @brief This functions can get the time as well as a timezone.
     * 
     * @param tv The tv argument is a struct timeval.
     * @return return 0 for success, or -1 for failure 
     */
    EZ_OS_API_EXTERN int EZ_OS_API_CALL osal_time_gettimeofday(struct timeval *tv);

    /**
     * @brief returns the time as the number of seconds since the Epoch, 1970-01-01 00:00:00 +0000 (UTC).
     * 
     * @return On success, the value of time in seconds since the Epoch is returned. 
     *         On error, ((time_t) -1) is returned, and errno is set to indicate the error. 
     */
    EZ_OS_API_EXTERN time_t EZ_OS_API_CALL osal_time_gettimestamp(void);

    /**
     * @brief Return the `struct tm' representation of *TIMER in local time.
     * 
     * @param timep 
     * @param result return the address of the structure pointed to by result. 
     * @return void
     */
    EZ_OS_API_EXTERN void EZ_OS_API_CALL osal_localtime(const time_t *timep, struct tm *result);

    /**
     * @brief This function suspends execution of the calling thread for (at least) microseconds.
     * 
     * @param time_ms suspends time, as millisecond.
     * @return returns 0 for success, or -1 for failure.
     */
    EZ_OS_API_EXTERN int EZ_OS_API_CALL osal_time_delay(unsigned int time_ms);

#ifdef __cplusplus
}
#endif

#endif
