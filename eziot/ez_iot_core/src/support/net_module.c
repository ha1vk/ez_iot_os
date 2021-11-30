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
 * 2021-11-25     zoujinwei    first version
 *******************************************************************************/

#include <ezos.h>
#include <ezlog.h>
#include "net_module.h"

/** 
 *  \brief		网络连接的创建,默认为TCP协议
 *  \method		net_create
 *  \param[in] 	nic_name	网卡名称，如果nic_name不为空或指向的地址是有效值，创建的socket将绑定这个网卡
 *  \return 	成功返回网络连接上下文 失败返回NULL
 */
int net_create(char *nic_name)
{
    int socket_fd;
    const int opt = 1400;
    int ret = 0;
    //struct ifreq ifr;
    //memset(&ifr, 0, sizeof(ifr));

    socket_fd = ezos_socket(EZ_AF_INET, EZ_SOCK_STREAM, EZ_IPPROTO_TCP);
    if (socket_fd == -1)
    {
        return -1;
    }

    ret = ezos_setsockopt(socket_fd, EZ_IPPROTO_TCP, EZ_TCP_MAXSEG, &opt, sizeof(opt));
    if (ret < 0)
    {
        // ezos_printf("set socket opt, TCP_MAXSEG error");
    }

    return socket_fd;
}

mkernel_internal_error net_connect(int socket_fd, const char *server_ip, int server_port, int timeout_ms, char szRealIp[ezdev_sdk_ip_max_len])
{
    ez_sockaddr_in_t dst_addr;

    int return_value = 0;
    int lasterror;
    int socket_err = 0;
    ez_socklen_t socklen = sizeof(socket_err);
    ez_hostent_t *host;

    ezos_setnonblock(socket_fd);
    ezos_memset(szRealIp, 0, ezdev_sdk_ip_max_len);

    dst_addr.sin_family = EZ_AF_INET;
    host = ezos_gethostbyname(server_ip);
    if (host == NULL)
    {
        ezlog_e(TAG_CORE, "gethost error");
        return mkernel_internal_net_gethostbyname_error;
    }

    ezos_memcpy(&dst_addr.sin_addr, host->h_addr_list[0], sizeof(ez_in_addr_t));
    ezos_inet_ntop(EZ_AF_INET, (void *)host->h_addr_list[0], szRealIp, ezdev_sdk_ip_max_len);
    dst_addr.sin_port = ezos_htons(server_port);
    if (ezos_connect(socket_fd, (const ez_sockaddr_t *)(&dst_addr), sizeof(dst_addr)) == -1)
    {
        lasterror = ezos_getlasterror();
        if ((lasterror != EZ_EINPROGRESS) && (lasterror != 0))
        {
            ezlog_e(TAG_CORE, "connect, errno:%d", lasterror);
            return mkernel_internal_net_connect_error;
        }
    }

    if (timeout_ms <= 0)
    {
        timeout_ms = -1;
    }

    return_value = ezos_poll(socket_fd, POLL_CONNECT, timeout_ms);
    if (return_value < 0)
    {
        return mkernel_internal_net_connect_error;
    }
    else if (return_value == 0)
    {
        return mkernel_internal_net_socket_timeout;
    }

    if (ezos_getsockopt(socket_fd, EZ_SOL_SOCKET, EZ_SO_ERROR, &socket_err, (ez_socklen_t *)&socklen) == -1)
    {
        return mkernel_internal_net_connect_error;
    }

    if (socket_err != 0)
    {
        return mkernel_internal_net_connect_error;
    }

    return mkernel_internal_succ;
}

mkernel_internal_error net_read(int socket_fd, unsigned char *read_buf, int read_buf_maxsize, int read_timeout_ms)
{
    int rev_size = 0;
    int rev_total_size = 0;
    mkernel_internal_error return_value = 0;

    do
    {
        return_value = ezos_poll(socket_fd, POLL_RECV, read_timeout_ms);
        if (return_value < 0)
            return mkernel_internal_net_socket_error;
        else if (return_value == 0)
            return mkernel_internal_net_socket_timeout;

        rev_size = ezos_recv(socket_fd, read_buf + rev_total_size, read_buf_maxsize - rev_total_size, 0);
        if (rev_size < 0)
        {
            return mkernel_internal_net_socket_error;
        }
        else if (rev_size == 0)
        {
            return mkernel_internal_net_socket_closed;
        }
        rev_total_size += rev_size;

    } while (rev_total_size < read_buf_maxsize);

    return mkernel_internal_succ;
}

mkernel_internal_error net_write(int socket_fd, unsigned char *write_buf, int write_buf_size, int send_timeout_ms, int *real_write_buf_size)
{
    int send_size = 0;
    int send_total_size = 0;

    mkernel_internal_error return_value = 0;
    if (NULL == real_write_buf_size)
    {
        return mkernel_internal_input_param_invalid;
    }

    do
    {
        return_value = ezos_poll(socket_fd, POLL_SEND, send_timeout_ms);
        if (return_value < 0)
            return mkernel_internal_net_socket_error;
        else if (return_value == 0)
            return mkernel_internal_net_socket_timeout;

        send_size = ezos_send(socket_fd, write_buf + send_total_size, write_buf_size - send_total_size, 0);
        if (send_size == -1)
        {
            return mkernel_internal_net_socket_error;
        }

        *real_write_buf_size = send_size;

    } while (0);

    return mkernel_internal_succ;
}

void net_disconnect(int socket_fd)
{
    ezos_closesocket(socket_fd);
}
