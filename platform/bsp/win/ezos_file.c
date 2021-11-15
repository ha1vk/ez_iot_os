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

#include "ezos_file.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <Windows.h>
#include <io.h>
#include <direct.h>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <Windows.h>
#include <malloc.h>
#include <stdlib.h>
#include <stdio.h>

#define VFS_MAX_FILE_NUM	50

typedef struct {
	FILE *ff;
	int fd;
}vfs_file_t;

static vfs_file_t g_files[VFS_MAX_FILE_NUM];

EZOS_API DIR * ez_opendir(const char *name)
{
    DIR *dir;
    WIN32_FIND_DATA FindData;
    char namebuf[512];
    HANDLE handle = NULL;

    sprintf(namebuf, "%s\\*.*", name);

    handle = FindFirstFile(namebuf, &FindData);
    if (handle == INVALID_HANDLE_VALUE)
    {
        printf("FindFirstFile failed (%d)\n", GetLastError());
        return 0;
    }

    dir = (DIR *)malloc(sizeof(DIR));
    if (!dir)
    {
        printf("DIR memory allocate fail\n");
        return 0;
    }

    memset(dir, 0, sizeof(DIR));
    dir->dd_fd = 0;
    dir->dd_handle = handle;

    return dir;
}

EZOS_API struct dirent * ez_readdir(DIR *dir)
{
    int i;

    BOOL bf;
    WIN32_FIND_DATA FileData;
    if (!dir)
    {
        return 0;
    }

    bf = FindNextFile(dir->dd_handle, &FileData);
    //fail or end  
    if (!bf)
    {
        return 0;
    }

    struct dirent *dirent = (struct dirent *)malloc(sizeof(struct dirent) + sizeof(FileData.cFileName));

    for (i = 0; i < 256; i++)
    {
        dirent->d_name[i] = FileData.cFileName[i];
        if (FileData.cFileName[i] == '\0') break;
    }
    dirent->d_reclen = i;
    dirent->d_reclen = (unsigned short)FileData.nFileSizeLow;

    //check there is file or directory  
    if (FileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
    {
        dirent->d_type = 2;
    }
    else
    {
        dirent->d_type = 1;
    }

    return dirent;
}

EZOS_API int ez_closedir(DIR *dir)
{
    if (!dir) return -1;
    dir->dd_handle = 0;
    free(dir);
    return 0;
}

EZOS_API int ez_open(const char *name, int flags, ...)
{
	FILE *tmp_fp;
	char mode[3];
	int fd;
	int i;

	if ((flags & O_BINARY) == O_BINARY) {
		if (((flags & O_RDWR) == O_RDWR) && ((flags & O_APPEND) == O_APPEND)) {
			_snprintf(mode, 3, "ab+");
		}
		else if (((flags & O_RDWR) == O_RDWR) && ((flags & O_CREAT) == O_CREAT)) {
			_snprintf(mode, 3, "wb+");
		}
		else if (((flags & O_WRONLY) == O_WRONLY) && ((flags & O_CREAT) == O_CREAT)) {
			_snprintf(mode, 3, "wb+");
		}
		else if ((flags & O_RDWR) == O_RDWR) {
			_snprintf(mode, 3, "rb+");
		}
		else if ((flags & O_WRONLY) == O_WRONLY) {
			_snprintf(mode, 3, "wb");
		}
		else if ((flags & O_RDONLY) == O_RDONLY) {
			_snprintf(mode, 3, "rb");
		}
		else {
			_snprintf(mode, 3, "wb+");
		}
	}else {
		if (((flags & O_RDWR) == O_RDWR) && ((flags & O_APPEND) == O_APPEND)) {
			_snprintf(mode, 3, "a+");
		}else if (((flags & O_RDWR) == O_RDWR) && ((flags & O_CREAT) == O_CREAT)) {
			_snprintf(mode, 3, "w+");
		}else if (((flags & O_WRONLY) == O_WRONLY) && ((flags & O_CREAT) == O_CREAT)) {
			_snprintf(mode, 3, "w+");
		}else if ((flags & O_RDWR) == O_RDWR) {
			_snprintf(mode, 3, "r+");
		}else if ((flags & O_WRONLY) == O_WRONLY) {
			_snprintf(mode, 3, "w");
		}else if ((flags & O_RDONLY) == O_RDONLY) {
			_snprintf(mode, 3, "r");
		}else {
			_snprintf(mode, 3, "w+");
		}
	}

	tmp_fp = fopen(name, mode);
	if (tmp_fp) {
		fd = _fileno(tmp_fp);
		if (fd < VFS_MAX_FILE_NUM) {
			if (g_files[fd].ff == NULL) {
				g_files[fd].ff = tmp_fp;
				g_files[fd].fd = fd;
			}
			else{
				for (i = 0; i < VFS_MAX_FILE_NUM; i++)
				{
					if (g_files[i].ff == NULL) {
						g_files[i].ff = tmp_fp;
						g_files[i].fd = fd;
					}
				}
			}
		}
		else {
			for (i = 0; i < VFS_MAX_FILE_NUM; i++)
			{
				if (g_files[i].ff == NULL) {
					g_files[i].ff = tmp_fp;
					g_files[i].fd = fd;
				}
			}
		}
		if(g_files)
		return fd;
	}
	else
		return -1;

	return fd;
}

EZOS_API int ez_close(int fd)
{
	int i;
	int ret = -1;

	if (fd < VFS_MAX_FILE_NUM) {
		if (g_files[fd].ff != NULL && g_files[fd].fd == fd) {
			ret = fclose(g_files[fd].ff);
			g_files[fd].ff = NULL;
			g_files[fd].fd = 0;
			return ret;
		}
	}

	for (i = 0; i < VFS_MAX_FILE_NUM; i++)
	{
		if (g_files[i].fd == fd && g_files[i].ff != NULL) {
			fclose(g_files[i].ff);
			g_files[i].ff = NULL;
			g_files[i].fd = 0;
			return ret;
		}
	}
	return ret;
	
}
