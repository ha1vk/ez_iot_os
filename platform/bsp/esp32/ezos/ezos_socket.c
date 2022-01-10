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
 * 2021-10-27     zoujinwei    first version
 *******************************************************************************/

#include "ezos_socket.h"
#include "ezos_libc.h"
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include "lwip/sockets.h"
#include <netdb.h>
#include <net/if.h>
#include <errno.h>
#include <stddef.h>
#include <sys/types.h>
#include <ezlog.h>

typedef struct
{
    int ez_so_type;
    int bsp_so_type;
} so_map_pair_t;

static so_map_pair_t g_so_maping[] = {
    {EZ_SO_SNDBUF, SO_SNDBUF},
    {EZ_SO_RCVBUF, SO_RCVBUF},
    {EZ_SO_SNDLOWAT, SO_SNDLOWAT},
    {EZ_SO_RCVLOWAT, SO_RCVLOWAT},
    {EZ_SO_SNDTIMEO, SO_SNDTIMEO},
    {EZ_SO_RCVTIMEO, SO_RCVTIMEO},
    {EZ_SO_ERROR, SO_ERROR},
    {EZ_SO_TYPE, SO_TYPE},
    {EZ_SOL_SOCKET, SOL_SOCKET},
};

static int ezso2bspso(int so_type);


int ezos_setnonblock(int socket_fd)
{
    int flag = fcntl(socket_fd, F_GETFL);
    if (flag == -1)
    {
        return -1;
    }
    if (fcntl(socket_fd, F_SETFL, (flag | O_NONBLOCK)) == -1)
    {
        return -1;
    }
    return 0;
}

int ezos_setblock(int socket_fd)
{
    int flag;
    flag = fcntl(socket_fd, F_GETFL);
    if (flag == -1)
    {
        return -1;
    }
    flag &= ~O_NONBLOCK;
    if (fcntl(socket_fd, F_SETFL, flag) < 0)
    {
        perror("fcntl");
        return -1;
    }
    return 0;
}

ez_hostent_t *ezos_gethostbyname(const char *host)
{
    ezlog_a(TAG_APP,"host = %s",host);

    struct hostent *hostinfo = gethostbyname(host);

//     ezlog_a(TAG_APP,"host h_name= %s",hostinfo->h_name);
//     ezlog_a(TAG_APP,"host h_addrtype= %d",hostinfo->h_addrtype);
//     ezlog_a(TAG_APP,"host h_length= %d",hostinfo->h_length);
//  //   ezlog_a(TAG_APP,"host h_addr_list= %d",hostinfo->h_addr_list);

//     char **pptr; 
//     switch(hostinfo->h_addrtype)	
// 		{	
// 			case AF_INET:	
// 			case AF_INET6:
// 				pptr = hostinfo->h_addr_list;  
// 				for(; *pptr!=NULL; pptr++)
//                    ezlog_a(TAG_APP,"address:%s\n",inet_ntop(hostinfo->h_addrtype, *pptr, host,64));
// 			    ezlog_a(TAG_APP,"first address: %s\n",inet_ntop(hostinfo->h_addrtype, hostinfo->h_addr,host,64));				
// 				break;
// 			default:
// 				ezlog_a(TAG_APP,"Unkown address type\n");
// 		}

    return (ez_hostent_t *)hostinfo;
}

int ezos_poll(int socket_fd, ez_poll_type_e type, int timeout)
{
    int ret = 0;
    fd_set select_fd;
    struct timeval tmval = {timeout / 1000, (timeout % 1000) * 1000};

    FD_ZERO(&select_fd);
    FD_SET(socket_fd, &select_fd);
    if (POLL_RECV == type)
    {
        ret = lwip_select(socket_fd + 1, &select_fd, NULL, NULL, &tmval);
    }
    else
    {
        ret = lwip_select(socket_fd + 1, NULL, &select_fd, NULL, &tmval);
    }

    if (ret < 0)
    {
        return EZ_POLL_ERR;
    }
    else if (ret == 0)
    {
        return EZ_POLL_TIMEOUT;
    }

    return EZ_POLL_OK;
}

int ezos_closesocket(int socket_fd)
{
    return lwip_close(socket_fd);
}

