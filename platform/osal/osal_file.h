#ifndef H_FILE_INTERFACE_H_
#define H_FILE_INTERFACE_H_

#if (defined(_WIN32) || defined(_WIN64))
#  if defined(EZ_OS_API_EXPORTS)
#    define EZ_OS_API_EXTERN __declspec(dllexport)
#  else
#    define EZ_OS_API_EXTERN __declspec(dllimport)
#  endif
#  define EZ_OS_API_CALL __stdcall
#elif defined(__linux__)
#  define EZ_OS_API_EXTERN
#  define EZ_OS_API_CALL
#else
#  define EZ_OS_API_EXTERN
#  define EZ_OS_API_CALL
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if defined (_FREE_RTOS_)
#define _VFS_INTERFACE_
#endif

#if defined (_RT_THREAD_)
#include <unistd.h>
#endif

#if defined (_WIN32) || defined(_WIN64) || defined (WIN32) || defined(WIN64)
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <Windows.h>
#include <io.h>
#include <direct.h>
//#pragma warning(disable: 4996)
#else
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <utime.h>
#include <dirent.h>
#endif

#if defined (_VFS_INTERFACE_)
#include "vfs.h"
#else
#define ez_open     open
#define ez_close    close
#define ez_write    write
#define ez_read     read
#define ez_lseek    lseek
#define ez_sync     fsync
#define ez_unlink   unlink
#define ez_fstat    fstat
#define ez_stat     stat
#define ez_rename   rename
#define ez_remove   remove
#define ez_utimes   utimes
#define ez_fsync    fsync

#if defined (_WIN32) || defined(_WIN64) || defined (WIN32) || defined(WIN64)

#define 	F_OK  0
#define		ez_access   _access
#define		ez_mkdir(path, mode)    _mkdir(path)
typedef struct _dirdesc
{
    int     dd_fd;      /** file descriptor associated with directory */
    long    dd_loc;     /** offset in current buffer */
    long    dd_size;    /** amount of data returned by getdirentries */
    char    *dd_buf;    /** data buffer */
    int     dd_len;     /** size of data buffer */
    long    dd_seek;    /** magic cookie returned by getdirentries */
    HANDLE  dd_handle;  /** Win32 search handle */
} DIR;

struct dirent
{
    long d_ino;              /* inode number*/
    off_t d_off;             /* offset to this dirent*/
    unsigned short d_reclen; /* length of this d_name*/
    unsigned char d_type;    /* the type of d_name*/
    char d_name[1];          /* file name (null-terminated)*/
};

EZ_OS_API_EXTERN DIR * EZ_OS_API_CALL ez_opendir(const char *);
EZ_OS_API_EXTERN struct dirent * EZ_OS_API_CALL ez_readdir(DIR *);
EZ_OS_API_EXTERN int EZ_OS_API_CALL ez_closedir(DIR *);

#else
#define ez_mkdir    mkdir
#define ez_opendir  opendir
#define ez_readdir  readdir
#define ez_closedir closedir
#define ez_sync     fsync
#define ez_access   access
#endif

#endif

#ifdef __cplusplus
}
#endif

#endif//H_FILE_INTERFACE_H_