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
#include "ez_iot_model_def.h"
#include "ez_model_comm.h"
#include "ez_model_user.h"
#include "ez_model_bus.h"
#include <ezos.h>

#define EZ_CODE_VERSION         "V1.1.0"


static int ez_make_event_report_msg(ez_model_msg_t* msg, char** context)
{
    cJSON* pjsvalue  = NULL;
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
            ezlog_e(TAG_MOD,"event ez_value_to_json json print err\n");
            ret = EZ_CODE_JSON_ERR;
            break;
        }
        
        event_msg = cJSON_PrintUnformatted(pjsvalue);
        if(NULL == event_msg)
        {
            ezlog_e(TAG_MOD,"make event report msg json print err\n");
            ret = EZ_CODE_JSON_PRINT_ERR;
            break;
        }
    }while(0);
	
    if(event_msg)
    {
        *context = event_msg;
	    ezlog_v(TAG_MOD,"event report msg: %s \n", *context);
    }
	if(pjsvalue)
	{
		cJSON_Delete(pjsvalue);
	}
    return ret;
}


static int ez_make_attribute_report_msg(ez_model_msg_t* msg, char** context)
{
    cJSON* pjsvalue  = NULL;
    cJSON* pjsdata  = NULL;
    cJSON* pjsroot  = NULL;
    char* attr_msg = NULL;
    int ret = EZ_CODE_SUCESS;
    if(NULL == msg|| NULL == context)
    {
        ezlog_e(TAG_MOD,"make_attribute_msg input invalid\n");
        return EZ_CODE_INVALID_PARAM;
    }
    do
    {
        pjsroot = cJSON_CreateObject();
        if(NULL == pjsroot)
        {
            ezlog_e(TAG_MOD,"create jsroot err\n");
            ret = EZ_CODE_JSON_CREATE_ERR;
            break;
        }
        
        pjsdata = cJSON_AddObjectToObject(pjsroot,"data");
        if(NULL == pjsdata)
        {
            ezlog_e(TAG_MOD,"create pjsdata err\n");
            ret = EZ_CODE_JSON_CREATE_ERR;
            break;
        }
        pjsvalue = ez_value_to_json(msg);
        if(NULL == pjsvalue)
        {
            cJSON_Delete(pjsdata);
            ret = EZ_CODE_JSON_ERR;
            ezlog_e(TAG_MOD,"attr ez_value_to_json err\n");
            break;
        }
        cJSON_AddItemToObject(pjsdata,"Value", pjsvalue);
        
        attr_msg = cJSON_PrintUnformatted(pjsroot);
        if(NULL == attr_msg)
        {
            ezlog_e(TAG_MOD,"attr json print err\n");
            ret = EZ_CODE_JSON_PRINT_ERR;
            break;
        }

    }while(0);
	
    if(attr_msg)
    {
        *context = attr_msg;
	    ezlog_v(TAG_MOD,"attr report msg: %s \n", *context);
    }
    
	if(pjsroot)
	{
		cJSON_Delete(pjsroot);
	}
    return ret;
}

static int ez_make_attribute_reply(ez_model_msg_t* msg, ez_err_info_t* status, char* msg_type, char** context)
{
    cJSON* pjsvalue  = NULL;
    cJSON* pjsroot  = NULL;
    cJSON* pjsdata  = NULL;
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
            ezlog_e(TAG_MOD,"make_attribute_msg input err\n");
            ret = EZ_CODE_INVALID_PARAM ;
            break;
        }
        pjsroot = cJSON_CreateObject();
        if(NULL == pjsroot)
        {
            ezlog_e(TAG_MOD,"make_attribute_msg creat root obj err\n");
            ret = EZ_CODE_JSON_CREATE_ERR;
            break;
        }
        cJSON_AddNumberToObject(pjsroot, "status", status->status);
        cJSON_AddStringToObject(pjsroot, "code", status->err_code);
        cJSON_AddStringToObject(pjsroot, "errorMsg", status->err_msg);
        if(0 == ezos_strcmp(msg_type, "set_reply"))
        {
            attr_reply = cJSON_PrintUnformatted(pjsroot);
            if(NULL == attr_reply)
            {
                ezlog_e(TAG_MOD,"make attribute msg json print err\n"); 
                ret = EZ_CODE_JSON_PRINT_ERR;
            }
            break;
        }
        if(NULL == msg)
        {
            ezlog_e(TAG_MOD,"make attribute reply msg, input NULL,msg_type:%s\n", msg_type); 
            ret = EZ_CODE_INVALID_PARAM;
            break;
        }
        pjsvalue = ez_value_to_json(msg);
        if(NULL == pjsvalue)
        {
            ezlog_e(TAG_MOD,"attribute get_reply value_to_json err\n");
            ret = EZ_CODE_JSON_ERR;
            break;
        }
        
        pjsdata = cJSON_AddObjectToObject(pjsroot, "data");
        if(NULL == pjsdata)
        {
            cJSON_Delete(pjsvalue);
            ezlog_e(TAG_MOD,"make_attribute_reply creat data obj err\n");
            ret = EZ_CODE_JSON_CREATE_ERR;
            break;
        }
        cJSON_AddItemToObject(pjsdata, "Value", pjsvalue);
        
        attr_reply = cJSON_PrintUnformatted(pjsroot);
        if(NULL == attr_reply)
        {
            ezlog_e(TAG_MOD,"make attribute msg json print err\n");
            ret = EZ_CODE_JSON_PRINT_ERR;
            break;
        }
    }while(0);
	
    if(attr_reply)
    {
        *context = attr_reply;
	    ezlog_v(TAG_MOD,"attr reply msg: %s \n", attr_reply);
    }
    
	if(pjsroot)
	{
		cJSON_Delete(pjsroot);
	}
    return ret;
}

