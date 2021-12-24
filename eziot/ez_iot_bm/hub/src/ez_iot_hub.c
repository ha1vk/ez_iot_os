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
 * Brief:
 * 
 * 
 * Change Logs:
 * Date           Author       Notes
 * 2021-11-25    zhangdi29     
 *******************************************************************************/
#include "ezos.h"
#include "ez_iot_hub.h"
#include "hub_extern.h"
#include "hub_func.h"
#include "ezlog.h"

static ez_int_t g_hub_inited = 0;

ez_err_t ez_iot_hub_init(ez_hub_callbacks_t *phub_cbs)
{
    ezlog_w(TAG_HUB, "init");
    ez_err_t rv = 0;

    if (g_hub_inited)
    {
        return EZ_HUB_ERR_SUCC;
    }

    CHECK_COND_RETURN(!phub_cbs, EZ_HUB_ERR_PARAM_INVALID);
    CHECK_COND_RETURN(!phub_cbs->recv_event, EZ_HUB_ERR_PARAM_INVALID);
    CHECK_COND_RETURN(hub_extern_init(), EZ_HUB_ERR_INTERNAL);
    CHECK_COND_RETURN(hub_func_init(phub_cbs), EZ_HUB_ERR_INTERNAL);

    g_hub_inited = 1;
    return rv;
}

ez_err_t ez_iot_hub_add(const ez_subdev_info_t *subdev_info)
{
    ezlog_w(TAG_HUB, "add");

    hub_subdev_info_internal_t subdev_obj = {0};

    CHECK_COND_RETURN(!g_hub_inited, EZ_HUB_ERR_NOT_INIT);
    CHECK_COND_RETURN(!subdev_info, EZ_HUB_ERR_PARAM_INVALID);
    ezos_memcpy(&subdev_obj, subdev_info, sizeof(ez_subdev_info_t));

    return hub_add_do(&subdev_obj);
}

ez_err_t ez_iot_hub_del(const ez_char_t *subdev_sn)
{
    ezlog_w(TAG_HUB, "del");

    CHECK_COND_RETURN(!g_hub_inited, EZ_HUB_ERR_NOT_INIT);
    CHECK_COND_RETURN(!subdev_sn, EZ_HUB_ERR_PARAM_INVALID);

    return hub_del_do(subdev_sn);
}

ez_err_t ez_iot_hub_ver_update(const ez_char_t *subdev_sn, const ez_char_t *subdev_ver)
{
    ezlog_w(TAG_HUB, "ver up");

    CHECK_COND_RETURN(!g_hub_inited, EZ_HUB_ERR_NOT_INIT);
    CHECK_COND_RETURN(!subdev_sn, EZ_HUB_ERR_PARAM_INVALID);
    CHECK_COND_RETURN(!subdev_ver, EZ_HUB_ERR_PARAM_INVALID);

    return hub_ver_update_do(subdev_sn, subdev_ver);
}

ez_err_t ez_iot_hub_status_update(const ez_char_t *subdev_sn, ez_bool_t online)
{
    ezlog_w(TAG_HUB, "sta up");

    CHECK_COND_RETURN(!g_hub_inited, EZ_HUB_ERR_NOT_INIT);
    CHECK_COND_RETURN(!subdev_sn, EZ_HUB_ERR_PARAM_INVALID);

    return hub_status_update_do(subdev_sn, online);
}

ez_err_t ez_iot_hub_subdev_query(const ez_char_t *subdev_sn, ez_subdev_info_t *subdev_info)
{
    ez_err_t rv = 0;
    hub_subdev_info_internal_t subdev_obj = {0};

    CHECK_COND_RETURN(!g_hub_inited, EZ_HUB_ERR_NOT_INIT);
    CHECK_COND_RETURN(!subdev_sn, EZ_HUB_ERR_PARAM_INVALID);
    CHECK_COND_RETURN(!subdev_info, EZ_HUB_ERR_PARAM_INVALID);
    ezos_memcpy(&subdev_obj, subdev_info, sizeof(ez_subdev_info_t));

    if (EZ_HUB_ERR_SUCC == (rv = hub_subdev_query(subdev_sn, &subdev_obj)))
    {
        ezos_memcpy(subdev_info, &subdev_obj, sizeof(ez_subdev_info_t));
    }

    return rv;
}

ez_err_t ez_iot_hub_subdev_next(ez_subdev_info_t *subdev_info)
{
    ez_err_t rv = 0;
    hub_subdev_info_internal_t subdev_obj = {0};

    CHECK_COND_RETURN(!g_hub_inited, EZ_HUB_ERR_NOT_INIT);
    CHECK_COND_RETURN(!subdev_info, EZ_HUB_ERR_PARAM_INVALID);
    ezos_memcpy(&subdev_obj, subdev_info, sizeof(ez_subdev_info_t));

    if (EZ_HUB_ERR_SUCC == (rv = hub_subdev_next(&subdev_obj)))
    {
        ezos_memcpy(subdev_info, &subdev_obj, sizeof(ez_subdev_info_t));
    }

    return rv;
}

ez_err_t ez_iot_hub_clean(void)
{
    ezlog_w(TAG_HUB, "clean");

    CHECK_COND_RETURN(!g_hub_inited, EZ_HUB_ERR_NOT_INIT);

    return hub_clean_do();
}

ez_err_t ez_iot_hub_deinit(void)
{
    ezlog_w(TAG_HUB, "deinit");

    CHECK_COND_RETURN(!g_hub_inited, EZ_HUB_ERR_NOT_INIT);
    CHECK_COND_RETURN(hub_extern_finit(), EZ_HUB_ERR_INTERNAL);
    hub_func_deinit();

    g_hub_inited = 0;
    return EZ_HUB_ERR_SUCC;
}