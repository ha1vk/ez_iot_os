#ifndef H_TIME_INTERFACE_H_
#define H_TIME_INTERFACE_H_

#if (defined(_WIN32) || defined(_WIN64))
#  if defined(EZ_OS_API_EXPORTS)
#    define EZ_OS_API_EXTERN __declspec(dllexport)
#  else
#    define EZ_OS_API_EXTERN __declspec(dllimport)
#  endif
#  define EZ_OS_API_CALL __stdcall
#elif defined(__linux__)
#  define EZ_OS_API_EXTERN
#  define EZ_OS_API_CALL
#else
#  define EZ_OS_API_EXTERN
#  define EZ_OS_API_CALL
#endif

#ifdef __cplusplus
extern "C" {
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

typedef struct
{
    time_t tv_sec;      //秒
    time_t tv_nsec;     //纳秒
}ez_timespec;

/** 
 *  \brief		获取系统时钟
 *  \method		ez_get_clock_time
 *  \param[in] 	
 *  \return 	成功返回0 失败返回-1
 */
EZ_OS_API_EXTERN int EZ_OS_API_CALL ez_get_clock_time(ez_timespec *clock);

/** 
 *  \brief		获取系统时钟
 *  \method		ez_gettimeofday
 *  \param[in] 	
 *  \return 	成功返回0 失败返回-1
 */
EZ_OS_API_EXTERN int EZ_OS_API_CALL ez_gettimeofday(struct timeval *tv);
/** 
 *  \brief		获取系统时间戳(以秒数表示)
 *  \method		ez_get_clock_time
 *  \param[in] 	
 *  \return 	成功返回时间错 失败返回-1
 */
EZ_OS_API_EXTERN time_t EZ_OS_API_CALL ez_get_time_stamp(time_t *__time);

/** 
 *  \brief		设置系统时间戳(以秒数表示)
 *  \method		ez_set_time_stamp
 *  \param[in] 	
 *  \return 	成功返回0 失败返回-1
 */
EZ_OS_API_EXTERN int EZ_OS_API_CALL ez_set_time_stamp(time_t *__time);

/** 
 *  \brief		系统延时
 *  \method		ez_delay_ms
 *  \param[in] 	time_ms 延时ms
 *  \return 	成功返回0 失败返回-1
 */
EZ_OS_API_EXTERN void EZ_OS_API_CALL ez_delay_ms(unsigned int time_ms);

#ifdef __cplusplus
}
#endif

#endif
