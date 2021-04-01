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
#include <string.h>
#include "ez_sdk_log.h"
#include "ez_base_api.h"

#define CHECK_NULL(IN)\
        if((IN))      \
		{             \
           return -1; \
		}        

int base_sample_recv(ez_msg2dev_t *msg)
{
    CHECK_NULL(!msg);
	CHECK_NULL(!(msg->data));

	switch(msg->type)
	{
	case EZ_SET_USERID:
		{
			ez_set_userid_t* puser_id = (ez_set_userid_t*)msg->data;
			ez_log_d(TAG_APP,"set_user_id :%s\n", puser_id->user_id);
		}
		break;
	case EZ_TRANS_CMD:	
		{
			ez_trans_cmd_t*  pcmd= (ez_trans_cmd_t* )msg->data;
			ez_log_d(TAG_APP,"trans_cmd :%d,msg_type:%d,seq:%d\n", pcmd->cmd_id, pcmd->msg_seq, pcmd->msg_seq);	
		}
		break;
    default:
	    break;
	}

	return 0;
}

int base_sample_start()
{
    ez_base_err err = EZ_SUCCESS;
	ez_base_init_t base_init;
	memset(&base_init, 0, sizeof(base_init));
    base_init.cb.recv_msg = base_sample_recv;
    err = ez_base_init(&base_init);
	if(EZ_SUCCESS!=err)
	{
		ez_log_e(TAG_APP,"ez_base_init failed:%d\n", err);
		return -1;
	}
    ez_log_d(TAG_APP,"---------base_sample_start:%d-------------\n", err);
	return 0;
}

int base_sample_query_user_id()
{
	ez_base_err err = EZ_SUCCESS;
    err = ez_base_query_userid();
	if(EZ_SUCCESS!=err)
	{
		ez_log_e(TAG_APP,"ez_base_query_userid failed:%d\n", err);
		return -1;
	}
	ez_log_d(TAG_APP,"---------ez_base_query_userid -------------\n");
	return 0;
}

int base_sample_set_operation_code()
{
	ez_base_err err = EZ_SUCCESS;
	char operat_code[32] = {0};
	strncpy(operat_code, "test1234",sizeof(operat_code) -1);
    err = ez_base_set_operation_code(operat_code, strlen(operat_code));
	if(EZ_SUCCESS!=err)
	{
		ez_log_e(TAG_APP,"ez_base_query_userid failed:%d\n", err);
		return -1;
	}
	return 0;
}

int base_sample_stop()
{
    ez_base_deinit();
	return 0;
}


