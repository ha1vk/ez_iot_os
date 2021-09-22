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
#include "ez_model_def.h"
#include "ez_model_comm.h"
#include "ez_model_user.h"
#include "ez_model_bus.h"
#include "file_interface.h"
#include "io_interface.h"
#include "mem_interface.h"
#include "network_interface.h"
#include "thread_interface.h"
#include "time_interface.h"

#define EZ_CODE_VERSION         "V1.1.0"


static int ez_make_event_report_msg(ez_model_msg* msg, char** context)
{
    bscJSON* pjsvalue  = NULL;
    char* event_msg = NULL;
    int ret = EZ_CODE_SUCESS;

    if(NULL == msg|| NULL == context)
    {
        return EZ_CODE_INVALID_PARAM;
    }
    do
    {
        pjsvalue = ez_value_to_json(msg);
        if(NULL == pjsvalue)
        {
            ez_log_e(TAG_MOD,"event ez_value_to_json json print err\n");
            ret = EZ_CODE_JSON_ERR;
            break;
        }
        
        event_msg = bscJSON_PrintUnformatted(pjsvalue);
        if(NULL == event_msg)
        {
            ez_log_e(TAG_MOD,"make event report msg json print err\n");
            ret = EZ_CODE_JSON_PRINT_ERR;
            break;
        }
    }while(0);
	
    if(event_msg)
    {
        *context = event_msg;
	    ez_log_v(TAG_MOD,"event report msg: %s \n", *context);
    }
	if(pjsvalue)
	{
		bscJSON_Delete(pjsvalue);
	}
    return ret;
}


static int ez_make_attribute_report_msg(ez_model_msg* msg, char** context)
{
    bscJSON* pjsvalue  = NULL;
    bscJSON* pjsdata  = NULL;
    bscJSON* pjsroot  = NULL;
    char* attr_msg = NULL;
    int ret = EZ_CODE_SUCESS;
    if(NULL == msg|| NULL == context)
    {
        ez_log_e(TAG_MOD,"make_attribute_msg input invalid\n");
        return EZ_CODE_INVALID_PARAM;
    }
    do
    {
        pjsroot = bscJSON_CreateObject();
        if(NULL == pjsroot)
        {
            ez_log_e(TAG_MOD,"create jsroot err\n");
            ret = EZ_CODE_JSON_CREATE_ERR;
            break;
        }
        pjsdata = bscJSON_CreateObject();
        if(NULL == pjsdata)
        {
            ez_log_e(TAG_MOD,"create pjsdata err\n");
            ret = EZ_CODE_JSON_CREATE_ERR;
            break;
        }
        pjsvalue = ez_value_to_json(msg);
        if(NULL == pjsvalue)
        {
            bscJSON_Delete(pjsdata);
            ret = EZ_CODE_JSON_ERR;
            ez_log_e(TAG_MOD,"attr ez_value_to_json err\n");
            break;
        }
        bscJSON_AddItemToObject(pjsdata,"Value", pjsvalue);
        bscJSON_AddObjectToObject(pjsroot,"data", pjsdata);
        attr_msg = bscJSON_PrintUnformatted(pjsroot);
        if(NULL == attr_msg)
        {
            ez_log_e(TAG_MOD,"attr json print err\n");
            ret = EZ_CODE_JSON_PRINT_ERR;
            break;
        }

    }while(0);
	
    if(attr_msg)
    {
        *context = attr_msg;
	    ez_log_v(TAG_MOD,"attr report msg: %s \n", *context);
    }
    
	if(pjsroot)
	{
		bscJSON_Delete(pjsroot);
	}
    return ret;
}

