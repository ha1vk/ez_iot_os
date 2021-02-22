#include "ezDevSDK_boot.h"
#include "ezdev_sdk_kernel.h"
#include "platform_define.h"
#include "thread_interface.h"
#include "base_typedef.h"
#include "ezDevSDK_Common_Module.h"
#include "net_platform_wrapper.h"
#include <../src/sdk_kernel_def.h>
#include "ezdev_sdk_kernel_struct.h"

thread_handle g_main_thread = {0};
thread_handle g_user_thread = {0};


ezDevSDK_all_config g_all_config = {{0}, {0}};

EZDEVSDK_CONFIG_INTERFACE
NET_PLATFORM_INTERFACE
LOG_PLATFORM_INTERFACE
TIME_PLATFORM_INTERFACE
MUTEX_PLATFORM_INTERFACE

#define BOOT_MAIN_THREAD_NAME "ez_kernel_main"
#define BOOT_USER_THREAD_NAME "ez_kernel_user"

#define BOOT_MAX_COMMON_KEY_SIZE 128
#define BOOT_MAX_SETSWITCHENABLE_TYPE_SIZE 128
#define BOOT_MAX_QUERYSTATUS_TYPE_SIZE 128
#define BOOT_MAX_SETDEVPLAN_TYPE_SIZE 128
#define BOOT_MAX_DEFAULT_TYPE_SIZE 128

static EZDEV_SDK_INT8 g_init = 0;
static EZDEV_SDK_INT8 g_running = 0;
void sdk_kernel_logprint(sdk_log_level level, EZDEV_SDK_INT32 sdk_error, EZDEV_SDK_INT32 othercode, const char *buf);

unsigned int sdk_main_thread(void *user_data)
{
    ezdev_sdk_kernel_error sdk_error = ezdev_sdk_kernel_succ;
    EZDEV_SDK_UNUSED(user_data);
    do
    {
        sdk_error = ezdev_sdk_kernel_yield();
        if (sdk_error == ezdev_sdk_kernel_das_force_offline)
        {
            break;
        }
        sdk_thread_sleep(10);
    } while (g_running && sdk_error != ezdev_sdk_kernel_invald_call);

    sdk_kernel_logprint(sdk_log_info, 0, 0, "sdk_main_thread exist");
    return 0;
}

unsigned int sdk_user_thread(void *user_data)
{
    ezdev_sdk_kernel_error sdk_error = ezdev_sdk_kernel_succ;
    EZDEV_SDK_UNUSED(user_data);
    do
    {
        sdk_error = ezdev_sdk_kernel_yield_user();
        sdk_thread_sleep(10);
    } while (g_running && sdk_error != ezdev_sdk_kernel_invald_call);

    sdk_kernel_logprint(sdk_log_info, 0, 0, "sdk_user_thread exist");
    return 0;
}

static ezdev_sdk_kernel_error common_init(void)
{
    ezdevsdk_common_module_error common_error = ezdevsdk_cm_succ;
    ezdev_sdk_kernel_error sdk_error = ezdev_sdk_kernel_succ;

    common_error = ezDevSDK_Common_Module_Init(BOOT_MAX_COMMON_KEY_SIZE, BOOT_MAX_SETSWITCHENABLE_TYPE_SIZE, BOOT_MAX_QUERYSTATUS_TYPE_SIZE, BOOT_MAX_SETDEVPLAN_TYPE_SIZE, BOOT_MAX_DEFAULT_TYPE_SIZE);
    if (ezdevsdk_cm_succ != common_error)
    {
        switch (common_error)
        {
        case ezdevsdk_cm_malloc_error:
            sdk_error = ezdev_sdk_kernel_memory;
            break;
        case ezdevsdk_cm_load_kernel_error:
        case ezdevsdk_cm_already_construct:
            sdk_error = ezdev_sdk_kernel_invald_call;
            break;
        default:
            sdk_error = ezdev_sdk_kernel_internal;
            break;
        }
    }

    return sdk_error;
}

#ifdef _WIN32
int win_socket_init()
{
    WORD wVersionRequested;
    WSADATA wsaData;
    int ret;
    //WinSock初始化
    wVersionRequested = MAKEWORD(2, 2);            //希望使用的WinSock DLL的版本
    ret = WSAStartup(wVersionRequested, &wsaData); //加载套接字库
    if (ret != 0)
    {
        return -1;
    }

    //确认WinSock DLL支持版本2.2
    if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
    {
        WSACleanup(); //释放为该程序分配的资源，终止对winsock动态库的使用
        return -1;
    }

    return 0;
}

