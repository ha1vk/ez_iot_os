#ifndef H_EZOS_FILE_H_
#define H_EZOS_FILE_H_

#if (defined(_WIN32) || defined(_WIN64))
#  if defined(EZOS_API_EXPORTS)
#    define EZOS_API __declspec(dllexport)
#  else
#    define EZOS_API __declspec(dllimport)
#  endif
#  define EZOS_CALL __stdcall
#elif defined(__linux__)
#  define EZOS_API
#  define EZOS_CALL
#else
#  define EZOS_API
#  define EZOS_CALL
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>

#if (defined(_WIN32) || defined(_WIN64))
#define 	F_OK  0
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
#endif

EZOS_API int EZOS_CALL ezos_open(const char *file, int flags,...);
EZOS_API int EZOS_CALL ezos_read(int fd, void *buf, size_t len);
EZOS_API int EZOS_CALL ezos_write(int fd, const void *buf, size_t len);
EZOS_API int EZOS_CALL ezos_close(int fd);
EZOS_API int EZOS_CALL ezos_rename(const char *old,const char *new);
EZOS_API int EZOS_CALL ezos_unlink(const char *pathname);
EZOS_API int EZOS_CALL ezos_stat(const char *file,struct stat *buf);
EZOS_API int EZOS_CALL ezos_fstat(int fildes,struct stat *buf);
EZOS_API int EZOS_CALL ezos_sync(int fildes);
EZOS_API int EZOS_CALL ezos_lseek(int fd, int offset, int whence);
EZOS_API int EZOS_CALL ezos_remove(const char *path);
EZOS_API int EZOS_CALL ezos_mkdir(const char *path,mode_t mode);
EZOS_API int EZOS_CALL ezos_fsync(int fildes);
EZOS_API int EZOS_CALL ezos_access(const char *path,int amode);
EZOS_API DIR * EZOS_CALL ezos_opendir(const char *);
EZOS_API struct dirent * EZOS_CALL ezos_readdir(DIR *);
EZOS_API int EZOS_CALL ezos_closedir(DIR *);


#ifdef __cplusplus
}
#endif

#endif//H_EZOS_FILE_H_