static int ez_make_service_msg(ez_model_msg_t* msg, char** context)
{
    cJSON* pjsvalue  = NULL;
    cJSON* pjsroot  = NULL;
    char* attr_msg = NULL;
    int ret = EZ_CODE_SUCESS;
    
    if(NULL == msg|| NULL == context)
    {
        ezlog_e(TAG_MOD,"ez_make_service_msg input invalid\n");
        return EZ_CODE_INVALID_PARAM;
    }
    do
    {
        pjsroot = cJSON_CreateObject();
        if(NULL == pjsroot)
        {
            ezlog_e(TAG_MOD,"service create jsroot err\n");
            ret = EZ_CODE_JSON_CREATE_ERR;
            break;
        }
        pjsvalue = ez_value_to_json(msg);
        if(NULL == pjsvalue)
        {
            ezlog_e(TAG_MOD,"service ez_value_to_json err\n");
            ret = EZ_CODE_JSON_ERR;
            break;
        }
        cJSON_AddItemToObject(pjsroot,"data", pjsvalue);
        attr_msg = cJSON_PrintUnformatted(pjsroot);
        if(NULL == attr_msg)
        {
            ezlog_e(TAG_MOD,"service json print err\n");
            ret = EZ_CODE_JSON_PRINT_ERR;
            break;
        }
    }while(0);
	
    if(attr_msg)
    {
        *context = attr_msg;
	    ezlog_v(TAG_MOD,"service msg: %s \n", *context);
    }
    
	if(pjsroot)
	{
		cJSON_Delete(pjsroot);
	}
    return ret;
}

static int ez_make_service_reply(ez_model_msg_t* msg, ez_err_info_t* status, char** context)
{
    cJSON* pjsvalue  = NULL;
    cJSON* pjsroot  = NULL;
    char* service_msg = NULL;
    int ret = EZ_CODE_SUCESS;

    if(NULL == msg|| NULL == context||NULL == status)
    {
        ezlog_e(TAG_MOD,"make_service_reply input err\n");
        return EZ_CODE_INVALID_PARAM;
    }
    do
    {
        pjsroot = cJSON_CreateObject();
        if(NULL == pjsroot)
        {
            ezlog_e(TAG_MOD,"make_service_reply creat root obj err\n");
            ret = EZ_CODE_JSON_CREATE_ERR;
            break;
        }
        cJSON_AddNumberToObject(pjsroot, "status", status->status);
        cJSON_AddStringToObject(pjsroot, "code", status->err_code);
        cJSON_AddStringToObject(pjsroot, "errorMsg", status->err_msg);
        pjsvalue = ez_value_to_json(msg);
        if(NULL == pjsvalue)
        {
            ezlog_e(TAG_MOD, "make_service_reply value_to_json err\n");
            ret = EZ_CODE_JSON_ERR;
            break;
        }
        cJSON_AddItemToObject(pjsroot, "data", pjsvalue);
        service_msg = cJSON_PrintUnformatted(pjsroot);
        if(NULL == service_msg)
        {
            ezlog_e(TAG_MOD,"make service_reply msg json print err\n");
            ret = EZ_CODE_JSON_PRINT_ERR;
            break;
        }
  
    }while(0);
	
    if(service_msg)
    {
        *context = service_msg;
	    ezlog_v(TAG_MOD,"service_reply: %s \n",  *context);
    }

	if(pjsroot)
	{
		cJSON_Delete(pjsroot);
	}
    return ret;
}


