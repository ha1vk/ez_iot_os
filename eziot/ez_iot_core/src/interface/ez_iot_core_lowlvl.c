#include <mbedtls/platform.h>
#include <ezlog.h>
#include "ez_iot_core.h"
#include "ez_iot_core_ctx.h"
#include "ez_iot_core_lowlvl.h"
#include "ezdev_sdk_kernel_struct.h"
#include "ezdev_sdk_kernel_extend.h"
#include "ezdev_sdk_kernel_event.h"
#include "ezdev_sdk_kernel_access.h"
#include "ezdev_sdk_kernel_risk_control.h"
#include "das_transport.h"
#include "utils.h"

#define ACCESS_DEFAULT_KEEPALIVEINTERVAL 30

static ez_mutex_t g_mutex_lock;
extern EZDEV_SDK_UINT32 g_das_transport_seq;
extern ezdev_sdk_kernel g_ezdev_sdk_kernel;

EZDEV_SDK_KERNEL_EXTEND_INTERFACE
DAS_TRANSPORT_INTERFACE
EZDEV_SDK_KERNEL_EVENT_INTERFACE
EZDEV_SDK_KERNEL_ACCESS_INTERFACE
EZDEV_SDK_KERNEL_RISK_CONTROL_INTERFACE

static EZDEV_SDK_UINT32 genaral_seq()
{
    int seq = 0;
    ezos_mutex_lock(g_mutex_lock);
    seq = ++g_das_transport_seq;
    ezos_mutex_unlock(g_mutex_lock);
    return seq;
}

EZOS_API ez_err_t ez_kernel_init(const ez_server_info_t *psrv_info, const ez_dev_info_t *pdev_info,
                                 const ez_char_t *devid, sdk_kernel_event_notice kernel_event_notice_cb)

