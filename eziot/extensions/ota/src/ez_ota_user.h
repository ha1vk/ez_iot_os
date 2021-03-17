#ifndef H_EZ_OTA_USER_H_
#define H_EZ_OTA_USER_H_

#include "ez_sdk_ota.h"

#ifdef  __cplusplus
extern "C" {
#endif

void  ez_ota_user_init(ota_msg_cb_t cb);

ota_msg_cb_t *ez_ota_get_callback();

#ifdef __cplusplus
}
#endif

#endif
