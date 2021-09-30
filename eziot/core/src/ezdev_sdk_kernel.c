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
 *******************************************************************************/

#include "ezdev_sdk_kernel.h"
#include "mbedtls/platform.h"
#include "sdk_kernel_def.h"
#include "ezdev_sdk_kernel_extend.h"
#include "ezdev_sdk_kernel_access.h"
#include "das_transport.h"
#include "json_parser.h"
#include "ase_support.h"
#include "ezdev_sdk_kernel_common.h"
#include "ezdev_sdk_kernel_extend.h"
#include "ezdev_sdk_kernel_event.h"
#include "mkernel_internal_error.h"
#include "ezdev_sdk_kernel_struct.h"
#include "ezdev_sdk_kernel_risk_control.h"
#include "cJSON.h"
#include "utils.h"
#include "dev_protocol_def.h"
#include "ezdev_sdk_kerne_queuel.h"
#include "MQTTPublish.h"
#include "file_interface.h"
#include "io_interface.h"
#include "mem_interface.h"
#include "network_interface.h"
#include "thread_interface.h"
#include "time_interface.h"



EZDEV_SDK_KERNEL_RISK_CONTROL_INTERFACE
EZDEV_SDK_KERNEL_EXTEND_INTERFACE
DAS_TRANSPORT_INTERFACE
EZDEV_SDK_KERNEL_ACCESS_INTERFACE
JSON_PARSER_INTERFACE
EZDEV_SDK_KERNEL_COMMON_INTERFACE
ASE_SUPPORT_INTERFACE
EZDEV_SDK_KERNEL_EVENT_INTERFACE
EXTERN_QUEUE_FUN(pubmsg_exchange)


extern ezdev_sdk_kernel g_ezdev_sdk_kernel;
extern char g_binding_nic[ezdev_sdk_name_len];
extern EZDEV_SDK_UINT32 g_das_transport_seq;
static const char *g_default_value = "invalidkey";
static ez_mutex_t g_mutex_lock;

static EZDEV_SDK_UINT32 genaral_seq()
{
    int seq = 0;
    g_ezdev_sdk_kernel.platform_handle.thread_mutex_lock(g_mutex_lock);
    seq = ++g_das_transport_seq;
    g_ezdev_sdk_kernel.platform_handle.thread_mutex_unlock(g_mutex_lock);
    return seq;
}

static void *_calloc_func(size_t nmemb, size_t size)
{
    size_t mem_size;
    void *ptr = NULL;
    mem_size = nmemb * size;
    ptr = ez_malloc(mem_size);
    if (ptr)
    {
        memset(ptr, 0, mem_size);
    }
    return ptr;
}

