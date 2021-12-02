// Copyright 2018 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <string.h>
#include <unistd.h>
#ifdef linux 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

#include "ezos_libc.h"
#include "ezos_socket.h"
#include "ctrl_sock.h"

/* Control socket, because in some network stacks select can't be woken up any
 * other way
 */
int cs_create_ctrl_sock(int port)
{
    int fd = ezos_socket(EZ_AF_INET, EZ_SOCK_DGRAM, EZ_IPPROTO_UDP);
    if (fd < 0) {
        return -1;
    }

    int ret;
    ez_sockaddr_in_t addr;
    ezos_memset(&addr, 0, sizeof(addr));
    addr.sin_family = EZ_AF_INET;
    addr.sin_port = ezos_htons(port);
    ezos_inet_aton("127.0.0.1", &addr.sin_addr);
    ret = ezos_bind(fd, (ez_sockaddr_t *)&addr, sizeof(addr));
    if (ret < 0) {
        ezos_closesocket(fd);
        return -1;
    }
    return fd;
}

void cs_free_ctrl_sock(int fd)
{
    ezos_closesocket(fd);
}

int cs_send_to_ctrl_sock(int send_fd, int port, void *data, unsigned int data_len)
{
    int ret;
    ez_sockaddr_in_t to_addr;
    to_addr.sin_family = EZ_AF_INET;
    to_addr.sin_port = ezos_htons(port);
    ezos_inet_aton("127.0.0.1", &to_addr.sin_addr);
    ret = ezos_sendto(send_fd, data, data_len, 0, (ez_sockaddr_t *)&to_addr, sizeof(to_addr));

    if (ret < 0) {
        return -1;
    }
    return ret;
}

int cs_recv_from_ctrl_sock(int fd, void *data, unsigned int data_len)
{
    int ret;
    ret = ezos_recvfrom(fd, data, data_len, 0, NULL, NULL);

    if (ret < 0) {
        return -1;
    }
    return ret;
}
