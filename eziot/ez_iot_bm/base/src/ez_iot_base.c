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
 * 2021-11-25     xurongjun    first version 
 *******************************************************************************/

#include "ez_iot_base.h"
#include "ez_iot_base_def.h"
#include "ez_iot_base_extern.h"
#include "ez_iot_base_protocol.h"
#include "ez_iot_base_ctx.h"

static ez_bool_t g_base_is_inited = ez_false;

EZOS_API ez_err_t ez_iot_base_init(const ez_base_notice pfunc)
{
    FUNC_IN();

    ez_err_t rv = EZ_BASE_ERR_SUCC;
    CHECK_COND_DONE(!pfunc, EZ_BASE_ERR_PARAM_INVALID);

    rv = base_extern_init();
    CHECK_RV_DONE(rv);

    base_notice_set(pfunc);
    g_base_is_inited = ez_true;
done:
    FUNC_OUT();

    return rv;
}

EZOS_API ez_err_t ez_iot_base_bind_query()
{
    FUNC_IN();

    ez_err_t rv = EZ_BASE_ERR_SUCC;

    CHECK_COND_DONE(!g_base_is_inited, EZ_BASE_ERR_NOT_INIT);

    rv = base_protocol_bind_status_query_req();
    CHECK_RV_DONE(rv);

done:
    FUNC_OUT();

    return rv;
}

EZOS_API ez_err_t ez_iot_base_bind_near(ez_char_t *bind_token)
{
    FUNC_IN();

    ez_err_t rv = EZ_BASE_ERR_SUCC;

    CHECK_COND_DONE(!g_base_is_inited, EZ_BASE_ERR_NOT_INIT);
    CHECK_COND_DONE(!bind_token, EZ_BASE_ERR_PARAM_INVALID);

    rv = base_protocol_near_bind_req(bind_token);
    CHECK_RV_DONE(rv);

done:
    FUNC_OUT();

    return rv;
}

EZOS_API ez_err_t ez_iot_base_bind_response(ez_int32_t challenge_code)
{
    FUNC_IN();

    ez_err_t rv = EZ_BASE_ERR_SUCC;

    CHECK_COND_DONE(!g_base_is_inited, EZ_BASE_ERR_NOT_INIT);

    rv = base_protocol_bind_response_req(challenge_code + 1);
    CHECK_RV_DONE(rv);

done:
    FUNC_OUT();

    return rv;
}

EZOS_API ez_void_t ez_iot_base_deinit(void)
{
    FUNC_IN();

    if (!g_base_is_inited)
    {
        return;
    }

    base_extern_deinit();

    g_base_is_inited = ez_false;

    FUNC_OUT();
}

ez_err_t ez_iot_base_lowlvl_profile_query(const ez_char_t *req_msg, ez_void_t* func_rsp)
{
    return base_protocol_query_profile_req(req_msg, func_rsp);
}