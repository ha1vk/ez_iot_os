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

#include <ezos.h>
#include "cJSON.h"
#include "ez_iot_core_def.h"
#include "ez_iot_core_lowlvl.h"
#include "ezlog.h"
#include "ez_ota_def.h"
#include "ez_ota_extern.h"
#include "ez_ota_bus.h"
#include "ez_ota_user.h"

static int g_quit = 0;

//通知设备升级
static int ez_ota_upgrade_to_device(ez_ota_res_t* pres, unsigned int seq, void* buf)
{
    int ret = -1,i = 0, len = 0;
    cJSON* proot = NULL;
    cJSON* pjson_rsp = NULL;
    char* response = NULL;
    char errcode[16] = {0};
    ez_ota_upgrade_info_t ez_upgrade_info;
    ezos_memset(&ez_upgrade_info, 0, sizeof(ez_ota_upgrade_info_t));
	
    do
    {
        if(NULL == buf)
        {
            ezlog_e(TAG_OTA, "upgrade_to_device,input null "); 
            break;
        }
        ezlog_d(TAG_OTA,"upgrade req: %s ",(const char*)buf); 
        proot = cJSON_Parse((const char*)buf);
        if(NULL == proot)
        {
            ezlog_e(TAG_OTA,"parse PacketRsp err"); 
            break;
        }
        cJSON* pJSmaxtry = cJSON_GetObjectItem(proot,"maxtry");
        if(NULL != pJSmaxtry && cJSON_Number==pJSmaxtry->type)
        {
            ez_upgrade_info.retry_max = pJSmaxtry->valueint;
        }
        cJSON* pJSinterval = cJSON_GetObjectItem(proot,"interval");
        if(NULL != pJSinterval && cJSON_Number==pJSinterval->type)
        {
            ez_upgrade_info.interval = pJSinterval->valueint;
            ezlog_d(TAG_OTA,"ota prograss report interval:%d", ez_upgrade_info.interval); 
        }
       
        cJSON* pJSfile_info = cJSON_GetObjectItem(proot,"file_info");
        if(NULL == pJSfile_info||cJSON_Array!=pJSfile_info->type)
        {
            ezlog_e(TAG_OTA,"file_info is not array"); 
            break;
        }
        int array_size = cJSON_GetArraySize(pJSfile_info);
        ezlog_d(TAG_OTA,"ota upgrade file num:%d", array_size); 
        ez_upgrade_info.file_num = array_size;
        ez_upgrade_info.pota_files = (ez_ota_file_info_t*)ezos_malloc(sizeof(ez_ota_file_info_t)*array_size);
        if(NULL == ez_upgrade_info.pota_files)
        {
            ezlog_e(TAG_OTA,"pota_files malloc err"); 
            break;
        }
        ezos_memset(ez_upgrade_info.pota_files, 0, sizeof(ez_ota_file_info_t)*array_size);
        while(i < array_size)
        {
           cJSON* tmp = cJSON_GetArrayItem(pJSfile_info,i);
           if(NULL == tmp)
           {
               ezlog_e(TAG_OTA,"GetArrayItem err, i:%d", i); 
               break;
           }
           cJSON* pJSmodule = cJSON_GetObjectItem(tmp, "module");
           if(pJSmodule)
           {
               ezos_snprintf((char*)ez_upgrade_info.pota_files[i].mod_name, sizeof(ez_upgrade_info.pota_files[i].mod_name),"%s", pJSmodule->valuestring);
           }
           cJSON* pJSurl = cJSON_GetObjectItem(tmp, "url");
           if(pJSurl)
           {
               ezos_snprintf((char*)ez_upgrade_info.pota_files[i].url, sizeof(ez_upgrade_info.pota_files[i].url),"%s", pJSurl->valuestring);
           }
           cJSON* pJSdegist = cJSON_GetObjectItem(tmp, "degist");
           if(pJSdegist)
           {
               ezos_snprintf((char*)ez_upgrade_info.pota_files[i].degist, sizeof(ez_upgrade_info.pota_files[i].degist),"%s",pJSdegist->valuestring);
           }
           cJSON* pJSfw_ver = cJSON_GetObjectItem(tmp, "fw_ver");
           if(pJSfw_ver)
           {
               ezos_snprintf((char*)ez_upgrade_info.pota_files[i].fw_ver, sizeof(ez_upgrade_info.pota_files[i].fw_ver),"%s",pJSfw_ver->valuestring);
           }
           cJSON* pJSsize = cJSON_GetObjectItem(tmp, "size");
           if(pJSsize)
           {
               ez_upgrade_info.pota_files[i].size = pJSsize->valueint;
           }
           cJSON* pJSdiffs = cJSON_GetObjectItem(tmp, "diffs");
           if(pJSdiffs&& cJSON_Object== pJSdiffs->type)
           {
                ez_upgrade_info.pota_files[i].pdiffs = (ez_ota_file_diff_t*)ezos_malloc(sizeof(ez_ota_file_diff_t));
                if(NULL == ez_upgrade_info.pota_files[i].pdiffs)
                {
                    ezlog_e(TAG_OTA,"pdiffs malloc err"); 
                    break;
                }
                ezos_memset(ez_upgrade_info.pota_files[i].pdiffs, 0, sizeof(ez_ota_file_diff_t));
                cJSON* diffs_url= cJSON_GetObjectItem(pJSdiffs,"url");
                if(diffs_url)
                {
                    ezos_snprintf((char*)ez_upgrade_info.pota_files[i].pdiffs->url, sizeof(ez_upgrade_info.pota_files[i].pdiffs->url),"%s", diffs_url->valuestring);
                }
                cJSON*diffs_degist = cJSON_GetObjectItem(pJSdiffs,"degist");
                if(diffs_degist)
                {
                    ezos_snprintf((char*)ez_upgrade_info.pota_files[i].pdiffs->degist,sizeof(ez_upgrade_info.pota_files[i].pdiffs->degist), "%s", diffs_degist->valuestring);
                }
                cJSON*diffs_fw_ver_dst = cJSON_GetObjectItem(pJSdiffs,"fw_ver_dst");
                if(diffs_fw_ver_dst)
                {
                    ezos_snprintf((char*)ez_upgrade_info.pota_files[i].pdiffs->fw_ver_dst, sizeof(ez_upgrade_info.pota_files[i].pdiffs->fw_ver_dst),"%s", diffs_fw_ver_dst->valuestring);
                }
                cJSON*diffs_size = cJSON_GetObjectItem(pJSdiffs,"size");
                if(diffs_size)
                {
                    ezlog_d(TAG_OTA,"diffs_size:%d", diffs_size->valueint); 
                    ez_upgrade_info.pota_files[i].pdiffs->size = diffs_size->valueint;
                }
           }
           i++;
        } 
        int len = sizeof(ez_upgrade_info);
        ret = ez_ota_get_callback()->ota_recv_msg(pres, START_UPGRADE,(void*)&ez_upgrade_info, len);
        
    } while (0);

    pjson_rsp = cJSON_CreateObject();
    if( 0 == ret)
    {
        if(pjson_rsp)
        {
            ezos_snprintf(errcode, sizeof(errcode), "0x%#08x", ret);
            cJSON_AddStringToObject(pjson_rsp,"code", errcode);
            cJSON_AddStringToObject(pjson_rsp,"errorMsg","Succeeded");
        }
        else
        {
            ezlog_e(TAG_OTA,"reply upgrade rsp to das ,json create err"); 
        }
    }
    else
    {
       
        ezos_snprintf(errcode, sizeof(errcode), "0x%#08x", ret);
        cJSON_AddStringToObject(pjson_rsp,"code", errcode);
        cJSON_AddStringToObject(pjson_rsp,"errorMsg","device receive failed!");
    }
    
    if(pjson_rsp)
    {
        response = cJSON_PrintUnformatted(pjson_rsp);
        if(response)
        {
            len = ezos_strlen(response);
            ez_err_t ota_err = ez_ota_send_msg_to_platform((unsigned char*)response, len, pres, "operate_reply", "upgrade", EZ_OTA_RSP, &seq, 0); 
            ezlog_d(TAG_OTA,"reply upgrade rsp to das,result:%d,seq:%d rsp:%s", ota_err, seq, response);
        }
        else
        {
            ezlog_e(TAG_OTA,"reply upgrade rsp to das ,json format err");
        }
    }
   
    if(ez_upgrade_info.pota_files)
    {
        for( i = 0; i < ez_upgrade_info.file_num; i++)
        {
            if(ez_upgrade_info.pota_files[i].pdiffs)
            {
                ezos_free(ez_upgrade_info.pota_files[i].pdiffs);
                ez_upgrade_info.pota_files[i].pdiffs = NULL;
            }
        }
        ezos_free(ez_upgrade_info.pota_files);
        ez_upgrade_info.pota_files = NULL;
    }
    if(response)
    {
        ezos_free(response);
    }

    if(proot)
    {
        cJSON_Delete(proot);
    }
    if(pjson_rsp)
    {
        cJSON_Delete(pjson_rsp);
    }
    
    return ret ;
}