ezdev_sdk_kernel_error ezdev_sdk_kernel_init(const sdk_config_t* pconfig, const ezdev_sdk_kernel_platform_handle* handle,
											 const sdk_kernel_event_notice event_notice_cb, const EZDEV_SDK_INT8 reg_mode)
{
    ezdev_sdk_kernel_error sdk_error = ezdev_sdk_kernel_succ;
    unsigned char szDev_vcode[ezdev_sdk_verify_code_maxlen] = {0};
    EZDEV_SDK_INT32 iDev_vcode = ezdev_sdk_verify_code_maxlen;

    if (sdk_idle0 != g_ezdev_sdk_kernel.my_state && sdk_idle2 != g_ezdev_sdk_kernel.my_state)
    {
        return ezdev_sdk_kernel_invald_call;
    }
    do
    {
        bscomptls_platform_set_calloc_free(_calloc_func, free);

        if(NULL == pconfig||NULL == pconfig->pdev_info||strlen(pconfig->server.host)>ezdev_sdk_name_len
           ||NULL == handle||NULL == event_notice_cb)
        {
            return ezdev_sdk_kernel_params_invalid;
        }

        if (handle->key_value_load == NULL || handle->net_work_connect == NULL || handle->net_work_read == NULL || handle->net_work_write == NULL ||
            handle->net_work_disconnect == NULL || handle->time_creator == NULL || handle->time_isexpired_bydiff == NULL ||
            handle->time_isexpired == NULL || handle->time_countdownms == NULL || handle->time_countdown == NULL || handle->time_leftms == NULL ||
            handle->time_destroy == NULL || handle->key_value_load == NULL || handle->key_value_save == NULL || 
            handle->curing_data_load == NULL || handle->curing_data_save == NULL)
        {
            sdk_error = ezdev_sdk_kernel_params_invalid;
            break;
        }

        memset(&g_ezdev_sdk_kernel, 0, sizeof(g_ezdev_sdk_kernel));
        g_ezdev_sdk_kernel.dev_def_auth_type = sdk_dev_auth_protocol_ecdh;
        g_ezdev_sdk_kernel.dev_cur_auth_type = sdk_dev_auth_protocol_ecdh;
        g_ezdev_sdk_kernel.dev_last_auth_type = sdk_dev_auth_protocol_ecdh;
        g_ezdev_sdk_kernel.dev_auth_type_count = 1;
        g_ezdev_sdk_kernel.dev_auth_type_group[0] = sdk_dev_auth_protocol_ecdh;

        g_ezdev_sdk_kernel.das_keepalive_interval = ezdev_sdk_das_default_keepaliveinterval;

        strncpy(g_ezdev_sdk_kernel.server_info.server_name, pconfig->server.host, ezdev_sdk_name_len - 1);
        g_ezdev_sdk_kernel.server_info.server_port = pconfig->server.port;

        memcpy(&g_ezdev_sdk_kernel.platform_handle, handle, sizeof(ezdev_sdk_kernel_platform_handle));
        sdk_error = mkiE2ezE(json_parse_devinfo(pconfig->pdev_info, &g_ezdev_sdk_kernel.dev_info));
        if (sdk_error != ezdev_sdk_kernel_succ)
        {
            break;
        }

        if (ezdev_sdk_kernel_succ == (sdk_error = handle->curing_data_load(sdk_curingdata_secretkey, szDev_vcode, &iDev_vcode)) &&iDev_vcode <= ezdev_sdk_verify_code_maxlen)
        {
            strncpy(g_ezdev_sdk_kernel.dev_info.dev_verification_code, (char *)szDev_vcode, iDev_vcode);
        }
        else
        {
            sdk_error = ezdev_sdk_kernel_value_load;
            break;
        }
  
        g_ezdev_sdk_kernel.lbs_redirect_times = 0;
        g_ezdev_sdk_kernel.das_retry_times = 0;
        g_ezdev_sdk_kernel.secretkey_applied = EZDEV_SDK_FALSE;
        g_ezdev_sdk_kernel.secretkey_interval = 0;
        g_ezdev_sdk_kernel.secretkey_duration = 0;

        g_ezdev_sdk_kernel.entr_state = sdk_entrance_normal;
        g_ezdev_sdk_kernel.my_state = sdk_idle0;
        g_ezdev_sdk_kernel.cnt_state = sdk_cnt_unredirect;
        g_ezdev_sdk_kernel.cnt_state_timer = g_ezdev_sdk_kernel.platform_handle.time_creator();
        if(NULL == g_ezdev_sdk_kernel.cnt_state_timer)
        {
            sdk_error = ezdev_sdk_kernel_memory;
            break;
        }

        g_ezdev_sdk_kernel.access_risk = sdk_no_risk_control;

        g_ezdev_sdk_kernel.platform_handle.key_value_load(sdk_keyvalue_devid, g_ezdev_sdk_kernel.dev_id, ezdev_sdk_devid_len);
        g_ezdev_sdk_kernel.platform_handle.key_value_load(sdk_keyvalue_masterkey, g_ezdev_sdk_kernel.master_key, ezdev_sdk_masterkey_len);
        if (1 > reg_mode || reg_mode > 5)
        {
            g_ezdev_sdk_kernel.reg_mode = 1;
        }
        else
        {
            g_ezdev_sdk_kernel.reg_mode = reg_mode;
        }

        if (pconfig->pdas_info && 0!= pconfig->pdas_info->bLightreg)
        {
            g_ezdev_sdk_kernel.redirect_das_info.das_port = pconfig->pdas_info->das_port;
            g_ezdev_sdk_kernel.redirect_das_info.das_udp_port = pconfig->pdas_info->das_udp_port;
            strncpy(g_ezdev_sdk_kernel.redirect_das_info.das_address, pconfig->pdas_info->das_address, ezdev_sdk_ip_max_len - 1);
            strncpy(g_ezdev_sdk_kernel.redirect_das_info.das_domain, pconfig->pdas_info->das_domain, ezdev_sdk_ip_max_len - 1);
            strncpy(g_ezdev_sdk_kernel.redirect_das_info.das_serverid, pconfig->pdas_info->das_serverid, ezdev_sdk_name_len - 1);
            memcpy(g_ezdev_sdk_kernel.session_key, pconfig->pdas_info->session_key, ezdev_sdk_sessionkey_len);

            if (1 == pconfig->pdas_info->bLightreg)
            {
                g_ezdev_sdk_kernel.cnt_state = sdk_cnt_das_fast_reg;
            }
            else if (2 == pconfig->pdas_info->bLightreg)
            {
                g_ezdev_sdk_kernel.cnt_state = sdk_cnt_das_fast_reg_v3;
            }
        }
        common_module_init();

        extend_init(event_notice_cb);
        das_object_init(&g_ezdev_sdk_kernel);

        g_mutex_lock = g_ezdev_sdk_kernel.platform_handle.thread_mutex_create();
        if(NULL == g_mutex_lock)
        {
            sdk_error = ezdev_sdk_kernel_memory;
            break;
        }

        g_das_transport_seq = 0;

    } while (0);

    if (sdk_error == ezdev_sdk_kernel_succ)
    {
        g_ezdev_sdk_kernel.my_state = sdk_idle;
    }
    else
    {
        if(NULL!=g_ezdev_sdk_kernel.cnt_state_timer)
        {
            g_ezdev_sdk_kernel.platform_handle.time_destroy(g_ezdev_sdk_kernel.cnt_state_timer);
            g_ezdev_sdk_kernel.cnt_state_timer = NULL;
        }
    }

    return sdk_error;
}

