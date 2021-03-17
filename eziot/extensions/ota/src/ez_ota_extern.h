#ifndef H_EZ_OTA_EXTERN_H_
#define H_EZ_OTA_EXTERN_H_

#include "ez_sdk_errno.h"
#include "ez_sdk_ota.h"
#include "ez_ota_def.h"

#ifdef __cplusplus
extern "C"
{
#endif

ez_err ez_ota_extern_init();

int ez_ota_extern_fini();

int ez_get_exit_status();

ez_err ezdev_ota_module_info_report(const ota_res_t *pres, const ota_modules_t* pmodules, const unsigned int timeout_ms);

#ifdef __cplusplus
}
#endif

#endif