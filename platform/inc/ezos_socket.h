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
* 2021-11-02     zoujinwei    first version
* 2021-11-15     xurongjun    Remove redundant functions and define the socket basic types
*******************************************************************************/

#ifndef _EZOS_SOCKET_H_
#define _EZOS_SOCKET_H_

#include <ezos_def.h>

#ifdef __cplusplus
extern "C"
{
#endif

/* errno */
#define EZ_EWOULDBLOCK 10035
#define EZ_EINPROGRESS 115
#define EZ_EAGAIN 11

#define EZ_AF_UNSPEC 0
#define EZ_AF_INET 2
#define EZ_PF_INET AF_INET
#define EZ_PF_UNSPEC AF_UNSPEC

#define EZ_AF_UNSPEC 0
#define EZ_AF_INET 2
#define EZ_PF_INET AF_INET
#define EZ_PF_UNSPEC AF_UNSPEC

#define EZ_IPPROTO_IP 0
#define EZ_IPPROTO_TCP 6
#define EZ_IPPROTO_UDP 17
#define EZ_IPPROTO_UDPLITE 136

/* Socket protocol types (TCP/UDP/RAW) */
#define EZ_SOCK_STREAM 1
#define EZ_SOCK_DGRAM 2
#define EZ_SOCK_RAW 3

#define EZ_IPPROTO_IP 0
#define EZ_IPPROTO_TCP 6
#define EZ_IPPROTO_UDP 17
#define EZ_IPPROTO_UDPLITE 136

#define EZ_SO_SNDBUF 0x1001   /* send buffer size */
#define EZ_SO_RCVBUF 0x1002   /* receive buffer size */
#define EZ_SO_SNDLOWAT 0x1003 /* send low-water mark */
#define EZ_SO_RCVLOWAT 0x1004 /* receive low-water mark */
#define EZ_SO_SNDTIMEO 0x1005 /* send timeout */
#define EZ_SO_RCVTIMEO 0x1006 /* receive timeout */
#define EZ_SO_ERROR 0x1007    /* get error status and clear */
#define EZ_SO_TYPE 0x1008     /* get socket type */

#define EZ_SOL_SOCKET 0xfff /* options for socket level */

#define EZ_TCP_MAXSEG 4

    typedef enum
    {
        POLL_RECV = 0,
        POLL_SEND,
        POLL_CONNECT,
        POLL_ALL
    } ez_poll_type_e;

    typedef enum
    {
        EZ_POLL_ERR = -1,
        EZ_POLL_TIMEOUT = 0,
        EZ_POLL_OK,
        EZ_POLL_EINTR
    } ez_poll_err_e;

    typedef int ez_socklen_t;

    /** For compatibility with BSD code */
    typedef struct ez_in_addr
    {
        unsigned int s_addr;
    } ez_in_addr_t;

    /* members are in network byte order */
    typedef struct ez_sockaddr_in
    {
        unsigned short sin_family;
        unsigned short sin_port;
        ez_in_addr_t sin_addr;
        char sin_zero[8];
    } ez_sockaddr_in_t;

    typedef struct ez_sockaddr
    {
        unsigned short sin_family;
        unsigned char sa_data[14];
    } ez_sockaddr_t;

    typedef struct ez_hostent
    {
        char *h_name;       /* official name of host */
        char **h_aliases;   /* alias list */
        int h_addrtype;     /* host address type */
        int h_length;       /* length of address */
        char **h_addr_list; /* list of addresses */
    } ez_hostent_t;

    EZOS_API int ezos_socket(int domain, int type, int protocol);

    EZOS_API int ezos_closesocket(int socket_fd);

    EZOS_API int ezos_shutdown(int socket_fd, int how);

    EZOS_API int ezos_setnonblock(int socket_fd);

    EZOS_API int ezos_setblock(int socket_fd);

    EZOS_API int ezos_poll(int socket_fd, ez_poll_type_e type, int timeout);

    EZOS_API int ezos_connect(int socket_fd, const ez_sockaddr_t *name, ez_socklen_t namelen);

    EZOS_API int ezos_recv(int socket_fd, void *buf, int len, int flags);

    EZOS_API int ezos_send(int socket_fd, const void *dataptr, int size, int flags);

    EZOS_API int ezos_getsockopt(int socket_fd, int level, int optname, void *optval, ez_socklen_t *optlen);

    EZOS_API int ezos_setsockopt(int socket_fd, int level, int optname, const void *optval, ez_socklen_t optlen);

    EZOS_API int ezos_getlasterror();

    EZOS_API ez_hostent_t *ezos_gethostbyname(const char *host);

    EZOS_API const char *ezos_inet_ntop(int af, const void *src, char *dst, ez_socklen_t size);

    EZOS_API unsigned short ezos_htons(unsigned short hostshort);

#ifdef __cplusplus
}
#endif

#endif //H_EZOS_NETWORK_H_