ezdev_sdk_kernel_error ezdev_sdk_kernel_fini()
{
    ezdev_sdk_kernel_log_debug(0, 0, "ezdev_sdk_kernel_fini,my_state: %d \n", g_ezdev_sdk_kernel.my_state);
    if (sdk_idle != g_ezdev_sdk_kernel.my_state && sdk_stop != g_ezdev_sdk_kernel.my_state)
    {
        return ezdev_sdk_kernel_invald_call;
    }

    das_object_fini(&g_ezdev_sdk_kernel);

    extend_fini();

    common_module_fini();

    g_ezdev_sdk_kernel.platform_handle.time_destroy(g_ezdev_sdk_kernel.cnt_state_timer);

    if(g_mutex_lock)
    {
        g_ezdev_sdk_kernel.platform_handle.thread_mutex_destroy(g_mutex_lock);
        g_mutex_lock = NULL;
    }

    memset(&g_ezdev_sdk_kernel, 0, sizeof(g_ezdev_sdk_kernel));

    return ezdev_sdk_kernel_succ;
}

ezdev_sdk_kernel_error ezdev_sdk_kernel_extend_load(const ezdev_sdk_kernel_extend *external_extend)
{
    if (sdk_idle != g_ezdev_sdk_kernel.my_state && sdk_start != g_ezdev_sdk_kernel.my_state)
    {
        return ezdev_sdk_kernel_invald_call;
    }

    if (NULL == external_extend ||
        NULL == external_extend->ezdev_sdk_kernel_extend_data_route ||
        NULL == external_extend->ezdev_sdk_kernel_extend_start ||
        NULL == external_extend->ezdev_sdk_kernel_extend_event ||
        NULL == external_extend->ezdev_sdk_kernel_extend_stop)
    {
        return ezdev_sdk_kernel_params_invalid;
    }

    return mkiE2ezE(extend_load(external_extend));
}


ezdev_sdk_kernel_error ezdev_sdk_kernel_extend_load_v3(const ezdev_sdk_kernel_extend_v3 *external_extend_v3)
{
    if (sdk_idle != g_ezdev_sdk_kernel.my_state && sdk_start != g_ezdev_sdk_kernel.my_state)
    {
        return ezdev_sdk_kernel_invald_call;
    }

    if (NULL == external_extend_v3 || NULL == external_extend_v3->ezdev_sdk_kernel_data_route||0 == strlen(external_extend_v3->module)||
        NULL ==external_extend_v3->ezdev_sdk_kernel_event_route)
    {
        return ezdev_sdk_kernel_params_invalid;
    }

    g_ezdev_sdk_kernel.v3_reg_status = sdk_v3_reged;

    return mkiE2ezE(extend_load_v3(external_extend_v3));
}


ezdev_sdk_kernel_error ezdev_sdk_kernel_common_module_load(const ezdev_sdk_kernel_common_module *common_module)
{
    if (sdk_idle != g_ezdev_sdk_kernel.my_state)
    {
        return ezdev_sdk_kernel_invald_call;
    }

    if (NULL == common_module ||NULL == common_module->ezdev_sdk_kernel_common_module_data_handle)
    {
        return ezdev_sdk_kernel_params_invalid;
    }

    return mkiE2ezE(common_module_load(common_module));
}

