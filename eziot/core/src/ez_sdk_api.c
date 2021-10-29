/*******************************************************************************
 * Copyright ? 2017-2021 Ezviz Inc.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *******************************************************************************/


#include "net_module.h"
#include "ez_sdk_config.h"
#include "ezos_thread.h"
#include "base_typedef.h"
#include "ezdev_sdk_kernel.h"
#include "ezdev_sdk_kernel_struct.h"
#include "ez_sdk_api.h"
#include "ez_sdk_log.h"
#include "ezos_mem.h"
#include "ezos_time.h"
#include <string.h>

ez_thread_t g_main_thread;
ez_thread_t g_user_thread;
ez_init_info_t g_init_config = {{0}, {0}};


#define BOOT_MAIN_THREAD_NAME "ez_kernel_main"
#define BOOT_USER_THREAD_NAME "ez_kernel_user"

#define BOOT_MAX_COMMON_KEY_SIZE           128
#define BOOT_MAX_SETSWITCHENABLE_TYPE_SIZE 128
#define BOOT_MAX_QUERYSTATUS_TYPE_SIZE     128
#define BOOT_MAX_SETDEVPLAN_TYPE_SIZE      128
#define BOOT_MAX_DEFAULT_TYPE_SIZE         128

static EZDEV_SDK_INT8 g_init    = 0;
static EZDEV_SDK_INT8 g_running = 0;

void sdk_kernel_logprint(sdk_log_level level, EZDEV_SDK_INT32 sdk_error, EZDEV_SDK_INT32 othercode, const char *buf);

void EZ_OS_API_CALL sdk_main_thread(void *user_data)
{
    ez_sdk_error sdk_error = ezdev_sdk_kernel_succ;
    EZDEV_SDK_UNUSED(user_data);
    do
    {
        sdk_error = ezdev_sdk_kernel_yield();
        if (sdk_error == ezdev_sdk_kernel_das_force_offline)
        {
            break;
        }
        ezos_delay_ms(10);
    } while (g_running && sdk_error != ezdev_sdk_kernel_invald_call);

    ez_log_i(TAG_MICRO,"sdk_main_thread exit\n");
}

void EZ_OS_API_CALL sdk_user_thread(void *user_data)
{
    ez_sdk_error sdk_error = ezdev_sdk_kernel_succ;
    EZDEV_SDK_UNUSED(user_data);
    do
    {
        sdk_error = ezdev_sdk_kernel_yield_user();

        ezos_delay_ms(10);

    } while (g_running && sdk_error != ezdev_sdk_kernel_invald_call);

    ez_log_i(TAG_MICRO,"sdk_user_thread exit\n");
}

#ifdef _WIN32
int win_socket_init()
{
    WORD wVersionRequested;
    WSADATA wsaData;
    int ret;
    
    wVersionRequested = MAKEWORD(2, 2);            
    ret = WSAStartup(wVersionRequested, &wsaData); 
    if (ret != 0)
    {
        return -1;
    }

    if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
    {
        WSACleanup();
        return -1;
    }

    return 0;
}

int win_socket_fini()
{
    WSACleanup();
}

#endif // WIN32


void value_load(sdk_keyvalue_type valuetype, unsigned char *keyvalue, EZDEV_SDK_INT32 keyvalue_maxsize)
{
    if (g_init_config.config.bUser && g_init_config.config.value_load != NULL)
    {
        if (valuetype == sdk_keyvalue_devid)
        {
            g_init_config.config.value_load(key_devid, keyvalue, keyvalue_maxsize);
        }
        else if (valuetype == sdk_keyvalue_masterkey)
        {
            g_init_config.config.value_load(key_masterkey, keyvalue, keyvalue_maxsize);
        }
    }
    else
    {
        if (valuetype == sdk_keyvalue_devid)
        {
            get_file_value(g_init_config.config.dev_id, keyvalue, keyvalue_maxsize);
        }
        else if (valuetype == sdk_keyvalue_masterkey)
        {
            get_file_value(g_init_config.config.dev_masterkey, keyvalue, keyvalue_maxsize);
        }
    }
}

EZDEV_SDK_INT32 value_save(sdk_keyvalue_type valuetype, unsigned char *keyvalue, EZDEV_SDK_INT32 keyvalue_size)
{
    EZDEV_SDK_INT32 iRv = ezdev_sdk_kernel_succ;

    if (g_init_config.config.bUser && g_init_config.config.value_save != NULL)
    {
        if (valuetype == sdk_keyvalue_devid)
        {
            iRv = g_init_config.config.value_save(key_devid, keyvalue, keyvalue_size);
        }
        else if (valuetype == sdk_keyvalue_masterkey)
        {
            iRv = g_init_config.config.value_save(key_masterkey, keyvalue, keyvalue_size);
        }
    }
    else
    {
        if (valuetype == sdk_keyvalue_devid)
        {
            iRv = set_file_value(g_init_config.config.dev_id, keyvalue, keyvalue_size);
        }
        else if (valuetype == sdk_keyvalue_masterkey)
        {
            iRv = set_file_value(g_init_config.config.dev_masterkey, keyvalue, keyvalue_size);
        }
    }

    return iRv;
}


