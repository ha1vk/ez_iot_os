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
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <assert.h>
#include <pthread.h>
#include "ez_sdk_log.h"

#include "ez_sdk_ota.h"
#include "ota_sample.h"

static int g_quit;

static int g_progress_interval = 0;

static int ota_sample_download_file(ota_upgrade_info_t* pupgrade_info);

int ota_event_notify(ota_res_t* pres, ota_event_e event, void *data, int len)
{
    int ret = 0, i = 0;
    ez_log_i(TAG_APP,"---------------------ota event :%d----------------\n", event);
    switch (event)
    {
    case start_upgrade:
        {
            ez_log_i(TAG_APP,"--------------------upgrade packet info ----------------\n");
            ota_upgrade_info_t* pupgrade_info = (ota_upgrade_info_t*)data;
            ez_log_i(TAG_APP,"file_num:%d \n", pupgrade_info->file_num);
            ez_log_i(TAG_APP,"retry_max:%d \n", pupgrade_info->retry_max);
            ez_log_i(TAG_APP,"interval:%d \n", pupgrade_info->interval);
      
            g_progress_interval = pupgrade_info->interval;

            for( i = 0;i < pupgrade_info->file_num; i++)
            {
                ez_log_i(TAG_APP,"pota_files[%d]->module:%s \n", i, pupgrade_info->pota_files[i].mod_name);
                ez_log_i(TAG_APP,"pota_files[%d]->url:%s \n", i, pupgrade_info->pota_files[i].url);
                ez_log_i(TAG_APP,"pota_files[%d]->fw_ver:%s \n", i, pupgrade_info->pota_files[i].fw_ver);
                ez_log_i(TAG_APP,"pota_files[%d]->digest:%s \n", i, pupgrade_info->pota_files[i].digest);
                ez_log_i(TAG_APP,"pota_files[%d]->size:%d \n", i, pupgrade_info->pota_files[i].size);
                if(NULL!=pupgrade_info->pota_files[i].pdiffs)
                {
                    ez_log_i(TAG_APP,"pdiffs.digest: %s \n", pupgrade_info->pota_files[i].pdiffs->digest);
                    ez_log_i(TAG_APP,"pdiffs.fw_ver_src: %s \n",pupgrade_info->pota_files[i].pdiffs->fw_ver_dst);
                    ez_log_i(TAG_APP,"pdiffs.url: %s \n",pupgrade_info->pota_files[i].pdiffs->url);
                    ez_log_i(TAG_APP,"pdiffs.size: %d \n",pupgrade_info->pota_files[i].pdiffs->size);
                }
            }

            ota_sample_download_file(pupgrade_info);
        
        }
        break;
    default:
        break;
    }
    return ret;
}

int  ota_sample_start()
{
    ez_err_e rev = ez_errno_succ;
    ota_init_t init_info;
    ez_log_d(TAG_APP,"-------------ota_sample_start----------\n");
    memset(&init_info, 0, sizeof(ota_init_t));
    init_info.cb.ota_recv_msg = ota_event_notify;
    rev = ez_iot_ota_init(&init_info);
    if(ez_errno_succ != rev)
    {  
        ez_log_e(TAG_APP,"-----------ez_iot_ota_init err----------\n");
        return -1;
    }
    return 0;
}

int  ota_sample_module_info_report()
{
    ez_err_e ret = ez_errno_succ;
    char pmodule[64]={0};
    char fw_ver[64]={0};

    ota_res_t  res_t;
    ota_modules_t module_list;
    ota_module_t list;

    memset(&module_list, 0, sizeof(ota_modules_t));
    memset(&res_t, 0, sizeof(ota_res_t));
    memset(&list, 0, sizeof(ota_module_t));

    ez_log_d(TAG_APP,"-------------ota_sample_module_info_report----------\n");

    strncpy(pmodule, "CS-C7-3CWFR", sizeof(pmodule) - 1);
    strncpy(fw_ver, "V5.1.3 build 170712", sizeof(fw_ver)-1);

    list.fw_ver = (int8_t*)fw_ver;
    list.mod_name = (int8_t*)pmodule;

    module_list.num = 1;
    module_list.plist = &list;

    ret = ez_iot_ota_modules_report(&res_t, &module_list, 5000);
    if(ez_errno_succ!=ret)
    {
        ez_log_e(TAG_APP,"ota report module info err:0x%#0x !\n", ret);
        return -1;
    }
    g_quit = 0;
    return 0;
}

int  ota_sample_stop()
{
    g_quit = 1;
    ez_iot_ota_deinit();
    ez_log_i(TAG_APP,"-------------ez_iot_ota_deinit end----------\n");
    return 0;
}

