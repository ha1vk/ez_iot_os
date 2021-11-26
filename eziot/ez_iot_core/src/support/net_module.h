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

#ifndef H_NET_MODULE_H_
#define H_NET_MODULE_H_

#include "ezdev_sdk_kernel_struct.h"
#include "mkernel_internal_error.h"

int net_create(char *nic_name);

mkernel_internal_error net_connect(int socket_fd, const char* server_ip, int server_port,  int timeout_ms, char szRealIp[ezdev_sdk_ip_max_len]);
mkernel_internal_error net_read(int socket_fd, unsigned char* read_buf, int read_buf_maxsize, int read_timeout_ms);
mkernel_internal_error net_write(int socket_fd, unsigned char* write_buf, int write_buf_size, int send_timeout_ms, int* real_write_buf_size);

void net_disconnect(int socket_fd);

#endif