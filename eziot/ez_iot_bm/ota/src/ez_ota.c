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
#include "ez_iot_core_def.h"
#include "ez_iot_core_lowlvl.h"
#include "cJSON.h"
#include "ezlog.h"
#include "webclient.h"
#include "ez_iot_ota.h"
#include "ez_ota_extern.h"
#include "ez_ota_def.h"
#include "ez_ota.h"
#include "ez_ota_bus.h"
#include "ez_ota_user.h"
#include <ezos.h>

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
} ez_ota_info_t;


ez_err_t ez_progress_report(const ez_ota_res_t *pres, const ez_int8_t* pmodule, const ez_int8_t* perr_msg, ez_ota_errcode_e errcode, ez_int8_t status, ez_int16_t progress)
{
    ez_err_t ota_err  = EZ_OTA_ERR_SUCC;
    char* sz_progress = NULL;
    char err_code[16] = {0};
    ez_int32_t  msg_len  = 0;
    ez_uint32_t msg_seq  = 0;

    cJSON* pJsRoot  = cJSON_CreateObject();
    ezlog_d(TAG_OTA, "ez_progress_report, status:%d code:%d, process:%d\n",status, errcode, progress);
    ezos_sprintf(err_code,"0x%08x", errcode);
    do
    {
        if(NULL== pJsRoot)
        {
            ezlog_e(TAG_OTA, "ez_progress_report json create err \n");
            ota_err = EZ_OTA_ERR_JSON_CREATE_ERR;
            break;
        }
        cJSON_AddNumberToObject(pJsRoot,"status", status);
        cJSON_AddNumberToObject(pJsRoot,"process", progress);
        cJSON_AddStringToObject(pJsRoot,"code", err_code);
        if(perr_msg)
        {
            ezlog_d(TAG_OTA,"ez_progress_report, errorMsg:%s\n", (const char*)perr_msg);
            cJSON_AddStringToObject(pJsRoot,"errorMsg", (const char*)perr_msg);
        }
        else
        {
            cJSON_AddStringToObject(pJsRoot,"errorMsg","success");
        }
        if(pmodule)
        {
            ezlog_d(TAG_OTA, "ez_progress_report, module:%s\n", (const char*)pmodule);
            cJSON_AddStringToObject(pJsRoot,"module", (const char*)pmodule);
        }
        else
        {
            cJSON_AddStringToObject(pJsRoot,"module","");
        }
        sz_progress = cJSON_PrintUnformatted(pJsRoot);
        if(NULL == sz_progress)
        {
            ezlog_e(TAG_OTA, "query_packet cJSON_Print err\n");
            ota_err = EZ_OTA_ERR_JSON_FORAMT_ERR;
            break;
        }
        msg_len = ezos_strlen(sz_progress);
        ota_err = ez_ota_send_msg_to_platform((unsigned char*)sz_progress, msg_len, pres, "report", "progress", EZ_OTA_REQ, &msg_seq, 0);
        if(EZ_OTA_ERR_SUCC != ota_err)
        {
            ezlog_e(TAG_OTA,"ez_progress_report failed, status:%d\n",status);
            break;  
        }

    }while(0);

    if(sz_progress)
    {
        ezos_free(sz_progress);
        sz_progress = NULL;
    }
    if(pJsRoot)
    {
        cJSON_Delete(pJsRoot);
    }

    return ota_err;
}

