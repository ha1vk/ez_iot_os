

#ifndef _H_OTA_SAMPLE_H_
#define _H_OTA_SAMPLE_H_

#include <assert.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef __cplusplus
extern "C"
{
#endif

    int ota_sample_start();

    int ota_sample_stop();

    int ota_sample_module_info_report();

    int ota_sample_status_report(int status);

#ifdef __cplusplus
}
#endif

#endif