ezdev_sdk_kernel_error ezdev_sdk_kernel_start()
{
    ezdev_sdk_kernel_log_debug(0, 0, "ezdev_sdk_kernel_start,my_state: %d \n", g_ezdev_sdk_kernel.my_state);
    if (sdk_idle != g_ezdev_sdk_kernel.my_state)
    {
        return ezdev_sdk_kernel_invald_call;
    }

    g_ezdev_sdk_kernel.my_state = sdk_start;
    ezdev_sdk_kernel_log_warn(0, 0, "broadcast_user_start");
    broadcast_user_start();
    return ezdev_sdk_kernel_succ;
}

ezdev_sdk_kernel_error ezdev_sdk_kernel_stop()
{
    int wait4times = 0;
    ezdev_sdk_kernel_pubmsg_exchange *ptr_pubmsg_exchange = NULL;
    ezdev_sdk_kernel_log_debug(0, 0, "ezdev_sdk_kernel_stop my_state:%d \n", g_ezdev_sdk_kernel.my_state);
    if (sdk_start != g_ezdev_sdk_kernel.my_state)
    {
        return ezdev_sdk_kernel_invald_call;
    }

    if (check_access_risk_control(&g_ezdev_sdk_kernel))
    {
        ezdev_sdk_kernel_log_error(0, 0, "dev is in black list true \n");
    }

    g_ezdev_sdk_kernel.my_state = sdk_stop;
    
    clear_queue_pubmsg_exchange();
    
    send_offline_msg_to_platform(genaral_seq());

    while (sdk_cnt_das_reged == g_ezdev_sdk_kernel.cnt_state)
    {
        if (mkernel_internal_succ != access_server_yield(&g_ezdev_sdk_kernel))
            break;

        if (mkernel_internal_queue_empty == get_queue_pubmsg_exchange(&ptr_pubmsg_exchange))
            break;

        g_ezdev_sdk_kernel.platform_handle.time_sleep(100);
        if (wait4times++ > 3)
            break;
    }
    ezdev_sdk_kernel_log_debug(0, 0, "stop_yield my_state:%d \n", g_ezdev_sdk_kernel.my_state);
    
    stop_yield(&g_ezdev_sdk_kernel);

    return ezdev_sdk_kernel_succ;
}

ezdev_sdk_kernel_error ezdev_sdk_kernel_yield()
{
    if (sdk_start != g_ezdev_sdk_kernel.my_state)
    {
        return ezdev_sdk_kernel_invald_call;
    }

    return mkiE2ezE(access_server_yield(&g_ezdev_sdk_kernel));
}

ezdev_sdk_kernel_error ezdev_sdk_kernel_yield_user()
{
    if (sdk_start != g_ezdev_sdk_kernel.my_state)
    {
        return ezdev_sdk_kernel_invald_call;
    }

    return mkiE2ezE(extend_yield(&g_ezdev_sdk_kernel));
}