static int file_download(char* url, unsigned int* total_len, unsigned int readlen, unsigned int* offset, \
                         get_file_cb file_cb, void* user_data, int retry_flg)
{
    int ret = -1;
    unsigned int need_readlen = 1;
    httpclient_t  *h_client = NULL;
    char* recvbuffer = (char*)ezos_malloc(readlen);
    int status = 0;
    unsigned int content_len = 0;
    ezlog_d(TAG_OTA,"file_download: total_len:%d, readlen:%d, offset:%d\n", *total_len, readlen, *offset);
    do
    {
        if(NULL == recvbuffer)
        {
            ezlog_e(TAG_OTA,"malloc recvbuffer err\n");
            break;
        }
        h_client = webclient_session_create(1024);
        if(NULL == h_client)
        {
            ezlog_e(TAG_OTA,"webclient_session_create failed\n");
            break;
        }
        ezlog_d(TAG_OTA,"url:%s\n", url);
        if(0!=retry_flg)
        {
            status = webclient_get_position(h_client, url, (int)*offset);
            if(status!=200 && 206!=status)
            {
                ezlog_e(TAG_OTA,"webclient_get_position faield,status:%d, offset:%d\n",status, *offset);
                break;
            }
        }
        else
        {
            status = webclient_get(h_client, url);
            if(status!=200)
            {
                ezlog_e(TAG_OTA,"webclient_get faield,status:%d\n", *offset);
                break;
            }
        }
        content_len = webclient_content_length_get(h_client);
        if(content_len!=*total_len-*offset)
        {
            ezlog_d(TAG_OTA,"content_len not match,content_len:%d,real:%d\n",content_len, *total_len-*offset);
            break;
        }
        ezlog_d(TAG_OTA,"content_len :%d\n",content_len);
        ezos_memset(recvbuffer, 0, readlen);
        while(need_readlen > 0)
        {
            need_readlen = *total_len - *offset;
            readlen = (need_readlen <= readlen) ? need_readlen:readlen;
            if (webclient_read(h_client, recvbuffer, readlen)!=(int)readlen)
            {
                ezlog_d(TAG_OTA,"read file err:readlen:%d, need_readlen:%d, offset:%d\n",readlen, need_readlen, *offset);
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
   
    ezlog_d(TAG_OTA,"download,ret: %d\n", ret);
    if(recvbuffer)
    {
        ezos_free(recvbuffer);
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
    ezlog_e(TAG_OTA,"ota_file_download_thread start\n");
    ez_ota_info_t*  file_info = (ez_ota_info_t*)arg;
    max_try_time = file_info->retry_max;
    unsigned int total_len = file_info->total_size;
    do 
    {
        ret = file_download(file_info->url, &total_len, file_info->recv_size, &offset,file_info->file_cb, file_info->user_data, retry_flag);
        if(0!=ret)
        {
            if(++retry_times > max_try_time)
            {
                ezlog_e(TAG_OTA,"download retry\n");
                break;
            }
            if(1 == ez_get_exit_status())
            {
                break;
            }
            ezlog_e(TAG_OTA,"get ota file fialed retry_times:%d \n", retry_times);
            retry_flag = 1;
        }

    }while(ret != 0);
    
    if(0!=ret)
    {
        ezlog_e(TAG_OTA,"http_upgrade_download return err,ret:%d, retry_times:%d \n", ret, retry_times);
        file_info->notify(RESULT_FAILED, file_info->user_data);
    }
    ezos_free(file_info);
    file_info = NULL;
    g_download_status = 0;
}

ez_err_t  ez_ota_file_download(ez_ota_download_info_t *input_info, get_file_cb file_cb, notify_cb notify, void* user_data)
{
    ez_err_t err = EZ_OTA_ERR_SUCC;
    ez_ota_info_t *file_info = NULL;

    ezlog_d(TAG_OTA, "ezdev_ota_download start!\n");
    if(0!=g_download_status)
    {
        return EZ_OTA_ERR_DOWNLOAD_ALREADY;
    }
    do
    {
        file_info = (ez_ota_info_t*)ezos_malloc(sizeof(ez_ota_info_t));
        if(NULL == file_info)
        {
            ezlog_e(TAG_OTA,"malloc download_param failed!\n");
            err = EZ_OTA_ERR_MEM_ERROR;
            break;
        }

        ezos_memset(file_info, 0, sizeof(ez_ota_info_t));
        file_info->file_cb = file_cb;
        file_info->notify= notify;
        file_info->recv_size = input_info->block_size;
        file_info->timeout_s = input_info->timeout_s;
        file_info->retry_max = input_info->retry_max;
        file_info->total_size = input_info->total_size;
        file_info->user_data = user_data;

        ezos_strncpy(file_info->url, (char*)input_info->url, sizeof(file_info->url)-1);
        ezos_strncpy(file_info->check_sum, (char*)input_info->degist, sizeof(file_info->check_sum)- 1);
        
        if(ezos_thread_create(NULL, "ez_ota_download", ota_file_download_thread, file_info, CONFIG_EZIOT_OTA_TASK_STACK_SIZE, CONFIG_EZIOT_OTA_TASK_PRIORITY))
        {
            ezlog_e(TAG_OTA,"ota_download_tread create error\n");
            err = EZ_OTA_ERR_MEM_ERROR;
            break;
        }

        g_download_status = 1;

    }while(0);

    if(EZ_OTA_ERR_SUCC!=err)
    {
        if(file_info)
        {
            ezos_free(file_info);
            file_info = NULL;
        }
    }

    return err;
}







