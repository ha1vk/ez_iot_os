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

#include <string.h>
#include "ezos_file.h"
#include "ezos_io.h"
#include "ezos_mem.h"
#include "ezos_network.h"
#include "ezos_thread.h"
#include "ezos_time.h"

int get_devinfo_fromconfig(const char* path, char* devinfo_context, int devinfo_context_len)
{
	int return_code = 0;
	int real_read = 0;
	int total_read = 0;
	unsigned char block[1024];
	int fd;
	fd = ez_open(path, O_RDONLY);
	if (fd < 0) {
        return_code = -1;
		return return_code;
    }

	do 
	{
		memset(block, 0, 1024);

		real_read = ez_read(fd, block, 1024);
		if (real_read < 0)
		{
			return_code = -1;
			break;
		}

		if (total_read + real_read >= devinfo_context_len)
		{
			return_code = -1;
			break;
		}
		memcpy(devinfo_context+total_read, block, real_read);
		total_read += real_read;
		if(real_read < 1024)
			break;
	} while (1);

	ez_close(fd);

	return return_code;
}



int set_file_value(const char* path, unsigned char* keyvalue, int keyvalue_size)
{
	int return_code = 0;
	int real_write = 0;
	int write_len=keyvalue_size;
	int offset=0;
	int fd;
	fd = ez_open(path, O_RDWR|O_CREAT|O_TRUNC, 0x777);
	if (fd < 0) {
        return_code = -1;
		return return_code;
    }
	
	do 
	{
		real_write = ez_write(fd, keyvalue+offset, write_len);
		if (real_write < 0){
			return_code = -1;
			break;
		}
		offset += real_write;
		write_len -= real_write;
		if(offset == keyvalue_size)
			break;
	} while (1);


	ez_close(fd);

	return return_code;
}

int get_file_value(const char* path, unsigned char* keyvalue, int keyvalue_maxsize)
{
	int return_code = 0;
	int real_read = 0;
	unsigned char block[64];
	int fd;
	fd = ez_open(path, O_RDONLY, 0x777);
	if (fd < 0) {
        return_code = -1;
		return return_code;
    }

	do 
	{
		memset(block, 0, 64);
		real_read = ez_read(fd, block, 64);
		if (real_read > 64 || real_read <= 0 )
		{
			return_code = -1;
			break;
		}

		if (real_read > keyvalue_maxsize)
		{
			return_code = -1;
			break;
		}
		memcpy(keyvalue, block, real_read);
	} while (0);

	ez_close(fd);

	return return_code;
}
