/*******************************************************************************
 * Copyright © 2017-2022 Ezviz Inc.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 * 
 * Contributors:
 * ChenTengfei (chentengfei5@ezvizlife.com) - Smart bulb application Production test light status
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-03-15     ChenTengfei  first version 
 *******************************************************************************/

#ifndef EZ_PT_LIGHT_MODE_H
#define EZ_PT_LIGHT_MODE_H

#include "ezos.h"

#ifdef __cplusplus
extern "C"
{
#endif

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

#ifdef __cplusplus
}
#endif

#endif