{
    FUNC_IN();
    ez_err_t rv = EZ_CORE_ERR_SUCC;

    if (sdk_idle0 != g_ezdev_sdk_kernel.my_state)
    {
        goto done;
    }

    ezos_memset(&g_ezdev_sdk_kernel, 0, sizeof(g_ezdev_sdk_kernel));
    mbedtls_platform_set_calloc_free(ezos_calloc, ezos_free);

    g_ezdev_sdk_kernel.key_value_save = ez_iot_value_save;
    /* init auth method */
    g_ezdev_sdk_kernel.dev_def_auth_type = sdk_dev_auth_protocol_ecdh;
    g_ezdev_sdk_kernel.dev_cur_auth_type = sdk_dev_auth_protocol_ecdh;
    g_ezdev_sdk_kernel.dev_last_auth_type = sdk_dev_auth_protocol_ecdh;
    g_ezdev_sdk_kernel.dev_auth_type_count = 1;
    g_ezdev_sdk_kernel.dev_auth_type_group[0] = sdk_dev_auth_protocol_ecdh;
    g_ezdev_sdk_kernel.das_keepalive_interval = ACCESS_DEFAULT_KEEPALIVEINTERVAL;

    /* set server info */
    ezos_strncpy(g_ezdev_sdk_kernel.server_info.server_name, (char *)psrv_info->host, ezdev_sdk_name_len - 1);
    g_ezdev_sdk_kernel.server_info.server_port = psrv_info->port;

    /* set devinfo */
    g_ezdev_sdk_kernel.dev_info.dev_auth_mode = pdev_info->auth_mode;
    g_ezdev_sdk_kernel.dev_info.dev_status = 1;
    ezos_strncpy(g_ezdev_sdk_kernel.dev_info.dev_subserial, (char *)pdev_info->dev_subserial, sizeof(g_ezdev_sdk_kernel.dev_info.dev_subserial) - 1);
    ezos_strncpy(g_ezdev_sdk_kernel.dev_info.dev_verification_code, (char *)pdev_info->dev_verification_code, sizeof(g_ezdev_sdk_kernel.dev_info.dev_verification_code) - 1);
    ezos_strncpy(g_ezdev_sdk_kernel.dev_info.dev_firmwareversion, (char *)pdev_info->dev_firmwareversion, sizeof(g_ezdev_sdk_kernel.dev_info.dev_firmwareversion) - 1);
    ezos_strncpy(g_ezdev_sdk_kernel.dev_info.dev_type, (char *)pdev_info->dev_type, sizeof(g_ezdev_sdk_kernel.dev_info.dev_type) - 1);
    ezos_strncpy(g_ezdev_sdk_kernel.dev_info.dev_typedisplay, (char *)pdev_info->dev_typedisplay, sizeof(g_ezdev_sdk_kernel.dev_info.dev_typedisplay) - 1);
    ezos_strncpy(g_ezdev_sdk_kernel.dev_info.dev_mac, (char *)pdev_info->dev_mac, sizeof(g_ezdev_sdk_kernel.dev_info.dev_mac) - 1);

    ezlog_i(TAG_CORE, "type:%s", g_ezdev_sdk_kernel.dev_info.dev_type);
    ezlog_i(TAG_CORE, "sn:%s", g_ezdev_sdk_kernel.dev_info.dev_subserial);
    ezlog_i(TAG_CORE, "vcode:%s", g_ezdev_sdk_kernel.dev_info.dev_verification_code);
    ezlog_i(TAG_CORE, "fw:%s", g_ezdev_sdk_kernel.dev_info.dev_firmwareversion);
    ezlog_i(TAG_CORE, "display:%s", g_ezdev_sdk_kernel.dev_info.dev_typedisplay);
    ezlog_i(TAG_CORE, "devid:");

    ezos_strncpy((char *)g_ezdev_sdk_kernel.dev_id, devid, sizeof(g_ezdev_sdk_kernel.dev_id));
    size_t key_len = sizeof(g_ezdev_sdk_kernel.master_key);
    CHECK_COND_DONE(ezos_kv_raw_get(EZ_KV_DEFALUT_KEY_MASTERKEY, g_ezdev_sdk_kernel.master_key, &key_len), EZ_CORE_ERR_STORAGE);

    ezlog_hexdump(TAG_CORE, 16, (ez_uint8_t*)g_ezdev_sdk_kernel.dev_id, sizeof(g_ezdev_sdk_kernel.dev_id));

    /* 初始化链接状态 */
    g_ezdev_sdk_kernel.lbs_redirect_times = 0;
    g_ezdev_sdk_kernel.das_retry_times = 0;
    g_ezdev_sdk_kernel.entr_state = sdk_entrance_normal;
    g_ezdev_sdk_kernel.my_state = sdk_idle0;
    g_ezdev_sdk_kernel.cnt_state = sdk_cnt_unredirect;
    g_ezdev_sdk_kernel.access_risk = sdk_no_risk_control;
    g_ezdev_sdk_kernel.reg_mode = 1;
    ezos_memset(&g_ezdev_sdk_kernel.cnt_state_timer, 0, sizeof(g_ezdev_sdk_kernel.cnt_state_timer));

    /* 初始化领域和公共领域 */
    extend_init(kernel_event_notice_cb);

    /* 初始化MQTT和消息队列 */
    das_object_init(&g_ezdev_sdk_kernel);
    g_mutex_lock = ezos_mutex_create();
    g_das_transport_seq = 0;
    g_ezdev_sdk_kernel.my_state = sdk_idle;

done:
    FUNC_OUT();

    return rv;
}

EZOS_API ez_err_t ez_kernel_start()
{
    FUNC_IN();
    ez_err_t rv = EZ_CORE_ERR_SUCC;

    CHECK_COND_DONE(sdk_idle0 == g_ezdev_sdk_kernel.my_state, EZ_CORE_ERR_NOT_INIT);
    CHECK_COND_DONE(sdk_start == g_ezdev_sdk_kernel.my_state, EZ_CORE_ERR_GENERAL);

    g_ezdev_sdk_kernel.my_state = sdk_start;
    broadcast_user_start();

done:
    FUNC_OUT();

    return rv;
}

