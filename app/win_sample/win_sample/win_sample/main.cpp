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
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h> 
#include "base_typedef.h"
#include "ez_sdk_api_struct.h"
#include "ez_sdk_log.h"
#include "ez_sdk_api.h"
#include "base_sample.h"


#define CONFIG_LOACL_DIR	"\\fileDepens\\"
#define SERVER_NAME         "test12.ys7.com"
#define SERVER_PORT         8666

#define FILE_PATH_MAX_LENGTH  114 


int get_local_file_path(char* file_path, int path_len, char* file_name)
{
    char *pSep = NULL;
    GetModuleFileNameA(NULL, file_path, path_len - 1);
    pSep = strrchr(file_path, '\\');
    if (pSep)
    {
        *pSep = '\0';
    }
    strcat(file_path, file_name);

    return 0;
}

static sdk_sessionkey_context g_skey_ctx;

void event_notice_callback(ez_event_e event_id, void * context)
{	
	//TODO
	/* 开发者添加自己的业务逻辑 */
	switch(event_id)
	{
	case device_online:
		{
			ez_log_d(TAG_APP,"-------------------------APP get event device_online----------------------------\n");
			memcpy(&g_skey_ctx, context, sizeof(g_skey_ctx));
			ez_log_d(TAG_APP,"das_ip:%s,lbs ip:%s ,das port:%d\n",g_skey_ctx.das_ip,g_skey_ctx.lbs_ip,g_skey_ctx.das_port);
			break;
		}
	case device_offline:
		{
			ez_log_d(TAG_APP,"-------------------------APP get event device_offline----------------------------\n");
			break;
		}
	case device_switch:
		{
			ez_log_d(TAG_APP,"-------------------------APP get event device_switch----------------------------\n");
			break;
		}
	case invaild_authcode:
		{
			ez_log_d(TAG_APP,"-------------------------APP get event invaild_authcode----------------------------\n");
			break;
		}
	case fast_reg_online:
		{
			ez_log_d(TAG_APP,"-------------------------APP get event fast_reg_online----------------------------\n");
			break;
		}
	case runtime_cb:
		{
			ez_log_d(TAG_APP,"-------------------------APP get event runtime_cb----------------------------\n");
			sdk_runtime_err_context* rt_err_ctx = (sdk_runtime_err_context*)context;
			ez_log_d(TAG_APP,"tag:%d, errcode:0x%08x\n", rt_err_ctx->err_tag, rt_err_ctx->err_code);
			if(TAG_MSG_ACK_V3== rt_err_ctx->err_tag)
			{
				//信令发送回执
				sdk_send_msg_ack_context_v3* ptr_msg_ack = (sdk_send_msg_ack_context_v3*)rt_err_ctx->err_ctx;
				ez_log_d(TAG_APP,"module:%s, method:%s,res_id:%s,res_type:%s, msg_type:%s, ext_msg:%s, msg_seq:%d, msg_qos:%d\n", ptr_msg_ack->module, ptr_msg_ack->method, 
						ptr_msg_ack->resource_id, ptr_msg_ack->resource_type, ptr_msg_ack->msg_type,ptr_msg_ack->ext_msg,ptr_msg_ack->msg_seq,ptr_msg_ack->msg_qos);
			}
			break;
		}
	case reconnect_success:
	    {
		    ez_log_d(TAG_APP,"-------------------------APP get event reconnect_success----------------------------\n");
			break;
	    }
	case heartbeat_interval_changed:
	    {
		    ez_log_d(TAG_APP,"-------------------------APP get event heartbeat_interval_changed----------------------------\n");
			break;
		}
	default:
		{
			break;
		}
	}
}

void log_notice_callback(log_level_e level, int sdk_error, int othercode, const char *buf)
{	
	switch (level)
	{
	case log_error:
	    ez_log_e(TAG_SDK,"sdk_error:%d, othercode:%d, info:%s",sdk_error, othercode, buf);
	    break;
	case log_warn:
	    ez_log_w(TAG_SDK,"sdk_error:%d, othercode:%d, info:%s",sdk_error, othercode, buf);
	    break;
	case log_info:
	    ez_log_i(TAG_SDK,"sdk_error:%d, othercode:%d, info:%s",sdk_error, othercode, buf);
	    break;
	case log_debug:
	    ez_log_d(TAG_SDK,"sdk_error:%d, othercode:%d, info:%s",sdk_error, othercode, buf);
	    break;
	case log_trace:
	    ez_log_v(TAG_SDK,"sdk_error:%d, othercode:%d, info:%s",sdk_error, othercode, buf);
	    break;
	default:
	    break;
	}
}

void key_value_load_callback(ez_key_type_e type, unsigned char* keyvalue, EZDEV_SDK_UINT32 keyvalue_maxsize)
{
	switch(type)
	{
	case key_devid:
		{
			//TODO 开发者需要实现devid的读取操作
		}
		break;
	case key_masterkey:
		{
			//TODO 开发者需要实现masterkey的读取操作
		}
		break;
	default:
		{
			if (key_count <= type)
			{
				ez_log_d(TAG_APP,"Invalid valuetype\n");
			}
		}
	}
}