ezdev_sdk_kernel_error ezdev_sdk_kernel_send(ezdev_sdk_kernel_pubmsg *pubmsg)
{
    EZDEV_SDK_INT32 input_length_padding = 0;
    ezdev_sdk_kernel_pubmsg_exchange *new_pubmsg_exchange = NULL;
    ezdev_sdk_kernel_error kernel_error = ezdev_sdk_kernel_succ;
    char cRiskResult = 0;

    if (g_ezdev_sdk_kernel.my_state != sdk_start)
    {
        return ezdev_sdk_kernel_invald_call;
    }

    if (pubmsg->msg_body == NULL || pubmsg->msg_body_len == 0)
    {
        return ezdev_sdk_kernel_params_invalid;
    }

    ezdev_sdk_kernel_log_info(0, 0, "sdk send:domain:%d ,cmd:%d, seq:%d,qos:%d, len:%d, string:%s\n", pubmsg->msg_domain_id, \
                              pubmsg->msg_command_id, pubmsg->msg_seq, pubmsg->msg_qos, pubmsg->msg_body_len, pubmsg->msg_body);
                              
    if (pubmsg->msg_body_len > ezdev_sdk_send_buf_max)
    {
        return ezdev_sdk_kernel_data_len_range;
    }
    
    ezdev_sdk_kernel_log_debug(0, 0,"send buffer size: %d\n", ezdev_sdk_send_buf_max);

    cRiskResult = check_cmd_risk_control(&g_ezdev_sdk_kernel, pubmsg->msg_domain_id, pubmsg->msg_command_id);
    if (1 == cRiskResult)
        return mkiE2ezE(mkernel_internal_extend_no_find);
    else if (2 == cRiskResult)
        return mkiE2ezE(mkernel_internal_force_domain_risk);
    else if (3 == cRiskResult)
        return mkiE2ezE(mkernel_internal_force_cmd_risk);

    new_pubmsg_exchange = (ezdev_sdk_kernel_pubmsg_exchange *)ez_malloc(sizeof(ezdev_sdk_kernel_pubmsg_exchange));
    if (new_pubmsg_exchange == NULL)
    {
        return ezdev_sdk_kernel_memory;
    }

    memset(new_pubmsg_exchange, 0, sizeof(ezdev_sdk_kernel_pubmsg_exchange));
    strncpy(new_pubmsg_exchange->msg_conntext.command_ver, pubmsg->command_ver, version_max_len - 1);
    new_pubmsg_exchange->msg_conntext.msg_response = pubmsg->msg_response;
    new_pubmsg_exchange->msg_conntext.msg_qos = pubmsg->msg_qos;
    new_pubmsg_exchange->msg_conntext.msg_seq = pubmsg->msg_seq;
    new_pubmsg_exchange->msg_conntext.msg_domain_id = pubmsg->msg_domain_id;
    new_pubmsg_exchange->msg_conntext.msg_command_id = pubmsg->msg_command_id;

    input_length_padding = pubmsg->msg_body_len;
    new_pubmsg_exchange->msg_conntext.msg_body = (unsigned char *)ez_malloc(input_length_padding);
    if (new_pubmsg_exchange->msg_conntext.msg_body == NULL)
    {
        ez_free(new_pubmsg_exchange);
        new_pubmsg_exchange = NULL;

        ezdev_sdk_kernel_log_error(ezdev_sdk_kernel_memory, ezdev_sdk_kernel_memory, "malloc input_length_padding:%d error", input_length_padding);
        return ezdev_sdk_kernel_memory;
    }
    memset(new_pubmsg_exchange->msg_conntext.msg_body, 0, input_length_padding);
    new_pubmsg_exchange->msg_conntext.msg_body_len = input_length_padding;
    memcpy(new_pubmsg_exchange->msg_conntext.msg_body, pubmsg->msg_body, pubmsg->msg_body_len);
    buf_padding(new_pubmsg_exchange->msg_conntext.msg_body, input_length_padding, pubmsg->msg_body_len);
    new_pubmsg_exchange->max_send_count = ezdev_sdk_max_publish_count;

    if (pubmsg->msg_response == 0)
    {
        pubmsg->msg_seq = genaral_seq();
        new_pubmsg_exchange->msg_conntext.msg_seq = pubmsg->msg_seq;
    }

    if (NULL != pubmsg->externel_ctx && 0 != pubmsg->externel_ctx_len)
    {
        new_pubmsg_exchange->msg_conntext.externel_ctx = (unsigned char *)ez_malloc(pubmsg->externel_ctx_len);
        if (NULL == new_pubmsg_exchange->msg_conntext.externel_ctx)
        {
            ez_free(new_pubmsg_exchange->msg_conntext.msg_body);
            ez_free(new_pubmsg_exchange);
            ezdev_sdk_kernel_log_error(ezdev_sdk_kernel_memory, 0, "malloc externel_ctx:%d error", pubmsg->externel_ctx_len);
            return ezdev_sdk_kernel_memory;
        }

        memcpy(new_pubmsg_exchange->msg_conntext.externel_ctx, pubmsg->externel_ctx, pubmsg->externel_ctx_len);
        new_pubmsg_exchange->msg_conntext.externel_ctx_len = pubmsg->externel_ctx_len;
    }

    kernel_error = mkiE2ezE(das_send_pubmsg_async(&g_ezdev_sdk_kernel, new_pubmsg_exchange));
    if (kernel_error != ezdev_sdk_kernel_succ)
    {
        if (new_pubmsg_exchange != NULL)
        {
            if (new_pubmsg_exchange->msg_conntext.msg_body != NULL)
            {
                ez_free(new_pubmsg_exchange->msg_conntext.msg_body);
                new_pubmsg_exchange->msg_conntext.msg_body = NULL;
            }
            if (NULL != new_pubmsg_exchange->msg_conntext.externel_ctx)
            {
                ez_free(new_pubmsg_exchange->msg_conntext.externel_ctx);
            }
            ez_free(new_pubmsg_exchange);
            new_pubmsg_exchange = NULL;
        }
    }
    return kernel_error;
}