int ez_model_send_reply(ez_basic_info_t* basic_info, ez_model_msg_t* msg, ez_err_info_t* status, ez_msg_attr_t* msg_attr)
{
    unsigned int msg_seq = 0;
    int msg_qos = 0;
    int ret = -1;
    char* context = NULL;
    int context_len = 0;
    ez_model_info_t mod_info;
    ezos_memset(&mod_info , 0, sizeof(ez_model_info_t));
    if(ezos_strlen(basic_info->domain)<=0||ezos_strlen(basic_info->identifier)<=0|| ezos_strlen(basic_info->resource_id)<=0||ezos_strlen(basic_info->resource_type)<=0)
    {
        ezlog_e(TAG_MOD,"ez_model_send_reply, input length err \n");
        return EZ_CODE_INVALID_PARAM;
    }
    ezos_strncpy(mod_info.msg_type, msg_attr->msg_type, EZ_MSG_TYPE_LEN - 1);
    ezos_strncpy(mod_info.resource_type, basic_info->resource_type, EZ_RES_TYPE_LEN - 1);
    ezos_strncpy(mod_info.resource_id, basic_info->resource_id,  EZ_RES_ID_LEN- 1);
    ezos_strncpy(mod_info.sub_serial, basic_info->subserial,  EZ_SUB_SERIAL_LEN- 1);

    ezos_snprintf(mod_info.ext_msg, EZ_EXT_MSG_LEN,"%s/%s", basic_info->domain, basic_info->identifier);
    ezlog_v(TAG_MOD,"ez_model_send_reply, msg json type:%d\n", msg->type);
    msg_seq =  msg_attr->msg_seq;
    msg_qos =  msg_attr->msg_qos;
    
    switch(basic_info->type)
    {
        case ez_event:
            {
                ezlog_e(TAG_MOD,"ez_model_send_reply ,event msg no reply\n");
                return -1;
            }
            break;
        case ez_attribute:
            {
                ezos_strncpy(mod_info.method, "attribute", EZ_METHOD_LEN - 1);
                ret = ez_make_attribute_reply(msg, status, msg_attr->msg_type, &context);
            }
            break;
        case ez_service:
            {
                ezos_strncpy(mod_info.method, "service", EZ_METHOD_LEN - 1);
                ret = ez_make_service_reply(msg, status, &context);
            }
            break;
        default:
            break;
    }

    ezlog_v(TAG_MOD,"msg type:%s, ext_msg:%s\n", msg_attr->msg_type, mod_info.ext_msg); 

    if(0 == ret)
    {
        context_len = ezos_strlen(context);
        ret = ez_model_msg2platform((unsigned char*)context, context_len, &mod_info, &msg_seq, EZ_MODEL_RSP, msg_qos);
        if(ret)
        {
            ezlog_e(TAG_MOD,"ez_model_send_reply ,failed:,ret:%d\n", ret); 
        }
        msg_attr->msg_seq = msg_seq;
    }
    if(context)
    {
        ezos_free(context);
        context = NULL;
    }

    return ret;
}