static void event_notice_to_device(ezdev_sdk_kernel_event *ptr_event)
{
    if (g_init_config.notice.event_notice == NULL || ptr_event == NULL)
    {
        return;
    }
    switch (ptr_event->event_type)
    {
    case sdk_kernel_event_online:
        g_init_config.notice.event_notice(device_online, ptr_event->event_context);
        break;
    case sdk_kernel_event_break:
        g_init_config.notice.event_notice(device_offline, ptr_event->event_context);
        break;
    case sdk_kernel_event_switchover:
        g_init_config.notice.event_notice(device_switch, ptr_event->event_context);
        break;
    case sdk_kernel_event_invaild_authcode:
        g_init_config.notice.event_notice(invaild_authcode, ptr_event->event_context);
        break;
    case sdk_kernel_event_fast_reg_online:
        g_init_config.notice.event_notice(fast_reg_online, ptr_event->event_context);
        break;
    case sdk_kernel_event_reconnect_success:
        g_init_config.notice.event_notice(reconnect_success, ptr_event->event_context);
        break;
    case sdk_kernel_event_heartbeat_interval_changed:
        g_init_config.notice.event_notice(heartbeat_interval_changed, ptr_event->event_context);
        break;
    case sdk_kernel_event_runtime_err:
        g_init_config.notice.event_notice(runtime_cb, ptr_event->event_context);
    default:
        break;
    }
}

EZOS_API EZDEV_SDK_INT32 EZOS_CALL ez_sdk_init(const ez_server_info_t* pserver_info, const ez_init_info_t* pinit, EZDEV_SDK_UINT32 reg_mode)
{
    char devinfo_string[4 * 1024] = {0};
    int result_code = ezdev_sdk_kernel_succ;
    sdk_config_t  config;
    ezdev_sdk_kernel_platform_handle sdk_cb_fun;
    memset(&sdk_cb_fun, 0, sizeof(ezdev_sdk_kernel_platform_handle));
    memset(&config, 0, sizeof(sdk_config_t));

#ifdef _WIN32
    win_socket_init();
#endif 
    do
    {
        if (0 != g_init)
        {
            result_code = ezdev_sdk_kernel_invald_call;
            break;
        }
        if (NULL== pserver_info|| 0 == strlen(pserver_info->host)||NULL== pinit||pinit->notice.event_notice == NULL)
        {
            ez_log_e(TAG_MICRO,"input params null\n");
            result_code = ezdev_sdk_kernel_params_invalid;
            break;
        }

        if (0 == pinit->config.bUser)
        {
            if (0 == strlen(pinit->config.dev_id) || 0 == strlen(pinit->config.dev_masterkey))
            {
                ez_log_e(TAG_MICRO,"devid or mastrekey path is null\n");
                result_code = ezdev_sdk_kernel_params_invalid;
                break;
            }
        }
        else
        {
            if (pinit->config.value_load == NULL || pinit->config.value_save == NULL)
            {
                ez_log_e(TAG_MICRO,"value_load or value_save fun is null\n");
                result_code = ezdev_sdk_kernel_params_invalid;
                break;
            }
        }
        memset(&g_init_config, 0, sizeof(g_init_config));
        memcpy(&g_init_config, pinit, sizeof(ez_init_info_t));

        result_code = get_devinfo_fromconfig(g_init_config.config.devinfo_path, devinfo_string, sizeof(devinfo_string));
        if (result_code != 0)
        {
            ez_log_e(TAG_MICRO,"get_devinfo_fromconfig err,path:%s\n", g_init_config.config.devinfo_path);
            result_code = ezdev_sdk_kernel_value_load;
            break;
        }

        ez_log_v(TAG_MICRO,"devinfo_string:%s\n", devinfo_string);

        sdk_cb_fun.net_work_create       = net_create;
        sdk_cb_fun.net_work_connect      = net_connect;
        sdk_cb_fun.net_work_read         = net_read;
        sdk_cb_fun.net_work_write        = net_write;
        sdk_cb_fun.net_work_disconnect   = net_disconnect;
        sdk_cb_fun.key_value_load        = value_load;
        sdk_cb_fun.key_value_save        = value_save;
        sdk_cb_fun.thread_mutex_create   = ezos_mutex_create;
        sdk_cb_fun.thread_mutex_destroy  = ezos_mutex_destory;
        sdk_cb_fun.thread_mutex_lock     = ezos_mutex_lock;
        sdk_cb_fun.thread_mutex_unlock   = ezos_mutex_unlock;
        sdk_cb_fun.time_delay            = ezos_delay_ms;


        strncpy(config.server.host, pserver_info->host, sizeof(config.server.host) - 1);
        config.server.port  = pserver_info->port;
        config.pdev_info    = devinfo_string;

        if(NULL== pinit->config.pdas_info)
        {
            config.pdas_info = NULL;  
        }
        else
        {
            config.pdas_info = (kernel_das_info*)ezos_malloc(sizeof(kernel_das_info));
            if(NULL==config.pdas_info)
            {
                result_code = ezdev_sdk_kernel_memory;
                break;
            }
            memset(config.pdas_info, 0, sizeof(kernel_das_info));
            config.pdas_info->bLightreg    = pinit->config.pdas_info->bLightreg;
            config.pdas_info->das_port     = pinit->config.pdas_info->das_port;
            config.pdas_info->das_udp_port = pinit->config.pdas_info->das_udp_port;
            config.pdas_info->das_socket   = pinit->config.pdas_info->das_socket;

            strncpy(config.pdas_info->das_address, pinit->config.pdas_info->das_address, ezdev_sdk_ip_max_len -1);
            strncpy(config.pdas_info->das_domain, pinit->config.pdas_info->das_domain, ezdev_sdk_ip_max_len -1);
            strncpy(config.pdas_info->das_serverid, pinit->config.pdas_info->das_serverid, ezdev_sdk_name_len -1);
            memcpy(config.pdas_info->session_key, pinit->config.pdas_info->session_key, ezdev_sdk_sessionkey_len);
        }
        result_code = ezdev_sdk_kernel_init(&config, &sdk_cb_fun, event_notice_to_device, reg_mode);
        if (result_code != ezdev_sdk_kernel_succ)
        {
            ez_log_e(TAG_MICRO,"ezdev_sdk_kernel_init err:%#08x\n", result_code);
            break;
        }

        g_init = 1;

    } while (0);

    if(NULL!=config.pdas_info)
    {
        ezos_free(config.pdas_info);
        config.pdas_info = NULL;
    }

    return result_code;
}

