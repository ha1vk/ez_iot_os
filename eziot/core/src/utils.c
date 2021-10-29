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

#include <string.h>
#include "utils.h"
#include "mbedtls/rsa.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mkernel_internal_error.h"
#include "ez_sdk_error.h"

#if !defined(MBEDTLS_CONFIG_FILE)
#include "mbedtls/config.h"
#else
#include MBEDTLS_CONFIG_FILE
#endif

#if defined(MBEDTLS_PLATFORM_C)
#include "mbedtls/platform.h"
#else
#include <stdio.h>
#define mbedtls_printf     printf
#endif

#define TIMESPEC_THOUSAND 1000
#define TIMESPEC_MILLION 1000000
#define TIMESPEC_BILLION 1000000000

#ifndef _REALTEK_RTOS_

int ezRandomGen(unsigned char *buf, unsigned int len)
{
	int ret = 0;
	char *identifier = "ezDevSDK";
	mbedtls_ctr_drbg_context ctr_drbg;
	mbedtls_entropy_context entropy;
	mbedtls_ctr_drbg_init( &ctr_drbg );
	mbedtls_entropy_init( &entropy );
	do 
	{
		if(0 != (ret = mbedtls_ctr_drbg_seed( &ctr_drbg, mbedtls_entropy_func, &entropy, (unsigned char*)identifier, strlen(identifier))))
			break;

		mbedtls_ctr_drbg_set_prediction_resistance( &ctr_drbg, MBEDTLS_CTR_DRBG_PR_OFF );

		ret = mbedtls_ctr_drbg_random( &ctr_drbg, buf, len);
	} while (0);

	mbedtls_ctr_drbg_free( &ctr_drbg );
	mbedtls_entropy_free( &entropy );

	return ret;
}

int ezRsaEncrypt(const unsigned char *pIn, int iInLen, unsigned char *pOut, int *iOutLen, const char *pN, const char *pE)
{
    int ret = MBEDTLS_EXIT_FAILURE;
	char *identifier = "ezDevSDK";
    mbedtls_rsa_context rsa;
	mbedtls_ctr_drbg_context ctr_drbg;
	mbedtls_entropy_context entropy;
    mbedtls_rsa_init( &rsa, MBEDTLS_RSA_PKCS_V15, 0 );
	mbedtls_ctr_drbg_init( &ctr_drbg );
	mbedtls_entropy_init( &entropy );

	do 
	{
		if(0 != mbedtls_ctr_drbg_seed( &ctr_drbg, mbedtls_entropy_func,
			&entropy, (const unsigned char *) identifier,
			strlen( identifier ) ))
		{
			break;
		}

		if (0 != mbedtls_mpi_read_string(&rsa.N, 16, pN) ||
			0 != mbedtls_mpi_read_string(&rsa.E, 16, pE))
		{
			break;
		}

		rsa.len = ( mbedtls_mpi_bitlen( &rsa.N ) + 7 ) >> 3;
		if( *iOutLen < rsa.len )
		{
			///< buf too small
			break;
		}

		if (0 != mbedtls_rsa_pkcs1_encrypt( &rsa, mbedtls_ctr_drbg_random, &ctr_drbg, MBEDTLS_RSA_PUBLIC,
			iInLen, pIn, pOut))
		{
			break;
		}
		
		ret = MBEDTLS_EXIT_SUCCESS;
		*iOutLen = rsa.len;
	} while (0);

	mbedtls_ctr_drbg_free( &ctr_drbg );
	mbedtls_entropy_free( &entropy );
	mbedtls_rsa_free( &rsa );
	return ret;
}

