
#ifndef H_EZOS_MEM_H_
#define H_EZOS_MEM_H_

#ifdef __cplusplus
extern "C" {
#endif

#if defined (_FREE_RTOS_) 
#define ezos_malloc   malloc
#define ezos_free     free
#define ezos_calloc   calloc
#define ezos_realloc  realloc
#elif defined (_RT_THREAD_) 
#define ezos_malloc   rt_malloc
#define ezos_free     rt_free
#define ezos_calloc   rt_calloc
#define ezos_realloc  rt_realloc
#else
#include <stdlib.h>
#define ezos_malloc   malloc
#define ezos_free     free
#define ezos_calloc   calloc
#define ezos_realloc  realloc
#endif

#ifdef __cplusplus
}
#endif

#endif//H_EZOS_MEM_H_