int win_socket_fini()
{
    WSACleanup();
}

#endif // WIN32

void sdk_kernel_logprint(sdk_log_level level, EZDEV_SDK_INT32 sdk_error, EZDEV_SDK_INT32 othercode, const char *buf)
{
    if (g_all_config.notice.log_notice == NULL)
    {
        return;
    }
    switch (level)
    {
    case sdk_log_error:
        g_all_config.notice.log_notice(ezDevSDK_log_error, sdk_error, othercode, buf);
        break;
    case sdk_log_warn:
        g_all_config.notice.log_notice(ezDevSDK_log_warn, sdk_error, othercode, buf);
        break;
    case sdk_log_info:
        g_all_config.notice.log_notice(ezDevSDK_log_info, sdk_error, othercode, buf);
        break;
    case sdk_log_debug:
        g_all_config.notice.log_notice(ezDevSDK_log_debug, sdk_error, othercode, buf);
        break;
    case sdk_log_trace:
        g_all_config.notice.log_notice(ezDevSDK_log_trace, sdk_error, othercode, buf);
        break;
    default:
        break;
    }
}

void value_load(sdk_keyvalue_type valuetype, unsigned char *keyvalue, EZDEV_SDK_INT32 keyvalue_maxsize)
{
    if (g_all_config.config.bUser && g_all_config.config.keyValueLoadFun != NULL)
    {
        if (valuetype == sdk_keyvalue_devid)
        {
            g_all_config.config.keyValueLoadFun(ezDevSDK_keyvalue_devid, keyvalue, keyvalue_maxsize);
        }
        else if (valuetype == sdk_keyvalue_masterkey)
        {
            g_all_config.config.keyValueLoadFun(ezDevSDK_keyvalue_masterkey, keyvalue, keyvalue_maxsize);
        }
    }
    else
    {
        if (valuetype == sdk_keyvalue_devid)
        {
            get_file_value(g_all_config.config.dev_id, keyvalue, keyvalue_maxsize);
        }
        else if (valuetype == sdk_keyvalue_masterkey)
        {
            get_file_value(g_all_config.config.dev_masterkey, keyvalue, keyvalue_maxsize);
        }
    }
}

EZDEV_SDK_INT32 value_save(sdk_keyvalue_type valuetype, unsigned char *keyvalue, EZDEV_SDK_INT32 keyvalue_size)
{
    EZDEV_SDK_INT32 iRv = ezdev_sdk_kernel_succ;

    if (g_all_config.config.bUser && g_all_config.config.keyValueSaveFun != NULL)
    {
        if (valuetype == sdk_keyvalue_devid)
        {
            iRv = g_all_config.config.keyValueSaveFun(ezDevSDK_keyvalue_devid, keyvalue, keyvalue_size);
        }
        else if (valuetype == sdk_keyvalue_masterkey)
        {
            iRv = g_all_config.config.keyValueSaveFun(ezDevSDK_keyvalue_masterkey, keyvalue, keyvalue_size);
        }
    }
    else
    {
        if (valuetype == sdk_keyvalue_devid)
        {
            iRv = set_file_value(g_all_config.config.dev_id, keyvalue, keyvalue_size);
        }
        else if (valuetype == sdk_keyvalue_masterkey)
        {
            iRv = set_file_value(g_all_config.config.dev_masterkey, keyvalue, keyvalue_size);
        }
    }

    return iRv;
}

EZDEV_SDK_INT32 curing_data_load(sdk_curingdata_type datatype, unsigned char *keyvalue, EZDEV_SDK_INT32 *keyvalue_maxsize)
{
    EZDEV_SDK_INT32 iRv = ezdev_sdk_kernel_succ;

    if (sdk_curingdata_secretkey == datatype)
    {
        iRv = g_all_config.config.curingDataLoadFun(ezDevSDK_curingdata_secretkey, keyvalue, (EZDEV_SDK_UINT32 *)keyvalue_maxsize);
    }

    return iRv;
}

EZDEV_SDK_INT32 curing_data_save(sdk_curingdata_type datatype, unsigned char *keyvalue, EZDEV_SDK_INT32 keyvalue_size)
{
    EZDEV_SDK_INT32 iRv = ezdev_sdk_kernel_succ;

    if (sdk_curingdata_secretkey == datatype)
    {
        iRv = g_all_config.config.curingDataSaveFun(ezDevSDK_curingdata_secretkey, keyvalue, (EZDEV_SDK_UINT32)keyvalue_size);
    }

    return iRv;
}

