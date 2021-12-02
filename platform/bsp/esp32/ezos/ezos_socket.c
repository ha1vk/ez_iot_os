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
    struct hostent *hostinfo = gethostbyname(host);
    return (ez_hostent_t *)hostinfo;
}

int ezos_closesocket(int socket_fd)
{
    return close(socket_fd);
}
int ezos_connect(int socket_fd, const ez_sockaddr_t *name, ez_socklen_t namelen)
{
    return connect(socket_fd, (struct sockaddr *)name, (socklen_t)namelen);
}

int ezos_getsockopt(int socket_fd, int level, int optname, void *optval, ez_socklen_t *optlen)
{
    return getsockopt(socket_fd, ezso2bspso(level), ezso2bspso(optname), optval, (socklen_t *)optlen);
}

int ezos_setsockopt(int socket_fd, int level, int optname, const void *optval, ez_socklen_t optlen)
{
    return setsockopt(socket_fd, ezso2bspso(level), ezso2bspso(optname), optval, (socklen_t)optlen);
}

int ezos_recv(int socket_fd, void *mem, int len, int flags)
{
    return recv(socket_fd, mem, len, flags);
}

int ezos_send(int socket_fd, const void *dataptr, int size, int flags)
{
    return send(socket_fd, dataptr, size, flags);
}

int ezos_socket(int domain, int type, int protocol)
{
    return socket(domain, type, protocol);
}

const char *ezos_inet_ntop(int af, const void *src, char *dst, ez_socklen_t size)
{
    return inet_ntop(af, src, dst, size);
}

unsigned long ezos_inet_addr(const char *cp)
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
    return bind(socket_fd, (struct sockaddr *)addr, addrlen);
}

ez_ssize_t ezos_sendto(int socket_fd, const void *buf, ez_size_t len, int flags, const ez_sockaddr_t *dst_addr, ez_socklen_t addrlen)
{
    return sendto(socket_fd, buf, len, flags, (struct sockaddr *)dst_addr, addrlen);
}

ez_ssize_t ezos_recvfrom(int socket_fd, void *buf, ez_size_t len, int flags, ez_sockaddr_t *src_addr, ez_socklen_t *addrlen)
{
    return recvfrom(socket_fd, buf, len, flags, (struct sockaddr *)src_addr, addrlen);
}

int ezos_accept(int socket_fd, struct ez_sockaddr *addr, ez_socklen_t *addrlen)
{
    return accept(socket_fd, (struct sockaddr *)addr, addrlen);
}

int ezos_listen(int socket_fd, int back_log)
{
    return listen(socket_fd, back_log);
}

int ezos_fcntl(int socket_fd, int cmd, int val)
{
    return fcntl(socket_fd, cmd, val);
}
