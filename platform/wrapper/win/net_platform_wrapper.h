/**
 * \file		thread_interface.h
 *
 * \brief		线程相关操作接口，不同平台在同层次目录都有对应的实现
 *
 * \copyright	HangZhou Hikvision System Technology Co.,Ltd. All Right Reserved.
 *
 * \author		xurongjun
 *
 * \date		2018/6/27
 */

#ifndef H_NET_PLATFORM_WRAPPER_H_
#define H_NET_PLATFORM_WRAPPER_H_

typedef struct 
{
	int socket_fd;
}win_net_work;


#endif