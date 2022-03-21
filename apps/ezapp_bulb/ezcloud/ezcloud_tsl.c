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
 * XuRongjun (xurongjun@ezvizlife.com)
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-03-20     xurongjun    first version 
 *******************************************************************************/

#include "ezos.h"
#include "ezlog.h"
#include "ez_iot_tsl.h"

static ez_int32_t tsl_action2dev(const ez_char_t *sn, const ez_tsl_rsc_t *rsc_info, const ez_tsl_key_t *key_info,
                                 const ez_tsl_value_t *value_in, ez_tsl_value_t *value_out)
{
    return 0;
}

static ez_int32_t tsl_property2cloud(const ez_char_t *sn, const ez_tsl_rsc_t *rsc_info, const ez_tsl_key_t *key_info, ez_tsl_value_t *value_out)
{
    ez_int32_t rv;

    return rv;
}

static ez_int32_t tsl_property2dev(const ez_char_t *sn, const ez_tsl_rsc_t *rsc_info, const ez_tsl_key_t *key_info, const ez_tsl_value_t *value)
{
    ez_int32_t rv;

    return rv;
}

static ez_int32_t tsl_notice(ez_tsl_event_e event_type, ez_void_t *data, ez_int32_t len)
{

    switch (event_type)
    {
    case EZ_EVENT_PROPERTY_FULL_REPORT:
    {
        //TODO 全量上报
        break;
    }
    default:
    {
        break;
    }
    }

    return 0;
}

ez_void_t ezcloud_base_init(ez_void_t)
{
    ez_tsl_callbacks_t tsl_things_cbs = {tsl_notice, tsl_action2dev, tsl_property2cloud, tsl_property2dev};

    ez_iot_tsl_init(&tsl_things_cbs);
    ez_iot_tsl_reg(NULL);

    return 0;
}