void event_notice_from_sdk_kernel(ezdev_sdk_kernel_event *ptr_event)
{
    if (g_all_config.notice.event_notice == NULL || ptr_event == NULL)
    {
        return;
    }
    switch (ptr_event->event_type)
    {
    case sdk_kernel_event_online:
        g_all_config.notice.event_notice(ezDevSDK_App_Event_Online, ptr_event->event_context);
        break;
    case sdk_kernel_event_break:
        g_all_config.notice.event_notice(ezDevSDK_App_Event_Break, ptr_event->event_context);
        break;
    case sdk_kernel_event_switchover:
        g_all_config.notice.event_notice(ezDevSDK_App_Event_Switchover, ptr_event->event_context);
        break;
    case sdk_kernel_event_invaild_authcode:
        g_all_config.notice.event_notice(ezDevSDK_App_Event_Invaild_authcode, ptr_event->event_context);
        break;
    case sdk_kernel_event_fast_reg_online:
        g_all_config.notice.event_notice(ezDevSDK_App_Event_fast_reg_online, ptr_event->event_context);
        break;
    case sdk_kernel_event_reconnect_success:
        g_all_config.notice.event_notice(ezDevSDK_App_Event_Reconnect_success, ptr_event->event_context);
        break;
    case sdk_kernel_event_heartbeat_interval_changed:
        g_all_config.notice.event_notice(ezDevSDK_App_Event_heartbeat_interval_changed, ptr_event->event_context);
        break;
    case sdk_kernel_event_runtime_err:
        g_all_config.notice.event_notice(ezDevSDK_App_Event_Runtime_err, ptr_event->event_context);
    default:
        break;
    }
}

int ezDevSDK_Init(const char *server_name, unsigned int server_port, ezDevSDK_all_config *all_config, EZDEV_SDK_UINT32 reg_mode)
{
    char devinfo_string[4 * 1024] = {0};
    int result_code = ezdev_sdk_kernel_succ;
    ezdev_sdk_kernel_platform_handle kernel_platform_handle;
    memset(&kernel_platform_handle, 0, sizeof(kernel_platform_handle));
#ifdef _WIN32
    win_socket_init();
#endif // WIN32
    ezdev_sdk_kernel_log_debug(0, 0, "ezDevSDK_Init, g_init:%d \n",g_init);
    do
    {
        if (0 != g_init)
        {
            result_code = ezdev_sdk_kernel_invald_call;
            break;
        }

        //检查参数合法性
        if (all_config == NULL || all_config->notice.event_notice == NULL || all_config->notice.log_notice == NULL ||
            all_config->config.curingDataLoadFun == NULL || all_config->config.curingDataSaveFun == NULL)
        {
            ezdev_sdk_kernel_log_debug(0, 0, " some callback fun is null \n");
            result_code = ezdev_sdk_kernel_params_invalid;
            break;
        }

        //检查参数合法性
        if (0 == all_config->config.bUser)
        {
            if (!strlen(all_config->config.dev_id) || !strlen(all_config->config.dev_masterkey))
            {
                ezdev_sdk_kernel_log_debug(0, 0, "dev_id or masterkey is null \n");
                result_code = ezdev_sdk_kernel_params_invalid;
                break;
            }
        }
        else
        {
            if (all_config->config.keyValueLoadFun == NULL || all_config->config.keyValueSaveFun == NULL)
            {
                ezdev_sdk_kernel_log_debug(0, 0, "keyValueFun is null \n");
                result_code = ezdev_sdk_kernel_params_invalid;
                break;
            }
        }

        memset(&g_all_config, 0, sizeof(g_all_config));
        memcpy(&g_all_config, all_config, sizeof(ezDevSDK_all_config));

        result_code = get_devinfo_fromconfig(g_all_config.config.devinfo_path, devinfo_string, 4 * 1024);
        if (result_code != 0)
        {
            log_print("get_devinfo_fromconfig error :%d, path:%s", result_code, g_all_config.config.devinfo_path);
            result_code = ezdev_sdk_kernel_value_load;
            break;
        }

        kernel_platform_handle.net_work_create = net_create;
        kernel_platform_handle.net_work_connect = net_connect;
        kernel_platform_handle.net_work_read = net_read;
        kernel_platform_handle.net_work_write = net_write;
        kernel_platform_handle.net_work_disconnect = net_disconnect;
        kernel_platform_handle.net_work_destroy = net_destroy;
        kernel_platform_handle.net_work_getsocket = net_getsocket;
        kernel_platform_handle.time_creator = Platform_TimerCreater;
        kernel_platform_handle.time_isexpired_bydiff = Platform_TimeIsExpired_Bydiff;
        kernel_platform_handle.time_isexpired = Platform_TimerIsExpired;
        kernel_platform_handle.time_countdownms = Platform_TimerCountdownMS;
        kernel_platform_handle.time_countdown = Platform_TimerCountdown;
        kernel_platform_handle.time_leftms = Platform_TimerLeftMS;
        kernel_platform_handle.time_destroy = Platform_TimeDestroy;
        kernel_platform_handle.sdk_kernel_log = sdk_kernel_logprint;
        kernel_platform_handle.key_value_load = value_load;
        kernel_platform_handle.key_value_save = value_save;
        kernel_platform_handle.curing_data_load = curing_data_load;
        kernel_platform_handle.curing_data_save = curing_data_save;
        kernel_platform_handle.thread_mutex_create = sdk_platform_thread_mutex_create;
        kernel_platform_handle.thread_mutex_destroy = sdk_platform_thread_mutex_destroy;
        kernel_platform_handle.thread_mutex_lock = sdk_platform_thread_mutex_lock;
        kernel_platform_handle.thread_mutex_unlock = sdk_platform_thread_mutex_unlock;
        kernel_platform_handle.time_sleep = sdk_thread_sleep;

        result_code = ezdev_sdk_kernel_init(server_name, server_port, &kernel_platform_handle, event_notice_from_sdk_kernel, devinfo_string, (kernel_das_info *)all_config->config.reg_das_info, reg_mode);
        if (result_code != ezdev_sdk_kernel_succ)
        {
            log_print("ezdev_sdk_kernel_init error :%d\n", result_code);
            break;
        }

        result_code = common_init();
        if (result_code != ezdev_sdk_kernel_succ)
        {
            log_print("ezDevSDK_Common_Module_Init error :%d\n", result_code);
            break;
        }

        g_init = 1;
    } while (0);

    return result_code;
}

