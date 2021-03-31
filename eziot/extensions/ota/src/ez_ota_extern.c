#include <string.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include "pthread.h"
#include "bscJSON.h"
#include "hal_thread.h"
#include "ezdev_sdk_kernel.h"
#include "ezdev_sdk_kernel_error.h"
#include "ezdev_sdk_kernel_struct.h"
#include "ez_sdk_log.h"
#include "ez_ota_def.h"
#include "ez_ota_extern.h"
#include "ez_ota_bus.h"
#include "ez_ota_user.h"


static int g_quit = 0;

//通知设备升级
static int ez_ota_upgrade_to_device(ota_res_t* pres, unsigned int seq, void* buf)
{
    int ret = -1,i = 0, len = 0;
    bscJSON* proot = NULL;
    bscJSON* pjson_rsp = NULL;
    char* response = NULL;
    char errcode[16] = {0};
    ota_upgrade_info_t ez_upgrade_info;
    memset(&ez_upgrade_info, 0, sizeof(ota_upgrade_info_t));
	
    do
    {
        if(NULL == buf)
        {
            ez_log_e(TAG_OTA, "upgrade_to_device,input null \n"); 
            break;
        }
        ez_log_d(TAG_OTA,"upgrade req: %s \n",(const char*)buf); 
        proot = bscJSON_Parse((const char*)buf);
        if(NULL == proot)
        {
            ez_log_e(TAG_OTA,"parse PacketRsp err\n"); 
            break;
        }
        bscJSON* pJSmaxtry = bscJSON_GetObjectItem(proot,"maxtry");
        if(NULL != pJSmaxtry && bscJSON_Number==pJSmaxtry->type)
        {
            ez_upgrade_info.retry_max = pJSmaxtry->valueint;
        }
        bscJSON* pJSinterval = bscJSON_GetObjectItem(proot,"interval");
        if(NULL != pJSinterval && bscJSON_Number==pJSinterval->type)
        {
            ez_upgrade_info.interval = pJSinterval->valueint;
            ez_log_d(TAG_OTA,"ota prograss report interval:%d\n", ez_upgrade_info.interval); 
        }
       
        bscJSON* pJSfile_info = bscJSON_GetObjectItem(proot,"file_info");
        if(NULL == pJSfile_info||bscJSON_Array!=pJSfile_info->type)
        {
            ez_log_e(TAG_OTA,"file_info is not array\n"); 
            break;
        }
        int array_size = bscJSON_GetArraySize(pJSfile_info);
        ez_log_d(TAG_OTA,"ota upgrade file num:%d\n", array_size); 
        ez_upgrade_info.file_num = array_size;
        ez_upgrade_info.pota_files = (ota_file_info_t*)malloc(sizeof(ota_file_info_t)*array_size);
        if(NULL == ez_upgrade_info.pota_files)
        {
            ez_log_e(TAG_OTA,"pota_files malloc err\n"); 
            break;
        }
        memset(ez_upgrade_info.pota_files, 0, sizeof(ota_file_info_t)*array_size);
        while(i < array_size)
        {
           bscJSON* tmp = bscJSON_GetArrayItem(pJSfile_info,i);
           if(NULL == tmp)
           {
               ez_log_e(TAG_OTA,"GetArrayItem err, i:%d\n", i); 
               break;
           }
           bscJSON* pJSmodule = bscJSON_GetObjectItem(tmp, "module");
           if(pJSmodule)
           {
               snprintf((char*)ez_upgrade_info.pota_files[i].mod_name, sizeof(ez_upgrade_info.pota_files[i].mod_name),"%s", pJSmodule->valuestring);
           }
           bscJSON* pJSurl = bscJSON_GetObjectItem(tmp, "url");
           if(pJSurl)
           {
               snprintf((char*)ez_upgrade_info.pota_files[i].url, sizeof(ez_upgrade_info.pota_files[i].url),"%s", pJSurl->valuestring);
           }
           bscJSON* pJSdigest = bscJSON_GetObjectItem(tmp, "digest");
           if(pJSdigest)
           {
               snprintf((char*)ez_upgrade_info.pota_files[i].digest, sizeof(ez_upgrade_info.pota_files[i].digest),"%s",pJSdigest->valuestring);
           }
           bscJSON* pJSfw_ver = bscJSON_GetObjectItem(tmp, "fw_ver");
           if(pJSfw_ver)
           {
               snprintf((char*)ez_upgrade_info.pota_files[i].fw_ver, sizeof(ez_upgrade_info.pota_files[i].fw_ver),"%s",pJSfw_ver->valuestring);
           }
           bscJSON* pJSsize = bscJSON_GetObjectItem(tmp, "size");
           if(pJSsize)
           {
               ez_upgrade_info.pota_files[i].size = pJSsize->valueint;
           }
           bscJSON* pJSdiffs = bscJSON_GetObjectItem(tmp, "diffs");
           if(pJSdiffs&& bscJSON_Object== pJSdiffs->type)
           {
                ez_upgrade_info.pota_files[i].pdiffs = (ota_file_diff_t*)malloc(sizeof(ota_file_diff_t));
                if(NULL == ez_upgrade_info.pota_files[i].pdiffs)
                {
                    ez_log_e(TAG_OTA,"pdiffs malloc err\n"); 
                    break;
                }
                memset(ez_upgrade_info.pota_files[i].pdiffs, 0, sizeof(ota_file_diff_t));
                bscJSON* diffs_url= bscJSON_GetObjectItem(pJSdiffs,"url");
                if(diffs_url)
                {
                    snprintf((char*)ez_upgrade_info.pota_files[i].pdiffs->url, sizeof(ez_upgrade_info.pota_files[i].pdiffs->url),"%s", diffs_url->valuestring);
                }
                bscJSON*diffs_digest = bscJSON_GetObjectItem(pJSdiffs,"digest");
                if(diffs_digest)
                {
                    snprintf((char*)ez_upgrade_info.pota_files[i].pdiffs->digest,sizeof(ez_upgrade_info.pota_files[i].pdiffs->digest), "%s", diffs_digest->valuestring);
                }
                bscJSON*diffs_fw_ver_dst = bscJSON_GetObjectItem(pJSdiffs,"fw_ver_dst");
                if(diffs_fw_ver_dst)
                {
                    snprintf((char*)ez_upgrade_info.pota_files[i].pdiffs->fw_ver_dst, sizeof(ez_upgrade_info.pota_files[i].pdiffs->fw_ver_dst),"%s", diffs_fw_ver_dst->valuestring);
                }
                bscJSON*diffs_size = bscJSON_GetObjectItem(pJSdiffs,"size");
                if(diffs_size)
                {
                    ez_log_d(TAG_OTA,"diffs_size:%d\n", diffs_size->valueint); 
                    ez_upgrade_info.pota_files[i].pdiffs->size = diffs_size->valueint;
                }
           }
           i++;
        } 
        int len = sizeof(ez_upgrade_info);
        ret = ez_ota_get_callback()->ota_recv_msg(pres, start_upgrade,(void*)&ez_upgrade_info, len);
        
    } while (0);

    pjson_rsp = bscJSON_CreateObject();
    if( 0 == ret)
    {
        if(pjson_rsp)
        {
            snprintf(errcode, sizeof(errcode), "0x%#08x", ret);
            bscJSON_AddStringToObject(pjson_rsp,"code", errcode);
            bscJSON_AddStringToObject(pjson_rsp,"errorMsg","Succeeded");
        }
        else
        {
            ez_log_e(TAG_OTA,"reply upgrade rsp to das ,json create err\n"); 
        }
    }
    else
    {
       
        snprintf(errcode, sizeof(errcode), "0x%#08x", ret);
        bscJSON_AddStringToObject(pjson_rsp,"code", errcode);
        bscJSON_AddStringToObject(pjson_rsp,"errorMsg","device receive failed!");
    }
    
    if(pjson_rsp)
    {
        response = bscJSON_PrintUnformatted(pjson_rsp);
        if(response)
        {
            len = strlen(response);
            ez_err_e ota_err = ez_ota_send_msg_to_platform((unsigned char*)response, len, pres, "operate_reply", "upgrade", EZ_OTA_RSP, &seq, 0); 
            ez_log_d(TAG_OTA,"reply upgrade rsp to das,result:%d,seq:%d rsp:%s\n", ota_err, seq, response);
        }
        else
        {
            ez_log_e(TAG_OTA,"reply upgrade rsp to das ,json format err\n");
        }
    }
   
    if(ez_upgrade_info.pota_files)
    {
        for( i = 0; i < ez_upgrade_info.file_num; i++)
        {
            if(ez_upgrade_info.pota_files[i].pdiffs)
            {
                free(ez_upgrade_info.pota_files[i].pdiffs);
                ez_upgrade_info.pota_files[i].pdiffs = NULL;
            }
        }
        free(ez_upgrade_info.pota_files);
        ez_upgrade_info.pota_files = NULL;
    }
    if(response)
    {
        free(response);
    }

    if(proot)
    {
        bscJSON_Delete(proot);
    }
    if(pjson_rsp)
    {
        bscJSON_Delete(pjson_rsp);
    }
    
    return ret ;
}

