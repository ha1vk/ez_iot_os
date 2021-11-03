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
 * Contributors:
 * XuRongjun (xurongjun@ezvizlife.com)
 *******************************************************************************/
#include <stdarg.h>
#include <elog.h>
#include "ezlog.h"

ez_int32_t ezlog_init(ez_void_t)
{
    ez_int32_t rv = elog_init();

    if (ELOG_NO_ERR == rv)
    {
        elog_set_fmt(ELOG_LVL_ASSERT, ELOG_FMT_ALL);
        elog_set_fmt(ELOG_LVL_ERROR, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME | ELOG_FMT_LINE);
        elog_set_fmt(ELOG_LVL_WARN, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME | ELOG_FMT_LINE);
        elog_set_fmt(ELOG_LVL_INFO, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME | ELOG_FMT_LINE);
        elog_set_fmt(ELOG_LVL_DEBUG, ELOG_FMT_ALL & ~ELOG_FMT_FUNC & ~ELOG_FMT_P_INFO & ~ELOG_FMT_DIR);
        elog_set_fmt(ELOG_LVL_VERBOSE, ELOG_FMT_ALL & ~ELOG_FMT_FUNC & ~ELOG_FMT_P_INFO & ~ELOG_FMT_DIR);
#ifdef ELOG_COLOR_ENABLE
        elog_set_text_color_enabled(true);
#endif
    }

    return rv;
}

ez_void_t ezlog_start(ez_void_t)
{
    elog_start();
}

ez_void_t ezlog_stop(ez_void_t)
{
    /* enable output */
    elog_set_output_enabled(ez_false);

#if defined(ELOG_ASYNC_OUTPUT_ENABLE)
    elog_async_enabled(ez_false);
#elif defined(ELOG_BUF_OUTPUT_ENABLE)
    elog_buf_enabled(ez_false);
#endif
}

ez_void_t ezlog_output(ez_uint8_t level, const ez_char_t *tag, const ez_char_t *file, const ez_char_t *func,
                       const long line, const ez_char_t *format, ...)
{
    va_list ap;
    va_start(ap, format);
    elog_output(level, tag, file, func, line, format, ap);
    va_end(ap);
}

ez_void_t ezlog_filter_lvl(ez_uint8_t level)
{
    elog_set_filter_lvl(level);
}

ez_void_t ezlog_filter_tag(const ez_char_t *tag)
{
    elog_set_filter_tag(tag);
}

ez_void_t ezlog_filter_kw(const ez_char_t *keyword)
{
    elog_set_filter_kw(keyword);
}