static void ez_ota_data_route_cb(ez_kernel_submsg_v3_t* ptr_submsg)
{
	int result_code = 0;
    ez_ota_res_t res;
	if (ptr_submsg == NULL||NULL == ptr_submsg->buf)
	{
        ezlog_e(TAG_OTA,"ez_ota_data_route_cb input NULL");
		return;
	}
    
    ezlog_d(TAG_OTA,"ota recv buf:%s", (const char*)ptr_submsg->buf);
    ezos_memset(&res, 0, sizeof(ez_ota_res_t));
    ezos_strncpy((char*)res.dev_serial, ptr_submsg->sub_serial, ezdev_sdk_max_serial_len-1); 

    ezlog_d(TAG_OTA,"ota recv msg,child_id: %s,res_id:%s,res_type:%s,method:%s",res.dev_serial,\
             ptr_submsg->resource_id, ptr_submsg->resource_type, ptr_submsg->method);
    if(0 == ezos_strcmp(ptr_submsg->method, "inform"))
    {
      
    }
    else if(0 == ezos_strcmp(ptr_submsg->method,"upgrade"))
    {
        result_code = ez_ota_upgrade_to_device(&res, ptr_submsg->msg_seq, ptr_submsg->buf);
    }

	ezlog_i(TAG_OTA,"ota notify :result_code %d,msg_type:%s,seq:%d", result_code, ptr_submsg->msg_type, ptr_submsg->msg_seq);
    return;
}

