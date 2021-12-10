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
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-11-25     zoujinwei    first version 
 *******************************************************************************/
#include "ez_ota_bus.h"
#include "cJSON.h"
#include "ez_iot_ota.h"
#include "ez_ota_def.h"
#include "ezlog.h"

#include "ez_iot_core_def.h"
#include "ez_iot_core_lowlvl.h"
#include <ezos.h>

ez_err_t ez_ota_send_msg_to_platform(unsigned char *msg, int msg_len, const ez_ota_res_t *pres, const char *msg_type,
									 const char *method, int response, unsigned int *msg_seq, int msg_qos)
{
	ez_kernel_pubmsg_v3_t pubmsg;
	ez_err_t sdk_error = EZ_CORE_ERR_SUCC;
	ezos_memset(&pubmsg, 0, sizeof(ez_kernel_pubmsg_v3_t));

	if (NULL == msg || msg_len <= 0 || NULL == method || NULL == msg_type)
	{
		ezlog_e(TAG_OTA, "ota_send to_platform, input null \n");
		return EZ_OTA_ERR_PARAM_INVALID;
	}
	if (EZ_OTA_RSP == response)
	{
		pubmsg.msg_seq = *msg_seq;
	}
	pubmsg.msg_response = response;
	pubmsg.msg_qos = (ez_kernel_qos_e)msg_qos;
	pubmsg.msg_body = (ez_char_t *)msg;
	pubmsg.msg_body_len = msg_len;

	ezos_strncpy(pubmsg.resource_type, "global", sizeof(pubmsg.resource_type) - 1);
	ezos_strncpy(pubmsg.resource_id, "0", sizeof(pubmsg.resource_id) - 1);
	if (pres && ezos_strlen((char *)pres->dev_serial) > 0)
	{
		ezlog_d(TAG_OTA, "ota dev_serial:%s \n", pres->dev_serial);
		ezos_strncpy(pubmsg.sub_serial, (char *)pres->dev_serial, sizeof(pubmsg.sub_serial) - 1);
	}
	ezos_strncpy(pubmsg.module, ota_module_name, sizeof(pubmsg.module) - 1);
	ezos_strncpy(pubmsg.method, method, sizeof(pubmsg.method) - 1);
	ezos_strncpy(pubmsg.msg_type, msg_type, sizeof(pubmsg.msg_type) - 1);

	ezlog_i(TAG_OTA, "ota_send: type:%s, seq:%d \n", msg_type, *msg_seq);
	ezlog_d(TAG_OTA, "msg: %s\n", msg);
	sdk_error = ez_kernel_send_v3(&pubmsg);
	if (sdk_error != EZ_CORE_ERR_SUCC)
	{
		ezlog_e(TAG_OTA, "ota_send_msg_to_platform failed: %#02x\n", sdk_error);
		return EZ_OTA_ERR_SEND_MSG_ERR;
	}

	if (EZ_OTA_REQ == response)
	{
		ezlog_d(TAG_OTA, "ota_send_msg_to_platform seq: %d\n", pubmsg.msg_seq);
		*msg_seq = pubmsg.msg_seq;
	}

	return EZ_OTA_ERR_SUCC;
}