int ezRsaDecrypt(const unsigned char *pIn, int iInLen, unsigned char *pOut, int *iOutLen, const char *pP, const char *pQ, 
				 const char *pN, const char *pD, const char *pE)
{
	int ret = MBEDTLS_EXIT_FAILURE;
	char *identifier = "ezDevSDK";
	mbedtls_rsa_context rsa;
	mbedtls_ctr_drbg_context ctr_drbg;
	mbedtls_entropy_context entropy;
    size_t olen = *iOutLen;
	mbedtls_rsa_init( &rsa, MBEDTLS_RSA_PKCS_V15, 0 );
	mbedtls_ctr_drbg_init( &ctr_drbg );
	mbedtls_entropy_init( &entropy );
	

	do 
	{
		if(0 != mbedtls_ctr_drbg_seed( &ctr_drbg, mbedtls_entropy_func,
			&entropy, (const unsigned char *) identifier,
			strlen( identifier ) ))
		{
			break;
		}

		if (0 != mbedtls_mpi_read_string(&rsa.N, 16, pN) ||
			0 != mbedtls_mpi_read_string(&rsa.D, 16, pD) ||
			0 != mbedtls_mpi_read_string(&rsa.E, 16, pE) ||
			0 != mbedtls_mpi_read_string(&rsa.P, 16, pP) ||
			0 != mbedtls_mpi_read_string(&rsa.Q, 16, pQ))
		{
			break;
		}

		rsa.len = ( mbedtls_mpi_bitlen( &rsa.N ) + 7 ) >> 3;

		if( iInLen != rsa.len)		///< Invalid RSA signature format
		{
			break;
		}

		if (0 != mbedtls_rsa_pkcs1_decrypt( &rsa, mbedtls_ctr_drbg_random, &ctr_drbg, MBEDTLS_RSA_PRIVATE,
				&olen, pIn, pOut, olen))
		{
			break;
		}

		*iOutLen = (int)olen; 
		ret = MBEDTLS_EXIT_SUCCESS;
	} while (0);

	mbedtls_ctr_drbg_free( &ctr_drbg );
	mbedtls_entropy_free( &entropy );
	mbedtls_rsa_free( &rsa );
	return ret;
}

void md5_hexdump(unsigned const char* src, int len,  int upper, unsigned char* dst)
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
        sprintf((char*)dst + 2 * i, fmt, src[i]);
    }
}

#endif // _REALTEK_RTOS_

