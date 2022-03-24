/**
 * @file ezcloud_ota.h
 * @author xurongjun (xurongjun@.com)
 * @brief 
 * @version 0.1
 * @date 2021-03-11
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#ifndef _EZCLOUD_OTA_H_
#define _EZCLOUD_OTA_H_
#include "ezos.h"
#ifdef __cplusplus
extern "C"
{
#endif
    ez_void_t ezcloud_ota_init(ez_void_t);

    ez_int_t ez_ota_reboot_report_reuslt();
#ifdef __cplusplus
}
#endif

#endif