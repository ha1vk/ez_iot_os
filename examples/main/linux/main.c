#include <stdio.h>
#include <unistd.h>

#include "ezlog.h"
#include "cJSON.h"
#include "cli.h"
#include "eztimer.h"
#include "ezxml.h"
#include "flashdb.h"
#include "ez_iot_core.h"
#include "ez_iot_hub.h"
#include "ezos.h"
#include "ezconn.h"

int main()
{
    ezlog_init();
    ezlog_start();
    ezlog_filter_lvl(EZ_ELOG_LVL_VERBOSE);
    ezlog_e("MAIN","Hello world!");
    while (1)
    {
        // ezlog_e("MAIN", "Hello world!");
        sleep(1);
    }
    return 0;
}