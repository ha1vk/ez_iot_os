#include <ezlog.h>
#include "ez_iot_core.h"
#include "ez_iot_core_ctx.h"
#include "ez_iot_core_lowlvl.h"
#include "sdk_kernel_def.h"
#include "ezdev_sdk_kernel_struct.h"

static ez_thread_t g_main_thread;
#if CONFIG_EZIOT_CORE_MULTI_TASK
static ez_thread_t g_user_thread;
#endif

static ez_bool_t g_running = ez_false;
static ez_bool_t g_is_inited = 0;

static ez_kv_default_node_t m_kv_default_table[] = {
    {EZ_KV_DEFALUT_KEY_DEVID, "", 0},     /* basic kv */
    {EZ_KV_DEFALUT_KEY_MASTERKEY, "", 0}, /* basic kv */
    {EZ_KV_DEFALUT_KEY_TSLMAP, "", 0},    /* tslmap profile map */
    {EZ_KV_DEFALUT_KEY_HUBLIST, "", 0},   /* hub module sublist */
};

static const ez_kv_default_t m_default_kv = {m_kv_default_table, sizeof(m_kv_default_table) / sizeof(ez_kv_default_node_t)};

static void main_thread_func(void *param);
static void user_thread_func(void *param);

EZOS_API ez_err_t ez_iot_core_init(const ez_server_info_t *psrv_info, const ez_dev_info_t *pdev_info, const ez_event_notice pfunc)
{
    FUNC_IN();

    ez_err_t rv = EZ_CORE_ERR_SUCC;
    ez_char_t devid[32] = {0};
    ez_int32_t devid_len = sizeof(devid);

    if (g_is_inited)
    {
        goto done;
    }

    CHECK_COND_DONE(!psrv_info, EZ_CORE_ERR_PARAM_INVALID);
    CHECK_COND_DONE(!pdev_info, EZ_CORE_ERR_PARAM_INVALID);
    CHECK_COND_DONE(!pfunc, EZ_CORE_ERR_PARAM_INVALID);
    CHECK_COND_DONE(0 == ezos_strlen(psrv_info->host), EZ_CORE_ERR_PARAM_INVALID);

    ez_iot_event_notice_set(pfunc);
    CHECK_COND_DONE(ezos_kv_init(&m_default_kv), EZ_CORE_ERR_STORAGE);
    CHECK_COND_DONE(ezos_kv_raw_get(EZ_KV_DEFALUT_KEY_DEVID, (ez_void_t*)devid, &devid_len), EZ_CORE_ERR_STORAGE);

    int ez_rv = ez_kernel_init(psrv_info, pdev_info, devid, ez_iot_event_adapt);
    CHECK_COND_DONE(ez_rv, EZ_CORE_ERR_GENERAL);
    g_is_inited = ez_true;

done:
    FUNC_OUT();

    return rv;
}

EZOS_API ez_err_t ez_iot_core_start(ez_void_t)
{
    FUNC_IN();
    ez_err_t rv = EZ_CORE_ERR_SUCC;

    CHECK_RV_DONE(ez_kernel_start());
    g_running = ez_true;

    const ez_char_t *main_thread_name = "ez_core_main";
    rv = ezos_thread_create(&g_main_thread, main_thread_name, main_thread_func, (void *)main_thread_name,
                            CONFIG_EZIOT_CORE_ACEESS_TASK_STACK_SIZE, CONFIG_EZIOT_CORE_ACEESS_TASK_PRIORITY);

    CHECK_COND_DONE(rv, EZ_CORE_ERR_MEMORY);

#if CONFIG_EZIOT_CORE_MULTI_TASK
    const ez_char_t *user_thread_name = "ez_core_user";
    rv = ezos_thread_create(&g_user_thread, user_thread_name, user_thread_func, (void *)user_thread_name,
                            CONFIG_EZIOT_CORE_USER_TASK_STACK_SIZE, CONFIG_EZIOT_CORE_USER_TASK_PRIORITY);

    CHECK_COND_DONE(rv, EZ_CORE_ERR_MEMORY);
#endif

done:
    if (EZ_CORE_ERR_SUCC != rv)
    {
        g_running = ez_false;
        ez_kernel_stop();
        ezos_thread_destroy(g_main_thread);
        g_main_thread = NULL;

#if CONFIG_EZIOT_CORE_MULTI_TASK
        ezos_thread_destroy(g_user_thread);
        g_user_thread = NULL;
#endif
    }

    FUNC_OUT();

    return rv;
}

EZOS_API ez_err_t ez_iot_core_stop(ez_void_t)
{
    FUNC_IN();

    ez_err_t rv = EZ_CORE_ERR_SUCC;

    CHECK_RV_DONE(ez_kernel_stop());

    g_running = ez_false;
    ezos_thread_destroy(g_main_thread);
    g_main_thread = NULL;

#if CONFIG_EZIOT_CORE_MULTI_TASK
    ezos_thread_destroy(g_user_thread);
    g_user_thread = NULL;
#endif

done:
    FUNC_OUT();

    return rv;
}

EZOS_API ez_void_t ez_iot_core_deinit(ez_void_t)
{
    FUNC_IN();

    if (!g_is_inited)
    {
        goto done;
    }

    ezos_kv_deinit();
    ez_kernel_fini();
    g_is_inited = ez_false;

done:
    FUNC_OUT();
}

EZOS_API ez_err_t ez_iot_core_ctrl(ez_cmd_e cmd, ez_void_t *arg)
{
    ez_err_t rv = EZ_CORE_ERR_SUCC;

    switch (cmd)
    {
    case EZ_CMD_DEVID_SET:
    {
        ez_char_t *pdevid = (ez_char_t *)arg;
        ez_iot_devid_set(pdevid);
        break;
    }
    case EZ_CMD_KVIMPL_SET:
    {
        ez_kv_func_t *pfuncs = (ez_kv_func_t *)arg;
        ezos_kv_callback_set(pfuncs);
        break;
    }
    default:
        rv = EZ_CORE_ERR_PARAM_INVALID;
        break;
    }

    return rv;
}

static void main_thread_func(void *param)
{
    ez_err_t rv = EZ_CORE_ERR_SUCC;
    char *thread_name = (char *)param;

    do
    {
        rv = ez_kernel_yield();

        if (EZ_CORE_ERR_NOT_READY == rv || EZ_CORE_ERR_RISK_CRTL == rv)
        {
            ezlog_w(TAG_CORE, "thread %s exit, rv:%d", thread_name, rv);
            break;
        }

#ifndef CONFIG_EZIOT_CORE_MULTI_TASK
        rv = ez_kernel_yield_user();
        if (EZ_CORE_ERR_NOT_READY == rv || EZ_CORE_ERR_RISK_CRTL == rv)
        {
            ezlog_w(TAG_CORE, "thread %s exit, rv:%d", thread_name, rv);
            break;
        }
#endif

        rv = EZ_CORE_ERR_SUCC;
        ezos_delay_ms(10);
    } while (g_running);

    return;
}

#if CONFIG_EZIOT_CORE_MULTI_TASK
static void user_thread_func(void *param)
{
    ez_err_t rv = EZ_CORE_ERR_SUCC;
    char *thread_name = (char *)param;

    do
    {
        rv = ez_kernel_yield_user();
        if (EZ_CORE_ERR_NOT_READY == rv || EZ_CORE_ERR_RISK_CRTL == rv)
        {
            ezlog_w(TAG_CORE, "thread %s exit, rv:%d", thread_name, rv);
            break;
        }

        ezos_delay_ms(10);
    } while (g_running);

    return;
}

#endif