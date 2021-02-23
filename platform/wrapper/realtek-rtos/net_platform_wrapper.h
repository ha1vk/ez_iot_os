#ifndef H_NET_PLATFORM_WRAPPER_H_
#define H_NET_PLATFORM_WRAPPER_H_
#include "lwip/api.h"

/**
 * \brief   free rtos й╣ож
 */
typedef struct 
{
	int lwip_fd;
}freertos_net_work;

#endif