ezdev_sdk_kernel_error ezdev_sdk_kernel_send_v3(ezdev_sdk_kernel_pubmsg_v3 *pubmsg)
{
    EZDEV_SDK_INT32 input_length_padding = 0;
    ezdev_sdk_kernel_pubmsg_exchange_v3 *new_pubmsg_exchange = NULL;
    ezdev_sdk_kernel_error kernel_error = ezdev_sdk_kernel_succ;

    if (g_ezdev_sdk_kernel.my_state != sdk_start)
    {
        return ezdev_sdk_kernel_invald_call;
    }

    if (pubmsg->msg_body == NULL || pubmsg->msg_body_len == 0)
    {
        ezdev_sdk_kernel_log_info(0, 0, " _v3 send input msg invalid\n");
        return ezdev_sdk_kernel_params_invalid;
    }
    
     ezdev_sdk_kernel_log_info(0, 0, "_v3 sdk send:module:%s ,resource_type:%s,msg_type:%s, method:%s, ext_msg:%s, seq:%d, len:%d, string:%s\n", pubmsg->module,\
                              pubmsg->resource_type, pubmsg->msg_type, pubmsg->method, pubmsg->ext_msg, pubmsg->msg_seq, pubmsg->msg_body_len, pubmsg->msg_body);

    if (pubmsg->msg_body_len > ezdev_sdk_send_buf_max)
    {
        return ezdev_sdk_kernel_data_len_range;
    }
    
    ezdev_sdk_kernel_log_debug(0, 0,"_v3 send buffer size: %d\n", ezdev_sdk_send_buf_max);

    new_pubmsg_exchange = (ezdev_sdk_kernel_pubmsg_exchange_v3 *)ez_malloc(sizeof(ezdev_sdk_kernel_pubmsg_exchange_v3));
    if (new_pubmsg_exchange == NULL)
    {
        return ezdev_sdk_kernel_memory;
    }

    memset(new_pubmsg_exchange, 0, sizeof(ezdev_sdk_kernel_pubmsg_exchange_v3));
    new_pubmsg_exchange->msg_conntext_v3.msg_qos = pubmsg->msg_qos;
    if(0 != pubmsg->msg_response)
    {
        new_pubmsg_exchange->msg_conntext_v3.msg_seq = pubmsg->msg_seq;
    }
    else
    {
        pubmsg->msg_seq = genaral_seq();
        new_pubmsg_exchange->msg_conntext_v3.msg_seq = pubmsg->msg_seq;
    }
    ezdev_sdk_kernel_log_debug(0, 0,"msg seq: %d\n", new_pubmsg_exchange->msg_conntext_v3.msg_seq);
    strncpy(new_pubmsg_exchange->msg_conntext_v3.module, pubmsg->module, ezdev_sdk_module_name_len - 1);
    strncpy(new_pubmsg_exchange->msg_conntext_v3.resource_id, pubmsg->resource_id, ezdev_sdk_resource_id_len - 1);
    strncpy(new_pubmsg_exchange->msg_conntext_v3.resource_type, pubmsg->resource_type, ezdev_sdk_resource_type_len - 1);
    strncpy(new_pubmsg_exchange->msg_conntext_v3.msg_type, pubmsg->msg_type, ezdev_sdk_msg_type_len - 1);
    strncpy(new_pubmsg_exchange->msg_conntext_v3.method, pubmsg->method, ezdev_sdk_method_len - 1);
    strncpy(new_pubmsg_exchange->msg_conntext_v3.sub_serial, pubmsg->sub_serial, ezdev_sdk_max_serial_len - 1);
    strncpy(new_pubmsg_exchange->msg_conntext_v3.ext_msg, pubmsg->ext_msg, ezdev_sdk_ext_msg_len - 1);
 
    input_length_padding = pubmsg->msg_body_len;
    new_pubmsg_exchange->msg_conntext_v3.msg_body = (unsigned char *)ez_malloc(input_length_padding);
    if (new_pubmsg_exchange->msg_conntext_v3.msg_body == NULL)
    {
        ez_free(new_pubmsg_exchange);
        new_pubmsg_exchange = NULL;

        ezdev_sdk_kernel_log_error(ezdev_sdk_kernel_memory, 0, "malloc input_length_padding:%d error", input_length_padding);
        return ezdev_sdk_kernel_memory;
    }
    memset(new_pubmsg_exchange->msg_conntext_v3.msg_body, 0, input_length_padding);
    new_pubmsg_exchange->msg_conntext_v3.msg_body_len = input_length_padding;
    memcpy(new_pubmsg_exchange->msg_conntext_v3.msg_body, pubmsg->msg_body, pubmsg->msg_body_len);
    buf_padding(new_pubmsg_exchange->msg_conntext_v3.msg_body, input_length_padding, pubmsg->msg_body_len);
    new_pubmsg_exchange->max_send_count = ezdev_sdk_max_publish_count;
    new_pubmsg_exchange->msg_conntext_v3.msg_seq = pubmsg->msg_seq;
    kernel_error = mkiE2ezE(das_send_pubmsg_async_v3(&g_ezdev_sdk_kernel, new_pubmsg_exchange));
    if (kernel_error != ezdev_sdk_kernel_succ)
    {
        if (new_pubmsg_exchange != NULL)
        {
            if (new_pubmsg_exchange->msg_conntext_v3.msg_body != NULL)
            {
                ez_free(new_pubmsg_exchange->msg_conntext_v3.msg_body);
                new_pubmsg_exchange->msg_conntext_v3.msg_body = NULL;
            }
            ez_free(new_pubmsg_exchange);
            new_pubmsg_exchange = NULL;
        }
    }
    return kernel_error;
}