static void ez_ota_data_route_cb(ezdev_sdk_kernel_submsg_v3* ptr_submsg)
{
	int result_code = 0;
    ota_res_t res;
	if (ptr_submsg == NULL||NULL == ptr_submsg->buf)
	{
        ez_log_e(TAG_OTA,"ez_ota_data_route_cb input NULL\n");
		return;
	}

    ez_log_d(TAG_OTA,"ota recv buf:%s\n", (const char*)ptr_submsg->buf);
    memset(&res, 0, sizeof(ota_res_t));

    strncpy((char*)res.dev_serial, ptr_submsg->sub_serial, ezdev_sdk_max_serial_len-1); 

    ez_log_d(TAG_OTA,"ota recv msg,child_id: %s,res_id:%s,res_type:%s,method:%s\n",res.dev_serial,\
             ptr_submsg->resource_id, ptr_submsg->resource_type, ptr_submsg->method);

    if(0 == strcmp(ptr_submsg->method, "inform"))
    {
      
    }
    else if(0 == strcmp(ptr_submsg->method,"upgrade"))
    {
        result_code = ez_ota_upgrade_to_device(&res, ptr_submsg->msg_seq, ptr_submsg->buf);
    }

	ez_log_i(TAG_OTA,"ota notify :result_code %d,msg_type:%s,seq:%d\n", result_code, ptr_submsg->msg_type, ptr_submsg->msg_seq);
}

