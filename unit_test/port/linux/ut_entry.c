#include <stdio.h>
#include <stdlib.h>
#include "ezos_config.h"
#include "utest.h"

int main(int argc, char **argv)
{
    utest_log_lv_set(CONFIG_EZIOT_UNIT_TEST_REPORT_LOGLVL);

    system("mkdir -p cache");
    utest_testcase_run(argc, argv);

    return 0;
}