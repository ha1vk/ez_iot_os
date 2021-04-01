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
#include "ez_sdk_log.h"
#include "ezdev_sdk_kernel.h"
#include "ezdev_sdk_kernel_struct.h"
#include "ez_model_bus.h"

int ez_model_msg2platform(unsigned char *context, unsigned int context_len, ez_model_info_t *param, unsigned int *msg_seq, int msg_response, int msg_qos)
{
	if (NULL == context || NULL == param || NULL == msg_seq)
	{
		ez_log_e(TAG_MOD, "ez_model_msg2platform: input NULL\n");
		return EZ_CODE_INVALID_PARAM;
	}
	ezdev_sdk_kernel_pubmsg_v3 pubmsg;
	memset(&pubmsg, 0, sizeof(ezdev_sdk_kernel_pubmsg_v3));

	ez_log_v(TAG_MOD, "msg2platform seq: %d, msg:%s \n", *msg_seq, (char *)context);
	pubmsg.msg_response = msg_response;
	pubmsg.msg_seq = *msg_seq;
	pubmsg.msg_body = context;
	pubmsg.msg_body_len = context_len;
	pubmsg.msg_qos = msg_qos;

	ez_log_v(TAG_MOD, "prime_msg2platform:resource_id:%s,resource_type:%s\n", param->resource_id, param->resource_type);

	strncpy(pubmsg.resource_type, param->resource_type, ezdev_sdk_resource_type_len - 1);
	strncpy(pubmsg.resource_id, param->resource_id, ezdev_sdk_resource_id_len - 1);
	strncpy(pubmsg.msg_type, param->msg_type, ezdev_sdk_msg_type_len - 1);
	strncpy(pubmsg.sub_serial, param->sub_serial, ezdev_sdk_max_serial_len - 1);
	strncpy(pubmsg.ext_msg, param->ext_msg, ezdev_sdk_ext_msg_len - 1);
	strncpy(pubmsg.method, param->method, ezdev_sdk_method_len - 1);

	strncpy(pubmsg.module, "model", ezdev_sdk_module_name_len - 1);
	ezdev_sdk_kernel_error sdk_error = ezdev_sdk_kernel_send_v3(&pubmsg);
	if (sdk_error != ezdev_sdk_kernel_succ)
	{
		ez_log_e(TAG_MOD, "ez_model_msg2platform err:%#02x \n", sdk_error);
		return EZ_CODE_KERNEL_SEND_ERR;
	}
	*msg_seq = pubmsg.msg_seq;

	ez_log_v(TAG_MOD, "msg2platform  success , seq:%d\n", pubmsg.msg_seq);

	return EZ_CODE_SUCESS;
}