#ifndef EZ_PT_LIGHT_MODE_H
#define EZ_PT_LIGHT_MODE_H

typedef enum
{
    MODE_MIN = 0,

    MODE_NO_ROUTE,
    MODE_WEAK_SIGNAL,

    MODE_PT1_NORMAL,
    MODE_PT1_RETEST,

    MODE_PT2_NORMAL,
    MODE_PT2_END,

    MODE_PT3_NORMAL,

    MODE_RESET_FACTORY,

    MODE_AP_START,
    MODE_AP_CLIENT_CONN,
    MODE_AP_CONN_ROUTE,
    MODE_AP_CONN_SUCC,
    MODE_AP_CONN_FAIL,
    MODE_AP_TIMEOUT,
} pt_light_mode_e;

int pt_light_init(int type);

int pt_light_stage2_time(int stage2_time);

int pt_light_set_mode(pt_light_mode_e mode);

int pt_light_deinit();

#endif