EZOS_API ez_err_t ez_kernel_stop()
{
    FUNC_IN();
    ez_err_t rv = EZ_CORE_ERR_SUCC;

    CHECK_COND_DONE(sdk_idle0 == g_ezdev_sdk_kernel.my_state, EZ_CORE_ERR_NOT_INIT);
    CHECK_COND_DONE(sdk_idle == g_ezdev_sdk_kernel.my_state, EZ_CORE_ERR_NOT_READY);
    CHECK_COND_DONE(sdk_stop == g_ezdev_sdk_kernel.my_state, EZ_CORE_ERR_GENERAL);

    g_ezdev_sdk_kernel.my_state = sdk_stop;
    access_stop_yield(&g_ezdev_sdk_kernel);

done:
    FUNC_OUT();

    return rv;
}

EZOS_API ez_err_t ez_kernel_fini()
{
    FUNC_IN();
    ez_err_t rv = EZ_CORE_ERR_SUCC;

    CHECK_COND_DONE(sdk_idle0 == g_ezdev_sdk_kernel.my_state, EZ_CORE_ERR_NOT_INIT);
    CHECK_COND_DONE(sdk_start == g_ezdev_sdk_kernel.my_state, EZ_CORE_ERR_GENERAL);

    das_object_fini(&g_ezdev_sdk_kernel);
    extend_fini();

    if (g_mutex_lock)
    {
        ezos_mutex_destroy(g_mutex_lock);
        g_mutex_lock = NULL;
    }

    ezos_memset(&g_ezdev_sdk_kernel, 0, sizeof(g_ezdev_sdk_kernel));
done:
    FUNC_OUT();

    return rv;
}

EZOS_API ez_err_t ez_kernel_yield()
{
    ez_err_t rv = EZ_CORE_ERR_NOT_READY;
    if (sdk_start == g_ezdev_sdk_kernel.my_state)
    {
        rv = mkiE2ezE(access_server_yield(&g_ezdev_sdk_kernel));
    }

    // ezlog_v(TAG_CORE, "yield rv:%d", rv);
    return rv;
}

EZOS_API ez_err_t ez_kernel_yield_user()
{
    ez_err_t rv = EZ_CORE_ERR_NOT_READY;
    if (sdk_start == g_ezdev_sdk_kernel.my_state)
    {
        rv = mkiE2ezE(extend_yield(&g_ezdev_sdk_kernel));
    }

    // ezlog_v(TAG_CORE, "yield_user rv:%d", rv);
    return rv;
}

EZOS_API ez_err_t ez_kernel_extend_load(const ez_kernel_extend_t *external_extend)
{
    FUNC_IN();
    ez_err_t rv = EZ_CORE_ERR_SUCC;

    if (sdk_idle != g_ezdev_sdk_kernel.my_state && sdk_start != g_ezdev_sdk_kernel.my_state)
    {
        rv = EZ_CORE_ERR_NOT_INIT;
        goto done;
    }

    CHECK_COND_DONE(!external_extend, EZ_CORE_ERR_PARAM_INVALID);
    CHECK_COND_DONE(!external_extend->ezdev_sdk_kernel_extend_data_route, EZ_CORE_ERR_PARAM_INVALID);
    CHECK_COND_DONE(!external_extend->ezdev_sdk_kernel_extend_start, EZ_CORE_ERR_PARAM_INVALID);
    CHECK_COND_DONE(!external_extend->ezdev_sdk_kernel_extend_stop, EZ_CORE_ERR_PARAM_INVALID);
    CHECK_COND_DONE(extend_load(external_extend), EZ_CORE_ERR_GENERAL);

done:
    FUNC_OUT();

    return rv;
}