EZOS_API ezdev_sdk_kernel_error ezdev_sdk_kernel_set_net_option(int optname, const void *optval, int optlen)
{
    ezdev_sdk_kernel_error rv = ezdev_sdk_kernel_succ;

    switch (optname)
    {
    case 1:
    {
        if (NULL == optval || optlen + 1 > sizeof(g_binding_nic))
        {
            rv = ezdev_sdk_kernel_buffer_too_small;
            break;
        }
        memset(g_binding_nic, 0, sizeof(g_binding_nic));
        memcpy(g_binding_nic, (char *)optval, optlen);
        break;
    }
    case 3:
    {
        ezdev_sdk_kernel_log_trace(0, 0, "curr state:%d, force break das link", g_ezdev_sdk_kernel.cnt_state);
        if (sdk_cnt_das_reged == g_ezdev_sdk_kernel.cnt_state)
            g_ezdev_sdk_kernel.cnt_state = sdk_cnt_das_break;

        break;
    }
    default:
    {
        rv = ezdev_sdk_kernel_params_invalid;
        break;
    }
    }

    return rv;
}

const char *inner_get(const char *key)
{
    if (strcmp(key, "dev_subserial") == 0)
    {
        return g_ezdev_sdk_kernel.dev_info.dev_subserial;
    }
    else if (strcmp(key, "dev_serial") == 0)
    {
        return g_ezdev_sdk_kernel.dev_info.dev_serial;
    }
    else if (strcmp(key, "dev_firmwareversion") == 0)
    {
        return g_ezdev_sdk_kernel.dev_info.dev_firmwareversion;
    }
    else if (strcmp(key, "dev_type") == 0)
    {
        return g_ezdev_sdk_kernel.dev_info.dev_type;
    }
    else if (strcmp(key, "dev_typedisplay") == 0)
    {
        return g_ezdev_sdk_kernel.dev_info.dev_typedisplay;
    }
    else if (strcmp(key, "dev_mac") == 0)
    {
        return g_ezdev_sdk_kernel.dev_info.dev_mac;
    }
    else if (strcmp(key, "dev_nickname") == 0)
    {
        return g_ezdev_sdk_kernel.dev_info.dev_nickname;
    }
    else if (strcmp(key, "dev_firmwareidentificationcode") == 0)
    {
        return g_ezdev_sdk_kernel.dev_info.dev_firmwareidentificationcode;
    }
    else if (strcmp(key, "dev_verification_code") == 0)
    {
        return g_ezdev_sdk_kernel.dev_info.dev_verification_code;
    }
    else if (strcmp(key, "dev_binding_nic") == 0)
    {
        return g_binding_nic;
    }
    else
    {
        return g_default_value;
    }
}