static void *ota_process_report(void* arg)
{
    int percent = 0;
    g_progress_interval = 1;
    ota_res_t res;
    char pmodule[64]={0};
    memset(&res ,0, sizeof(ota_res_t));
    strncpy(pmodule, "CS-C7-3CWFR", sizeof(pmodule) - 1);

    ez_log_i(TAG_APP,"-------------ota_process_report start ----------\n");
    ota_status_e  status = ota_state_starting;
    while(percent <= 100 && g_quit != 1)
    {
        switch(status)
        {
        case ota_state_starting:
             {
                if(percent>10)
                    status = ota_state_downloading;
                ez_iot_ota_progress_report(&res, (int8_t*)pmodule, status, percent);
             }
             break;
        case ota_state_downloading:
             {
                if(percent>=20)
                    status = ota_state_download_completed;
                 ez_iot_ota_progress_report(&res, (int8_t*)pmodule, status, percent);
             }
             break;
        case ota_state_download_completed:
             {
                if(percent>=25)
                    status = ota_state_burning;
                ez_iot_ota_progress_report(&res, (int8_t*)pmodule, status, percent);
             }
             break;
        case ota_state_burning:
             {
                if(percent>=95)
                    status = ota_state_burning_completed;
                ez_iot_ota_progress_report(&res, (int8_t*)pmodule, status, percent);
             }
             break;
        case ota_state_burning_completed:
             {
                if(percent==100)
                    status = ota_state_rebooting;
                ez_iot_ota_progress_report(&res, (int8_t*)pmodule, status, percent);
             }
             break;
        default:
             break;
        }
        percent += 5;
        //按照下发的间隔上报
        usleep(g_progress_interval*1000*1000);
    }
    usleep(5000*1000);
    ez_iot_ota_status_succ(&res, (int8_t*)pmodule);
    return 0;
}

static int  ota_sample_progress_report()
{
    pthread_t thread;
    ez_log_i(TAG_APP,"-------------progress_report report ----------\n");
    int ret = pthread_create(&thread, NULL, ota_process_report, NULL);
    if (0!=ret)
    {
        ez_log_i(TAG_APP,"test ota_start_report task create error\n");
        return -1;
    }
    pthread_detach(thread);

    return 0;
}

static int file_download(uint32_t total_len, uint32_t offset, void *data, uint32_t len, void* user_data)
{
    ez_log_i(TAG_APP,"total_len:%d,offset:%d,len:%d\n", total_len, offset, len);
    return 0;
}

static void notify_fun(ota_cb_result_e result, void* user_data)
{
    ez_log_i(TAG_APP,"result:%d\n", (int)result);
}

int ota_sample_download_file(ota_upgrade_info_t* pupgrade_info)
{
    ota_download_info_t download_info;

    get_file_cb file_cb = file_download;

    notify_cb notify = notify_fun;

    memset(&download_info, 0, sizeof(ota_download_info_t));

    download_info.block_size = 1024*10;
    download_info.timeout_s = 5;
    download_info.retry_max = pupgrade_info->retry_max;

    ez_log_i(TAG_APP,"retry_max:%d\n", pupgrade_info->retry_max);

    if(NULL!=pupgrade_info->pota_files)
    {
        download_info.total_size = pupgrade_info->pota_files[0].size;

        snprintf((char*)download_info.url, sizeof(download_info.url), "https://%s", (char*)pupgrade_info->pota_files[0].url); 
        ez_log_i(TAG_APP,"url:%s \n",(char*)download_info.url);
        strncpy((char*)download_info.digest, (char*)pupgrade_info->pota_files[0].digest, sizeof(download_info.digest) -1); 
        ez_iot_ota_download(&download_info,file_cb, notify, NULL);
    }

    ota_sample_progress_report();

    return 0;
}

int ota_sample_status_report(int status)
{
    ez_err_e err= ez_errno_succ;
    ota_res_t res;
    memset(&res ,0, sizeof(ota_res_t));
    char pmodule [128]= {0};
    strncpy(pmodule, "CS-C7-3CWFR", sizeof(pmodule) - 1);
    char perr_msg[64] = {0};

    switch(status)
    {
    case 0:
        {
            err = ez_iot_ota_status_ready(&res, (int8_t*)pmodule);
        }
        break;
    case 1:
        {
            err = ez_iot_ota_status_succ(&res, (int8_t*)pmodule);
        }
        break;
    case 2:
        {
            strncpy(perr_msg, "failed!", sizeof(perr_msg) -1);
            err = ez_iot_ota_status_fail(&res, (int8_t*)pmodule, (int8_t*)perr_msg, ota_code_genneral);
        }
        break;
    default:
        break;
    }
    ez_log_i(TAG_APP,"report result:%#02x \n",err);

    return 0;
}

