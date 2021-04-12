/*-
 * Copyright 2003-2005 Colin Percival
 * All rights reserved
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted providing that the following conditions 
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#if 0
__FBSDID("$FreeBSD: src/usr.bin/bsdiff/bsdiff/bsdiff.c,v 1.1 2005/08/06 01:59:05 cperciva Exp $");
#endif

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#define EZDIFF_EXPORTS 1
typedef long off_t;
#else
#include <sys/types.h>
#include <err.h>
#include <unistd.h>
#endif

#include <bzlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ez_diff.h"

#define MIN(x, y) (((x) < (y)) ? (x) : (y))

static void split(off_t *I, off_t *V, off_t start, off_t len, off_t h)
{
    off_t i, j, k, x, tmp, jj, kk;

    if (len < 16)
    {
        for (k = start; k < start + len; k += j)
        {
            j = 1;
            x = V[I[k] + h];
            for (i = 1; k + i < start + len; i++)
            {
                if (V[I[k + i] + h] < x)
                {
                    x = V[I[k + i] + h];
                    j = 0;
                };
                if (V[I[k + i] + h] == x)
                {
                    tmp = I[k + j];
                    I[k + j] = I[k + i];
                    I[k + i] = tmp;
                    j++;
                };
            };
            for (i = 0; i < j; i++)
                V[I[k + i]] = k + j - 1;
            if (j == 1)
                I[k] = -1;
        };
        return;
    };

    x = V[I[start + len / 2] + h];
    jj = 0;
    kk = 0;
    for (i = start; i < start + len; i++)
    {
        if (V[I[i] + h] < x)
            jj++;
        if (V[I[i] + h] == x)
            kk++;
    };
    jj += start;
    kk += jj;

    i = start;
    j = 0;
    k = 0;
    while (i < jj)
    {
        if (V[I[i] + h] < x)
        {
            i++;
        }
        else if (V[I[i] + h] == x)
        {
            tmp = I[i];
            I[i] = I[jj + j];
            I[jj + j] = tmp;
            j++;
        }
        else
        {
            tmp = I[i];
            I[i] = I[kk + k];
            I[kk + k] = tmp;
            k++;
        };
    };

    while (jj + j < kk)
    {
        if (V[I[jj + j] + h] == x)
        {
            j++;
        }
        else
        {
            tmp = I[jj + j];
            I[jj + j] = I[kk + k];
            I[kk + k] = tmp;
            k++;
        };
    };

    if (jj > start)
        split(I, V, start, jj - start, h);

    for (i = 0; i < kk - jj; i++)
        V[I[jj + i]] = kk - 1;
    if (jj == kk - 1)
        I[jj] = -1;

    if (start + len > kk)
        split(I, V, kk, start + len - kk, h);
}

static void qsufsort(off_t *I, off_t *V, u_char *old, off_t oldsize)
{
    off_t buckets[256];
    off_t i, h, len;

    for (i = 0; i < 256; i++)
        buckets[i] = 0;
    for (i = 0; i < oldsize; i++)
        buckets[old[i]]++;
    for (i = 1; i < 256; i++)
        buckets[i] += buckets[i - 1];
    for (i = 255; i > 0; i--)
        buckets[i] = buckets[i - 1];
    buckets[0] = 0;

    for (i = 0; i < oldsize; i++)
        I[++buckets[old[i]]] = i;
    I[0] = oldsize;
    for (i = 0; i < oldsize; i++)
        V[i] = buckets[old[i]];
    V[oldsize] = 0;
    for (i = 1; i < 256; i++)
        if (buckets[i] == buckets[i - 1] + 1)
            I[buckets[i]] = -1;
    I[0] = -1;

    for (h = 1; I[0] != -(oldsize + 1); h += h)
    {
        len = 0;
        for (i = 0; i < oldsize + 1;)
        {
            if (I[i] < 0)
            {
                len -= I[i];
                i -= I[i];
            }
            else
            {
                if (len)
                    I[i - len] = -len;
                len = V[I[i]] + 1 - i;
                split(I, V, i, len, h);
                i += len;
                len = 0;
            };
        };
        if (len)
            I[i - len] = -len;
    };

    for (i = 0; i < oldsize + 1; i++)
        I[V[i]] = i;
}

static off_t matchlen(u_char *old, off_t oldsize, u_char *new, off_t newsize)
{
    off_t i;

    for (i = 0; (i < oldsize) && (i < newsize); i++)
        if (old[i] != new[i])
            break;

    return i;
}
static off_t search(off_t *I, u_char *old, off_t oldsize,
                    u_char *new, off_t newsize, off_t st, off_t en, off_t *pos)
{
    off_t x, y;
    if (en - st < 2)
    {
        x = matchlen(old + I[st], oldsize - I[st], new, newsize);
        y = matchlen(old + I[en], oldsize - I[en], new, newsize);

        if (x > y)
        {
            *pos = I[st];
            return x;
        }
        else
        {
            *pos = I[en];
            return y;
        }
    };
    x = st + (en - st) / 2;
    if (memcmp(old + I[x], new, MIN(oldsize - I[x], newsize)) < 0)
    {
        return search(I, old, oldsize, new, newsize, x, en, pos);
    }
    else
    {
        return search(I, old, oldsize, new, newsize, st, x, pos);
    };
}
static void offtout(off_t x, u_char *buf)
{
    off_t y;
    if (x < 0)
        y = -x;
    else
        y = x;
    buf[0] = y % 256;
    y -= buf[0];
    y = y / 256;
    buf[1] = y % 256;
    y -= buf[1];
    y = y / 256;
    buf[2] = y % 256;
    y -= buf[2];
    y = y / 256;
    buf[3] = y % 256;
    y -= buf[3];
    y = y / 256;
    buf[4] = y % 256;
    y -= buf[4];
    y = y / 256;
    buf[5] = y % 256;
    y -= buf[5];
    y = y / 256;
    buf[6] = y % 256;
    y -= buf[6];
    y = y / 256;
    buf[7] = y % 256;

    if (x < 0)
        buf[7] |= 0x80;
}

EZDIFF_API int ez_diff(const char *old_path, const char *new_path, const char *patch_path, int level)
{
    int compress_level = level;
    u_char *old, *new;
    off_t oldsize, newsize;
    off_t *I, *V;
    off_t scan, pos, len;
    off_t lastscan, lastpos, lastoffset;
    off_t oldscore, scsc;
    off_t s, Sf, lenf, Sb, lenb;
    off_t overlap, Ss, lens;
    off_t i;
    off_t dblen, eblen;
    u_char *db, *eb;
    u_char buf[8];
    u_char header[32];
    FILE *pf, *pfc, *pfd, *pfe;
	const char *temp_c = "temp_c";
	const char *temp_d = "temp_d";
	const char *temp_e = "temp_e";
    BZFILE *pfbz2;
    int bz2err;
    char mode[10] = {0};
    int rv = -1;

    if (compress_level < 1)
        compress_level = 1;
    else if (compress_level > 9)
        compress_level = 9;
    pfc = 0;
    pfd = 0;
    pfe = 0;

    /* Allocate oldsize+1 bytes instead of oldsize bytes to ensure
        that we never try to malloc(0) and get a NULL pointer */
    if (((pf = fopen(old_path, "rb")) < 0) ||
        ((fseek(pf, 0, SEEK_END)) != 0) ||
        ((oldsize = ftell(pf)) == -1) ||
        ((old = malloc(oldsize + 1)) == NULL) ||
        (fseek(pf, 0, SEEK_SET) != 0) ||
        (fread(old, 1, oldsize, pf) != oldsize) ||
        (fclose(pf) == -1))
    {
        printf("read old file err\n");
        goto done;
    }

    if (((I = malloc((oldsize + 1) * sizeof(off_t))) == NULL) ||
        ((V = malloc((oldsize + 1) * sizeof(off_t))) == NULL))
    {
        printf("no mem\n");
        goto done;
    }

    qsufsort(I, V, old, oldsize);

    free(V);

    /* Allocate newsize+1 bytes instead of newsize bytes to ensure
        that we never try to malloc(0) and get a NULL pointer */
    if (((pf = fopen(new_path, "rb")) < 0) ||
        ((fseek(pf, 0, SEEK_END)) != 0) ||
        ((newsize = ftell(pf)) == -1) ||
        ((new = malloc(newsize + 1)) == NULL) ||
        (fseek(pf, 0, SEEK_SET) != 0) ||
        (fread(new, 1, newsize, pf) != newsize) ||
        (fclose(pf) == -1))
    {
        printf("read new file err\n");
        goto done;
    }

    if (((db = malloc(newsize + 1)) == NULL) ||
        ((eb = malloc(newsize + 1)) == NULL))
    {
        printf("no mem\n");
        goto done;
    }

    dblen = 0;
    eblen = 0;

    /* Compute the differences, writing ctrl as we go */
    sprintf(mode, "w%d", compress_level);
    if ((pfbz2 = BZ2_bzopen(temp_c, mode)) == NULL)
    {
        printf("BZ2_bzopen c");
        goto done;
    }

    scan = 0;
    len = 0;
    lastscan = 0;
    lastpos = 0;
    lastoffset = 0;
    while (scan < newsize)
    {
        oldscore = 0;

        for (scsc = scan += len; scan < newsize; scan++)
        {
            len = search(I, old, oldsize, new + scan, newsize - scan,
                         0, oldsize, &pos);

            for (; scsc < scan + len; scsc++)
                if ((scsc + lastoffset < oldsize) &&
                    (old[scsc + lastoffset] == new[scsc]))
                    oldscore++;

            if (((len == oldscore) && (len != 0)) ||
                (len > oldscore + 8))
                break;

            if ((scan + lastoffset < oldsize) &&
                (old[scan + lastoffset] == new[scan]))
                oldscore--;
        };

        if ((len != oldscore) || (scan == newsize))
        {
            s = 0;
            Sf = 0;
            lenf = 0;
            for (i = 0; (lastscan + i < scan) && (lastpos + i < oldsize);)
            {
                if (old[lastpos + i] == new[lastscan + i])
                    s++;
                i++;
                if (s * 2 - i > Sf * 2 - lenf)
                {
                    Sf = s;
                    lenf = i;
                };
            };

            lenb = 0;
            if (scan < newsize)
            {
                s = 0;
                Sb = 0;
                for (i = 1; (scan >= lastscan + i) && (pos >= i); i++)
                {
                    if (old[pos - i] == new[scan - i])
                        s++;
                    if (s * 2 - i > Sb * 2 - lenb)
                    {
                        Sb = s;
                        lenb = i;
                    };
                };
            };

            if (lastscan + lenf > scan - lenb)
            {
                overlap = (lastscan + lenf) - (scan - lenb);
                s = 0;
                Ss = 0;
                lens = 0;
                for (i = 0; i < overlap; i++)
                {
                    if (new[lastscan + lenf - overlap + i] ==
                        old[lastpos + lenf - overlap + i])
                        s++;
                    if (new[scan - lenb + i] ==
                        old[pos - lenb + i])
                        s--;
                    if (s > Ss)
                    {
                        Ss = s;
                        lens = i + 1;
                    };
                };

                lenf += lens - overlap;
                lenb -= lens;
            };

            for (i = 0; i < lenf; i++)
                db[dblen + i] = new[lastscan + i] - old[lastpos + i];
            for (i = 0; i < (scan - lenb) - (lastscan + lenf); i++)
                eb[eblen + i] = new[lastscan + lenf + i];

            dblen += lenf;
            eblen += (scan - lenb) - (lastscan + lenf);

            offtout(lenf, buf);
            BZ2_bzWrite(&bz2err, pfbz2, buf, 8);
            if (bz2err != BZ_OK)
            {
                printf("BZ2_bzWrite, bz2err = %d", bz2err);
                goto done;
            }

            offtout((scan - lenb) - (lastscan + lenf), buf);
            BZ2_bzWrite(&bz2err, pfbz2, buf, 8);
            if (bz2err != BZ_OK)
            {
                printf("BZ2_bzWrite, bz2err = %d", bz2err);
                goto done;
            }

            offtout((pos - lenb) - (lastpos + lenf), buf);
            BZ2_bzWrite(&bz2err, pfbz2, buf, 8);
            if (bz2err != BZ_OK)
            {
                printf("BZ2_bzWrite, bz2err = %d", bz2err);
                goto done;
            }

            lastscan = scan - lenb;
            lastpos = pos - lenb;
            lastoffset = pos - scan;
        };
    };

    BZ2_bzclose(pfbz2);

    /* Write compressed diff data */
    if ((pfbz2 = BZ2_bzopen(temp_d, mode)) == NULL)
    {
        printf("BZ2_bzopen d");
        goto done;
    }

    BZ2_bzWrite(&bz2err, pfbz2, db, dblen);
    if (bz2err != BZ_OK)
    {
        printf("BZ2_bzWrite, bz2err = %d", bz2err);
        goto done;
    }

    BZ2_bzclose(pfbz2);

    /* Write compressed extra data */
    if ((pfbz2 = BZ2_bzopen(temp_e, mode)) == NULL)
    {
        printf("BZ2_bzopen e");
        goto done;
    }

    BZ2_bzWrite(&bz2err, pfbz2, eb, eblen);
    if (bz2err != BZ_OK)
    {
        printf("BZ2_bzWrite, bz2err = %d", bz2err);
        goto done;
    }

    BZ2_bzclose(pfbz2);

    /* Header is
	0	8	 "BSDIFF40"
	8	8	length of bzip2ed ctrl block
	16	8	length of bzip2ed diff block
	24	8	length of new file */

    /* File is
    0   32	Header
    32  ??	Bzip2ed ctrl block
    ??  ??  Bzip2ed diff block
    ??  ??  Bzip2ed extra block */

    /* Create the patch file */
    if ((pf = fopen(patch_path, "wb")) == NULL)
    {
        printf("read patch file\n");
        goto done;
    }

    memcpy(header, "BSDIFF40", 8);
    offtout(0, header + 8);
    offtout(0, header + 16);
    offtout(newsize, header + 24);

    if (fwrite(header, 32, 1, pf) != 1)
    {
        printf("fwrite patch file\n");
        goto done;
    }

    /* Compute size of compressed ctrl data */
    if (((pfc = fopen("temp_c", "rb")) < 0) ||
        ((fseek(pfc, 0, SEEK_END)) != 0) ||
        ((len = ftell(pfc)) == -1) ||
		(fseek(pfc, 0, SEEK_SET) != 0))
    {
        printf("read temp_c\n");
        goto done;
    }

    offtout(len, header + 8);

    while (len = fread(new, 1, newsize, pfc))
    {
        fwrite(new, 1, len, pf);
    }

    /* Compute size of compressed diff data */
    if (((pfd = fopen("temp_d", "rb")) < 0) ||
        ((fseek(pfd, 0, SEEK_END)) != 0) ||
        ((len = ftell(pfd)) == -1) ||
		(fseek(pfd, 0, SEEK_SET) != 0))
    {
        printf("read temp_d\n");
        goto done;
    }

    offtout(len, header + 16);

    while (len = fread(new, 1, newsize, pfd))
    {
        fwrite(new, 1, len, pf);
    }

    /* Compute size of compressed diff data */
    if (((pfe = fopen("temp_e", "rb")) < 0) ||
        ((fseek(pfe, 0, SEEK_END)) != 0) ||
        ((len = ftell(pfe)) == -1) ||
		(fseek(pfe, 0, SEEK_SET) != 0))
    {
        printf("read temp_e\n");
        goto done;
    }

    while (len = fread(new, 1, newsize, pfe))
    {
        fwrite(new, 1, len, pf);
    }

    /* Seek to the beginning, write the header, and close the file */
    if (fseek(pf, 0, SEEK_SET))
    {
        printf("fseeko");
        goto done;
    }
    if (fwrite(header, 32, 1, pf) != 1)
    {
        printf("write patch file err\n");
        goto done;
    }

    rv = 0;
done:

    if (pf)
        fclose(pf);

    if (pfc)
	{
		fclose(pfc);
		remove(temp_c);
	}

    if (pfd)
	{
		fclose(pfd);
		remove(temp_d);
	}

    if (pfe)
	{
		fclose(pfe);
		remove(temp_e);
	}

    /* Free the memory we used */
    if (db)
        free(db);
    if (eb)
        free(eb);
    if (I)
        free(I);
    if (old)
        free(old);
    if (new)
        free(new);

    return rv;
}