EZOS_API EZDEV_SDK_INT32 EZOS_CALL ez_sdk_start()
{
    int result = 0;
    ez_task_init_parm task_para;
    ez_sdk_error kernel_error = ezdev_sdk_kernel_succ;

    kernel_error = ezdev_sdk_kernel_start();
    if (kernel_error != ezdev_sdk_kernel_succ)
    {
        return kernel_error;
    }
    do
    {
        g_running = 1;
        memset(&task_para, 0, sizeof(ez_task_init_parm));

        task_para.task_fun = sdk_main_thread;
        ezos_snprintf(task_para.task_name, 16, BOOT_MAIN_THREAD_NAME);
        g_main_thread = ezos_thread_create(&task_para);
        if (g_main_thread == NULL){
            result = 1;
            break;
        }

        memset(&task_para, 0, sizeof(ez_task_init_parm));
        task_para.task_fun = sdk_user_thread;
        ezos_snprintf(task_para.task_name, 16, BOOT_USER_THREAD_NAME);
        g_user_thread = ezos_thread_create(&task_para);
        if (g_user_thread == NULL){
            result = 2;
            break;
        }

    } while (0);

    if (result != 0)
    {
        ez_log_e(TAG_MICRO,"sdk_thread_create err \n");

        g_running = 0;
        ezdev_sdk_kernel_stop();

        ezos_thread_destroy(g_main_thread);
        ezos_thread_destroy(g_user_thread);
        g_main_thread = NULL;
        g_user_thread = NULL;

        result = ezdev_sdk_kernel_internal;
    }

    return result;
}

EZOS_API EZDEV_SDK_INT32 EZOS_CALL ez_sdk_stop()
{
    ez_sdk_error kernel_error = ezdev_sdk_kernel_succ;
    
    if (g_running)
    {
        g_running = 0;
        ezos_thread_destroy(g_main_thread);
        ezos_thread_destroy(g_user_thread);
        g_main_thread = NULL;
        g_user_thread = NULL;
    }

    kernel_error = ezdev_sdk_kernel_stop();

    if (kernel_error != ezdev_sdk_kernel_succ)
    {
        ez_log_e(TAG_MICRO,"ezdev_sdk_kernel_stop err :%#08x\n", kernel_error);
        return kernel_error;
    }
    
    return 0;
}

EZOS_API EZDEV_SDK_INT32 EZOS_CALL ez_sdk_deinit()
{
    if (1 != g_init)
    {
        return ezdev_sdk_kernel_invald_call;
    }

    ezdev_sdk_kernel_fini();
    ez_sdk_log_stop();
    g_init = 0;

    return ezdev_sdk_kernel_succ;
}