#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include <unistd.h>
#include <time.h>
#include "ezdev_sdk_kernel.h"
#include "bscJSON.h"
#include "ez_sdk_log.h"
#include "hal_thread.h"
#include "webclient.h"
#include "ez_sdk_ota.h"
#include "ez_ota_extern.h"
#include "ez_ota_def.h"
#include "ez_ota.h"
#include "ez_ota_bus.h"
#include "ez_ota_user.h"

#define ez_ota_url_len    270
#define ez_ota_md5_len    32

static int g_download_status = 0;

/**
 * \brief   
 */
typedef struct {
    char url[270];
    char check_sum[33];
    unsigned int total_size;
    int recv_size;     
    int timeout_s;            
    int retry_max;
    get_file_cb file_cb;
    notify_cb   notify;
    void* user_data;
} ez_ota_file_info_t;


ez_err_e ez_progress_report(const ota_res_t *pres, const int8_t* pmodule, const int8_t* perr_msg, ota_errcode_e errcode, int8_t status, int16_t progress)
{
    ez_err_e ota_err  = ez_errno_succ;
    char* sz_progress = NULL;
    char err_code[16] = {0};
    int32_t  msg_len  = 0;
    uint32_t msg_seq  = 0;

    bscJSON* pJsRoot  = bscJSON_CreateObject();
    ez_log_d(TAG_OTA, "ez_progress_report, status:%d code:%d, process:%d\n",status, errcode, progress);
    sprintf(err_code,"0x%08x", errcode);
    do
    {
        if(NULL== pJsRoot)
        {
            ez_log_e(TAG_OTA, "ez_progress_report json create err \n");
            ota_err = ez_errno_ota_json_creat_err;
            break;
        }
        bscJSON_AddNumberToObject(pJsRoot,"status", status);
        bscJSON_AddNumberToObject(pJsRoot,"process", progress);
        bscJSON_AddStringToObject(pJsRoot,"code", err_code);
        if(perr_msg)
        {
            ez_log_d(TAG_OTA,"ez_progress_report, errorMsg:%s\n", (const char*)perr_msg);
            bscJSON_AddStringToObject(pJsRoot,"errorMsg", (const char*)perr_msg);
        }
        else
        {
            bscJSON_AddStringToObject(pJsRoot,"errorMsg","success");
        }
        if(pmodule)
        {
            ez_log_d(TAG_OTA, "ez_progress_report, module:%s\n", (const char*)pmodule);
            bscJSON_AddStringToObject(pJsRoot,"module", (const char*)pmodule);
        }
        else
        {
            bscJSON_AddStringToObject(pJsRoot,"module","");
        }
        sz_progress = bscJSON_PrintUnformatted(pJsRoot);
        if(NULL == sz_progress)
        {
            ez_log_e(TAG_OTA, "query_packet bscJSON_Print err\n");
            ota_err = ez_errno_ota_json_format_err;
            break;
        }
        msg_len = strlen(sz_progress);
        ota_err = ez_ota_send_msg_to_platform((unsigned char*)sz_progress, msg_len, pres, "report", "progress", EZ_OTA_REQ, &msg_seq, 0);
        if(ez_errno_succ != ota_err)
        {
            ez_log_e(TAG_OTA,"ez_progress_report failed, status:%d\n",status);
            break;  
        }

    }while(0);

    if(sz_progress)
    {
        free(sz_progress);
        sz_progress = NULL;
    }
    if(pJsRoot)
    {
        bscJSON_Delete(pJsRoot);
    }

    return ota_err;
}

