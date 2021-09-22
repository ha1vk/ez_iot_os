/*
 * This file is part of the EasyLogger Library.
 *
 * Copyright (c) 2015-2019, Qintl, <qintl_linux@163.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * 'Software'), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Function: Save log to file.
 * Created on: 2019-01-05
 */

#include <string.h>
#include <elog_file.h>
#include <elog_file_cfg.h>
#include "file_interface.h"
#include "io_interface.h"
#include "mem_interface.h"
#include "network_interface.h"
#include "thread_interface.h"
#include "time_interface.h"

/* initialize OK flag */
static bool init_ok = false;
static int g_logfile_fd = -1;
static ElogFileCfg local_cfg;

ElogErrCode elog_file_init(void)
{
    ElogErrCode result = ELOG_NO_ERR;
    ElogFileCfg cfg;
    if (init_ok)
        goto __exit;
    do
    {
        result = elog_file_port_init();
        if(ELOG_NO_ERR != result)
        {
            break;
        }
        cfg.name = ELOG_FILE_NAME;
        cfg.max_size = ELOG_FILE_MAX_SIZE;
        cfg.max_rotate = ELOG_FILE_MAX_ROTATE;

        elog_file_config(&cfg);

        init_ok = true;
    } while (0);

__exit:
    return result;
}

/*
 * Reopen file
 */
static bool elog_file_reopen(void)
{
    int tmp_fd;

    tmp_fd = ez_open(local_cfg.name, O_RDWR|O_CREAT, S_IRWXU|S_IRWXG|S_IRWXO);
    if (tmp_fd != -1) {
        if (g_logfile_fd != -1)
            ez_close(g_logfile_fd);

        g_logfile_fd = tmp_fd;
        return true;
    }

    return false;
}

/*
 * rotate the log file xxx.log.n-1 => xxx.log.n, and xxx.log => xxx.log.0
 */
static void elog_file_rotate(void)
{
#define SUFFIX_LEN                     10
    /* mv xxx.log.n-1 => xxx.log.n, and xxx.log => xxx.log.0 */
    int n;
    char oldpath[256], newpath[256];
    size_t base = strlen(local_cfg.name);

    memcpy(oldpath, local_cfg.name, base);
    memcpy(newpath, local_cfg.name, base);

    for (n = local_cfg.max_rotate - 1; n >= 0; --n) {
        ez_snprintf(oldpath + base, SUFFIX_LEN, n ? ".%d" : "", n);
        ez_snprintf(newpath + base, SUFFIX_LEN, ".%d", n);
        ez_rename(oldpath, newpath);
    }
}

/*
 * Check if it needed retate
 */
static bool elog_file_retate_check(void)
{
    struct stat statbuf;
    statbuf.st_size = 0;
    if (ez_stat(local_cfg.name, &statbuf) < 0)
        return false;

    if (statbuf.st_size > local_cfg.max_size)
        return true;

    return false;
}

void elog_file_write(const char *log, size_t size)
{
    ELOG_ASSERT(init_ok);
    ELOG_ASSERT(log);
    struct stat statbuf;
    int ret;

    statbuf.st_size = 0;
    if(g_logfile_fd == -1)
        return;
    elog_file_port_lock();

    ez_fstat(g_logfile_fd, &statbuf);

    if (unlikely(statbuf.st_size > local_cfg.max_size)) {
#if ELOG_FILE_MAX_ROTATE > 0
	    if (elog_file_retate_check()) {
                /* rotate the log file */
                elog_file_rotate();
        }

        if (!elog_file_reopen()) {
            elog_file_port_unlock();
            return;
        }
#else
        return ;
#endif
    }

    ret = ez_write(g_logfile_fd, log, size);
    if(ret < 0){
        
    }

    elog_file_port_unlock();
}

void elog_file_deinit(void)
{
    ELOG_ASSERT(init_ok);
    elog_file_port_deinit();
    if (g_logfile_fd != -1)
        ez_close(g_logfile_fd);
}

void elog_file_config(ElogFileCfg *cfg)
{
    if (g_logfile_fd != -1) {
        ez_close(g_logfile_fd);
    }

    elog_file_port_lock();

    local_cfg.name = cfg->name;
    local_cfg.max_size = cfg->max_size;
    local_cfg.max_rotate = cfg->max_rotate;

    g_logfile_fd = ez_open(local_cfg.name,  O_RDWR|O_CREAT, S_IRWXU|S_IRWXG|S_IRWXO);
    if (g_logfile_fd == -1)
    {

    }
    else
    {
        ez_lseek(g_logfile_fd, 0, SEEK_END);
    }

    elog_file_port_unlock();
}
