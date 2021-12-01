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
 * 
 * Change Logs:
 * Date           Author       Notes
 * 2021-12-01     xurongjun    Remove redundant functions
 *******************************************************************************/

#include <string.h>
#include "utils.h"
#include "mbedtls/rsa.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mkernel_internal_error.h"
#include "ez_iot_core_def.h"

#if !defined(MBEDTLS_CONFIG_FILE)
#include "mbedtls/config.h"
#else
#include MBEDTLS_CONFIG_FILE
#endif

#if defined(MBEDTLS_PLATFORM_C)
#include "mbedtls/platform.h"
#else
#include <stdio.h>
#define mbedtls_printf printf
#endif

#define TIMESPEC_THOUSAND 1000
#define TIMESPEC_MILLION 1000000
#define TIMESPEC_BILLION 1000000000

void md5_hexdump(unsigned const char *src, int len, int upper, unsigned char *dst)
{
    int i;
    char fmt[8];

    if (upper)
    {
        strcpy(fmt, "%02X");
    }
    else
    {
        strcpy(fmt, "%02x");
    }

    for (i = 0; i < len; ++i)
    {
        sprintf((char *)dst + 2 * i, fmt, src[i]);
    }
}

unsigned int mkiE2ezE(unsigned int mkernel_err)
{
    unsigned int rv = 0;

    switch (mkernel_err)
    {
    case mkernel_internal_succ:
        rv = EZ_CORE_ERR_SUCC;
        break;
    case mkernel_internal_no_start:
    case mkernel_internal_haven_stop:
    case mkernel_internal_invald_call:
        rv = EZ_CORE_ERR_NOT_READY;
        break;
    case mkernel_internal_input_param_invalid:
        rv = EZ_CORE_ERR_PARAM_INVALID;
        break;
    case mkernel_internal_call_mqtt_buffer_too_short:
        rv = EZ_CORE_ERR_OUT_RANGE;
        break;
    case mkernel_internal_mem_lack:
    case mkernel_internal_malloc_error:
    case mkernel_internal_queue_full:
    case mkernel_internal_extend_full:
        rv = EZ_CORE_ERR_MEMORY;
        break;
    case mkernel_internal_extend_no_find:
        rv = EZ_CORE_ERR_NO_EXTEND;
        break;
    case mkernel_internal_value_load_err:
    case mkernel_internal_value_save_err:
        rv = EZ_CORE_ERR_STORAGE;
        break;
    case mkernel_internal_create_sock_error:
    case mkernel_internal_net_poll_err:
    case mkernel_internal_net_getsockopt_error:
    case mkernel_internal_net_poll_event_err:
    case mkernel_internal_net_gethostbyname_error:
    case mkernel_internal_net_connect_error:
    case mkernel_internal_net_connect_timeout:
    case mkernel_internal_lbs_connect_error:
    case mkernel_internal_call_mqtt_connect:
    case mkernel_internal_das_need_reconnect:
    case mkernel_internal_net_socket_closed:
    case mkernel_internal_net_socket_err:
    case mkernel_internal_net_send_error:
    case mkernel_internal_net_read_error:
    case mkernel_internal_net_read_error_request:
    case mkernel_internal_net_socket_error:
    case mkernel_internal_net_socket_timeout:
    case mkernel_internal_rev_invalid_packet:
    case mkernel_internal_call_mqtt_sub_error:
    case mkernel_internal_call_mqtt_pub_error:
        rv = EZ_CORE_ERR_NET;
        break;
    case mkernel_internal_platform_lbs_signcheck_error:
    case mkernel_internal_platform_lbs_order_error:
    case mkernel_internal_platform_stun_process_invalid:
    case mkernel_internal_platform_das_process_invalid:
    case mkernel_internal_platform_invalid_data:
    case mkernel_internal_platform_devid_inconformity:
    case mkernel_internal_platform_masterkey_invalid:
    case mkernel_internal_platform_query_authcode_error:
    case mkernel_internal_platform_stun_sessionkey_inconformity:
    case mkernel_internal_platform_lbs_check_sessionkey_fail:
    case mkernel_internal_platform_lbs_sign_check_fail:
    case mkernel_internal_platform_query_authcode_redis:
    case mkernel_internal_platform_dec_error:
    case mkernel_internal_platform_enc_error:
    case mkernel_internal_platform_getstun_error:
    case mkernel_internal_platform_secretkey_decrypt_fail:
    case mkernel_internal_platform_secretkey_overflow_windows:
    case mkernel_internal_platform_secretkey_no_user:
    case mkernel_internal_platform_secretkey_serial_not_exist:
    case mkernel_internal_platform_secretkey_again:
        rv = EZ_CORE_ERR_AUTH;
        break;
    case mkernel_internal_mqtt_blacklist:
        rv = EZ_CORE_ERR_RISK_CRTL;
        break;
    default:
        rv = EZ_CORE_ERR_GENERAL;
        break;
    }

    return rv;
}

int get_module_build_date(char *pbuf)
{
    int year = 0;
    int month = 0;
    int day = 0;
    char month_name[4] = {0};
    const char *all_mon_names[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

    sscanf(__DATE__, "%s%d%d", month_name, &day, &year);

    for (month = 0; month < 12; month++)
    {
        if (strcmp(month_name, all_mon_names[month]) == 0)
        {
            break;
        }
    }

    month++;
    year -= 2000;
    sprintf(pbuf, " build %02d%02d%02d", year, month, day);

    return 0;
}

char ezcore_time_isexpired_bydiff(ezos_timespec_t *assign_timer, unsigned int time_ms)
{
    ezos_timespec_t now, res;
    if (NULL == assign_timer)
    {
        return (char)1;
    }

    ezos_get_clock(&now);
    res.tv_sec = now.tv_sec - assign_timer->tv_sec;
    res.tv_nsec = now.tv_nsec - assign_timer->tv_nsec;
    if (res.tv_nsec < 0)
    {
        --res.tv_sec;
        res.tv_nsec += TIMESPEC_BILLION;
    }

    if ((res.tv_sec * TIMESPEC_THOUSAND + res.tv_nsec / TIMESPEC_MILLION) > time_ms)
    {
        return 1;
    }
    else
    {
        return 0;
    }

    return res.tv_sec < 0 || (res.tv_sec == 0 && res.tv_nsec <= 0);
}

void ezcore_time_countdown(ezos_timespec_t *assign_timer, unsigned int time_count)
{
    ezos_timespec_t now;
    if (NULL == assign_timer)
    {
        return;
    }

    ezos_get_clock(&now);
    assign_timer->tv_sec = assign_timer->tv_sec + now.tv_sec;
    assign_timer->tv_nsec = assign_timer->tv_sec + now.tv_nsec;

    if (assign_timer->tv_nsec >= TIMESPEC_BILLION)
    {
        ++assign_timer->tv_sec;
        assign_timer->tv_nsec -= TIMESPEC_BILLION;
    }
}