int ezDevSDK_Fini()
{
    ezdev_sdk_kernel_log_debug(0, 0, "ezDevSDK_Fini is g_init:%d \n", g_init);
    if (1 != g_init)
    {
        return ezdev_sdk_kernel_invald_call;
    }
   
    ezdev_sdk_kernel_fini();
    ezDevSDK_Common_Module_Fini();

    g_init = 0;

    return ezdev_sdk_kernel_succ;
}

int ezDevSDK_Start()
{
    int result = 0;
    ezdev_sdk_kernel_error kernel_error = ezdev_sdk_kernel_succ;

    kernel_error = ezdev_sdk_kernel_start();
    if (kernel_error != ezdev_sdk_kernel_succ)
    {
        return kernel_error;
    }

    do
    {
        g_running = 1;
        memset(&g_main_thread, 0, sizeof(g_main_thread));
        memset(&g_user_thread, 0, sizeof(g_user_thread));

        g_main_thread.task_do = sdk_main_thread;
        snprintf(g_main_thread.thread_name, 16, BOOT_MAIN_THREAD_NAME);
        result = sdk_thread_create(&g_main_thread);
        if (result != 0)
        {
            break;
        }

        g_user_thread.task_do = sdk_user_thread;
        snprintf(g_user_thread.thread_name, 16, BOOT_USER_THREAD_NAME);
        result = sdk_thread_create(&g_user_thread);
    } while (0);

    if (result != 0)
    {
        ezdev_sdk_kernel_log_debug(result, 0, "sdk_thread_create err \n");
        g_running = 0;
        ezdev_sdk_kernel_stop();
        sdk_thread_destroy(&g_main_thread);
        sdk_thread_destroy(&g_user_thread);

        result = ezdev_sdk_kernel_internal;
    }

    return result;
}

int ezDevSDK_Stop()
{
    ezdev_sdk_kernel_error kernel_error = ezdev_sdk_kernel_succ;
    
    if (g_running)
    {
        g_running = 0;
        sdk_thread_destroy(&g_main_thread);
        sdk_thread_destroy(&g_user_thread);
    }
    ezdev_sdk_kernel_log_debug(0, 0, "ezDevSDK_Stop\n");
    kernel_error = ezdev_sdk_kernel_stop();
    if (kernel_error != ezdev_sdk_kernel_succ)
    {
        return kernel_error;
    }

    return 0;
}