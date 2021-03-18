
#ifndef _EZ_OTA_BUS_H_
#define _EZ_OTA_BUS_H_

#include <stdio.h>

#include "ez_sdk_errno.h"
#include "ez_sdk_ota.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define EZ_OTA_RSP 1
#define EZ_OTA_REQ 0

    ez_err_e ez_ota_send_msg_to_platform(unsigned char *msg, int msg_len, const ota_res_t *pres, const char *msg_type,
                                         const char *method, int response, unsigned int *msg_seq, int msg_qos);
#ifdef __cplusplus
}
#endif
#endif