EZDEV_SDK_INT32 key_value_save_callback(ez_key_type_e valuetype, unsigned char* keyvalue, EZDEV_SDK_UINT32 keyvalue_size)
{
	EZDEV_SDK_INT32 ezRv = ezdev_sdk_kernel_succ;

	switch(valuetype)
	{
	case key_devid:
		{
			//TODO 开发者需要实现devid的写入操作,永久固化到flash,确保断电或者设备重置,这个值都不能丢失 
		}
		break;
	case  key_masterkey:
		{
			//TODO 开发者需要实现masterkey的写入操作 ,需要固化到flash,有效期一般为三个月,过期自动更新
		}
		break;
	default:
		{
			if (key_count <= valuetype)
			{
				ez_log_d(TAG_APP,"Invalid valuetype\n");
				ezRv = ezdev_sdk_kernel_params_invalid;
			}
		}
	}

	return ezRv;
}

EZDEV_SDK_INT32 curing_data_load_cb(ez_data_type_e datatype, unsigned char* data, EZDEV_SDK_UINT32 *data_maxsize)
{
	EZDEV_SDK_INT32 ezRv = ezdev_sdk_kernel_succ;
	if(datatype >= type_count)
	{
		ezRv = ezdev_sdk_kernel_params_invalid;
		return ezRv;
	}
	if(data_secretkey == datatype)
	{
		//TODO 验证码不合规设备需要处理返回固化的secretkey。如果没有secretkey,返回长度0,用devinfo中的验证码用于登录 
		*data_maxsize = 0;
	}
	return ezRv;
}

EZDEV_SDK_INT32 curing_data_save_cb(ez_data_type_e datatype, unsigned char* data, EZDEV_SDK_UINT32 data_size)
{
	EZDEV_SDK_INT32 ezRv = ezdev_sdk_kernel_succ;

	if(datatype >= type_count)
	{
		ezRv = ezdev_sdk_kernel_params_invalid;
		return ezRv;
	}
	if(data_secretkey == datatype)
	{
		//TODO 验证码不合规设备需要处理secretkey的固化
	}
	return ezRv;
}

int _tmain(int argc, _TCHAR* argv[])
{
    EZDEV_SDK_INT32 sdk_err = ezdev_sdk_kernel_succ;
    EZDEV_SDK_INT32 ret = 0;
    int log_level =5;
    int lbs_port  = 8666;
    ez_init_info_t init_info;
	ez_server_info_t  server;
    char dev_file_path[FILE_PATH_MAX_LENGTH] = {0};
    get_local_file_path(dev_file_path, sizeof(dev_file_path), CONFIG_LOACL_DIR);
    
    do
    {
        if(strlen(dev_file_path) > FILE_PATH_MAX_LENGTH)
        {
            break;
        }
        ret = ez_sdk_log_start();
        if(0!= ret)
        {
            break;
        }
        /*set log level*/
        ez_sdk_set_log_level(log_level);
        memset(&server, 0, sizeof(server));
        server.port = SERVER_PORT;
        strncpy(server.host, SERVER_NAME, sizeof(server.host) - 1);

        memset(&init_info, 0, sizeof(init_info));
        sprintf(init_info.config.devinfo_path, "%s%s", dev_file_path, "dev_info");
        ez_log_d(TAG_APP, "devinfo_path:%s\n", init_info.config.devinfo_path);

        init_info.config.bUser = 0; 
        if (0 == init_info.config.bUser)
		{
            sprintf(init_info.config.dev_id, "%s%s", dev_file_path, "dev_id");
            sprintf(init_info.config.dev_masterkey, "%s%s", dev_file_path, "dev_masterkey");
		}
		else
		{
			init_info.config.value_load = key_value_load_callback; 
            init_info.config.value_save = key_value_save_callback;
		}
        init_info.config.data_load = curing_data_load_cb;  
        init_info.config.data_save = curing_data_save_cb;  
       
        init_info.notice.event_notice = event_notice_callback;
        init_info.notice.log_notice = log_notice_callback;

        while (true)
		{
			char message[100];
			fputs("Input message(Q/q to quit):\n", stdout);
			fputs("Input message(start to start!):\n", stdout);
			fputs("Input message(stop to stop!):\n", stdout);
			fgets(message, 100, stdin);
			int len = strlen(message);
			for (int i = 0; i < len; i++)
			{
				if ('A' <= message[i] && message[i] <= 'Z')
				{
					message[i] = message[i] - ('Z' - 'z');
				}
			}
			if (!strcmp(message, "q\n") || !strcmp(message, "Q\n"))
			{
				break;
			}
			if (!strcmp(message, "start\n"))
			{
				sdk_err = ez_sdk_init(&server, &init_info, 1);
				if (ezdev_sdk_kernel_succ != sdk_err)
				{
					ez_log_e(TAG_APP,"ez_sdk_init failed, code: %d\n", sdk_err);
					break;
				}
				else
				{
					ez_log_d(TAG_APP,"---------ez_sdk_init Succ-------------\n");
				}
			    base_sample_start();
	            ez_sdk_start();
			} 

            if (!strcmp(message, "tk0\n"))
            {
                base_report_token(0);
            }
            if (!strcmp(message, "tk1\n"))
            {
                base_report_token(1);
            }


			if (!strcmp(message, "stop\n"))
			{
				ez_sdk_stop();
				base_sample_stop();
	            ez_sdk_deinit();
			}
		}

        /* code */
    } while (0);

	return 0;
}