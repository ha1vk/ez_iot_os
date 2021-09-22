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
#include <stdio.h>
#include <string.h>
#include <WinSock2.h>
#include <winsock.h>
#include <Windows.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#pragma comment(lib, "Shlwapi.lib")
#pragma comment(lib, "ws2_32.lib")

EZ_OS_API_EXTERN int EZ_OS_API_CALL ez_socket_setnonblock(int socket_fd)
{
	int arg = 1;
	ioctlsocket(socket_fd, FIONBIO, &arg);
	return 0;
}

EZ_OS_API_EXTERN int EZ_OS_API_CALL ez_socket_setblock(int socket_fd)
 {
	int arg = 0;
	ioctlsocket(socket_fd, FIONBIO, &arg);
	return 0;
 }

EZ_OS_API_EXTERN struct hostent* EZ_OS_API_CALL ez_gethostbyname(const char* host)
{
    return gethostbyname(host);
}

EZ_OS_API_EXTERN int EZ_OS_API_CALL ez_socket_poll(int socket_fd, POLL_TYPE type, int timeout)
{
	int return_value = -1;
    fd_set cnt_fdset;
    struct timeval cnt_timeout;

    if (socket_fd < 0)
    {
        return return_value;
    }

	FD_ZERO(&cnt_fdset); 
	FD_SET(socket_fd, &cnt_fdset); 

	cnt_timeout.tv_sec = timeout/1000; 
	cnt_timeout.tv_usec = (timeout%1000)*1000;

	switch(type)
	{
        case POLL_RECV:
        {
            return_value = select(socket_fd + 1, &cnt_fdset, NULL, NULL, &cnt_timeout);
            break;
        }
        case POLL_SEND:
        {
            return_value = select(socket_fd + 1, NULL, &cnt_fdset, NULL, &cnt_timeout);
            break;
        }
        case POLL_CONNECT:
        {
            return_value = select(socket_fd + 1, 0, &cnt_fdset, 0, &cnt_timeout);
            break;
        }
        default:
        {
            break;
        }
	}

	return return_value;
}

EZ_OS_API_EXTERN int EZ_OS_API_CALL ez_dev_ping(const char *dest, unsigned int c, unsigned int packetLen, struct ping_result *res)
{
	char cmd[256] = { 0 };
	char line[512] = { 0 };
	FILE *pp = NULL;
	char ip[64] = { 0 };
	int transmitted = -1, received = -1, loss = -1;
	float min = -1.0, avrg = -1.0, max = -1.0;

	if (!dest || !res || c == 0 || packetLen == 0) {
		return -1;
	}
	memset(res->destaddr, 0, sizeof(res->destaddr));
	memset(res->ip, 0, sizeof(res->ip));
	res->loss = -1;
	res->avrg = -1.0;

	/* 向目标地址发送c个ping包 */
	snprintf(cmd, sizeof(cmd), "ping %s -c %d -s %d", dest, c, packetLen);
	pp = _popen(cmd, "r");
	if (pp == NULL) {
		printf("open pipe to call ping error\n");
		return -1;
	}

	while (fgets(line, sizeof(line), pp) != NULL) {
		if (line[strlen(line) - 1] == '\n') {
			line[strlen(line) - 1] = '\0';
		}
		if (strstr(line, "PING")) {
			sscanf(line, "%*[^(](%[^)]", ip);
		}
		if (strstr(line, "packets")) {
			sscanf(line, "%d %*[^,], %d %*[^,], %d", &transmitted, &received, &loss);
		}
		if (strstr(line, "min/avg/max")) {
			sscanf(line, "%*[^=] = %f/%f/%f", &min, &avrg, &max);
		}
		memset(line, 0, sizeof(line));
	}
	_pclose(pp);

	printf("ping %s: ip=%s, loss=%d, avrg=%f\n", dest, ip, loss, avrg);

	snprintf(res->destaddr, sizeof(res->destaddr), "%s", dest);
	if (strlen(ip) != 0) {
		snprintf(res->ip, sizeof(res->ip), "%s", ip);
	}
	if (loss >= 0) {
		res->loss = loss;
	}
	if (avrg >= 0) {
		res->avrg = avrg;
	}
	res->receivedNum = received;
	res->sendedNum = transmitted;
	res->maxMs = max;
	res->minMs = min;
	return 0;
}

EZ_OS_API_EXTERN int EZ_OS_API_CALL ez_bind_network(int socket_fd, const char *interface_name)
{
	return 0;
}

EZ_OS_API_EXTERN int EZ_OS_API_CALL ez_get_interface_mask(const char* ifname, struct in_addr* mask)
{
	return 0;
}

EZ_OS_API_EXTERN int EZ_OS_API_CALL ez_get_interface_ip(const char* ifname, struct in_addr* ip)
{
	return 0;
}

EZ_OS_API_EXTERN int ez_get_all_interface(char* ifname_list[], int* count)
{
	return 0;
}

EZ_OS_API_EXTERN int ez_get_active_interface(char* if_name)
{
	return 0;
}
