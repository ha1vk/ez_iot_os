#include "log_platform_wrapper.h"
#include "diag.h"

//realtek(free rtos) й╣ож

void time_print(int sdk_error, int othercode, const char * buf)
{
// 	char now_time[16];
// 	time_t t = time(0);
// 	strftime(now_time, 16, "%m-%d %H:%M:%S", localtime(&t));

	DBG_8195A("[NULL][%d][%d]sdk_error:%d, othercode:%d, info:%s \n", 0, 0, sdk_error, othercode, buf);
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