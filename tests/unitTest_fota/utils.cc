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
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <memory.h>
#include "md5.h"
#include "utils.h"

int ez_calc_file_md5(char *file_path, unsigned char out[16])
{
    int rv = 0;
    int fd = 0;
    int file_size = 0;
    int buf[1024] = {0};
    int read_len = 0;
    int i = 0;
    bscomptls_md5_context md5_ctx = {0};
    unsigned char out_string[33] = {0};
    bscomptls_md5_init (&md5_ctx);
    bscomptls_md5_starts(&md5_ctx);
    do
    {
        if (((fd = open(file_path, O_RDONLY, 0)) < 0) ||
            ((file_size = lseek(fd, 0, SEEK_END)) == -1))
        {
            break;
        }
        for (i = 0; i < file_size;)
        {
            if (lseek(fd, i, SEEK_SET) != i)
            {
                break;
            }
            memset(buf, 0, sizeof(buf));
            if (-1 == (read_len = read(fd, buf, sizeof(buf))))
            {
                break;
            }
            i += read_len;
            bscomptls_md5_update(&md5_ctx, (unsigned char*)buf, read_len);
        }

        if (i == file_size)
        {
            bscomptls_md5_finish (&md5_ctx, out);
            bscomptls_md5_free(&md5_ctx);
            bscomptls_hexdump(out, 16, 1, out_string);
            rv = 0;
            printf("%-*s md5:%s\n", 30, file_path, out_string);
        }

    } while (0);

    if (fd)
    {
        close(fd);
    }

    return rv;
}