static void ez_ota_event_route_cb(ez_kernel_event_t* ptr_event)
{
}

ez_err_t ezdev_ota_module_info_report(const ez_ota_res_t *pres, const ez_ota_modules_t* pmodules, const unsigned int timeout_ms)
{
    cJSON* pJsRoot = NULL;
    cJSON* pJsModule_array  = NULL;
    char* sz_pmodules = NULL;
    unsigned int msg_seq = 0;
    ez_err_t ota_err = EZ_OTA_ERR_SUCC;
    int i = 0;
    do
    {
        pJsRoot = cJSON_CreateObject();
        if(NULL == pJsRoot)
        {
            ezlog_e(TAG_OTA,"report pJsRoot Create err");
            ota_err = EZ_OTA_ERR_JSON_CREATE_ERR;
            break;
        }
        pJsModule_array = cJSON_AddArrayToObject(pJsRoot, "modules");
        if(NULL == pJsModule_array)
        {
            ezlog_e(TAG_OTA,"pJsModule_array Create err");
            ota_err = EZ_OTA_ERR_JSON_CREATE_ERR;
            break;
        }
        for( i = 0; i < pmodules->num; i++)
        {  
            cJSON* item = cJSON_CreateObject();
            if(NULL == item)
            {
                ezlog_e(TAG_OTA," CreateObject err,i:%d", i);
                continue;
            }
            cJSON_AddNumberToObject(item, "index", 0);
            cJSON_AddStringToObject(item, "module", (const char*)pmodules->plist[i].mod_name);
            cJSON_AddStringToObject(item, "fw_ver", (const char*)pmodules->plist[i].fw_ver);
            cJSON_AddItemToObject(pJsModule_array,"", item);
        }
        
        cJSON_AddStringToObject(pJsRoot, "ver", ota_version);
        sz_pmodules = cJSON_PrintUnformatted(pJsRoot);
        if(NULL == sz_pmodules)
        {
            ezlog_e(TAG_OTA,"sz_pmodules PrintUnformatted err");
            ota_err = EZ_OTA_ERR_JSON_FORAMT_ERR;
            break;
        }
        ezlog_d(TAG_OTA,"module report PrintUnformatted :%s ", sz_pmodules);
        int msg_len = ezos_strlen(sz_pmodules);
        ota_err = ez_ota_send_msg_to_platform((unsigned char*)sz_pmodules, msg_len, pres, "report", "inform", EZ_OTA_REQ, &msg_seq, 0);
        if(EZ_OTA_ERR_SUCC != ota_err)
        {
            ezlog_e(TAG_OTA," module report send_msg_to_platform err");  
            ota_err = EZ_OTA_ERR_SEND_MSG_ERR;
            break;
        } 
        ezlog_d(TAG_OTA,"module report seq :%d ", msg_seq);
    }while(0); 

    if(pJsRoot)
    {
        cJSON_Delete(pJsRoot);
    }
    if(sz_pmodules)
    {
        ezos_free(sz_pmodules);
        sz_pmodules = NULL;
    }
    return ota_err;
}

ez_err_t ez_ota_extern_init()
{
    g_quit = 0;
	ezlog_d(TAG_OTA,"ota_extern_init enter");
	ez_err_t sdk_error = EZ_OTA_ERR_SUCC;
	ez_kernel_extend_v3_t extern_info_v3;
	ezos_memset(&extern_info_v3, 0, sizeof(ez_kernel_extend_v3_t));
    
	extern_info_v3.ez_kernel_data_route = ez_ota_data_route_cb;
    extern_info_v3.ez_kernel_event_route = ez_ota_event_route_cb;
	ezos_strncpy(extern_info_v3.module, ota_module_name, ezdev_sdk_module_name_len -1);
    
	sdk_error = ez_kernel_extend_load_v3(&extern_info_v3);
	if (EZ_CORE_ERR_SUCC != sdk_error)
	{
        ezlog_e(TAG_OTA,"ezdev_sdk_kernel_extend_load ota module err:%#x", sdk_error);
		return EZ_OTA_ERR_REGISTER_ERR;
    }
    return EZ_OTA_ERR_SUCC;
}

int ez_ota_extern_fini()
{
	g_quit = 1;
	
    return 0;
}

int ez_get_exit_status()
{
    return g_quit;
}
