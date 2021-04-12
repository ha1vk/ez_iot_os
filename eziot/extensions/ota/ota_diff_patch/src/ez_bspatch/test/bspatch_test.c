#include <ez_bspatch.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <err.h>
#include <fcntl.h>

static void *open_cb(const char *filename, int type, int *filesize)
{
    FILE *fp = fopen(filename, "r");
    if (!fp)
    {
        return -1;
    }

    fseek(fp, 0, SEEK_END);
    *filesize = ftell(fp);

    return (void *)fp;
}

static int read_cb(const void *fd, int offset, int length, void *buffer)
{
    fseek((FILE *)fd, offset, SEEK_SET);
    return fread(buffer, 1, length, (FILE *)fd);
}

static int close_cb(void *fd)
{
    return fclose((FILE *)fd);
}

int main(int argc, char *argv[])
{
    ez_file_cb_t file_cb = {open_cb, read_cb, close_cb};
    int fd = 0;
    char *patch = NULL;
    char *new = NULL;
    ssize_t patchsize, newsize = 0;
    int ret = 0;

    do
    {
        if (argc != 4)
        {
            printf("usage: %s oldfile newfile patchfile\n", argv[0]);
            break;
        }

        /* Open patch file */
        if (((fd = open(argv[3], O_RDONLY, 0)) < 0) ||
            ((patchsize = lseek(fd, 0, SEEK_END)) == -1) ||
            ((patch = malloc(patchsize + 1)) == NULL) ||
            (lseek(fd, 0, SEEK_SET) != 0) ||
            (read(fd, patch, patchsize) != patchsize) ||
            (close(fd) == -1))
        {
            printf("read patch file err!\n");
            break;
        }

        if (0 != (ret = ez_bspatch(argv[1], patch, patchsize, NULL, &newsize, &file_cb)))
        {
            printf("ez_bspatch1 err!, ret = %d\n", ret);
            break;
        }

        if (NULL == (new = malloc(newsize + 1)))
        {
            printf("no memory!, newsize = %d\n", newsize);
            break;
        }

        if (0 != (ret = ez_bspatch(argv[1], patch, patchsize, new, &newsize, &file_cb)))
        {
            printf("ez_bspatch2 err!, ret = %d\n", ret);
            break;
        }

        /* Write the new file */
        if (((fd = open(argv[2], O_CREAT | O_TRUNC | O_WRONLY, 0666)) < 0) ||
            (write(fd, new, newsize) != newsize) || (close(fd) == -1))
        {
            printf("write new file err!\n");
            break;
        }
    } while (0);

    patch ? free(patch) : 0;
    new ? free(new) : 0;

    return 0;
}