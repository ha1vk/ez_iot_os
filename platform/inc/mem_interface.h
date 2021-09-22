
#ifndef H_MEM_INTERFACE_H_
#define H_MEM_INTERFACE_H_

#ifdef __cplusplus
extern "C" {
#endif

#if defined (_FREE_RTOS_) 
#define ez_malloc   malloc
#define ez_free     free
#define ez_calloc   calloc
#define ez_realloc  realloc
#elif defined (_RT_THREAD_) 
#define ez_malloc   rt_malloc
#define ez_free     rt_free
#define ez_calloc   rt_calloc
#define ez_realloc  rt_realloc
#else
#include <stdlib.h>
#define ez_malloc   malloc
#define ez_free     free
#define ez_calloc   calloc
#define ez_realloc  realloc
#endif

#ifdef __cplusplus
}
#endif

#endif
