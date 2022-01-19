#include "ezos.h"
#include "ezlog.h"
#include "cli.h"

static void example_hello(char *buf, int len, int argc, char **argv)
{
    ezlog_init();
    ezlog_start();
    ezlog_filter_lvl(EZ_ELOG_LVL_VERBOSE);

    ezlog_a(TAG_APP, "ezapp, hello world!");
    ezlog_e(TAG_APP, "ezapp, hello world!");
    ezlog_w(TAG_APP, "ezapp, hello world!");
    ezlog_i(TAG_APP, "ezapp, hello world!");
    ezlog_d(TAG_APP, "ezapp, hello world!");
    ezlog_v(TAG_APP, "ezapp, hello world!");

    ezlog_stop();
}

#ifdef FINSH_USING_MSH
MSH_CMD_EXPORT(example_hello, eziot example helloworld);
#else
EZOS_CLI_EXPORT("example_helloworld", "helloworld test", example_hello);
#endif