EZOS_API ez_err_t ez_kernel_send(ez_kernel_pubmsg_t *pubmsg)
{
    FUNC_IN();

    ezdev_sdk_kernel_pubmsg_exchange *new_pubmsg_exchange = NULL;
    ez_err_t rv = EZ_CORE_ERR_SUCC;
    char cRiskResult = 0;

    CHECK_COND_DONE(sdk_start != g_ezdev_sdk_kernel.my_state, EZ_CORE_ERR_NOT_INIT);
    CHECK_COND_DONE(!pubmsg, EZ_CORE_ERR_PARAM_INVALID);
    CHECK_COND_DONE(!pubmsg->msg_body, EZ_CORE_ERR_PARAM_INVALID);
    CHECK_COND_DONE(!pubmsg->msg_body_len, EZ_CORE_ERR_PARAM_INVALID);

    CHECK_COND_DONE(pubmsg->msg_body_len > CONFIG_EZIOT_CORE_MESSAGE_SIZE_MAX, EZ_CORE_ERR_OUT_RANGE);

    cRiskResult = check_cmd_risk_control(&g_ezdev_sdk_kernel, pubmsg->msg_domain_id, pubmsg->msg_command_id);
    CHECK_COND_DONE(1 == cRiskResult, EZ_CORE_ERR_NO_EXTEND);
    CHECK_COND_DONE((2 == cRiskResult || 3 == cRiskResult), EZ_CORE_ERR_RISK_CRTL);

    new_pubmsg_exchange = (ezdev_sdk_kernel_pubmsg_exchange *)ezos_malloc(sizeof(ezdev_sdk_kernel_pubmsg_exchange));
    CHECK_COND_DONE(!new_pubmsg_exchange, EZ_CORE_ERR_MEMORY);

    ezos_memset(new_pubmsg_exchange, 0, sizeof(ezdev_sdk_kernel_pubmsg_exchange));
    ezos_strncpy(new_pubmsg_exchange->msg_conntext.command_ver, pubmsg->command_ver, version_max_len - 1);
    new_pubmsg_exchange->msg_conntext.msg_response = pubmsg->msg_response;
    new_pubmsg_exchange->msg_conntext.msg_qos = pubmsg->msg_qos;
    new_pubmsg_exchange->msg_conntext.msg_seq = pubmsg->msg_seq;
    new_pubmsg_exchange->msg_conntext.msg_domain_id = pubmsg->msg_domain_id;
    new_pubmsg_exchange->msg_conntext.msg_command_id = pubmsg->msg_command_id;

    new_pubmsg_exchange->msg_conntext.msg_body = (ez_char_t *)ezos_malloc(pubmsg->msg_body_len);
    CHECK_COND_DONE(!new_pubmsg_exchange->msg_conntext.msg_body, EZ_CORE_ERR_MEMORY);

    ezos_memset(new_pubmsg_exchange->msg_conntext.msg_body, 0, pubmsg->msg_body_len);
    new_pubmsg_exchange->msg_conntext.msg_body_len = pubmsg->msg_body_len;
    ezos_memcpy(new_pubmsg_exchange->msg_conntext.msg_body, pubmsg->msg_body, pubmsg->msg_body_len);
    new_pubmsg_exchange->max_send_count = CONFIG_EZIOT_CORE_DEFAULT_PUBLISH_RETRY;

    if (pubmsg->msg_response == 0)
    {
        pubmsg->msg_seq = genaral_seq();
        new_pubmsg_exchange->msg_conntext.msg_seq = pubmsg->msg_seq;
    }

    ezlog_w(TAG_CORE, "s: %d, seq:%d", pubmsg->msg_command_id, pubmsg->msg_seq);
    ezlog_d(TAG_CORE, "len:%d, payload:%s", pubmsg->msg_body_len, pubmsg->msg_body);

    if (NULL != pubmsg->externel_ctx && 0 != pubmsg->externel_ctx_len)
    {
        new_pubmsg_exchange->msg_conntext.externel_ctx = (unsigned char *)ezos_malloc(pubmsg->externel_ctx_len);
        CHECK_COND_DONE(!new_pubmsg_exchange->msg_conntext.externel_ctx, EZ_CORE_ERR_MEMORY);

        ezos_memcpy(new_pubmsg_exchange->msg_conntext.externel_ctx, pubmsg->externel_ctx, pubmsg->externel_ctx_len);
        new_pubmsg_exchange->msg_conntext.externel_ctx_len = pubmsg->externel_ctx_len;
    }

    CHECK_COND_DONE(das_send_pubmsg_async(&g_ezdev_sdk_kernel, new_pubmsg_exchange), EZ_CORE_ERR_MEMORY);
done:

    if (rv != EZ_CORE_ERR_SUCC && NULL != new_pubmsg_exchange)
    {
        if (NULL != new_pubmsg_exchange->msg_conntext.msg_body)
        {
            ezos_free(new_pubmsg_exchange->msg_conntext.msg_body);
        }

        if (NULL != new_pubmsg_exchange->msg_conntext.externel_ctx)
        {
            ezos_free(new_pubmsg_exchange->msg_conntext.externel_ctx);
        }

        ezos_free(new_pubmsg_exchange);
    }

    FUNC_OUT();
    return rv;
}

