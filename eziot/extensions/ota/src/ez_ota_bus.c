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
#include "ez_ota_bus.h"
#include "bscJSON.h"
#include "ez_ota.h"
#include "ez_ota_def.h"
#include "ez_sdk_log.h"
#include "ezdev_sdk_kernel.h"
#include "ezdev_sdk_kernel_error.h"
#include "ezdev_sdk_kernel_struct.h"

ez_err_e ez_ota_send_msg_to_platform(unsigned char *msg, int msg_len, const ota_res_t *pres, const char *msg_type,
									 const char *method, int response, unsigned int *msg_seq, int msg_qos)
{
	ezdev_sdk_kernel_pubmsg_v3 pubmsg;
	ezdev_sdk_kernel_error sdk_error = ezdev_sdk_kernel_succ;
	memset(&pubmsg, 0, sizeof(ezdev_sdk_kernel_pubmsg_v3));

	if (NULL == msg || msg_len <= 0 || NULL == method || NULL == msg_type)
	{
		ez_log_e(TAG_OTA, "ota_send to_platform, input null \n");
		return ez_errno_ota_param_invalid;
	}
	if (EZ_OTA_RSP == response)
	{
		pubmsg.msg_seq = *msg_seq;
	}
	pubmsg.msg_response = response;
	pubmsg.msg_qos = (enum QOS_T)msg_qos;
	pubmsg.msg_body = msg;
	pubmsg.msg_body_len = msg_len;

	strncpy(pubmsg.resource_type, "global", sizeof(pubmsg.resource_type) - 1);
	strncpy(pubmsg.resource_id, "0", sizeof(pubmsg.resource_id) - 1);
	if (pres && strlen((char *)pres->dev_serial) > 0)
	{
		ez_log_d(TAG_OTA, "ota dev_serial:%s \n", pres->dev_serial);
		strncpy(pubmsg.sub_serial, (char *)pres->dev_serial, sizeof(pubmsg.sub_serial) - 1);
	}
	strncpy(pubmsg.module, ota_module_name, sizeof(pubmsg.module) - 1);
	strncpy(pubmsg.method, method, sizeof(pubmsg.method) - 1);
	strncpy(pubmsg.msg_type, msg_type, sizeof(pubmsg.msg_type) - 1);

	ez_log_i(TAG_OTA, "ota_send: type:%s, seq:%d \n", msg_type, *msg_seq);
	ez_log_d(TAG_OTA, "msg: %s\n", msg);
	sdk_error = ezdev_sdk_kernel_send_v3(&pubmsg);
	if (sdk_error != ezdev_sdk_kernel_succ)
	{
		ez_log_e(TAG_OTA, "ota_send_msg_to_platform failed: %#02x\n", sdk_error);
		return ez_errno_ota_msg_send_err;
	}

	if (EZ_OTA_REQ == response)
	{
		ez_log_d(TAG_OTA, "ota_send_msg_to_platform seq: %d\n", pubmsg.msg_seq);
		*msg_seq = pubmsg.msg_seq;
	}

	return ez_errno_succ;
}
