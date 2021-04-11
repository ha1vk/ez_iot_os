
#if defined(__linux__)
#include <err.h>
#include <unistd.h>
#endif

#include <bzlib.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <sys/types.h>
#include <fcntl.h>

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
typedef long off_t;
#define BSPATCH_EXPORTS 1
#endif

#include <ez_bspatch.h>

#define FIRMWARE_VERSION "V1.0.0"

static off_t offtin(unsigned char *buf)
{
    off_t y;

    y = buf[7] & 0x7F;
    y = y * 256;
    y += buf[6];
    y = y * 256;
    y += buf[5];
    y = y * 256;
    y += buf[4];
    y = y * 256;
    y += buf[3];
    y = y * 256;
    y += buf[2];
    y = y * 256;
    y += buf[1];
    y = y * 256;
    y += buf[0];

    if (buf[7] & 0x80)
        y = -y;

    return y;
}

/**
 *  File format:
 *  0   8   "BSDIFF40"
 *  8   8   X
 *  16  8   Y
 *  24  8   sizeof(newfile)
 *  32  X   bzip2(control block) cpf cpfbz2
 *  32+X    Y   bzip2(diff block) dpf dpfbz2
 *  32+X+Y  ??? bzip2(extra block) epf epfbz2
 *  with control block a set of triples (x,y,z) meaning "add x bytes
 *  from oldfile to x bytes from the diff block; copy y bytes from the
 *  extra block; seek forwards in oldfile by z bytes".
 */