const char *ezdev_sdk_kernel_getdevinfo_bykey(const char *key)
{
    if (g_ezdev_sdk_kernel.my_state == sdk_idle0)
    {
        return g_default_value;
    }

    return inner_get(key);
}

EZOS_API ezdev_sdk_kernel_error ezdev_sdk_kernel_get_sdk_version(char *pbuf, int *pbuflen)
{
    ezdev_sdk_kernel_error rv = ezdev_sdk_kernel_succ;
    char buf[64] = {0};

    if (NULL == pbuflen)
        return ezdev_sdk_kernel_params_invalid;

    get_module_build_date(buf);

    do
    {
        if (NULL == pbuf)
        {
            *pbuflen = strlen(buf) + strlen(DEV_ACCESS_DOMAIN_VERSION);
            break;
        }

        if (*pbuflen < strlen(buf) + strlen(DEV_ACCESS_DOMAIN_VERSION))
        {
            rv = ezdev_sdk_kernel_buffer_too_small;
            break;
        }

        sprintf(pbuf, "%s%s", DEV_ACCESS_DOMAIN_VERSION, buf);
        *pbuflen = strlen(buf) + strlen(DEV_ACCESS_DOMAIN_VERSION);
    } while (0);

    return rv;
}

EZOS_API ezdev_sdk_kernel_error ezdev_sdk_kernel_get_server_info(server_info_s *ptr_server_info, int *ptr_count)
{
    if (g_ezdev_sdk_kernel.my_state != sdk_start)
        return ezdev_sdk_kernel_invald_call;

    if (NULL == ptr_count || (NULL != ptr_server_info && 1 > *ptr_count))
        return ezdev_sdk_kernel_params_invalid;

    if (NULL == ptr_server_info)
    {
        *ptr_count = 1;
        return ezdev_sdk_kernel_succ;
    }

    strncpy((*ptr_server_info).lbs_domain, g_ezdev_sdk_kernel.server_info.server_name, ezdev_sdk_ip_max_len - 1);
    strncpy((*ptr_server_info).lbs_ip, g_ezdev_sdk_kernel.server_info.server_ip, ezdev_sdk_ip_max_len - 1);
    (*ptr_server_info).lbs_port = g_ezdev_sdk_kernel.server_info.server_port;

    strncpy((*ptr_server_info).das_domain, g_ezdev_sdk_kernel.redirect_das_info.das_domain, ezdev_sdk_ip_max_len - 1);
    strncpy((*ptr_server_info).das_ip, g_ezdev_sdk_kernel.redirect_das_info.das_address, ezdev_sdk_ip_max_len - 1);
    (*ptr_server_info).das_port = g_ezdev_sdk_kernel.redirect_das_info.das_port;
    (*ptr_server_info).das_udp_port = g_ezdev_sdk_kernel.redirect_das_info.das_udp_port;
    (*ptr_server_info).das_socket = ezdev_sdk_kernel_get_das_socket(&g_ezdev_sdk_kernel);
    memcpy((*ptr_server_info).session_key, g_ezdev_sdk_kernel.session_key, ezdev_sdk_sessionkey_len);
    *ptr_count = 1;

    return ezdev_sdk_kernel_succ;
}

EZOS_API ezdev_sdk_kernel_error ezdev_sdk_kernel_show_key_info(showkey_info *ptr_showkey_info)
{
    if (g_ezdev_sdk_kernel.my_state != sdk_start)
        return ezdev_sdk_kernel_invald_call;

    if (NULL == ptr_showkey_info)
        return ezdev_sdk_kernel_params_invalid;

    memset(ptr_showkey_info, 0, sizeof(showkey_info));

    if (0 != strlen((const char *)g_ezdev_sdk_kernel.master_key))
    {
        strncpy((char *)ptr_showkey_info->master_key, (const char *)g_ezdev_sdk_kernel.master_key, ezdev_sdk_masterkey_len);
    }
    if (0 != strlen((const char *)g_ezdev_sdk_kernel.dev_id))
    {
        strncpy((char *)ptr_showkey_info->dev_id, (const char *)g_ezdev_sdk_kernel.dev_id, ezdev_sdk_devid_len);
    }
    if (0 != strlen((const char *)g_ezdev_sdk_kernel.dev_info.dev_verification_code))
    {
        strncpy((char *)ptr_showkey_info->dev_verification_code, (const char *)g_ezdev_sdk_kernel.dev_info.dev_verification_code, ezdev_sdk_verify_code_maxlen);
    }

    return ezdev_sdk_kernel_succ;
}
