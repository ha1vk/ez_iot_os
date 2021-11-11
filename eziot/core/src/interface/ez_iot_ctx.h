
#ifndef _EZ_IOT_CTX_H_
#define _EZ_IOT_CTX_H_

#include "ez_iot_def.h"
#include "ezdev_sdk_kernel_struct.h"

#ifdef __cplusplus
extern "C"
{
#endif

    ez_void_t ez_iot_event_adapt(ez_kernel_event_t *ptr_event);

    ez_int32_t ez_iot_value_save(sdk_keyvalue_type valuetype, ez_int8_t *keyvalue, ez_int32_t keyvalue_size);

    void ez_iot_event_notice_set(const ez_event_notice pfunc);

    void ez_iot_devid_set(const ez_byte_t *devid);

    const ez_byte_t *ez_iot_devid_get(ez_void_t);

#ifdef __cplusplus
}
#endif

#endif