EZ_RV ez_bspatch(const char *oldfilename, const char *patch, int patchsize,
                 char *newdata, int *newsize, ez_file_cb_t *file_cb)
{
    int bzctrllen = 0, bzdatalen = 0, oldsize = 0, newsize_ = 0, blockn = 0;
    void *oldfd = 0;
    bz_stream cstrm = {0}, dstrm = {0}, estrm = {0};
    off_t oldpos = 0, newpos = 0, cpos = 0, dpos = 0, epos = 0;
    char buf[8] = {0};
    off_t ctrl[3] = {0};
    u_char oldblock[1024] = {0};
    off_t lenread = 0, i = 0, blockoff;
    int ret = 0;

    if (!oldfilename || !patch || !newsize || !file_cb)
    {
        return EZBS_INVALID_PARAM;
    }

    /* Check for appropriate magic */
    if (32 > patchsize || memcmp(patch, "BSDIFF40", 8) != 0)
        return EZBS_INVALID_PATCH_TYPE;

    /* Read lengths from header */
    bzctrllen = offtin((unsigned char *)patch + 8);
    bzdatalen = offtin((unsigned char *)patch + 16);
    newsize_ = offtin((unsigned char *)patch + 24);
    if ((bzctrllen < 0) || (bzdatalen < 0) || (newsize_ < 0))
    {
        return EZBS_UNEXPECTED_HEADER;
    }

    if (!newdata)
    {
        *newsize = newsize_;
        return 0;
    }

    if (BZ_OK != BZ2_bzDecompressInit(&cstrm, 0, 1) ||
        BZ_OK != BZ2_bzDecompressInit(&dstrm, 0, 1) ||
        BZ_OK != BZ2_bzDecompressInit(&estrm, 0, 1))
    {
        BZ2_bzDecompressEnd(&cstrm);
        BZ2_bzDecompressEnd(&dstrm);
        BZ2_bzDecompressEnd(&estrm);
        return EZBS_GENERAL;
    }

    cpos = 32;                         ///< control block offset
    dpos = 32 + bzctrllen;             ///< diff block offset
    epos = 32 + bzctrllen + bzdatalen; ///< extra block offset
    cstrm.avail_in = bzctrllen;
    cstrm.next_in = (char *)patch + cpos;
    dstrm.avail_in = bzdatalen;
    dstrm.next_in = (char *)patch + dpos;
    estrm.avail_in = patchsize - epos;
    estrm.next_in = (char *)patch + epos;

    if ((oldfd = file_cb->open(oldfilename, 0, &oldsize)) < 0)
    {
        return EZBS_OLDFILE_OPEN;
    }
    while (newpos < *newsize)
    {
        /* Read control data */
        for (i = 0; i <= 2; i++)
        {
            cstrm.avail_out = 8;
            cstrm.next_out = buf;
            ret = BZ2_bzDecompress(&cstrm);
            if ((ret != BZ_OK && ret != BZ_STREAM_END) ||
                (ret == BZ_OK && cstrm.avail_in == 0 && cstrm.avail_out > 0))
            {
                //BZ_UNEXPECTED_EOF
                ret = EZBS_UNEXPECTED_CDATA;
                goto exit;
            };

            if (ret == BZ_STREAM_END)
                lenread = 8 - cstrm.avail_out;

            if (cstrm.avail_out == 0)
                lenread = 8;

            if (lenread < 8)
            {
                ret = EZBS_UNEXPECTED_CDATA;
                goto exit;
            }

            ctrl[i] = offtin((unsigned char *)buf);
        };

        /* Sanity-check */
        if (newpos + ctrl[0] > *newsize)
        {
            ret = EZBS_UNEXPECTED_RDATA_LEN;
            goto exit;
        }

        /* Read diff string */
        if (0 != ctrl[0])
        {
            dstrm.avail_out = ctrl[0];
            dstrm.next_out = newdata + newpos;
            ret = BZ2_bzDecompress(&dstrm);
            if ((ret != BZ_OK && ret != BZ_STREAM_END) ||
                (ret == BZ_OK && dstrm.avail_in == 0 && dstrm.avail_out > 0))
            {
                //BZ_UNEXPECTED_EOF
                ret = EZBS_UNEXPECTED_DDATA;
                goto exit;
            };

            if (ret == BZ_STREAM_END)
                lenread = ctrl[0] - dstrm.avail_out;

            if (dstrm.avail_out == 0)
                lenread = ctrl[0];

            if (lenread < ctrl[0])
            {
                ret = EZBS_UNEXPECTED_DDATA;
                goto exit;
            }
        }

        /* Add old data to diff string */
        blockn = 0;
        for (i = 0; i < ctrl[0]; i++)
        {
            if (0 == blockn)
            {
                if (sizeof(oldblock) > ctrl[0] - i)
                    blockn = ctrl[0] - i;
                else
                    blockn = sizeof(oldblock);

                memset(oldblock, 0, sizeof(oldblock));
                lenread = file_cb->read(oldfd, oldpos + i, blockn, oldblock);

                if (lenread < 0 || (lenread < blockn && oldpos + i + lenread < oldsize))
                {
                    ret = EZBS_OLDFILE_READ;
                    goto exit;
                }

                blockoff = 0;
            }

            if ((oldpos + i >= 0) && (oldpos + i < oldsize))
            {
                newdata[newpos + i] += oldblock[blockoff++];
                blockn--;
            }
        }

        /* Adjust pointers */
        newpos += ctrl[0];
        oldpos += ctrl[0];

        /* Sanity-check */
        if (newpos + ctrl[1] > *newsize)
        {
            ret = EZBS_UNEXPECTED_RDATA_LEN;
            goto exit;
        }

        /* Read extra string */
        if (0 != ctrl[1])
        {
            estrm.avail_out = ctrl[1];
            estrm.next_out = newdata + newpos;
            ret = BZ2_bzDecompress(&estrm);
            if ((ret != BZ_OK && ret != BZ_STREAM_END) ||
                (ret == BZ_OK && estrm.avail_in == 0 && estrm.avail_out > 0))
            {
                //BZ_UNEXPECTED_EOF
                ret = EZBS_UNEXPECTED_EDATA;
                goto exit;
            };

            if (ret == BZ_STREAM_END)
                lenread = ctrl[1] - estrm.avail_out;

            if (estrm.avail_out == 0)
                lenread = ctrl[1];

            if (lenread < ctrl[1])
            {
                ret = EZBS_UNEXPECTED_EDATA;
                goto exit;
            }
        }

        /* Adjust pointers */
        newpos += ctrl[1];
        oldpos += ctrl[2];
    };
    ret = 0;
exit:
    file_cb->close(oldfd);
    BZ2_bzDecompressEnd(&cstrm);
    BZ2_bzDecompressEnd(&dstrm);
    BZ2_bzDecompressEnd(&estrm);
    return ret;
}
BSPATCH_API EZ_RV ez_patch_init(ez_bspatch_ctx_t *ctx, ez_file_cb_t *file_cb, const char *oldfilename,
                                int oldoff, int oldsize, const char *patch, int patchsize, int bufsizemax)
{
    int oldsize_total = 0;
	int bzctrllen = 0;
    int bzdatalen = 0;
    int newdatalen = 0;
    if (!ctx || !patch || !file_cb || !oldfilename)
    {
        return EZBS_INVALID_PARAM;
    }
    memset(ctx, 0, sizeof(ez_bspatch_ctx_t));
    /* Check for appropriate magic */
    if (32 > patchsize || memcmp(patch, "BSDIFF40", 8) != 0)
        return EZBS_INVALID_PATCH_TYPE;
    bzctrllen = offtin((unsigned char *)patch + 8);
    bzdatalen = offtin((unsigned char *)patch + 16);
    newdatalen = offtin((unsigned char *)patch + 24);
    if ((bzctrllen < 0) || (bzdatalen < 0) || (newdatalen < 0))
    {
        return EZBS_UNEXPECTED_HEADER;
    }
    if (NULL == (ctx->cstrm = (char *)malloc(sizeof(bz_stream))) ||
        NULL == (ctx->dstrm = (char *)malloc(sizeof(bz_stream))) ||
        NULL == (ctx->estrm = (char *)malloc(sizeof(bz_stream))))
    {
        return EZBS_NO_MEMORY;
    }
    memset(ctx->cstrm, 0, sizeof(bz_stream));
    memset(ctx->dstrm, 0, sizeof(bz_stream));
    memset(ctx->estrm, 0, sizeof(bz_stream));
    if (BZ_OK != BZ2_bzDecompressInit((bz_stream *)ctx->cstrm, 0, 1) ||
        BZ_OK != BZ2_bzDecompressInit((bz_stream *)ctx->dstrm, 0, 1) ||
        BZ_OK != BZ2_bzDecompressInit((bz_stream *)ctx->estrm, 0, 1))
    {
        BZ2_bzDecompressEnd((bz_stream *)ctx->cstrm);
        BZ2_bzDecompressEnd((bz_stream *)ctx->dstrm);
        BZ2_bzDecompressEnd((bz_stream *)ctx->estrm);
        return EZBS_GENERAL;
    }
    if ((ctx->fd = (int)file_cb->open(oldfilename, 0, &oldsize_total)) < 0)
    {
        BZ2_bzDecompressEnd((bz_stream *)ctx->cstrm);
        BZ2_bzDecompressEnd((bz_stream *)ctx->dstrm);
        BZ2_bzDecompressEnd((bz_stream *)ctx->estrm);
        return EZBS_OLDFILE_OPEN;
    }
    if (oldsize_total < oldsize)
    {
        BZ2_bzDecompressEnd((bz_stream *)ctx->cstrm);
        BZ2_bzDecompressEnd((bz_stream *)ctx->dstrm);
        BZ2_bzDecompressEnd((bz_stream *)ctx->estrm);
        return EZBS_INVALID_PARAM;
    }
    ctx->new_size = newdatalen;
    ctx->old_size = oldsize ? oldsize : oldsize_total;
    ctx->buf_size = bufsizemax ? bufsizemax : ctx->old_size;
    ctx->old_base = oldoff;
    if (NULL == (ctx->buf = (char *)malloc(ctx->buf_size)))
    {
        BZ2_bzDecompressEnd((bz_stream *)ctx->cstrm);
        BZ2_bzDecompressEnd((bz_stream *)ctx->dstrm);
        BZ2_bzDecompressEnd((bz_stream *)ctx->estrm);
        return EZBS_NO_MEMORY;
    }
    memset(ctx->buf, 0, ctx->buf_size);
    memcpy(&ctx->file_cb, file_cb, sizeof(ez_file_cb_t));
    ((bz_stream *)ctx->cstrm)->avail_in = bzctrllen;
    ((bz_stream *)ctx->cstrm)->next_in = (char *)patch + 32;
    ((bz_stream *)ctx->dstrm)->avail_in = bzdatalen;
    ((bz_stream *)ctx->dstrm)->next_in = (char *)patch + 32 + bzctrllen;
    ((bz_stream *)ctx->estrm)->avail_in = patchsize - (32 + bzctrllen + bzdatalen);
    ((bz_stream *)ctx->estrm)->next_in = (char *)patch + (32 + bzctrllen + bzdatalen);
    return 0;
}
BSPATCH_API EZ_RV ez_patch_update(ez_bspatch_ctx_t *ctx, char *newdata, int *newsize)
{
    off_t newpos = 0;
    off_t lenread = 0, lenread_diff = 0, i = 0;
    char buf[8] = {0};
    int ret = 0;
    if (!ctx || !newsize)
    {
        return EZBS_INVALID_PARAM;
    }
    if (!newdata)
    {
        if (newsize)
        {
            *newsize = ctx->new_size;
            return EZBS_SUCESS;
        }
        else
        {
            return EZBS_INVALID_PARAM;
        }
    }
    memset(newdata, 0, *newsize);
    if (0 == ctx->data_len)
    {
        memset(ctx->buf, 0, ctx->buf_size);
        lenread = ctx->file_cb.read((void *)ctx->fd, ctx->old_base + ctx->old_off, ctx->buf_size, ctx->buf);

        if (lenread < 0 || (lenread < ctx->buf_size && ctx->old_base + ctx->old_off + lenread < ctx->old_size))
        {
            ret = EZBS_OLDFILE_READ;
            goto exit;
        }
        ctx->data_len = lenread;
        ctx->data_off = ctx->old_off;
    }
    while (ctx->new_off < ctx->new_size)
    {
        lenread_diff = 0;
        if (0 == ctx->ctrl[0] && 0 == ctx->ctrl[1])
        {
            /* Read one block form control data */
            for (i = 0; i <= 2; i++)
            {
                ((bz_stream *)ctx->cstrm)->avail_out = 8;
                ((bz_stream *)ctx->cstrm)->next_out = buf;
                ret = BZ2_bzDecompress((bz_stream *)ctx->cstrm);
                if ((ret != BZ_OK && ret != BZ_STREAM_END) ||
                    (ret == BZ_OK && ((bz_stream *)ctx->cstrm)->avail_in == 0 && ((bz_stream *)ctx->cstrm)->avail_out > 0))
                {
                    //BZ_UNEXPECTED_EOF
                    ret = EZBS_UNEXPECTED_CDATA;
                    goto exit;
                };
                if (ret == BZ_STREAM_END)
                    lenread = 8 - ((bz_stream *)ctx->cstrm)->avail_out;
                if (((bz_stream *)ctx->cstrm)->avail_out == 0)
                    lenread = 8;
                if (lenread < 8)
                {
                    ret = EZBS_UNEXPECTED_CDATA;
                    goto exit;
                }

                ctx->ctrl[i] = offtin((unsigned char *)buf);
            };
        }
        /* Read diff string */
        if (0 != ctx->ctrl[0])
        {
            if (newpos == *newsize)
            {
                ret = EZBS_NEW_DATA_BUF_FULL;
                goto exit;
            }
            else if (newpos > *newsize)
            {
                ret = EZBS_UNEXPECTED_CDATA;
                goto exit;
            }
            else if (newpos + ctx->ctrl[0] > *newsize)
            {
                lenread_diff = *newsize - newpos;
            }
            else
            {
                lenread_diff = ctx->ctrl[0];
            }
            ((bz_stream *)ctx->dstrm)->avail_out = lenread_diff;
            ((bz_stream *)ctx->dstrm)->next_out = newdata + newpos;
            ret = BZ2_bzDecompress((bz_stream *)ctx->dstrm);
            if ((ret != BZ_OK && ret != BZ_STREAM_END) ||
                (ret == BZ_OK && ((bz_stream *)ctx->dstrm)->avail_in == 0 && ((bz_stream *)ctx->dstrm)->avail_out > 0))
            {
                //BZ_UNEXPECTED_EOF
                ret = EZBS_UNEXPECTED_DDATA;
                goto exit;
            };

            if (ret == BZ_STREAM_END && ((bz_stream *)ctx->dstrm)->avail_out != 0)
            {
                ret = EZBS_UNEXPECTED_DDATA;
                goto exit;
            }
        }
        /* 如果缓存的值不在偏移范围内，需要重新读取*/
        if (ctx->old_off < ctx->data_off || ctx->old_off + lenread_diff > ctx->data_off + ctx->data_len)
        {
            ctx->data_off = 0;
            ctx->data_len = 0;
        }

        /* Add old data to diff string */
        for (i = 0; i < lenread_diff; i++)
        {
            if (0 == ctx->data_len)
            {
                memset(ctx->buf, 0, ctx->buf_size);
                lenread = ctx->file_cb.read((void *)ctx->fd, ctx->old_base + ctx->old_off + i, ctx->buf_size, ctx->buf);

                if (lenread < 0 || (lenread < ctx->buf_size && ctx->old_base + ctx->old_off + i + lenread < ctx->old_size))
                {
                    ret = EZBS_OLDFILE_READ;
                    goto exit;
                }
                ctx->data_len = lenread;
                ctx->data_off = ctx->old_off + i;
            }
            if ((ctx->old_off + i >= 0) && (ctx->old_off + i < ctx->old_size))
            {
                newdata[newpos + i] += ctx->buf[ctx->old_off - ctx->data_off + i];
            }
        }
        /* Adjust pointers */
        ctx->new_off += lenread_diff;
        newpos += lenread_diff;
        ctx->old_off += lenread_diff;
        ctx->ctrl[0] -= lenread_diff;
        if (ctx->ctrl[0] > 0)
        {
            *newsize = newpos;
            ret = EZBS_NEW_DATA_BUF_FULL;
            goto exit;
        }

        if (0 != ctx->ctrl[1])
        {
            if (newpos == *newsize)
            {
                ret = EZBS_NEW_DATA_BUF_FULL;
                goto exit;
            }
            else if (newpos > *newsize)
            {
                ret = EZBS_UNEXPECTED_EDATA;
                goto exit;
            }
            if (newpos + ctx->ctrl[1] > *newsize)
            {
                lenread_diff = *newsize - newpos;
            }
            else
            {
                lenread_diff = ctx->ctrl[1];
            }

            /* Read extra string */
            ((bz_stream *)ctx->estrm)->avail_out = lenread_diff;
            ((bz_stream *)ctx->estrm)->next_out = newdata + newpos;
            ret = BZ2_bzDecompress((bz_stream *)ctx->estrm);
            if ((ret != BZ_OK && ret != BZ_STREAM_END) ||
                (ret == BZ_OK && ((bz_stream *)ctx->estrm)->avail_in == 0 && ((bz_stream *)ctx->estrm)->avail_out > 0))
            {
                //BZ_UNEXPECTED_EOF
                ret = EZBS_UNEXPECTED_EDATA;
                goto exit;
            };

            if (ret == BZ_STREAM_END && ((bz_stream *)ctx->estrm)->avail_out != 0)
            {
                ret = EZBS_UNEXPECTED_EDATA;
                goto exit;
            }

            ctx->new_off += lenread_diff;
            newpos += lenread_diff;
            ctx->ctrl[1] -= lenread_diff;
        }

        if (ctx->ctrl[1] > 0)
        {
            *newsize = newpos;
            ret = EZBS_NEW_DATA_BUF_FULL;
            goto exit;
        }

        /* Adjust pointers */
        ctx->old_off += ctx->ctrl[2];
    };

    *newsize = newpos;
    ret = 0;
exit:

    return ret;
}