unsigned int mkiE2ezE(unsigned int mkernel_err)
{
	unsigned int rv = 0;
	
	switch(mkernel_err)
	{
	case mkernel_internal_succ:
		rv = ezdev_sdk_kernel_succ;
		break;
	case mkernel_internal_no_start:
	case mkernel_internal_haven_stop:
	case mkernel_internal_invald_call:
		rv = ezdev_sdk_kernel_invald_call;
		break;
	case mkernel_internal_input_param_invalid:
		rv = ezdev_sdk_kernel_params_invalid;
		break;
	case mkernel_internal_mem_lack:
	case mkernel_internal_call_mqtt_buffer_too_short:
		rv = ezdev_sdk_kernel_buffer_too_small;
		break;
	case mkernel_internal_malloc_error:
		rv = ezdev_sdk_kernel_memory;
		break;
	case mkernel_internal_json_parse_error:
		rv = ezdev_sdk_kernel_json_invalid;
		break;
	case mkernel_internal_get_error_json:
		rv = ezdev_sdk_kernel_json_format;
		break;
	case mkernel_internal_extend_no_find:
		rv = ezdev_sdk_kernel_extend_no_find;
		break;
	case mkernel_internal_extend_full:
		rv = ezdev_sdk_kernel_extend_full;
		break;
	case mkernel_internal_extend_ready:
		rv = ezdev_sdk_kernel_extend_existed;
		break;
	case mkernel_internal_queue_full:
		rv = ezdev_sdk_kernel_queue_full;
		break;
	case mkernel_internal_value_load_err:
		rv = ezdev_sdk_kernel_value_load;
		break;
	case mkernel_internal_value_save_err:
		rv = ezdev_sdk_kernel_value_save;
		break;
	case mkernel_internal_create_sock_error:
	case mkernel_internal_net_poll_err:
	case mkernel_internal_net_getsockopt_error:
	case mkernel_internal_net_poll_event_err:
		rv = ezdev_sdk_kernel_net_create;
		break;
	case mkernel_internal_net_gethostbyname_error:
		rv = ezdev_sdk_kernel_net_dns;
		break;
	case mkernel_internal_net_connect_error:
	case mkernel_internal_net_connect_timeout:
	case mkernel_internal_lbs_connect_error:
	case mkernel_internal_call_mqtt_connect:
		rv = ezdev_sdk_kernel_net_connect;
		break;
	case mkernel_internal_das_need_reconnect:
	case mkernel_internal_net_socket_closed:
	case mkernel_internal_net_socket_err:
		rv = ezdev_sdk_kernel_net_disconnected;
		break;
	case mkernel_internal_net_send_error:
	case mkernel_internal_net_read_error:
	case mkernel_internal_net_read_error_request:
	case mkernel_internal_net_socket_error:
	case mkernel_internal_net_socket_timeout:
	case mkernel_internal_rev_invalid_packet:
	case mkernel_internal_call_mqtt_sub_error:
	case mkernel_internal_call_mqtt_pub_error:
		rv = ezdev_sdk_kernel_net_transmit;
		break;
	case mkernel_internal_platform_lbs_signcheck_error:
		rv = ezdev_sdk_kernel_lbs_authcode_mismatch;
		break;
	case mkernel_internal_platform_lbs_order_error:
	case mkernel_internal_platform_stun_process_invalid:
	case mkernel_internal_platform_das_process_invalid:
		rv = ezdev_sdk_kernel_lbs_invalid_call;
		break;
	case mkernel_internal_platform_invalid_data:
		rv = ezdev_sdk_kernel_lbs_invalid_call;
		break;
	case mkernel_internal_platform_devid_inconformity:
		rv = ezdev_sdk_kernel_lbs_devid_mismatch;
		break;
	case mkernel_internal_platform_masterkey_invalid:
		rv = ezdev_sdk_kernel_lbs_masterkey_mismatch;
		break;
	case mkernel_internal_platform_query_authcode_error:
		rv = ezdev_sdk_kernel_lbs_invalid_dev;
		break;
	case mkernel_internal_platform_stun_sessionkey_inconformity:
		rv = ezdev_sdk_kernel_lbs_sessionkey_mismatch;
		break;
    case mkernel_internal_platform_lbs_check_sessionkey_fail:
		rv = ezdev_sdk_kernel_lbs_check_sessionkey_fail;
		break;
	case mkernel_internal_platform_lbs_sign_check_fail:
		rv = ezdev_sdk_kernel_lbs_sign_check_fail;
		break;
	case mkernel_internal_platform_query_authcode_redis:
		rv = ezdev_sdk_kernel_lbs_server_exception;
		break;
	case mkernel_internal_platform_dec_error:
	case mkernel_internal_platform_enc_error:
		rv = ezdev_sdk_kernel_lbs_server_crypto;
		break;
	case mkernel_internal_platform_getstun_error:
		rv = ezdev_sdk_kernel_lbs_get_data;
		break;
	case mkernel_internal_platform_secretkey_decrypt_fail:
		rv = ezdev_sdk_kernel_secretkey_decrypt_fail;
		break;
	case mkernel_internal_platform_secretkey_overflow_windows:
		rv = ezdev_sdk_kernel_secretkey_overflow_windows;
		break;
	case mkernel_internal_platform_secretkey_no_user:
		rv = ezdev_sdk_kernel_secretkey_no_user;
		break;
	case mkernel_internal_platform_secretkey_serial_not_exist:
		rv = ezdev_sdk_kernel_secretkey_sn_not_exist;
		break;
	case mkernel_internal_platform_secretkey_again:
		rv = ezdev_sdk_kernel_secretkey_again;
		break;
	case mkernel_internal_mqtt_nosupport_protocol_version:
		rv = ezdev_sdk_kernel_das_nosupport_protocol_ver;
		break;
	case mkernel_internal_mqtt_unqualified_client_id:
		rv = ezdev_sdk_kernel_das_client_id_invalid;
		break;
	case mkernel_internal_mqtt_server_unusable:
		rv = ezdev_sdk_kernel_das_server_unusable;
		break;
	case mkernel_internal_mqtt_invalid_username:
		rv = ezdev_sdk_kernel_das_invalid_username;
		break;
	case mkernel_internal_mqtt_unauthorized:
		rv = ezdev_sdk_kernel_das_unauthorized;
		break;
	case mkernel_internal_mqtt_redirect:
		rv = ezdev_sdk_kernel_das_session_invaild;
		break;
	case mkernel_internal_mqtt_blacklist:
		rv = ezdev_sdk_kernel_das_force_dev_risk;
		break;
	case mkernel_internal_force_domain_risk:
		rv = ezdev_sdk_kernel_das_force_domain_risk;
		break;
	case mkernel_internal_force_cmd_risk:
		rv = ezdev_sdk_kernel_das_force_cmd_risk;
		break;
	case mkernel_internal_force_offline:
		rv = ezdev_sdk_kernel_das_force_offline;
		break;
	default:
		rv = ezdev_sdk_kernel_internal;
		break;
	}


	return rv;
}

int get_module_build_date(char* pbuf)
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

    ezos_time_get_clock(&now);
    res.tv_sec = assign_timer->tv_sec - now.tv_sec;
    res.tv_nsec = assign_timer->tv_nsec - now.tv_nsec;
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

    ezos_time_get_clock(&now);
    assign_timer->tv_sec = assign_timer->tv_sec + now.tv_sec;
    assign_timer->tv_nsec = assign_timer->tv_sec + now.tv_nsec;

    if (assign_timer->tv_nsec >= TIMESPEC_BILLION)
    {
        ++assign_timer->tv_sec;
        assign_timer->tv_nsec -= TIMESPEC_BILLION;
    }
}