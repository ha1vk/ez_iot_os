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

#include "ezos_network.h"
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>


EZOS_API int ezos_socket(int domain, int type, int protocol)
{
	return socket(domain, type, protocol);
}

EZOS_API int ezos_connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
	return connect(sockfd, addr, addrlen);
}

EZOS_API int ezos_bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
	return bind(sockfd, addr, addrlen);
}

EZOS_API ssize_t ezos_recv(int sockfd, void *buf, size_t len, int flags)
{
	return recv(sockfd, buf, len, flags);
}

EZOS_API ssize_t ezos_send(int sockfd, const void *buf, size_t len, int flags)
{
	return send(sockfd, buf, len, flags);
}

EZOS_API int ezos_socket_setnonblock(int socket_fd)
{
	int flag = fcntl(socket_fd, F_GETFL);
	if (flag == -1){
		return -1;
	}
	if (fcntl(socket_fd, F_SETFL, (flag | O_NONBLOCK)) == -1){
		return -1;
	}
	return 0;
}

EZOS_API int ezos_socket_setblock(int socket_fd)
{
	int flag;
	flag = fcntl(socket_fd, F_GETFL);
	if (flag == -1){
		return -1;
	}
	flag &= ~O_NONBLOCK;
	if(fcntl(socket_fd, F_SETFL, flag) < 0)
	{
		perror("fcntl");
		return -1;
	}
	return 0;
}

EZOS_API struct hostent* ezos_gethostbyname(const char* host)
{
    return gethostbyname(host);
}


EZOS_API int ezos_socket_poll(int socket_fd, POLL_TYPE type, int timeout)
{
	struct pollfd poll_fd;
	int nfds = 0;

	if (socket_fd < 0) {
		return EZ_POLL_ERR;
	}

	poll_fd.fd = socket_fd;
	if(type == POLL_RECV){
		poll_fd.events = POLLIN;
	}
	else if(type == POLL_SEND){
		poll_fd.events = POLLOUT;
	}
	else if(type == POLL_CONNECT){
		poll_fd.events = POLLOUT | POLLERR | POLLHUP | POLLNVAL;
	}
	else
		poll_fd.events = type;
	poll_fd.revents = 0;

	nfds = poll(&poll_fd, 1, timeout);
	if (nfds < 0) {
		if (errno == EINTR) 
        {
            return EZ_POLL_EINTR;
        } 
		else{
			return EZ_POLL_ERR;
		}
	}
	else if (nfds > 0) {
		if (poll_fd.revents & (POLLIN | POLLOUT)) {
			return EZ_POLL_OK;
		} 
		else if (poll_fd.revents & (POLLNVAL | POLLERR | POLLHUP)) {
			return EZ_POLL_ERR;
		} 
		else {
			return EZ_POLL_ERR;
		}
	} 
	else {
		return EZ_POLL_TIMEOUT;
	}
}