EZOS_API ez_err_t ez_kernel_extend_load_v3(const ez_kernel_extend_v3_t *external_extend)
{
    FUNC_IN();
    ez_err_t rv = EZ_CORE_ERR_SUCC;

    if (sdk_idle != g_ezdev_sdk_kernel.my_state && sdk_start != g_ezdev_sdk_kernel.my_state)
    {
        rv = EZ_CORE_ERR_NOT_INIT;
        goto done;
    }

    CHECK_COND_DONE(!external_extend, EZ_CORE_ERR_PARAM_INVALID);
    CHECK_COND_DONE(!external_extend->ez_kernel_data_route, EZ_CORE_ERR_PARAM_INVALID);
    CHECK_COND_DONE(!external_extend->ez_kernel_event_route, EZ_CORE_ERR_PARAM_INVALID);
    CHECK_COND_DONE(!ezos_strlen(external_extend->module), EZ_CORE_ERR_PARAM_INVALID);

    rv = extend_load_v3(external_extend);
    CHECK_COND_DONE(mkernel_internal_extend_full == rv, EZ_CORE_ERR_MEMORY);
    CHECK_COND_DONE(rv, EZ_CORE_ERR_GENERAL);
    g_ezdev_sdk_kernel.v3_reg_status = sdk_v3_reged;

done:
    FUNC_OUT();

    return rv;
}