static int ez_make_attribute_reply(ez_model_msg* msg, ez_err_info* status, char* msg_type, char** context)
{
    bscJSON* pjsvalue  = NULL;
    bscJSON* pjsroot  = NULL;
    bscJSON* pjsdata  = NULL;
    char* attr_reply = NULL;
    int ret = EZ_CODE_SUCESS;
    if(NULL == context)
    {
        return EZ_CODE_INVALID_PARAM;
    }
    do
    {
        if(NULL == status|| NULL== msg_type)
        {
            ez_log_e(TAG_MOD,"make_attribute_msg input err\n");
            ret = EZ_CODE_INVALID_PARAM ;
            break;
        }
        pjsroot = bscJSON_CreateObject();
        if(NULL == pjsroot)
        {
            ez_log_e(TAG_MOD,"make_attribute_msg creat root obj err\n");
            ret = EZ_CODE_JSON_CREATE_ERR;
            break;
        }
        bscJSON_AddNumberToObject(pjsroot, "status", status->status);
        bscJSON_AddStringToObject(pjsroot, "code", status->err_code);
        bscJSON_AddStringToObject(pjsroot, "errorMsg", status->err_msg);
        if(0 == strcmp(msg_type, "set_reply"))
        {
            attr_reply = bscJSON_PrintUnformatted(pjsroot);
            if(NULL == attr_reply)
            {
                ez_log_e(TAG_MOD,"make attribute msg json print err\n"); 
                ret = EZ_CODE_JSON_PRINT_ERR;
            }
            break;
        }
        if(NULL == msg)
        {
            ez_log_e(TAG_MOD,"make attribute reply msg, input NULL,msg_type:%s\n", msg_type); 
            ret = EZ_CODE_INVALID_PARAM;
            break;
        }
        pjsvalue = ez_value_to_json(msg);
        if(NULL == pjsvalue)
        {
            ez_log_e(TAG_MOD,"attribute get_reply value_to_json err\n");
            ret = EZ_CODE_JSON_ERR;
            break;
        }
        pjsdata = bscJSON_CreateObject();
        if(NULL == pjsdata)
        {
            bscJSON_Delete(pjsvalue);
            ez_log_e(TAG_MOD,"make_attribute_reply creat data obj err\n");
            ret = EZ_CODE_JSON_CREATE_ERR;
            break;
        }
        bscJSON_AddItemToObject(pjsdata, "Value", pjsvalue);
        bscJSON_AddObjectToObject(pjsroot, "data", pjsdata);
        attr_reply = bscJSON_PrintUnformatted(pjsroot);
        if(NULL == attr_reply)
        {
            ez_log_e(TAG_MOD,"make attribute msg json print err\n");
            ret = EZ_CODE_JSON_PRINT_ERR;
            break;
        }
    }while(0);
	
    if(attr_reply)
    {
        *context = attr_reply;
	    ez_log_v(TAG_MOD,"attr reply msg: %s \n", attr_reply);
    }
    
	if(pjsroot)
	{
		bscJSON_Delete(pjsroot);
	}
    return ret;
}

static int ez_make_service_msg(ez_model_msg* msg, char** context)
{
    bscJSON* pjsvalue  = NULL;
    bscJSON* pjsroot  = NULL;
    char* attr_msg = NULL;
    int ret = EZ_CODE_SUCESS;
    
    if(NULL == msg|| NULL == context)
    {
        ez_log_e(TAG_MOD,"ez_make_service_msg input invalid\n");
        return EZ_CODE_INVALID_PARAM;
    }
    do
    {
        pjsroot = bscJSON_CreateObject();
        if(NULL == pjsroot)
        {
            ez_log_e(TAG_MOD,"service create jsroot err\n");
            ret = EZ_CODE_JSON_CREATE_ERR;
            break;
        }
        pjsvalue = ez_value_to_json(msg);
        if(NULL == pjsvalue)
        {
            ez_log_e(TAG_MOD,"service ez_value_to_json err\n");
            ret = EZ_CODE_JSON_ERR;
            break;
        }
        bscJSON_AddItemToObject(pjsroot,"data", pjsvalue);
        attr_msg = bscJSON_PrintUnformatted(pjsroot);
        if(NULL == attr_msg)
        {
            ez_log_e(TAG_MOD,"service json print err\n");
            ret = EZ_CODE_JSON_PRINT_ERR;
            break;
        }
    }while(0);
	
    if(attr_msg)
    {
        *context = attr_msg;
	    ez_log_v(TAG_MOD,"service msg: %s \n", *context);
    }
    
	if(pjsroot)
	{
		bscJSON_Delete(pjsroot);
	}
    return ret;
}

static int ez_make_service_reply(ez_model_msg* msg, ez_err_info* status, char** context)
{
    bscJSON* pjsvalue  = NULL;
    bscJSON* pjsroot  = NULL;
    char* service_msg = NULL;
    int ret = EZ_CODE_SUCESS;

    if(NULL == msg|| NULL == context||NULL == status)
    {
        ez_log_e(TAG_MOD,"make_service_reply input err\n");
        return EZ_CODE_INVALID_PARAM;
    }
    do
    {
        pjsroot = bscJSON_CreateObject();
        if(NULL == pjsroot)
        {
            ez_log_e(TAG_MOD,"make_service_reply creat root obj err\n");
            ret = EZ_CODE_JSON_CREATE_ERR;
            break;
        }
        bscJSON_AddNumberToObject(pjsroot, "status", status->status);
        bscJSON_AddStringToObject(pjsroot, "code", status->err_code);
        bscJSON_AddStringToObject(pjsroot, "errorMsg", status->err_msg);
        pjsvalue = ez_value_to_json(msg);
        if(NULL == pjsvalue)
        {
            ez_log_e(TAG_MOD, "make_service_reply value_to_json err\n");
            ret = EZ_CODE_JSON_ERR;
            break;
        }
        bscJSON_AddItemToObject(pjsroot, "data", pjsvalue);
        service_msg = bscJSON_PrintUnformatted(pjsroot);
        if(NULL == service_msg)
        {
            ez_log_e(TAG_MOD,"make service_reply msg json print err\n");
            ret = EZ_CODE_JSON_PRINT_ERR;
            break;
        }
  
    }while(0);
	
    if(service_msg)
    {
        *context = service_msg;
	    ez_log_v(TAG_MOD,"service_reply: %s \n",  *context);
    }

	if(pjsroot)
	{
		bscJSON_Delete(pjsroot);
	}
    return ret;
}


