#ifndef H_EZOS_IO_H_
#define H_EZOS_IO_H_

#if (defined(_WIN32) || defined(_WIN64))
#  if defined(EZOS_API_EXPORTS)
#    define EZOS_API __declspec(dllexport)
#  else
#    define EZOS_API __declspec(dllimport)
#  endif
#  define EZOS_CALL __stdcall
#elif defined(__linux__)
#  define EZOS_API
#  define EZOS_CALL
#else
#  define EZOS_API
#  define EZOS_CALL
#endif

#ifdef __cplusplus
extern "C" {
#endif


#if defined  (_FREE_RTOS_)
#define ezos_printf   ets_printf
#elif defined (_RT_THREAD_)
#define ezos_printf   rt_kprintf
#elif defined (_WIN32) || defined(_WIN64) || defined (WIN32) || defined(WIN64)
#include <stdio.h>
#define ezos_snprintf		_snprintf
#define ezos_printf		printf
#define ezos_strcasecmp		stricmp
#else
#include <stdio.h>
#define ezos_snprintf snprintf
#define ezos_printf   printf
#endif

#ifdef __cplusplus
}
#endif

#endif//H_EZOS_IO_H_
