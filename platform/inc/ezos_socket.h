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

    typedef struct ez_sockaddr
    {
        unsigned char sa_len;
        unsigned char sa_family;
        unsigned char sa_data[14];
    } ez_sockaddr_t;

    EZOS_API int ezos_socket(int domain, int type, int protocol);

    EZOS_API int ezos_closesocket(int socket_fd);

    EZOS_API int ezos_shutdown(int socket_fd, int how);

    EZOS_API int ezos_setnonblock(int socket_fd);

    EZOS_API int ezos_setblock(int socket_fd);

    EZOS_API int ezos_poll(int socket_fd, ez_poll_type_e type, int timeout);

    EZOS_API int ezos_connect(int socket_fd, const ez_sockaddr_t *name, ez_socklen_t namelen);

    EZOS_API int ezos_recv(int socket_fd, void *buf, int len, int flags);

    EZOS_API int ezos_send(int socket_fd, const void *buf, int len, int flags);

    EZOS_API int ezos_setsockopt(int socket_fd, int level, int optname, const void *optval, ez_socklen_t optlen);

    EZOS_API int ezos_getlasterror();

#ifdef __cplusplus
}
#endif

#endif //H_EZOS_NETWORK_H_