int ez_model_send_reply(ez_basic_info* basic_info, ez_model_msg* msg, ez_err_info* status, ez_msg_attr* msg_attr)
{
    unsigned int msg_seq = 0;
    int msg_qos = 0;
    int ret = -1;
    char* context = NULL;
    int context_len = 0;
    ez_model_info_t mod_info;
    memset(&mod_info , 0, sizeof(ez_model_info_t));
    if(strlen(basic_info->domain)<=0||strlen(basic_info->identifier)<=0|| strlen(basic_info->resource_id)<=0||strlen(basic_info->resource_type)<=0)
    {
        ez_log_e(TAG_MOD,"ez_model_send_reply, input length err \n");
        return EZ_CODE_INVALID_PARAM;
    }
    strncpy(mod_info.msg_type, msg_attr->msg_type, EZ_MSG_TYPE_LEN - 1);
    strncpy(mod_info.resource_type, basic_info->resource_type, EZ_RES_TYPE_LEN - 1);
    strncpy(mod_info.resource_id, basic_info->resource_id,  EZ_RES_ID_LEN- 1);
    strncpy(mod_info.sub_serial, basic_info->subserial,  EZ_SUB_SERIAL_LEN- 1);

    ez_snprintf(mod_info.ext_msg, EZ_EXT_MSG_LEN,"%s/%s", basic_info->domain, basic_info->identifier);
    ez_log_v(TAG_MOD,"ez_model_send_reply, msg json type:%d\n", msg->type);
    msg_seq =  msg_attr->msg_seq;
    msg_qos =  msg_attr->msg_qos;
    
    switch(basic_info->type)
    {
        case ez_event:
            {
                ez_log_e(TAG_MOD,"ez_model_send_reply ,event msg no reply\n");
                return -1;
            }
            break;
        case ez_attribute:
            {
                strncpy(mod_info.method, "attribute", EZ_METHOD_LEN - 1);
                ret = ez_make_attribute_reply(msg, status, msg_attr->msg_type, &context);
            }
            break;
        case ez_service:
            {
                strncpy(mod_info.method, "service", EZ_METHOD_LEN - 1);
                ret = ez_make_service_reply(msg, status, &context);
            }
            break;
        default:
            break;
    }

    ez_log_v(TAG_MOD,"msg type:%s, ext_msg:%s\n", msg_attr->msg_type, mod_info.ext_msg); 

    if(0 == ret)
    {
        context_len = strlen(context);
        ret = ez_model_msg2platform((unsigned char*)context, context_len, &mod_info, &msg_seq, EZ_MODEL_RSP, msg_qos);
        if(ret)
        {
            ez_log_e(TAG_MOD,"ez_model_send_reply ,failed:,ret:%d\n", ret); 
        }
        msg_attr->msg_seq = msg_seq;
    }
    if(context)
    {
        ez_free(context);
        context = NULL;
    }

    return ret;
}