EZOS_API ez_err_t ez_kernel_send_v3(ez_kernel_pubmsg_v3_t *pubmsg)
{
    FUNC_IN();

    ezdev_sdk_kernel_pubmsg_exchange_v3 *new_pubmsg_exchange = NULL;
    ez_err_t rv = EZ_CORE_ERR_SUCC;

    CHECK_COND_DONE(sdk_start != g_ezdev_sdk_kernel.my_state, EZ_CORE_ERR_NOT_INIT);
    CHECK_COND_DONE(!pubmsg, EZ_CORE_ERR_PARAM_INVALID);
    CHECK_COND_DONE(!pubmsg->msg_body, EZ_CORE_ERR_PARAM_INVALID);
    CHECK_COND_DONE(!pubmsg->msg_body_len, EZ_CORE_ERR_PARAM_INVALID);

    CHECK_COND_DONE(pubmsg->msg_body_len > CONFIG_EZIOT_CORE_MESSAGE_SIZE_MAX, EZ_CORE_ERR_OUT_RANGE);

    new_pubmsg_exchange = (ezdev_sdk_kernel_pubmsg_exchange_v3 *)ezos_malloc(sizeof(ezdev_sdk_kernel_pubmsg_exchange_v3));
    CHECK_COND_DONE(!new_pubmsg_exchange, EZ_CORE_ERR_MEMORY);

    ezos_memset(new_pubmsg_exchange, 0, sizeof(ezdev_sdk_kernel_pubmsg_exchange_v3));
    new_pubmsg_exchange->msg_conntext_v3.msg_qos = pubmsg->msg_qos;
    new_pubmsg_exchange->msg_conntext_v3.msg_seq = pubmsg->msg_seq;
    ezos_strncpy(new_pubmsg_exchange->msg_conntext_v3.module, pubmsg->module, ezdev_sdk_module_name_len - 1);
    ezos_strncpy(new_pubmsg_exchange->msg_conntext_v3.resource_id, pubmsg->resource_id, ezdev_sdk_resource_id_len - 1);
    ezos_strncpy(new_pubmsg_exchange->msg_conntext_v3.resource_type, pubmsg->resource_type, ezdev_sdk_resource_type_len - 1);
    ezos_strncpy(new_pubmsg_exchange->msg_conntext_v3.msg_type, pubmsg->msg_type, ezdev_sdk_msg_type_len - 1);
    ezos_strncpy(new_pubmsg_exchange->msg_conntext_v3.method, pubmsg->method, ezdev_sdk_method_len - 1);
    ezos_strncpy(new_pubmsg_exchange->msg_conntext_v3.sub_serial, pubmsg->sub_serial, ezdev_sdk_max_serial_len - 1);
    ezos_strncpy(new_pubmsg_exchange->msg_conntext_v3.ext_msg, pubmsg->ext_msg, ezdev_sdk_ext_msg_len - 1);

    if (0 == pubmsg->msg_response)
    {
        pubmsg->msg_seq = genaral_seq();
    }

    ezlog_w(TAG_CORE, "s3:%s, seq:%d", pubmsg->ext_msg, pubmsg->msg_seq);
    ezlog_d(TAG_CORE, "resource_id:%s, resource_type:%s, business_type:%s, payload:%s", pubmsg->resource_id, pubmsg->resource_type, pubmsg->method, pubmsg->msg_body);

    new_pubmsg_exchange->msg_conntext_v3.msg_body = (ez_char_t *)ezos_malloc(pubmsg->msg_body_len);
    CHECK_COND_DONE(!new_pubmsg_exchange->msg_conntext_v3.msg_body, EZ_CORE_ERR_MEMORY);

    ezos_memset(new_pubmsg_exchange->msg_conntext_v3.msg_body, 0, pubmsg->msg_body_len);
    new_pubmsg_exchange->msg_conntext_v3.msg_body_len = pubmsg->msg_body_len;
    ezos_memcpy(new_pubmsg_exchange->msg_conntext_v3.msg_body, pubmsg->msg_body, pubmsg->msg_body_len);
    new_pubmsg_exchange->max_send_count = CONFIG_EZIOT_CORE_DEFAULT_PUBLISH_RETRY;
    new_pubmsg_exchange->msg_conntext_v3.msg_seq = pubmsg->msg_seq;

    CHECK_COND_DONE(das_send_pubmsg_async_v3(&g_ezdev_sdk_kernel, new_pubmsg_exchange), EZ_CORE_ERR_MEMORY);
done:

    if (rv != EZ_CORE_ERR_SUCC && NULL != new_pubmsg_exchange)
    {
        if (NULL != new_pubmsg_exchange->msg_conntext_v3.msg_body)
        {
            ezos_free(new_pubmsg_exchange->msg_conntext_v3.msg_body);
        }

        ezos_free(new_pubmsg_exchange);
    }

    FUNC_OUT();
    return rv;
}

EZOS_API const ez_char_t *ez_kernel_getdevinfo_bykey(const ez_char_t *key)
{
    static const ez_char_t *g_default_value = "invalidkey";

    if (ezos_strcmp(key, "dev_subserial") == 0)
    {
        return g_ezdev_sdk_kernel.dev_info.dev_subserial;
    }
    else if (ezos_strcmp(key, "dev_serial") == 0)
    {
        return g_ezdev_sdk_kernel.dev_info.dev_serial;
    }
    else if (ezos_strcmp(key, "dev_firmwareversion") == 0)
    {
        return g_ezdev_sdk_kernel.dev_info.dev_firmwareversion;
    }
    else if (ezos_strcmp(key, "dev_type") == 0)
    {
        return g_ezdev_sdk_kernel.dev_info.dev_type;
    }
    else if (ezos_strcmp(key, "dev_typedisplay") == 0)
    {
        return g_ezdev_sdk_kernel.dev_info.dev_typedisplay;
    }
    else if (ezos_strcmp(key, "dev_mac") == 0)
    {
        return g_ezdev_sdk_kernel.dev_info.dev_mac;
    }
    else if (ezos_strcmp(key, "dev_firmwareidentificationcode") == 0)
    {
        return g_ezdev_sdk_kernel.dev_info.dev_firmwareidentificationcode;
    }
    else if (ezos_strcmp(key, "dev_verification_code") == 0)
    {
        return g_ezdev_sdk_kernel.dev_info.dev_verification_code;
    }
    else
    {
        return g_default_value;
    }
}