static int transfer_sockaddr(struct sockaddr *dst_addr, const ez_sockaddr_t *addr)
{
    dst_addr->sa_family = addr->sin_family;
    ezos_memcpy(dst_addr->sa_data, addr->sa_data, 14);
    return 0;
}

int ezos_connect(int socket_fd, const ez_sockaddr_t *name, ez_socklen_t namelen)
{
    //return connect(socket_fd, (struct sockaddr *)name, (socklen_t)namelen);

    struct sockaddr tmp_addr = {0};
    transfer_sockaddr(&tmp_addr, name);

    return lwip_connect(socket_fd, (struct sockaddr *)&tmp_addr, (socklen_t)namelen);
}

int ezos_getsockopt(int socket_fd, int level, int optname, void *optval, ez_socklen_t *optlen)
{
    return lwip_getsockopt(socket_fd, ezso2bspso(level), ezso2bspso(optname), optval, (socklen_t *)optlen);
}

int ezos_setsockopt(int socket_fd, int level, int optname, const void *optval, ez_socklen_t optlen)
{
    return lwip_setsockopt(socket_fd, ezso2bspso(level), ezso2bspso(optname), optval, (socklen_t)optlen);
}

int ezos_recv(int socket_fd, void *mem, int len, int flags)
{
    return lwip_recv(socket_fd, mem, len, flags);
}

int ezos_send(int socket_fd, const void *dataptr, int size, int flags)
{
    return lwip_send(socket_fd, dataptr, size, flags);
}

int ezos_socket(int domain, int type, int protocol)
{
    return lwip_socket(domain, type, protocol);
}

const char *ezos_inet_ntop(int af, const void *src, char *dst, ez_socklen_t size)
{
    return inet_ntop(af, src, dst, size);
}

unsigned int ezos_inet_addr(const char *cp)
{
    return inet_addr(cp);
}

unsigned short ezos_htons(unsigned short hostshort)
{
    return htons(hostshort);
}

unsigned int ezos_htonl(unsigned int hostlong)
{
    return htonl(hostlong);
}

static int ezso2bspso(int so_type)
{
    int bspso = so_type;
    for (size_t i = 0; i < sizeof(g_so_maping) / sizeof(so_map_pair_t); i++)
    {
        if (g_so_maping[i].ez_so_type == so_type)
        {
            bspso = g_so_maping[i].bsp_so_type;
            break;
        }
    }

    return bspso;
}

int ezos_inet_aton(const char *cp, ez_in_addr_t *inp)
{
    return inet_aton(cp, inp);
}


int ezos_bind(int socket_fd, const ez_sockaddr_t *addr, ez_socklen_t addrlen)
{
    return lwip_bind(socket_fd, (struct sockaddr *)addr, (socklen_t)addrlen);
}

ez_ssize_t ezos_sendto(int socket_fd, const void *buf, ez_size_t len, int flags, const ez_sockaddr_t *dst_addr, ez_socklen_t addrlen)
{
    return lwip_sendto(socket_fd, buf, len, flags, (struct sockaddr *)dst_addr, (socklen_t)addrlen);

}

ez_ssize_t ezos_recvfrom(int socket_fd, void *buf, ez_size_t len, int flags, ez_sockaddr_t *src_addr, ez_socklen_t *addrlen)
{
    return lwip_recvfrom(socket_fd, buf, len, flags, (struct sockaddr *)src_addr, (socklen_t *)addrlen);

}

int ezos_accept(int socket_fd, struct ez_sockaddr *addr, ez_socklen_t *addrlen)
{
    return lwip_accept(socket_fd, (struct sockaddr *)addr, (socklen_t *)addrlen);

}

int ezos_listen(int socket_fd, int back_log)
{
    return lwip_listen(socket_fd, back_log);
}

int ezos_fcntl(int socket_fd, int cmd, int val)
{
    return fcntl(socket_fd, cmd, val);
}

extern int errno;

/*
static int errno_to_ezerr(int std_err)
{
    int ez_err = std_err;
    size_t i;
    for (i = 0; i < sizeof(g_err_maping) / sizeof(err_pair_t); i++)
    {
        if (g_err_maping[i].std_err == std_err)
        {
            ez_err = g_err_maping[i].ez_err;
            break;
        }
    }

    return ez_err;
}*/

int ezos_getlasterror()
{
//    return errno_to_ezerr(errno);
    return 0;
}