int ez_model_send_user_msg(ez_basic_info* basic_info, ez_model_msg* msg, ez_msg_attr* msg_attr)
{
    unsigned int msg_seq = 0;
    int msg_qos = 0;
    int ret = -1;
    char* context = NULL;
    int context_len = 0;
    ez_model_info_t mod_info;
    memset(&mod_info , 0, sizeof(ez_model_info_t));
    if(strlen(basic_info->domain)<=0||strlen(basic_info->identifier)<= 0||strlen(basic_info->resource_id)<=0||
       strlen(basic_info->resource_type)<=0)
    {
        ez_log_e(TAG_MOD,"ez_model_send_user_msg ,msg_param input error \n");
        return EZ_CODE_INVALID_PARAM;
    }
    strncpy(mod_info.msg_type, msg_attr->msg_type, EZ_MSG_TYPE_LEN - 1);
    strncpy(mod_info.resource_type, basic_info->resource_type, EZ_RES_TYPE_LEN - 1);
    strncpy(mod_info.resource_id, basic_info->resource_id,  EZ_RES_ID_LEN- 1);
    strncpy(mod_info.sub_serial, basic_info->subserial,  EZ_SUB_SERIAL_LEN- 1);
    ez_snprintf(mod_info.ext_msg, EZ_EXT_MSG_LEN,"%s/%s",basic_info->domain, basic_info->identifier);

    msg_seq =  msg_attr->msg_seq;
    msg_qos =  msg_attr->msg_qos;

    switch(basic_info->type)
    {
        case ez_event:
            {
                strncpy(mod_info.method, "event",  EZ_METHOD_LEN - 1);
                ret = ez_make_event_report_msg(msg, &context);
            }
            break;
        case ez_attribute:
            {
                strncpy(mod_info.method, "attribute",  EZ_METHOD_LEN - 1);
                ret = ez_make_attribute_report_msg(msg, &context);
            }
            break;
        case ez_service:
            {
                strncpy(mod_info.method, "service",  EZ_METHOD_LEN - 1);
                ret = ez_make_service_msg(msg, &context);
            }
            break;
        default:
            break;
    }
    if(0 == ret)
    {
        context_len = strlen(context);
        ret = ez_model_msg2platform((unsigned char*)context, context_len, &mod_info, &msg_seq, EZ_MODEL_REQ, msg_qos);
        if(0 != ret)
        {
            ez_log_e(TAG_MOD,"ez_model_msg2platform ,failed:,ret:%d\n", ret); 
        }

        msg_attr->msg_seq = msg_seq;
        ez_log_v(TAG_MOD,"send_user_msg msg_seq:%d\n", msg_attr->msg_seq); 
    }
    if(context)
    {
        ez_free(context);
        context = NULL;
    }
    return ret;
}

int ez_model_send_origin_msg(ez_basic_info* basic_info, const char* msg, unsigned int msg_len, int msg_response, ez_msg_attr* msg_attr)
{
    unsigned int msg_seq = 0;
    int msg_qos = 0;
    int ret = EZ_CODE_SUCESS;

    ez_model_info_t mod_info;
    memset(&mod_info , 0, sizeof(ez_model_info_t));
    if(strlen(basic_info->domain)<= 0||strlen(basic_info->identifier)<= 0||strlen(basic_info->resource_id)<=0||strlen(basic_info->resource_type)<= 0)
    {
        ez_log_e(TAG_MOD,"ez_model_send_origin_msg ,msg_param input error \n");
        return EZ_CODE_INVALID_PARAM;
    }
    strncpy(mod_info.msg_type, msg_attr->msg_type, EZ_MSG_TYPE_LEN - 1);
    strncpy(mod_info.resource_type, basic_info->resource_type, EZ_RES_TYPE_LEN - 1);
    strncpy(mod_info.resource_id, basic_info->resource_id,  EZ_RES_ID_LEN- 1);
    strncpy(mod_info.sub_serial, basic_info->subserial,  EZ_SUB_SERIAL_LEN- 1);
    ez_snprintf(mod_info.ext_msg, EZ_EXT_MSG_LEN,"%s/%s",basic_info->domain, basic_info->identifier);
    msg_seq =  msg_attr->msg_seq;
    msg_qos =  msg_attr->msg_qos;

    switch(basic_info->type)
    {
        case ez_event:
            {
                strncpy(mod_info.method, "event",  EZ_METHOD_LEN - 1);
            }
            break;
        case ez_attribute:
            {
                strncpy(mod_info.method, "attribute",  EZ_METHOD_LEN - 1);
            }
            break;
        case ez_service:
            {
                strncpy(mod_info.method, "service",  EZ_METHOD_LEN - 1);
            }
            break;
        default:
            break;
    }
    ret = ez_model_msg2platform((unsigned char*)msg, msg_len, &mod_info, &msg_seq, msg_response, msg_qos);
    if(ret)
    {
        ez_log_e(TAG_MOD,"ez_model_send_origin_msg ,failed:,ret:%d\n", ret); 
    }
    
    msg_attr->msg_seq = msg_seq;
    ez_log_v(TAG_MOD,"send_origin_msg msg_seq:%d\n", msg_attr->msg_seq); 
    
    return ret;
}

static char g_model_version[128] = {0};

const char* ez_model_get_version()
{
    ez_snprintf(g_model_version, sizeof(g_model_version), "%s", EZ_CODE_VERSION);
    ez_log_v(TAG_MOD,"ez_model_version:%s\n", g_model_version); 

    return g_model_version;
}







