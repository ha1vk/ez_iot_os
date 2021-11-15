#include "ez_iot_core.h"
#include <ezlog.h>
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
    {EZ_KV_DEFALUT_KEY_MASTERKEY, "", 0}, /* basic kv */
    {EZ_KV_DEFALUT_KEY_TSLMAP, "", 0},    /* tslmap profile map */
    {EZ_KV_DEFALUT_KEY_HUBLIST, "", 0},   /* hub module sublist */
};

static const ez_kv_default_t m_default_kv = {m_kv_default_table, sizeof(m_kv_default_table) / sizeof(ez_kv_default_node_t)};

static void main_thread_func(void *param);
static void user_thread_func(void *param);

EZOS_API ez_err_t EZOS_CALL ez_iot_init(const ez_server_info_t *psrv_info, const ez_dev_info_t *pdev_info,
                                        const ez_event_notice pfunc)
{
    FUNC_IN();

    ez_err_t rv = EZ_ERR_SUCC;

    if (g_is_inited)
    {
        goto done;
    }

    CHECK_COND_DONE(!psrv_info, EZ_ERR_PARAM_INVALID);
    CHECK_COND_DONE(!pdev_info, EZ_ERR_PARAM_INVALID);
    CHECK_COND_DONE(!pfunc, EZ_ERR_PARAM_INVALID);
    CHECK_COND_DONE(0 == ezos_strlen(psrv_info->host), EZ_ERR_PARAM_INVALID);

    ez_iot_event_notice_set(pfunc);
    CHECK_COND_DONE(!ez_iot_devid_get(), EZ_ERR_DEVID);
    CHECK_COND_DONE(!ezos_kv_init(&m_default_kv), EZ_ERR_STORAGE);

    int ez_rv = ez_kernel_init(psrv_info, pdev_info, ez_iot_devid_get(), ez_iot_event_adapt);
    CHECK_COND_DONE(ez_rv, EZ_ERR_GENERAL);

done:
    FUNC_OUT();

    return rv;
}

EZOS_API ez_err_t EZOS_CALL ez_iot_start(ez_void_t)
{
    FUNC_IN();
    ez_err_t rv = EZ_ERR_SUCC;

    CHECK_RV_DONE(ez_kernel_start());
    g_running = ez_true;

    const ez_char_t *main_thread_name = "ez_core_main";
    rv = ezos_thread_create(&g_main_thread, main_thread_name, main_thread_func, NULL,
                            CONFIG_EZIOT_CORE_ACEESS_TASK_STACK_SIZE, CONFIG_EZIOT_CORE_ACEESS_TASK_PRIORITY);

    CHECK_COND_DONE(rv, EZ_ERR_MEMORY);

#if CONFIG_EZIOT_CORE_MULTI_TASK
    const ez_char_t *user_thread_name = "ez_core_user";
    rv = ezos_thread_create(&g_user_thread, user_thread_name, user_thread_func, NULL,
                            CONFIG_EZIOT_CORE_USER_TASK_STACK_SIZE, CONFIG_EZIOT_CORE_USER_TASK_PRIORITY);

    CHECK_COND_DONE(rv, EZ_ERR_MEMORY);
#endif

    if (EZ_ERR_SUCC != rv)
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

done:
    FUNC_OUT();

    return rv;
}

EZOS_API ez_err_t EZOS_CALL ez_iot_stop(ez_void_t)
{
    FUNC_IN();

    ez_err_t rv = EZ_ERR_SUCC;

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

EZOS_API ez_void_t EZOS_CALL ez_iot_deinit(ez_void_t)
{
    FUNC_IN();

    if (!g_is_inited)
    {
        goto done;
    }

    ez_kernel_fini();
    g_is_inited = ez_false;

done:
    FUNC_OUT();
}

EZOS_API ez_err_t EZOS_CALL ez_iot_attr_ctrl(ez_cmd_e cmd, ez_void_t *arg)
{
    ez_err_t rv = EZ_ERR_SUCC;

    switch (cmd)
    {
    case EZ_CMD_DEVID_SET:
    {
        ez_byte_t *pdevid = (ez_byte_t *)arg;
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
        rv = EZ_ERR_PARAM_INVALID;
        break;
    }

    return rv;
}

static void main_thread_func(void *param)
{
    ez_err_t rv = EZ_ERR_SUCC;
    ez_bool_t *running = (ez_bool_t *)param;

    do
    {
        rv = ez_kernel_yield();
        if (EZ_ERR_NOT_READY == rv || EZ_ERR_RISK_CRTL == rv)
        {
            ezlog_e(TAG_CORE, "err occured, thread will exit, rv:%d", rv);
            break;
        }

#ifndef CONFIG_EZIOT_CORE_MULTI_TASK
        rv = ez_kernel_yield_user();
        if (EZ_ERR_NOT_READY == rv || EZ_ERR_RISK_CRTL == rv)
        {
            ezlog_e(TAG_CORE, "err occured, thread will exit, rv:%d", rv);
            break;
        }
#endif

        rv = EZ_ERR_SUCC;
        ezos_delay_ms(10);
    } while (*running);

    return;
}

#if CONFIG_EZIOT_CORE_MULTI_TASK
static void user_thread_func(void *param)
{
    ez_err_t rv = EZ_ERR_SUCC;
    ez_bool_t *running = (ez_bool_t *)param;

    do
    {
        rv = ez_kernel_yield_user();
        if (EZ_ERR_NOT_READY == rv || EZ_ERR_RISK_CRTL == rv)
        {
            ezlog_e(TAG_CORE, "err occured, thread will exit, rv:%d", rv);
            break;
        }

        ezos_delay_ms(10);
    } while (*running);

    return;
}

#endif