static int file_download(char* url, unsigned int* total_len, unsigned int readlen, unsigned int* offset, \
                         get_file_cb file_cb, void* user_data, int retry_flg)
{
    int ret = -1;
    unsigned int need_readlen = 1;
    httpclient_t  *h_client = NULL;
    char* recvbuffer = (char*)malloc(readlen);
    int status  = 0;
    unsigned int content_len = 0;
    ez_log_d(TAG_OTA,"file_download: total_len:%d, readlen:%d, offset:%d\n", *total_len, readlen, *offset);
    do
    {
        if(NULL == recvbuffer)
        {
            ez_log_e(TAG_OTA,"malloc recvbuffer err\n");
            break;
        }
        h_client = webclient_session_create(1024);
        if(NULL == h_client)
        {
            ez_log_e(TAG_OTA,"webclient_session_create failed\n");
            break;
        }
        ez_log_d(TAG_OTA,"url:%s\n", url);
        if(0!=retry_flg)
        {
            status = webclient_get_position(h_client, url, (int)*offset);
            if(status!=200 && 206!=status)
            {
                ez_log_e(TAG_OTA,"webclient_get_position faield,status:%d, offset:%d\n",status, *offset);
                break;
            }
        }
        else
        {
            status = webclient_get(h_client, url);
            if(status!=200)
            {
                ez_log_e(TAG_OTA,"webclient_get faield,status:%d\n", *offset);
                break;
            }
        }
        content_len = webclient_content_length_get(h_client);
        if(content_len!=*total_len-*offset)
        {
            ez_log_d(TAG_OTA,"content_len not match,content_len:%d,real:%d\n",content_len, *total_len-*offset);
            break;
        }
        ez_log_d(TAG_OTA,"content_len :%d\n",content_len);
        memset(recvbuffer, 0, readlen);
        while(need_readlen > 0)
        {
            need_readlen = *total_len - *offset;
            readlen = (need_readlen <= readlen) ? need_readlen:readlen;
            if (webclient_read(h_client, recvbuffer, readlen)!=(int)readlen)
            {
                ez_log_d(TAG_OTA,"read file err:readlen:%d, need_readlen:%d, offset:%d\n",readlen, need_readlen, *offset);
                ret = -1;
                break;
            }
            if(1 == ez_get_exit_status())
            {
                ret = 0;
                break;
            }
            need_readlen -= readlen;
            ret = file_cb(*total_len, *offset, recvbuffer, readlen, user_data);
            if(0 != ret)
            {
                ret = 0;
                break;
            }

            *offset += readlen;
        }
    }while(0);
   
    ez_log_d(TAG_OTA,"download,ret: %d\n", ret);
    if(recvbuffer)
    {
        free(recvbuffer);
        recvbuffer = NULL;
    }
    if(h_client)
    {
        webclient_close(h_client);
        h_client = NULL;
    }
    return ret;
}

static void ota_file_download_thread(void *arg)
{
    int ret= -1;
    int retry_times = 0;
    unsigned int offset = 0;
    int max_try_time = 0;
    int retry_flag = 0;
    ez_ota_file_info_t*  file_info = (ez_ota_file_info_t*)arg;
    max_try_time = file_info->retry_max;
    unsigned int total_len = file_info->total_size;
    do 
    {
        ret = file_download(file_info->url, &total_len, file_info->recv_size, &offset,file_info->file_cb, file_info->user_data, retry_flag);
        if(0!=ret)
        {
            if(++retry_times > max_try_time)
            {
                ez_log_e(TAG_OTA,"download retry\n");
                break;
            }
            if(1 == ez_get_exit_status())
            {
                break;
            }
            ez_log_e(TAG_OTA,"get ota file fialed retry_times:%d \n", retry_times);
            retry_flag = 1;
        }

    }while(ret != 0);
    
    if(0!=ret)
    {
        ez_log_e(TAG_OTA,"http_upgrade_download return err,ret:%d, retry_times:%d \n", ret, retry_times);
        file_info->notify(result_failed, file_info->user_data);
    }
    free(file_info);
    file_info = NULL;

    g_download_status = 0;

    return ;
}

ez_err_e  ez_ota_file_download(ota_download_info_t *input_info, get_file_cb file_cb, notify_cb notify, void* user_data)
{
    ez_err_e err = ez_errno_succ;
    ez_ota_file_info_t *file_info = NULL;
    char thread_name[32] = {0};
    void* handle =NULL;
    int stack_size = 64*1024;
    ez_log_d(TAG_OTA, "ezdev_ota_download start!\n");
    if(0!=g_download_status)
    {
        return ez_errno_ota_download_already;
    }
    do
    {
        file_info = (ez_ota_file_info_t*)malloc(sizeof(ez_ota_file_info_t));
        if(NULL == file_info)
        {
            ez_log_e(TAG_OTA,"malloc download_param failed!\n");
            err = ez_errno_ota_memory;
            break;
        }

        memset(file_info, 0, sizeof(ez_ota_file_info_t));
        file_info->file_cb = file_cb;
        file_info->notify= notify;
        file_info->recv_size = input_info->block_size;
        file_info->timeout_s = input_info->timeout_s;
        file_info->retry_max = input_info->retry_max;
        file_info->total_size = input_info->total_size;
        file_info->user_data = user_data;

        strncpy(file_info->url, (char*)input_info->url, sizeof(file_info->url)-1);
        strncpy(file_info->check_sum, (char*)input_info->digest, sizeof(file_info->check_sum)- 1);

        strncpy(thread_name, "ez_ota_download", sizeof(thread_name) -1);
        handle =  hal_thread_create((int8_t*)thread_name, ota_file_download_thread, stack_size, 0, (void*)file_info);
        if (NULL == handle)
        {
            ez_log_e(TAG_OTA,"ota_download_tread create error\n");
            err = ez_errno_ota_memory;
            break;
        }
        hal_thread_detach(handle);
        handle = NULL;

        g_download_status = 1;

    }while(0);

    if(ez_errno_succ!=err)
    {
        if(file_info)
        {
            free(file_info);
            file_info = NULL;
        }
    }

    return err;
}







