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
* Contributors:
 *    shenhongyin - initial API and implementation and/or initial documentation
 *******************************************************************************/

#include "ezlog.h"
#include "ez_iot_model.h"
#include "ez_model_bus.h"
#include "ez_iot_model_def.h"
#include "ez_model_extern.h"
#include "ez_model_user.h"
#include <ezos.h>

static int g_inited = 0;

static int ez_model_init()
{
	ez_model_errcode_e ret = EZ_CODE_SUCESS;
	do
	{
		ret = ez_model_extern_init();
		if (0 != ret)
		{
			ezlog_e(TAG_MOD, "ez_model_extern_init err\n");
			break;
		}
		g_inited = 1;

		ezlog_i(TAG_MOD, "ez_model_init success:%d\n", g_inited);
	} while (0);

	return ret;
}

static int ez_model_fini()
{
	int list_size = 0;
	ezlog_v(TAG_MOD, "ez_model_fini：%d\n", g_inited);
	if (!g_inited)
	{
		ezlog_e(TAG_MOD, "ez_model_fini, module not inited\n");
		return EZ_CODE_NOT_INITED;
	}
	list_size = ez_get_list_size();
	ezlog_v(TAG_MOD, "list size: %d\n", list_size);
	if (0 == list_size)
	{
		g_inited = 0;
	}
	ezlog_v(TAG_MOD, "model end , g_inited:%d\n", g_inited);
	ez_model_extern_deinit();
	return 0;
}

 EZOS_API int ez_iot_model_domain_reg(const ez_domain_reg_t *domain_reg)
{
	ez_model_errcode_e ret = EZ_CODE_SUCESS;
	if (NULL == domain_reg)
	{
		return EZ_CODE_INVALID_PARAM;
	}
	if (!g_inited)
	{
		ret = ez_model_init();
		if (0 != ret)
		{
			return ret;
		}
	}
	return ez_reg_domain(domain_reg);
}

 EZOS_API int ez_iot_model_domain_dereg(const char *domain)
{
	if (!g_inited)
	{
		return EZ_CODE_NOT_INITED;
	}
	if (NULL == domain)
	{
		return EZ_CODE_INVALID_PARAM;
	}
	ezlog_v(TAG_MOD, "ez_model_domain_dereg：%s\n", domain);

	ez_dereg_domain(domain);

	ez_model_fini();

	return EZ_CODE_SUCESS;
}

 EZOS_API int ez_iot_model_reply_to_das(ez_basic_info_t *basic_info, ez_model_msg_t *msg, ez_err_info_t *status, ez_msg_attr_t *msg_attr)
{
	int ret = -1;
	if (!g_inited)
	{
		ezlog_e(TAG_MOD, "ez_iot_model_reply_to_das, not inited\n");
		return EZ_CODE_NOT_INITED;
	}
	if (NULL == basic_info || NULL == status || NULL == msg_attr || NULL == msg)
	{
		ezlog_e(TAG_MOD, "ez_iot_model_reply_to_das, invalid input param\n");
		return EZ_CODE_INVALID_PARAM;
	}
	ezlog_v(TAG_MOD, "<ez_iot_model_reply_to_das>:domain_id:%s, identifier:%s \n", basic_info->domain, basic_info->identifier);
	ret = ez_model_send_reply(basic_info, msg, status, msg_attr);
	return ret;
}

 EZOS_API int ez_iot_model_send_msg(ez_basic_info_t *basic_info, ez_model_msg_t *msg, ez_msg_attr_t *msg_attr)
{
	int ret = -1;
	if (!g_inited)
	{
		ezlog_e(TAG_MOD, "ez_iot_model_send_msg, not inited\n");
		return EZ_CODE_NOT_INITED;
	}
	if (NULL == basic_info || NULL == msg || NULL == msg_attr)
	{
		ezlog_e(TAG_MOD, "ez_iot_model_send_msg, invalid param\n");
		return EZ_CODE_INVALID_PARAM;
	}
	ezlog_v(TAG_MOD, "<ez_iot_model_send_msg>:domain_id:%s, identifier:%s \n", basic_info->domain, basic_info->identifier);
	ret = ez_model_send_user_msg(basic_info, msg, msg_attr);

	return ret;
}

 EZOS_API int ez_iot_model_send_to_platform(ez_basic_info_t *basic_info, const char *msg, unsigned int msg_len, int msg_response, ez_msg_attr_t *msg_attr)
{
	int ret = -1;
	if (!g_inited)
	{
		return EZ_CODE_NOT_INITED;
	}
	if (NULL == basic_info || NULL == msg || NULL == msg_attr)
	{
		return EZ_CODE_INVALID_PARAM;
	}
	ezlog_v(TAG_MOD, "<ez_model_send_origin_msg>:domain_id:%s, identifier:%s \n", basic_info->domain, basic_info->identifier);
	ret = ez_model_send_origin_msg(basic_info, msg, msg_len, msg_response, msg_attr);

	return ret;
}

 EZOS_API const char * ez_iot_model_get_current_version()
{
	if (!g_inited)
	{
		return NULL;
	}
	return ez_model_get_version();
}

 EZOS_API int ez_iot_model_reg_default_cb(ez_model_default_cb_t *ez_data_router)
{
	int ret = -1;
	if (!g_inited)
	{
		ret = ez_model_init();
		if (0 != ret)
		{
			return ret;
		}
	}

	return ez_set_data_route_cb(ez_data_router);
}

 EZOS_API int ez_iot_model_dereg_default_cb(ez_model_default_cb_t *ez_data_router)
{
	ez_set_data_route_cb(ez_data_router);

	ez_model_fini();

	return EZ_CODE_SUCESS;
}
