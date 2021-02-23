#include "log_platform_wrapper.h"
#include <stdarg.h>
#include <time.h>


void time_print(int sdk_error, int othercode, const char * buf)
{
	char now_time[16];
	time_t t = time(0);
	strftime(now_time, 16, "%m-%d %H:%M:%S", localtime(&t));

	printf("[%s][%d][%d]sdk_error:%d, othercode:%d, info:%s \n", now_time, getpid(), 0, sdk_error, othercode, buf);
}

void log_print_error(int sdk_error, int othercode, const char * buf)
{
	time_print(sdk_error, othercode, buf);
}

void log_print_warn(int sdk_error, int othercode, const char * buf)
{
	time_print(sdk_error, othercode, buf);
}

void log_print_info(int sdk_error, int othercode, const char * buf)
{
	time_print(sdk_error, othercode, buf);
}

void log_print_debug(int sdk_error, int othercode, const char * buf)
{
	time_print(sdk_error, othercode, buf);
}

void log_print_trace(int sdk_error, int othercode, const char * buf)
{
	time_print(sdk_error, othercode, buf);
}


void log_print(const char *fmt, ...)
{
	va_list ap;
	char logbuf[256];
	memset(logbuf, 0, 256);

	va_start(ap, fmt);
	vsnprintf(logbuf, 256, fmt,ap);
	va_end(ap);

	printf("log_print: %s \n", logbuf);
}