BSPATCH_API EZ_RV ez_patch_finit(ez_bspatch_ctx_t *ctx)
{
    if (NULL != ctx->buf)
    {
        free(ctx->buf);
    }

    if (ctx->fd)
    {
        ctx->file_cb.close((void *)ctx->fd);
    }

    if (ctx->cstrm)
    {
        BZ2_bzDecompressEnd((bz_stream *)ctx->cstrm);
        free(ctx->cstrm);
    }

    if (ctx->dstrm)
    {
        BZ2_bzDecompressEnd((bz_stream *)ctx->dstrm);
        free(ctx->dstrm);
    }

    if (ctx->estrm)
    {
        BZ2_bzDecompressEnd((bz_stream *)ctx->estrm);
        free(ctx->estrm);
    }

    memset(ctx, 0, sizeof(*ctx));
    return EZBS_SUCESS;
}

char *ez_patch_version(void)
{
    static char buf[64] = {0};
    int year = 0;
    int month = 0;
    int day = 0;
    char month_name[4] = {0};
    const char *all_mon_names[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

    sscanf(__DATE__, "%s%d%d", month_name, &day, &year);

    for (month = 0; month < 12; month++)
    {
        if (strcmp(month_name, all_mon_names[month]) == 0)
        {
            break;
        }
    }

    month++;
    year -= 2000;
    sprintf(buf, "%s build %02d%02d%02d", FIRMWARE_VERSION, year, month, day);

    return buf;
}