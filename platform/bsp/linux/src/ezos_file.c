#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <utime.h>
#include <dirent.h>
#include "ezos_file.h"

EZOS_API int EZOS_CALL ezos_open(const char *file, int flags,...)
{
    return open(file, flags);
}

EZOS_API int EZOS_CALL ezos_read(int fd, void *buf, size_t len)
{
    return read(fd, buf, len);
}

EZOS_API int EZOS_CALL ezos_write(int fd, const void *buf, size_t len)
{
    return write(fd, buf, len);
}

EZOS_API int EZOS_CALL ezos_close(int fd)
{
    return close(fd);
}

EZOS_API int EZOS_CALL ezos_rename(const char *old,const char *new)
{
    return rename(old,new);
}

EZOS_API int EZOS_CALL ezos_unlink(const char *pathname)
{
    return unlink(pathname);
}

EZOS_API int EZOS_CALL ezos_stat(const char *file,struct stat *buf)
{
    return stat(file,buf);
}

EZOS_API int EZOS_CALL ezos_fstat(int fildes,struct stat *buf)
{
    return fstat(fildes,buf);
}

EZOS_API int EZOS_CALL ezos_sync(int fildes)
{
    return 0;
}

EZOS_API int EZOS_CALL ezos_lseek(int fd, int offset, int whence)
{
    return lseek(fd, offset, whence);
}

EZOS_API int EZOS_CALL ezos_remove(const char *path)
{
    return remove(path);
}


EZOS_API int EZOS_CALL ezos_mkdir(const char *path,mode_t mode)
{
    return mkdir(path,mode);
}

EZOS_API int EZOS_CALL ezos_fsync(int fildes)
{
    return fsync(fildes);
}

EZOS_API int EZOS_CALL ezos_access(const char *path,int amode)
{
    return access(path,amode);
}

EZOS_API DIR * EZOS_CALL ezos_opendir(const char *name)
{
    return opendir(name);
}

EZOS_API struct dirent * EZOS_CALL ezos_readdir(DIR *dir)
{
    return readdir(dir);
}

EZOS_API int EZOS_CALL ezos_closedir(DIR *dir)
{
    return closedir(dir);
}
