#include <stdarg.h>
#include "ez_sdk_log.h"
#include "elog.h"

int32_t ez_sdk_log_start(void)
{
    int32_t rv = elog_init();
    do
    {
        if (ELOG_NO_ERR != rv)
        {
            break;
        }
        
        elog_set_fmt(ELOG_LVL_ASSERT, ELOG_FMT_ALL);
        elog_set_fmt(ELOG_LVL_ERROR, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME | ELOG_FMT_LINE);
        elog_set_fmt(ELOG_LVL_WARN, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME | ELOG_FMT_LINE);
        elog_set_fmt(ELOG_LVL_INFO, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME | ELOG_FMT_LINE);
        elog_set_fmt(ELOG_LVL_DEBUG, ELOG_FMT_ALL & ~ELOG_FMT_FUNC & ~ELOG_FMT_P_INFO & ~ELOG_FMT_DIR);
        elog_set_fmt(ELOG_LVL_VERBOSE, ELOG_FMT_ALL & ~ELOG_FMT_FUNC & ~ELOG_FMT_P_INFO & ~ELOG_FMT_DIR);
#ifdef ELOG_COLOR_ENABLE
        elog_set_text_color_enabled(true);
#endif
        rv = elog_start();

    }while(0);

    return rv;
}

void ez_sdk_log_stop(void)
{
    /* enable output */
    elog_stop();
}

void ez_sdk_log_print(uint8_t level, const char *tag, const char *file, const char *func,
                       const long line, const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    elog_output(level, tag, file, func, line, format, ap);
    va_end(ap);
}

void ez_sdk_set_log_level(uint8_t level)
{
    elog_set_filter_lvl(level);
}

uint8_t ez_sdk_log_filter_lvl_get(void)
{
    return elog_get_filter_lvl();
}

void ez_sdk_log_filter_tag(const char *tag)
{
    if(NULL == tag)
    {
        return;
    }

    elog_set_filter_tag(tag);
}

void ez_sdk_log_filter_kw(const char *keyword)
{
    if(NULL == keyword)
    {
        return;
    }
    elog_set_filter_kw(keyword);
}

void ez_sdk_log_hexdump(const char *tag, uint8_t width, uint8_t *buf, uint16_t size)
{
    if(NULL ==tag||NULL == buf)
    {
        return;
    }
    elog_hexdump(tag, width, buf, size);
}