#ifndef _EZCLOUD_ACCESS_H_
#define _EZCLOUD_ACCESS_H_

#include "ezos_def.h"

#ifdef __cplusplus
extern "C"
{
#endif

    EZ_INT ez_cloud_init();

    EZ_INT ez_cloud_start();

    const ez_char_t *ez_cloud_get_sn();

    const ez_char_t *ez_cloud_get_ver();

    const ez_char_t *ez_cloud_get_type();

    void ez_cloud_deint();
    
    EZ_INT dev_event_waitfor(EZ_INT event_id, EZ_INT time_ms);

    EZ_INT ez_tsl_init(void);

#ifdef __cplusplus
}
#endif

#endif
