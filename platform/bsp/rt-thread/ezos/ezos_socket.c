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
 * 
 * 
 * Change Logs:
 * Date           Author       Notes
 * 2021-12-16     xurongjun    first version
 *******************************************************************************/

#include "ezos_socket.h"
#include <stdio.h>
#include <rtthread.h>
#include "lwip/netdb.h"
#include "lwip/def.h"
#include "lwip/sockets.h"

typedef struct
{
    int std_err;
    int ez_err;
} err_pair_t;

static err_pair_t g_err_maping[] = {
    {EWOULDBLOCK, EZ_EWOULDBLOCK},
    {EINPROGRESS, EZ_EINPROGRESS},
    {EAGAIN, EZ_EAGAIN}};

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
}

static int transfer_sockaddr(struct sockaddr *dst_addr, const ez_sockaddr_t *addr)
{
    dst_addr->sa_family = addr->sin_family;
    memcpy(dst_addr->sa_data, addr->sa_data, 14);
    return 0;
}

int ezos_setnonblock(int socket_fd)
{
    int flag = lwip_fcntl(socket_fd, F_GETFL, 0);
    if (flag == -1)
    {
        return -1;
    }
    if (lwip_fcntl(socket_fd, F_SETFL, (flag | O_NONBLOCK)) == -1)
    {
        return -1;
    }

    return 0;
}

int ezos_setblock(int socket_fd)
{
    int flag;
    flag = lwip_fcntl(socket_fd, F_GETFL, 0);
    if (flag == -1)
    {
        return -1;
    }

    flag &= ~O_NONBLOCK;
    if (lwip_fcntl(socket_fd, F_SETFL, flag) < 0)
    {
        perror("fcntl");
        return -1;
    }
    return 0;
}

ez_hostent_t *ezos_gethostbyname(const char *host)
{
    struct hostent *hostinfo = lwip_gethostbyname(host);
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

int ezos_getlasterror()
{
    return errno_to_ezerr(errno);
}

int ezos_closesocket(int socket_fd)
{
    return lwip_close(socket_fd);
}

int ezos_connect(int socket_fd, const ez_sockaddr_t *name, ez_socklen_t namelen)
{
    struct sockaddr tmp_addr = {0};
    transfer_sockaddr(&tmp_addr, name);

    return lwip_connect(socket_fd, (struct sockaddr *)&tmp_addr, (socklen_t)namelen);
}

int ezos_getsockopt(int socket_fd, int level, int optname, void *optval, ez_socklen_t *optlen)
{
    return lwip_getsockopt(socket_fd, level, optname, optval, (socklen_t *)optlen);
}

int ezos_setsockopt(int socket_fd, int level, int optname, const void *optval, ez_socklen_t optlen)
{
    return lwip_setsockopt(socket_fd, level, optname, optval, (socklen_t)optlen);
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

unsigned short ezos_htons(unsigned short hostshort)
{
    return htons(hostshort);
}

unsigned int ezos_htonl(unsigned int hostlong)
{
    return htonl(hostlong);
}

int ezos_inet_aton(const char *cp, ez_in_addr_t *inp)
{
    return inet_aton(cp, (struct in_addr *)inp);
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
    return lwip_fcntl(socket_fd, cmd, val);
}

unsigned int ezos_inet_addr(const char *ip)
{
    return (unsigned int)inet_addr(ip);
}
