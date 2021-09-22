#ifndef H_NETWORK_INTERFACE_H_
#define H_NETWORK_INTERFACE_H_

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

#if defined (_FREE_RTOS_) 

#elif defined (_RT_THREAD_) 
#include <rtthread.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <finsh.h>
#elif defined (_WIN32) || defined(_WIN64) || defined (WIN32) || defined(WIN64)
#include <WinSock2.h>
#include <winsock.h>
#include <Windows.h>
#include <ws2tcpip.h>
#define getlasterror   	     WSAGetLastError
#define setlasterror   		 WSASetLastError
#define ez_ioctlsocket		ioctlsocket

#define EZ_EINPROGRESS			WSAEINPROGRESS
#define EZ_EWOULDBLOCK          WSAEWOULDBLOCK
#define EZ_SHUT_RD				SD_RECEIVE
#define EZ_SHUT_WR				SD_SEND
#define EZ_SHUT_RDWR			SD_BOTH

#elif defined (__linux__)
#include <fcntl.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <net/if.h>
#include <poll.h>
#include <sys/ioctl.h>
#include <sys/types.h>      
#include <net/route.h>  
#include <stdio.h>  
#include <unistd.h>   
#define getlasterror()      errno
#define setlasterror(error) errno=error;
#define closesocket         close

#define EZ_EINPROGRESS			EINPROGRESS
#define EZ_EWOULDBLOCK          0

#else
#include <fcntl.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <net/if.h>
#include <poll.h>
#include <sys/ioctl.h>
#include <sys/types.h>      
#include <net/route.h> 
#include <stdio.h> 
#include <unistd.h> 
#define getlasterror()      errno
#define setlasterror(error) errno=error;
#define closesocket         close
#endif

#define HOSTNAME_MAX_LEN            64      /* 主机名字符串最大长度 */

typedef enum  
{
	POLL_RECV = 0,
	POLL_SEND,
	POLL_CONNECT,
	POLL_ALL
}POLL_TYPE;

typedef enum  
{
	EZ_POLL_ERR = -1,
	EZ_POLL_TIMEOUT = 0,
	EZ_POLL_OK,
	EZ_POLL_EINTR
}POLL_ERROR;

struct ping_result {
	char destaddr[HOSTNAME_MAX_LEN];			/* 目标地址 */
	char ip[16];                                /* 域名解析得到的IP地址字符串 */
	int loss;                                   /* 丢包率(百分比) */
	int sendedNum;                              /* 发包数 */
	int receivedNum;                            /* 收包数 */
	float minMs;                                /* 最小耗时 */
	float maxMs;                                /* 最大耗时 */
	float avrg;                                 /* 平均耗时 */
};

/** 
 *  \brief		设置socket非阻塞
 *  \param[in] 	socket_fd	网络文件描述符
 *  \return 	0:成功	-1：失败
 */
EZ_OS_API_EXTERN int EZ_OS_API_CALL ez_socket_setnonblock(int socket_fd);

/** 
 *  \brief		设置socket阻塞
 *  \param[in] 	socket_fd	网络文件描述符
 *  \return 	0:成功	-1：失败
 */
EZ_OS_API_EXTERN int EZ_OS_API_CALL ez_socket_setblock(int socket_fd);

/** 
 *  \brief		域名解析
 *  \param[in] 	host	域名	
 *  \return 	成功：ip	失败：INADDR_NONE
 */
EZ_OS_API_EXTERN struct hostent* EZ_OS_API_CALL ez_gethostbyname(const char* host);

/** 
 *  \brief		
 *  \param[in] 	socket_fd	网络文件描述符
 *  \param[in]	type		poll事件类型POLL_TYPE，win系统按照该接口实现
 *  \param[in]	timeout		超时等待时间
 *  \return 	0:超时  1:成功	-1：失败
 */
EZ_OS_API_EXTERN int EZ_OS_API_CALL ez_socket_poll(int socket_fd, POLL_TYPE type, int timeout);

/**
*  \brief
*  \param[in] 	
*  \param[in]	
*  \param[in]	
*  \return 	0:超时  1:成功	-1：失败
*/
EZ_OS_API_EXTERN int EZ_OS_API_CALL ez_dev_ping(const char *dest, unsigned int c, unsigned int packetLen, struct ping_result *res);

EZ_OS_API_EXTERN int EZ_OS_API_CALL ez_bind_network(int socket_fd, const char *interface_name);

EZ_OS_API_EXTERN int EZ_OS_API_CALL ez_get_interface_mask(const char* ifname, struct in_addr* mask);

EZ_OS_API_EXTERN int EZ_OS_API_CALL ez_get_interface_ip(const char* ifname, struct in_addr* ip);

EZ_OS_API_EXTERN int ez_get_all_interface(char* ifname_list[], int* count);

EZ_OS_API_EXTERN int ez_get_active_interface(char* if_name);
#ifdef __cplusplus
}
#endif

#endif
