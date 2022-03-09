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
} pt_light_mode_e;

int pt_light_init(int type);

int pt_light_stage2_time(int stage2_time);

int pt_light_set_mode(pt_light_mode_e mode);

int pt_light_deinit();

#endif