static void ez_ota_event_route_cb(ezdev_sdk_kernel_event* ptr_event)
{
	ez_log_i(TAG_OTA,"ota event router,type: %d\n", ptr_event->event_type);
    switch(ptr_event->event_type)
    {
        case sdk_kernel_event_online:
        case sdk_kernel_event_switchover:
        case sdk_kernel_event_fast_reg_online:
        case sdk_kernel_event_reconnect_success:
            {
                ez_log_d(TAG_OTA,"ota login\n");
            }
            break;
        case sdk_kernel_event_break:
            {
                ez_log_d(TAG_OTA,"ota break\n");
            }
            break;
        case sdk_kernel_event_runtime_err:
            {
                sdk_runtime_err_context *err_ctx = (sdk_runtime_err_context *)ptr_event->event_context;
                if (TAG_MSG_ACK_V3 == err_ctx->err_tag)
                {
                    sdk_send_msg_ack_context_v3 *ack_ctx = (sdk_send_msg_ack_context_v3 *)err_ctx->err_ctx;
                    ez_log_d(TAG_OTA,"ez ota_run_time err: code:%d, seq:%d \n", err_ctx->err_code, ack_ctx->msg_seq);
                }
            }
            break;
        default:
            break;
    }
}

ez_err_e ezdev_ota_module_info_report(const ota_res_t *pres, const ota_modules_t* pmodules, const unsigned int timeout_ms)
{
    bscJSON* pJsRoot = NULL;
    bscJSON* pJsModule_array  = NULL;
    char* sz_pmodules = NULL;
    unsigned int msg_seq = 0;
    ez_err_e ota_err = ez_errno_succ;
    int i = 0;
    do
    {
        pJsRoot = bscJSON_CreateObject();
        if(NULL == pJsRoot)
        {
            ez_log_e(TAG_OTA,"report pJsRoot Create err\n");
            ota_err = ez_errno_ota_json_creat_err;
            break;
        }
        pJsModule_array = bscJSON_CreateArray();
        if(NULL == pJsModule_array)
        {
            ez_log_e(TAG_OTA,"pJsModule_array Create err\n");
            ota_err = ez_errno_ota_json_creat_err;
            break;
        }
        for( i = 0; i < pmodules->num; i++)
        {  
            bscJSON* item = bscJSON_CreateObject();
            if(NULL == item)
            {
                ez_log_e(TAG_OTA," CreateObject err,i:%d\n", i);
                continue;
            }
            bscJSON_AddNumberToObject(item, "index", 0);
            bscJSON_AddStringToObject(item, "module", (const char*)pmodules->plist[i].mod_name);
            bscJSON_AddStringToObject(item, "fw_ver", (const char*)pmodules->plist[i].fw_ver);
            bscJSON_AddObjectToArray(pJsModule_array,"", item);
        }
        bscJSON_AddArrayToObject(pJsRoot, "modules", pJsModule_array);
        bscJSON_AddStringToObject(pJsRoot, "ver", ota_version);
        sz_pmodules = bscJSON_PrintUnformatted(pJsRoot);
        if(NULL == sz_pmodules)
        {
            ez_log_e(TAG_OTA,"sz_pmodules PrintUnformatted err\n");
            ota_err = ez_errno_ota_json_format_err;
            break;
        }
        ez_log_d(TAG_OTA,"module report PrintUnformatted :%s \n", sz_pmodules);
        int msg_len = strlen(sz_pmodules);
        ota_err = ez_ota_send_msg_to_platform((unsigned char*)sz_pmodules, msg_len, pres, "report", "inform", EZ_OTA_REQ, &msg_seq, 0);
        if(ez_errno_succ != ota_err)
        {
            ez_log_e(TAG_OTA," module report send_msg_to_platform err\n");  
            ota_err = ez_errno_ota_msg_send_err;
            break;
        } 
        ez_log_d(TAG_OTA,"module report seq :%d \n", msg_seq);
    }while(0); 

    if(pJsRoot)
    {
        bscJSON_Delete(pJsRoot);
    }
    if(sz_pmodules)
    {
        free(sz_pmodules);
        sz_pmodules = NULL;
    }
    return ota_err;
}

ez_err_e ez_ota_extern_init()
{
    g_quit = 0;
	ez_log_d(TAG_OTA,"ota_extern_init enter\n");
	ezdev_sdk_kernel_error sdk_error = ezdev_sdk_kernel_succ;
	ezdev_sdk_kernel_extend_v3 extern_info_v3;
	memset(&extern_info_v3, 0, sizeof(ezdev_sdk_kernel_extend_v3));
    
	extern_info_v3.ezdev_sdk_kernel_data_route = ez_ota_data_route_cb;
    extern_info_v3.ezdev_sdk_kernel_event_route = ez_ota_event_route_cb;
	strncpy(extern_info_v3.module, ota_module_name, ezdev_sdk_module_name_len -1);
    
	sdk_error = ezdev_sdk_kernel_extend_load_v3(&extern_info_v3);
	if (ezdev_sdk_kernel_succ != sdk_error)
	{
        ez_log_e(TAG_OTA,"ezdev_sdk_kernel_extend_load ota module err:%#x\n", sdk_error);
		return ez_errno_ota_register_failed;
    }
    return ez_errno_succ;
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