int ez_model_send_user_msg(ez_basic_info_t* basic_info, ez_model_msg_t* msg, ez_msg_attr_t* msg_attr)
{
    unsigned int msg_seq = 0;
    int msg_qos = 0;
    int ret = -1;
    char* context = NULL;
    int context_len = 0;
    ez_model_info_t mod_info;
    ezos_memset(&mod_info , 0, sizeof(ez_model_info_t));
    if(ezos_strlen(basic_info->domain)<=0||ezos_strlen(basic_info->identifier)<= 0||ezos_strlen(basic_info->resource_id)<=0||
       ezos_strlen(basic_info->resource_type)<=0)
    {
        ezlog_e(TAG_MOD,"ez_model_send_user_msg ,msg_param input error \n");
        return EZ_CODE_INVALID_PARAM;
    }
    ezos_strncpy(mod_info.msg_type, msg_attr->msg_type, EZ_MSG_TYPE_LEN - 1);
    ezos_strncpy(mod_info.resource_type, basic_info->resource_type, EZ_RES_TYPE_LEN - 1);
    ezos_strncpy(mod_info.resource_id, basic_info->resource_id,  EZ_RES_ID_LEN- 1);
    ezos_strncpy(mod_info.sub_serial, basic_info->subserial,  EZ_SUB_SERIAL_LEN- 1);
    ezos_snprintf(mod_info.ext_msg, EZ_EXT_MSG_LEN,"%s/%s",basic_info->domain, basic_info->identifier);

    msg_seq =  msg_attr->msg_seq;
    msg_qos =  msg_attr->msg_qos;

    switch(basic_info->type)
    {
        case ez_event:
            {
                ezos_strncpy(mod_info.method, "event",  EZ_METHOD_LEN - 1);
                ret = ez_make_event_report_msg(msg, &context);
            }
            break;
        case ez_attribute:
            {
                ezos_strncpy(mod_info.method, "attribute",  EZ_METHOD_LEN - 1);
                ret = ez_make_attribute_report_msg(msg, &context);
            }
            break;
        case ez_service:
            {
                ezos_strncpy(mod_info.method, "service",  EZ_METHOD_LEN - 1);
                ret = ez_make_service_msg(msg, &context);
            }
            break;
        default:
            break;
    }
    if(0 == ret)
    {
        context_len = ezos_strlen(context);
        ret = ez_model_msg2platform((unsigned char*)context, context_len, &mod_info, &msg_seq, EZ_MODEL_REQ, msg_qos);
        if(0 != ret)
        {
            ezlog_e(TAG_MOD,"ez_model_msg2platform ,failed:,ret:%d\n", ret); 
        }

        msg_attr->msg_seq = msg_seq;
        ezlog_v(TAG_MOD,"send_user_msg msg_seq:%d\n", msg_attr->msg_seq); 
    }
    if(context)
    {
        ezos_free(context);
        context = NULL;
    }
    return ret;
}

int ez_model_send_origin_msg(ez_basic_info_t* basic_info, const char* msg, unsigned int msg_len, int msg_response, ez_msg_attr_t* msg_attr)
{
    unsigned int msg_seq = 0;
    int msg_qos = 0;
    int ret = EZ_CODE_SUCESS;

    ez_model_info_t mod_info;
    ezos_memset(&mod_info , 0, sizeof(ez_model_info_t));
    if(ezos_strlen(basic_info->domain)<= 0||ezos_strlen(basic_info->identifier)<= 0||ezos_strlen(basic_info->resource_id)<=0||ezos_strlen(basic_info->resource_type)<= 0)
    {
        ezlog_e(TAG_MOD,"ez_model_send_origin_msg ,msg_param input error \n");
        return EZ_CODE_INVALID_PARAM;
    }
    ezos_strncpy(mod_info.msg_type, msg_attr->msg_type, EZ_MSG_TYPE_LEN - 1);
    ezos_strncpy(mod_info.resource_type, basic_info->resource_type, EZ_RES_TYPE_LEN - 1);
    ezos_strncpy(mod_info.resource_id, basic_info->resource_id,  EZ_RES_ID_LEN- 1);
    ezos_strncpy(mod_info.sub_serial, basic_info->subserial,  EZ_SUB_SERIAL_LEN- 1);
    ezos_snprintf(mod_info.ext_msg, EZ_EXT_MSG_LEN,"%s/%s",basic_info->domain, basic_info->identifier);
    msg_seq =  msg_attr->msg_seq;
    msg_qos =  msg_attr->msg_qos;

    switch(basic_info->type)
    {
        case ez_event:
            {
                ezos_strncpy(mod_info.method, "event",  EZ_METHOD_LEN - 1);
            }
            break;
        case ez_attribute:
            {
                ezos_strncpy(mod_info.method, "attribute",  EZ_METHOD_LEN - 1);
            }
            break;
        case ez_service:
            {
                ezos_strncpy(mod_info.method, "service",  EZ_METHOD_LEN - 1);
            }
            break;
        default:
            break;
    }
    ret = ez_model_msg2platform((unsigned char*)msg, msg_len, &mod_info, &msg_seq, msg_response, msg_qos);
    if(ret)
    {
        ezlog_e(TAG_MOD,"ez_model_send_origin_msg ,failed:,ret:%d\n", ret); 
    }
    
    msg_attr->msg_seq = msg_seq;
    ezlog_v(TAG_MOD,"send_origin_msg msg_seq:%d\n", msg_attr->msg_seq); 
    
    return ret;
}

static char g_model_version[128] = {0};

const char* ez_model_get_version()
{
    ezos_snprintf(g_model_version, sizeof(g_model_version), "%s", EZ_CODE_VERSION);
    ezlog_v(TAG_MOD,"ez_model_version:%s\n", g_model_version); 

    return g_model_version;
}







