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
 *******************************************************************************/

#include "network_interface.h"
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>


int ez_socket_setnonblock(int socket_fd)
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

int ez_socket_setblock(int socket_fd)
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

struct hostent* ez_gethostbyname(const char* host)
{
    return gethostbyname(host);
}


int ez_socket_poll(int socket_fd, POLL_TYPE type, int timeout)
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

