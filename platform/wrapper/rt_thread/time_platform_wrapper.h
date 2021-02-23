#ifndef H_TIME_PLATFORM_WRAPPER_H_
#define H_TIME_PLATFORM_WRAPPER_H_

#include <time.h>
#include <sys/time.h>
#include <pthread.h>
/**
* \brief   linux й╣ож
*/
typedef struct 
{
	struct timespec time_record;
}linux_time;


#endif