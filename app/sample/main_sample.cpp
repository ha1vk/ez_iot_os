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
#include <sys/types.h>
#include <sys/syscall.h>
#include <dirent.h>
#include <sys/time.h>
#include <time.h>
#include <string>
#include <unistd.h>
#include <signal.h>
#include "base_typedef.h"
#include "inireader.h"
#include "ezdev_sdk_kernel.h"
#include "ez_sdk_log.h"
#include "ez_sdk_api.h"

#ifdef TEST_BASE_ENABLE
#include "base_sample.h"
#endif

#ifdef TEST_OTA_ENABLE
#include "ota_sample.h"
#endif

#ifdef TEST_MODEL_ENABLE
#include "model_sample.h"
#endif

using namespace ez_name_space;

#define CONFIG_LOCAL_DIR  "./config/fileDepens/"

static char skey[48];

void virtual_domain_start_cb(EZDEV_SDK_PTR pUser)
{
	//TODO
	/* 开发者自己处理,如果不需要可以空实现 */
}

void virtual_domain_stop_cb(EZDEV_SDK_PTR pUser)
{
	//TODO
	/* 开发者自己处理,如果不需要可以空实现 */
}

void das_data_route_cb(ezdev_sdk_kernel_submsg_v3* ptr_submsg)
{
	int ret = 0;
	int code = 0;
	if(NULL == ptr_submsg->buf)
	{
		ez_log_d(TAG_APP,"recieve das msg is null\n");
		return ;
	}
	//TODO your business
}

void event_router_cb(ezdev_sdk_kernel_event* ptr_event)
{
	//TODO	
	switch(ptr_event->event_type)
	{
	case device_online:
		{
			static int flag = 0;
			flag++;
			if(1== flag)
			{
				#ifdef TEST_OTA_ENABLE
				    ota_sample_module_info_report();
				    ota_sample_status_report(0);
				#endif
			}
		}
	case runtime_cb:
		{
			ez_log_d(TAG_APP,"-------------------------APP get event ezDevSDK_App_Event_Runtime_err_v3----------------------------\n");
			sdk_runtime_err_context* rt_err_ctx = (sdk_runtime_err_context*)ptr_event->event_context;
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
	default:
		{
			break;
		}
	}
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
	    ez_log_e(TAG_APP,"sdk_error:%d, othercode:%d, info:%s \n",sdk_error, othercode, buf);
	    break;
	case log_warn:
	    ez_log_w(TAG_APP,"sdk_error:%d, othercode:%d, info:%s \n",sdk_error, othercode, buf);
	    break;
	case log_info:
	    ez_log_i(TAG_APP,"sdk_error:%d, othercode:%d, info:%s \n",sdk_error, othercode, buf);
	    break;
	case log_debug:
	    ez_log_d(TAG_APP,"sdk_error:%d, othercode:%d, info:%s \n",sdk_error, othercode, buf);
	    break;
	case log_trace:
	    ez_log_v(TAG_APP,"sdk_error:%d, othercode:%d, info:%s \n",sdk_error, othercode, buf);
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

int main(void)
{
    int log_level = 4;
    int lbs_port  = 8666;
    int ret = 0, host_len = 0;

    std::string lbs_domain;
    CIniReader iniReader;
    ez_init_info_t init_info;
	ez_server_info_t  server;
	EZDEV_SDK_INT32 sdk_err = ezdev_sdk_kernel_succ;

    do
    {
        signal(SIGPIPE,SIG_IGN);
        iniReader.LoadKeyMap("./config/config.ini");
        iniReader.GetInteger("log", "level", log_level);
        iniReader.GetString("lbs", "domain", lbs_domain);
        iniReader.GetInteger("lbs", "port", lbs_port);
        /*start log*/
        ret = ez_sdk_log_start();
        if(0 != ret)
        {
            break;
        }
        /*set log level*/
        ez_sdk_set_log_level(log_level);
        host_len = lbs_domain.length();
        memset(&server, 0, sizeof(server));

        server.port = lbs_port;
        memset(server.host, 0, sizeof(char)*host_len + 1);
	    strncpy(server.host, lbs_domain.c_str(), sizeof(server.host)-1);

        memset(&init_info, 0, sizeof(init_info));
        sprintf(init_info.config.devinfo_path, "%s%s", CONFIG_LOCAL_DIR, "dev_info");
        ez_log_d(TAG_APP, "devinfo_path:%s\n", init_info.config.devinfo_path);
        init_info.config.bUser = 0; 
        if (0 == init_info.config.bUser)
		{
			sprintf(init_info.config.dev_id, "%s%s", CONFIG_LOCAL_DIR, "dev_id");
            sprintf(init_info.config.dev_masterkey, "%s%s", CONFIG_LOCAL_DIR, "dev_masterkey");
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
	/*#ifdef TEST_BASE_ENABLE
			base_sample_start();
	#endif  
	#ifdef TEST_OTA_ENABLE
			ota_sample_start();
	#endif       
	#ifdef TEST_MODEL_ENABLE
			model_sample_start();
	#endif   */
	            ez_sdk_start();
			} 

	/*#ifdef TEST_BASE_ENABLE
			if (!strcmp(message, "query\n"))  
			{
				base_sample_query_user_id();
			}
			if (!strcmp(message, "set\n"))  
			{
				base_sample_set_operation_code();
			}
	#endif*/

	/*#ifdef TEST_OTA_ENABLE
		    ///< called first when device start 
			if (!strcmp(message, "modu\n"))  
			{
				ota_sample_module_info_report();
			}
			///< called when device upgrade succ.
			if (!strcmp(message, "suc\n"))  
			{
				ota_sample_status_report(1);
			}
			///< called when device upgrade failed 
			if (!strcmp(message, "fail\n"))  
			{
				ota_sample_status_report(2);
			}
	#endif*/

			if (!strcmp(message, "stop\n"))
			{
				ez_sdk_stop();
	/*#ifdef TEST_BASE_ENABLE
				base_sample_stop();
	#endif   
	#ifdef TEST_OTA_ENABLE
				ota_sample_stop();
	#endif  
    #ifdef TEST_MODEL_ENABLE
			    model_sample_stop();
	#endif */  
	            ez_sdk_deinit();
			}
		}

    }while(0);


    return 0;
}
