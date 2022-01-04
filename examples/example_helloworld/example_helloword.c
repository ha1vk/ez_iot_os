#include "ezos.h"
#include "ezlog.h"

static void example_hello(void)
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
// int main(int argc, char **argv)
// {
//     return example_hello(argc, argv);
// }
#endif