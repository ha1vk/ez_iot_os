/*******************************************************************************
 * Copyright © 2017-2021 Ezviz Inc.
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
 * 2021-11-23     xurongjun    first version 
 *******************************************************************************/

#include "ez_iot_shadow.h"
#include "ez_iot_shadow_def.h"
#include "ez_iot_shadow_extern.h"
#include "ez_iot_shadow_core.h"

static ez_bool_t g_shd_is_inited = ez_false;

EZOS_API ez_err_t ez_iot_shadow_init(ez_shadow_notice pfunc)
{
    FUNC_IN();

    ez_err_t rv = EZ_SHD_ERR_SUCC;

    rv = shadow_extern_init();
    CHECK_RV_DONE(rv);

    CHECK_COND_DONE(!shadow_core_start(pfunc), EZ_SHD_ERR_MEMORY);

    g_shd_is_inited = ez_true;
done:
    FUNC_OUT();

    return rv;
}

EZOS_API ez_err_t ez_iot_shadow_reg(ez_shadow_res_t *pres, ez_char_t *domain_id, ez_shadow_module_t *module)
{
    FUNC_IN();

    ez_err_t rv = EZ_SHD_ERR_SUCC;

    CHECK_COND_DONE(!g_shd_is_inited, EZ_SHD_ERR_NOT_INIT);
    CHECK_COND_DONE(!pres, EZ_SHD_ERR_PARAM_INVALID);
    CHECK_COND_DONE(!domain_id, EZ_SHD_ERR_PARAM_INVALID);
    CHECK_COND_DONE(!module, EZ_SHD_ERR_PARAM_INVALID);

    rv = shadow_core_module_addv3(pres->dev_serial, pres->res_type, pres->local_index,
                                  domain_id, module->num, (void *)module->business);

    CHECK_RV_DONE(rv);
    shadow_core_event_occured(SHADOW_EVENT_TYPE_ADD);

done:
    FUNC_OUT();

    return rv;
}

EZOS_API ez_err_t ez_iot_shadow_push(ez_shadow_res_t *pres, ez_char_t *domain_id, ez_char_t *pkey, ez_shadow_value_t *pvalue)
{
    FUNC_IN();

    ez_err_t rv = EZ_SHD_ERR_SUCC;

    CHECK_COND_DONE(!g_shd_is_inited, EZ_SHD_ERR_NOT_INIT);
    CHECK_COND_DONE(!pres, EZ_SHD_ERR_PARAM_INVALID);
    CHECK_COND_DONE(!domain_id, EZ_SHD_ERR_PARAM_INVALID);
    CHECK_COND_DONE(!pkey, EZ_SHD_ERR_PARAM_INVALID);

    rv = shadow_core_propertie_changed(pres->dev_serial, pres->res_type,
                                       pres->local_index, domain_id, pkey, pvalue);

    CHECK_RV_DONE(rv);
    shadow_core_event_occured(SHADOW_EVENT_TYPE_REPORT);

done:
    FUNC_OUT();

    return rv;
}

EZOS_API ez_err_t ez_iot_shadow_unreg(ez_char_t *dev_serial)
{
    FUNC_IN();

    ez_err_t rv = EZ_SHD_ERR_SUCC;

    CHECK_COND_DONE(!g_shd_is_inited, EZ_SHD_ERR_NOT_INIT);
    CHECK_COND_DONE(!dev_serial, EZ_SHD_ERR_PARAM_INVALID);

    rv = shadow_core_module_clear(dev_serial);
done:
    FUNC_OUT();

    return rv;
}

EZOS_API ez_void_t ez_iot_shadow_deini(ez_void_t)
{
    FUNC_IN();

    if (!g_shd_is_inited)
    {
        return;
    }

    shadow_extern_deini();

    shadow_core_stop();

    g_shd_is_inited = ez_false;

    FUNC_OUT();
}