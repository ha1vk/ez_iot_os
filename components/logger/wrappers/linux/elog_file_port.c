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
 * Function:  Portable interface for EasyLogger's file log pulgin.
 * Created on: 2019-01-05
 */

#include "elog_file.h"
#include "hal_thread.h"

static void *mutex_handler = NULL;

/**
 * EasyLogger flile log pulgin port initialize
 *
 * @return result
 */
ElogErrCode elog_file_port_init(void)
{
    ElogErrCode result = ELOG_NO_ERR;

    /* add your code here */
    mutex_handler = hal_thread_mutex_create();
    if(NULL == mutex_handler)
    {
        result = ELOG_MALLOC_ERR; 
    }

    return result;
}

/**
 * file log lock
 */
void elog_file_port_lock(void)
{

    /* add your code here */
    hal_thread_mutex_lock(mutex_handler);
}

/**
 * file log unlock
 */
void elog_file_port_unlock(void)
{
    /* add your code here */
    hal_thread_mutex_unlock(mutex_handler);
}

/**
 * file log deinit
 */
void elog_file_port_deinit(void)
{
    /* add your code here */
   if(NULL!=mutex_handler)
   {
        hal_thread_mutex_destroy(mutex_handler);
        mutex_handler = NULL;
    }
}