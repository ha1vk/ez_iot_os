#ifndef H_IO_INTERFACE_H_
#define H_IO_INTERFACE_H_

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


#if defined  (_FREE_RTOS_)
#define ez_printf   ets_printf
#elif defined (_RT_THREAD_)
#define ez_printf   rt_kprintf
#elif defined (_WIN32) || defined(_WIN64) || defined (WIN32) || defined(WIN64)
#include <stdio.h>
#define ez_snprintf		_snprintf
#define ez_printf		printf
#define strcasecmp		stricmp
#else
#include <stdio.h>
#define ez_snprintf snprintf
#define ez_printf   printf
#endif

#ifdef __cplusplus
}
#endif

#endif
