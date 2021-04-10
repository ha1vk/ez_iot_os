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
#include <stdio.h>
#include <malloc.h>
#include "lbs_transport.h"
#include "mkernel_internal_error.h"
#include "ezdev_sdk_kernel_struct.h"
#include "sdk_kernel_def.h"
#include "dev_protocol_def.h"
#include "ezdev_ecdh_support.h"
#include "bscJSON.h"
#include "mbedtls/sha256.h"
#include "mbedtls/md.h"
#include "mbedtls/aes.h"
#include "mbedtls/md5.h"
#include "mbedtls/ecdh.h"
#include "mbedtls/pkcs5.h"
#include "mbedtls/sha512.h"
#include "ase_support.h"
#include "json_parser.h"
#include "utils.h"

#if !defined(_REALTEK_RTOS_) && !defined(_WIN32)
#include <arpa/inet.h>
#endif


ASE_SUPPORT_INTERFACE
JSON_PARSER_INTERFACE
extern char g_binding_nic[ezdev_sdk_name_len];

#define iv_len  12
#define add_len 16
#define tag_len 16

static mkernel_internal_error parse_authentication_create_dev_id(lbs_affair *authi_affair, EZDEV_SDK_UINT32 remain_len);

static void generate_sharekey(ezdev_sdk_kernel *sdk_kernel, lbs_affair *redirect_affair, EZDEV_SDK_UINT8 nUpper)
{
	unsigned char sharekey_src[ezdev_sdk_total_len];
	EZDEV_SDK_UINT16 sharekey_src_len;
    EZDEV_SDK_UINT16 dev_verification_code_len;
    EZDEV_SDK_UINT16 sharekey_salt_len;

    unsigned char sharekey_dst[16];
    unsigned char sharekey_dst_hex[ezdev_sdk_md5_len + 1];

    unsigned char sharekey_sha256_dst[ezdev_sdk_sha256_len];
    unsigned char sharekey_sha256_dst_hex[ezdev_sdk_sha256_hex_len + 1];

    bscomptls_md_context_t sha1_ctx;
    const bscomptls_md_info_t *info_sha1 = NULL;

	memset(sharekey_src, 0, ezdev_sdk_total_len);
    memset(sharekey_dst, 0, 16);
    memset(sharekey_dst_hex, 0, ezdev_sdk_sharekey_len + 1);

    dev_verification_code_len = strlen(sdk_kernel->dev_info.dev_verification_code);
    sharekey_salt_len = strlen(ezdev_sdk_sharekey_salt);

    sharekey_src_len = dev_verification_code_len;
	memcpy(sharekey_src, sdk_kernel->dev_info.dev_verification_code, sharekey_src_len);
	memcpy(sharekey_src + sharekey_src_len, sdk_kernel->dev_info.dev_subserial, strlen(sdk_kernel->dev_info.dev_subserial));

	sharekey_src_len += strlen(sdk_kernel->dev_info.dev_subserial);

    bscomptls_md5(sharekey_src, sharekey_src_len, sharekey_dst);

	if (nUpper != 0)
	{
        bscomptls_hexdump(sharekey_dst, 16, 1, sharekey_dst_hex);
	}
	else
	{
        bscomptls_hexdump(sharekey_dst, 16, 0, sharekey_dst_hex);
	}

	memset(sharekey_src, 0, ezdev_sdk_total_len);

    memcpy(sharekey_src, sharekey_dst_hex, ezdev_sdk_md5_len);
    memcpy(sharekey_src + ezdev_sdk_md5_len, ezdev_sdk_sharekey_salt, sharekey_salt_len);

    sharekey_src_len = ezdev_sdk_md5_len + sharekey_salt_len;

    bscomptls_md5(sharekey_src, sharekey_src_len, sharekey_dst);
    bscomptls_hexdump(sharekey_dst, 16, 1, sharekey_dst_hex);

	memset(sharekey_src, 0, ezdev_sdk_total_len);
    memcpy(sharekey_src, sharekey_dst_hex, ezdev_sdk_md5_len);
    sharekey_src_len = ezdev_sdk_md5_len;

    bscomptls_md5(sharekey_src, sharekey_src_len, sharekey_dst);
    bscomptls_hexdump(sharekey_dst, 16, 1, sharekey_dst_hex);

    bscomptls_md_init(&sha1_ctx);
    info_sha1 = bscomptls_md_info_from_type(BSCOMPTLS_MD_SHA256);
    if (info_sha1 == NULL)
    {
        return;
    }

    if (bscomptls_md_setup(&sha1_ctx, info_sha1, 1) != 0)
    {
        return;
    }

    bscomptls_pkcs5_pbkdf2_hmac(&sha1_ctx, sharekey_dst_hex, ezdev_sdk_md5_len, \
                                    (const unsigned char*)ezdev_sdk_sharekey_salt, sharekey_salt_len, ezdev_sdk_pbkdf2_hmac_times, ezdev_sdk_sha256_len, sharekey_sha256_dst);
    bscomptls_hexdump(sharekey_sha256_dst, ezdev_sdk_sha256_len, 1, sharekey_sha256_dst_hex);

    bscomptls_md_free(&sha1_ctx);

    memcpy(redirect_affair->share_key, sharekey_sha256_dst_hex + ezdev_sdk_sha256_offset, ezdev_sdk_sharekey_len);
	redirect_affair->share_key_len = ezdev_sdk_sharekey_len;
}

static mkernel_internal_error init_lbs_affair(ezdev_sdk_kernel *sdk_kernel, lbs_affair *redirect_affair, EZDEV_SDK_UINT8 nUpper)
{
	redirect_affair->random_1 = rand() % 256;
	redirect_affair->random_2 = rand() % 256;
	redirect_affair->random_3 = rand() % 256;
	redirect_affair->random_4 = rand() % 256;

	redirect_affair->global_out_packet.head_buf = malloc(16);
	if(NULL == redirect_affair->global_out_packet.head_buf)
	{
		ezdev_sdk_kernel_log_error(0, 0, "malloc out_packet head_buf err");
		return mkernel_internal_mem_lack;
	}
	memset(redirect_affair->global_out_packet.head_buf, 0, 16);
	redirect_affair->global_out_packet.head_buf_off = 0;
	redirect_affair->global_out_packet.head_buf_Len = 16;

    redirect_affair->global_out_packet.var_head_buf = malloc(lbs_var_head_buf_max);
    if (NULL == redirect_affair->global_out_packet.var_head_buf)
    {
        ezdev_sdk_kernel_log_error(0, 0, "malloc out_packet head_buf err");
        return mkernel_internal_mem_lack;
    }
    memset(redirect_affair->global_out_packet.var_head_buf, 0, lbs_var_head_buf_max);
    redirect_affair->global_out_packet.var_head_buf_off = 0;
    redirect_affair->global_out_packet.var_head_buf_Len = lbs_var_head_buf_max;
	redirect_affair->global_out_packet.payload_buf = malloc(lbs_send_buf_max);
	if(NULL == redirect_affair->global_out_packet.payload_buf)
	{
		ezdev_sdk_kernel_log_error(0, 0, "malloc out_packet payload_buf err");
		return mkernel_internal_mem_lack;
	}
	memset(redirect_affair->global_out_packet.payload_buf, 0, lbs_send_buf_max);
	redirect_affair->global_out_packet.payload_buf_off = 0;
	redirect_affair->global_out_packet.payload_buf_Len = lbs_send_buf_max;
	redirect_affair->global_in_packet.head_buf = malloc(16);
	if(NULL == redirect_affair->global_in_packet.head_buf)
	{
		ezdev_sdk_kernel_log_error(0, 0, "malloc in_packet head_buf err ");
		return mkernel_internal_mem_lack;
	}
	memset(redirect_affair->global_in_packet.head_buf, 0, 16);
	redirect_affair->global_in_packet.head_buf_off = 0;
	redirect_affair->global_in_packet.head_buf_Len = 16;
    redirect_affair->global_in_packet.var_head_buf = malloc(lbs_var_head_buf_max);
    if (NULL == redirect_affair->global_in_packet.var_head_buf)
    {
        ezdev_sdk_kernel_log_error(0, 0, "malloc out_packet head_buf err");
        return mkernel_internal_mem_lack;
    }
    memset(redirect_affair->global_in_packet.var_head_buf, 0, lbs_var_head_buf_max);
    redirect_affair->global_in_packet.var_head_buf_off = 0;
    redirect_affair->global_in_packet.var_head_buf_Len = lbs_var_head_buf_max;
	redirect_affair->global_in_packet.payload_buf = malloc(lbs_recv_buf_max);
	if(NULL == redirect_affair->global_in_packet.payload_buf)
	{
		ezdev_sdk_kernel_log_error(0, 0, "malloc in_packet payload_buf err ");
		return mkernel_internal_mem_lack;
	}
	memset(redirect_affair->global_in_packet.payload_buf, 0, lbs_recv_buf_max);
	redirect_affair->global_in_packet.payload_buf_off = 0;
	redirect_affair->global_in_packet.payload_buf_Len = lbs_recv_buf_max;

	redirect_affair->dev_auth_mode = sdk_kernel->dev_info.dev_auth_mode;
	memcpy(redirect_affair->dev_subserial, sdk_kernel->dev_info.dev_subserial, ezdev_sdk_devserial_maxlen);
	memcpy(redirect_affair->dev_id, sdk_kernel->dev_id, ezdev_sdk_devid_len);
	memcpy(redirect_affair->master_key, sdk_kernel->master_key, ezdev_sdk_masterkey_len);

	redirect_affair->dev_access_mode = sdk_kernel->dev_info.dev_access_mode;

	generate_sharekey(sdk_kernel, redirect_affair, nUpper);

	redirect_affair->lbs_net_work = NULL;
	
	return mkernel_internal_succ;
}

static void fini_lbs_affair(lbs_affair *redirect_affair)
{
	if (NULL == redirect_affair)
	{
		return;
	}
	if (redirect_affair->global_out_packet.head_buf != NULL)
	{
		free(redirect_affair->global_out_packet.head_buf);
		redirect_affair->global_out_packet.head_buf = NULL;
	}
    if (redirect_affair->global_out_packet.var_head_buf != NULL)
    {
        free(redirect_affair->global_out_packet.var_head_buf);
        redirect_affair->global_out_packet.var_head_buf = NULL;
    }
	if (redirect_affair->global_out_packet.payload_buf != NULL)
	{
		free(redirect_affair->global_out_packet.payload_buf);
		redirect_affair->global_out_packet.payload_buf = NULL;
	}
	if (redirect_affair->global_in_packet.head_buf != NULL)
	{
		free(redirect_affair->global_in_packet.head_buf);
		redirect_affair->global_in_packet.head_buf = NULL;
	}
    if (redirect_affair->global_in_packet.var_head_buf != NULL)
    {
        free(redirect_affair->global_in_packet.var_head_buf);
        redirect_affair->global_in_packet.var_head_buf = NULL;
    }
	if (redirect_affair->global_in_packet.payload_buf != NULL)
	{
		free(redirect_affair->global_in_packet.payload_buf);
		redirect_affair->global_in_packet.payload_buf = NULL;
	}
	memset(redirect_affair, 0, sizeof(lbs_affair));
}

static void clear_lbs_affair_buf(lbs_affair *redirect_affair)
{
	memset(redirect_affair->global_out_packet.head_buf, 0, 16);
	redirect_affair->global_out_packet.head_buf_off = 0;

	memset(redirect_affair->global_out_packet.payload_buf, 0, lbs_send_buf_max);
	redirect_affair->global_out_packet.payload_buf_off = 0;

	memset(redirect_affair->global_in_packet.head_buf, 0, 16);
	redirect_affair->global_in_packet.head_buf_off = 0;

	memset(redirect_affair->global_in_packet.payload_buf, 0, lbs_recv_buf_max);
	redirect_affair->global_in_packet.payload_buf_off = 0;
}

static void save_key_value(ezdev_sdk_kernel *sdk_kernel, lbs_affair *affair)
{
    ezdev_sdk_kernel_log_info(0, 0, "save_key_value start !!!");
	memcpy(sdk_kernel->dev_id, affair->dev_id, ezdev_sdk_devid_len);
	memcpy(sdk_kernel->master_key, affair->master_key, ezdev_sdk_masterkey_len);
	memcpy(sdk_kernel->session_key, affair->session_key, ezdev_sdk_sessionkey_len);
    ezdev_sdk_kernel_log_info(0, 0, " save dev_id \n");
	sdk_kernel->platform_handle.key_value_save(sdk_keyvalue_devid, affair->dev_id, ezdev_sdk_devid_len);
	sdk_kernel->platform_handle.key_value_save(sdk_keyvalue_masterkey, affair->master_key, ezdev_sdk_masterkey_len);
}

static void save_das_info(ezdev_sdk_kernel *sdk_kernel, das_info *recv_das_info)
{
	memcpy(&sdk_kernel->redirect_das_info, recv_das_info, sizeof(das_info));
}

static mkernel_internal_error header_serialize(ezdev_sdk_kernel *sdk_kernel, lbs_packet *lbs_pack, EZDEV_SDK_UINT32 cmd, EZDEV_SDK_UINT32 remain_len)
{
    int i = 0;
	unsigned char byte_1 = 0;
	unsigned char byte_2 = 0;
	EZDEV_SDK_UINT32 total_remain_len = 0;  
	EZDEV_SDK_UINT32 remaining_count = 0;
    if (DEV_PROTOCOL_AUTHENTICATION_I == cmd)
    {
        *(lbs_pack->var_head_buf + lbs_pack->var_head_buf_off) = sdk_kernel->dev_cur_auth_type;
        lbs_pack->var_head_buf_off += 1;

        *(lbs_pack->var_head_buf + lbs_pack->var_head_buf_off) = sdk_kernel->dev_auth_type_count;
        lbs_pack->var_head_buf_off += 1;
        for (i = 0; i < sdk_kernel->dev_auth_type_count; i++)
        {
            *(lbs_pack->var_head_buf + lbs_pack->var_head_buf_off) = *(sdk_kernel->dev_auth_type_group + i);
            lbs_pack->var_head_buf_off += 1;
        }
    }
	if (lbs_pack->head_buf_Len < 5)
	{
		return mkernel_internal_mem_lack;
	}
	byte_1 = ((cmd & 0x0F) << 4)|0x2;
    if (DEV_PROTOCOL_AUTHENTICATION_I == cmd)
    {
        byte_1 = byte_1 | 0x8;
    }

	*(lbs_pack->head_buf + lbs_pack->head_buf_off) = byte_1;
	lbs_pack->head_buf_off += 1;

    if (DEV_PROTOCOL_AUTHENTICATION_I == cmd)
    {
        total_remain_len = remain_len + lbs_pack->var_head_buf_off;
    }
    else
    {
        total_remain_len = remain_len;
    }

	do
	{
		byte_2 = total_remain_len % 128;
		total_remain_len = total_remain_len / 128;
		if (total_remain_len > 0)
		{
			byte_2 = byte_2 | 0x80;
		}
		*(lbs_pack->head_buf + lbs_pack->head_buf_off) = byte_2;
		lbs_pack->head_buf_off++;
		remaining_count++;
	} while (total_remain_len > 0 && remaining_count < 4);

	return mkernel_internal_succ;
}

static mkernel_internal_error header_serialize_old(lbs_packet *lbs_pack, EZDEV_SDK_UINT32 cmd, EZDEV_SDK_UINT32 remain_len)
{
	unsigned char byte_1 = 0;
	unsigned char byte_2 = 0;
	EZDEV_SDK_UINT32 payload_len = 0;
	EZDEV_SDK_UINT32 remaining_count = 0;

	if (lbs_pack->head_buf_Len < 5)
	{
		return mkernel_internal_mem_lack;
	}
	byte_1 = ((cmd & 0x0F) << 4);

	*(lbs_pack->head_buf + lbs_pack->head_buf_off) = byte_1;
	lbs_pack->head_buf_off += 1;

	payload_len = remain_len;
	do
	{
		byte_2 = payload_len % 128;
		payload_len = payload_len / 128;
		if (payload_len > 0)
		{
			byte_2 = byte_2 | 0x80;
		}
		*(lbs_pack->head_buf + lbs_pack->head_buf_off) = byte_2;
		lbs_pack->head_buf_off++;
		remaining_count++;
	} while (payload_len > 0 && remaining_count < 4);

	return mkernel_internal_succ;
}

static mkernel_internal_error common_serialize(lbs_packet *lbs_pack, unsigned char pro_form_version, unsigned char pro_type_low_version, unsigned char pro_type_high_version)
{
	if ((lbs_pack->payload_buf_off + 3) > lbs_pack->payload_buf_Len)
	{
		return mkernel_internal_mem_lack;
	}
	*(lbs_pack->payload_buf + lbs_pack->payload_buf_off) = pro_form_version;
	lbs_pack->payload_buf_off++;

	*(lbs_pack->payload_buf + lbs_pack->payload_buf_off) = pro_type_low_version;
	lbs_pack->payload_buf_off++;

	*(lbs_pack->payload_buf + lbs_pack->payload_buf_off) = pro_type_high_version;
	lbs_pack->payload_buf_off++;
	return mkernel_internal_succ;
}


static mkernel_internal_error digital_sign_serialize_sha384(lbs_packet *lbs_pack, unsigned char *sign_src, EZDEV_SDK_UINT16 sign_src_len, unsigned char *master_key, EZDEV_SDK_UINT16 master_key_len)
{
	EZDEV_SDK_UINT32 sign_input_len = 0;
	unsigned char sign_input[ezdev_sdk_total_len];
	unsigned char sign_output[64];
	EZDEV_SDK_INT32 bscomptls_result;

	if ((lbs_pack->payload_buf_off + 64) > lbs_pack->payload_buf_Len)
	{
		return mkernel_internal_mem_lack;
	}

	if (sign_src_len > ezdev_sdk_total_len)
	{
		return mkernel_internal_input_param_invalid;
	}
	memset(sign_input, 0, ezdev_sdk_total_len);
	memset(sign_output, 0, 64);
	memcpy(sign_input, sign_src, sign_src_len);
	sign_input_len = sign_src_len;
	bscomptls_sha512(sign_input, sign_input_len, sign_output, 1);

	memset(sign_input, 0, ezdev_sdk_total_len);
	memcpy(sign_input, sign_output, 64);
	sign_input_len = strlen((const char*)sign_input);

	memset(sign_output, 0, 64);
	bscomptls_result = bscomptls_md_hmac(bscomptls_md_info_from_string("SHA384"), master_key, master_key_len, sign_input, 48, sign_output);
	if (bscomptls_result != 0)
	{
		return mkernel_internal_hmac_error;
	}
    
	memcpy(lbs_pack->payload_buf + lbs_pack->payload_buf_off, sign_output, 48);

	lbs_pack->payload_buf_off += 48;

	return mkernel_internal_succ;
}


static mkernel_internal_error digital_sign_serialize_and_check_sha384(unsigned char *target_sign, EZDEV_SDK_UINT16 target_sign_len,\
                             unsigned char *sign_src, EZDEV_SDK_UINT16 sign_src_len, unsigned char *master_key, EZDEV_SDK_UINT16 master_key_len)
{
	EZDEV_SDK_UINT32 sign_input_len = 0;
	unsigned char sign_input[ezdev_sdk_total_len];
	unsigned char sign_output[64];
	EZDEV_SDK_INT32 bscomptls_result = 0;

	if (sign_src_len > 128)
	{
		ezdev_sdk_kernel_log_debug(0, 0, "digital_sign_serialize_and_check_sha384 ,sign_src_len > 128\n");
		return mkernel_internal_input_param_invalid;
	}
	if (target_sign_len != 48)
	{
		ezdev_sdk_kernel_log_debug(0, 0, "digital_sign_serialize_and_check_sha384 ,target_sign_len != 64\n");
		return mkernel_internal_sign_check_error;
	}
	memset(sign_input, 0, ezdev_sdk_total_len);
	memset(sign_output, 0, 64);

	memcpy(sign_input, sign_src, sign_src_len);
	sign_input_len = sign_src_len;
	bscomptls_sha512(sign_input, sign_input_len, sign_output, 1);

	memset(sign_input, 0, ezdev_sdk_total_len);
	memcpy(sign_input, sign_output, 48);
	memset(sign_output, 0, 64);

	bscomptls_result = bscomptls_md_hmac(bscomptls_md_info_from_string("SHA384"), master_key, master_key_len, sign_input, 48, sign_output);
	if (bscomptls_result != 0)
	{
		ezdev_sdk_kernel_log_debug(bscomptls_result, 0, "bscomptls_md_hmac error\n");
		return mkernel_internal_hmac_error;
	}

	bscomptls_result = memcmp(target_sign, sign_output, 48);
	if (bscomptls_result != 0)
	{
		ezdev_sdk_kernel_log_debug(bscomptls_result, 0, "bscomptls_md_hmac check error\n");
		return mkernel_internal_hmac_compare_error;
	}

	return mkernel_internal_succ;
}
static mkernel_internal_error wait_assign_response(ezdev_sdk_kernel *sdk_kernel, lbs_affair *authi_affair, EZDEV_SDK_UINT32 *rev_cmd, EZDEV_SDK_UINT32 *remain_len)
{
	unsigned char byte_1 = 0;
	unsigned char byte_2 = 0;
	EZDEV_SDK_UINT32 remain_mult = 1;
	EZDEV_SDK_UINT32 remain_count = 0;
	char len = 0;
    unsigned char flag = 0;
	mkernel_internal_error sdk_error = sdk_kernel->platform_handle.net_work_read(authi_affair->lbs_net_work, &byte_1, 1, 5 * 1000);
	if (sdk_error != mkernel_internal_succ)
	{
		ezdev_sdk_kernel_log_debug(sdk_error, sdk_error, "wait_assign_response revc byte_1 error");
		return sdk_error;
	}

	*rev_cmd = 0;
	*rev_cmd = (byte_1 & 0xF0) >> 4;

	*remain_len = 0;
	do
	{
		byte_2 = 0;
		sdk_error = sdk_kernel->platform_handle.net_work_read(authi_affair->lbs_net_work, &byte_2, 1, 5 * 1000);
		if (sdk_error != mkernel_internal_succ)
		{
			ezdev_sdk_kernel_log_debug(sdk_error, sdk_error, "wait_assign_response revc byte_2 error");
			return sdk_error;
		}

		len = byte_2 & 0x7F;

		*remain_len += len * remain_mult;
		remain_mult *= 128;
		remain_count++;
	} while ((byte_2 & 0x80) != 0);

    authi_affair->global_in_packet.head_buf[0] = byte_1;
    authi_affair->global_in_packet.head_buf[1] = byte_2;
    flag = (byte_1 & 0x08) >> 3;
    if (0x01 == flag && DEV_PROTOCOL_AUTHENTICATION_II == *rev_cmd)
    {
        if (*remain_len > authi_affair->global_in_packet.payload_buf_Len + authi_affair->global_in_packet.var_head_buf_Len)
        {
            return mkernel_internal_mem_lack;
        }

        sdk_error = sdk_kernel->platform_handle.net_work_read(authi_affair->lbs_net_work, authi_affair->global_in_packet.var_head_buf, 1, 5 * 1000);
        if (sdk_error != 0)
        {
            ezdev_sdk_kernel_log_debug(sdk_error, sdk_error, "wait_assign_response revc var head error");
            return sdk_error;
        }
        authi_affair->global_in_packet.var_head_buf_off += 1;
        *remain_len -= authi_affair->global_in_packet.var_head_buf_off;
        if (*remain_len > authi_affair->global_in_packet.payload_buf_Len)
        {
            ezdev_sdk_kernel_log_debug(sdk_error, sdk_error, "wait_assign_response payload_buf size %d not enough for %d", authi_affair->global_in_packet.payload_buf_Len, remain_len);
            return mkernel_internal_mem_lack;
        }
    }
    else
    {
        if (*remain_len > authi_affair->global_in_packet.payload_buf_Len)
        {
            return mkernel_internal_mem_lack;
        }
    }
	sdk_error = sdk_kernel->platform_handle.net_work_read(authi_affair->lbs_net_work, authi_affair->global_in_packet.payload_buf, *remain_len, 5 * 1000);

	if (sdk_error != 0)
	{
		ezdev_sdk_kernel_log_debug(sdk_error, sdk_error, "wait_assign_response revc data error");
		return sdk_error;
	}
	ezdev_sdk_kernel_log_trace(sdk_error, sdk_error, "wait_assign_response succ, cmd:%d, len:%d", *rev_cmd, *remain_len);
	return mkernel_internal_succ;
}

static mkernel_internal_error send_lbs_msg(ezdev_sdk_kernel *sdk_kernel, lbs_affair *authi_affair)
{
	mkernel_internal_error result_ = mkernel_internal_succ;
	EZDEV_SDK_INT32 real_sendlen = 0;
    unsigned char cmd = 0;
    unsigned char flag = 0;
	result_ = sdk_kernel->platform_handle.net_work_write(authi_affair->lbs_net_work, authi_affair->global_out_packet.head_buf, authi_affair->global_out_packet.head_buf_off, 5 * 1000, &real_sendlen);
	if (result_ != mkernel_internal_succ)
	{
		return result_;
	}
    cmd = (authi_affair->global_out_packet.head_buf[0] & 0xf0) >> 4;
    flag = (authi_affair->global_out_packet.head_buf[0] & 0x08) >> 3;
    if (DEV_PROTOCOL_AUTHENTICATION_I == cmd && 0x01 == flag)
    {
        result_ = sdk_kernel->platform_handle.net_work_write(authi_affair->lbs_net_work, authi_affair->global_out_packet.var_head_buf, authi_affair->global_out_packet.var_head_buf_off, 5 * 1000, &real_sendlen);
        if (result_ != mkernel_internal_succ)
        {
            return result_;
        }
    }

	result_ = sdk_kernel->platform_handle.net_work_write(authi_affair->lbs_net_work, authi_affair->global_out_packet.payload_buf, authi_affair->global_out_packet.payload_buf_off, 5 * 1000, &real_sendlen);
	if (result_ != mkernel_internal_succ)
	{
		return result_;
	}

	return mkernel_internal_succ;
}
static mkernel_internal_error authentication_i_serialize(lbs_affair *authi_affair)
{
	EZDEV_SDK_UINT8 dev_serial_len = strlen(authi_affair->dev_subserial);

	*(EZDEV_SDK_UINT8 *)(authi_affair->global_out_packet.payload_buf + authi_affair->global_out_packet.payload_buf_off) = authi_affair->dev_auth_mode;
	authi_affair->global_out_packet.payload_buf_off++;

	*(EZDEV_SDK_UINT8 *)(authi_affair->global_out_packet.payload_buf + authi_affair->global_out_packet.payload_buf_off) = dev_serial_len;
	authi_affair->global_out_packet.payload_buf_off++;

	memcpy(authi_affair->global_out_packet.payload_buf + authi_affair->global_out_packet.payload_buf_off, authi_affair->dev_subserial, dev_serial_len);
	authi_affair->global_out_packet.payload_buf_off += dev_serial_len;

	return mkernel_internal_succ;
}

static mkernel_internal_error aes_128_encrypt_pubkey(lbs_affair *authi_affair, unsigned char* input_buf, EZDEV_SDK_UINT32 input_buf_len, \
                                                        unsigned char* output_buf, EZDEV_SDK_UINT32 *output_buf_len, \
                                                        unsigned char* output_tag_buf, EZDEV_SDK_UINT32 tag_buf_len)
{
    mkernel_internal_error sdk_error = mkernel_internal_succ;
    unsigned char *out_buf = NULL;
    EZDEV_SDK_UINT32 out_buf_len = 0;
    unsigned char aes_encrypt_key[16] = { 0 };
    unsigned char temp[512] = { 0 };
    EZDEV_SDK_UINT32 templen = 0;
    if (authi_affair == NULL || input_buf == NULL || input_buf_len == 0)
    {
        return mkernel_internal_input_param_invalid;
    }
    out_buf = (unsigned char*)malloc(input_buf_len + 1);
    if (NULL == out_buf)
    {
        sdk_error = mkernel_internal_malloc_error;
        return sdk_error;
    }
    memset(out_buf, 0, input_buf_len + 1);
    out_buf_len = input_buf_len;
    memcpy(aes_encrypt_key, authi_affair->share_key, 16);

    sdk_error = aes_gcm_128_enc_padding(aes_encrypt_key, input_buf, input_buf_len, out_buf, &out_buf_len, output_tag_buf, tag_buf_len);
    if (sdk_error != mkernel_internal_succ)
    {
        ezdev_sdk_kernel_log_warn(0, 0, "aes_gcm_128_enc_padding err!\n");
		free(out_buf);
        return sdk_error;
    }

    sdk_error = aes_gcm_128_dec_padding(aes_encrypt_key, out_buf, out_buf_len, temp, &templen, output_tag_buf, tag_buf_len);
    memcpy(output_buf, out_buf, out_buf_len);
    *output_buf_len = out_buf_len;
    if (out_buf)
    {
        free(out_buf);
    }
    ezdev_sdk_kernel_log_debug(0, 0, "aes_128_encrypt_pubkey end!\n");
    return mkernel_internal_succ;

}


static mkernel_internal_error append_encrypt_data(lbs_packet *send_data_buf, unsigned char* encrypt_data, EZDEV_SDK_UINT32 encrypt_data_len, \
                                                    unsigned char* tag_buf, EZDEV_SDK_UINT32 tag_buf_len)
{
	mkernel_internal_error sdk_error = mkernel_internal_succ;
	do
	{
		if(send_data_buf == NULL||encrypt_data == NULL)
		{
			sdk_error = mkernel_internal_input_param_invalid;
			break;
		}

        memcpy(send_data_buf->payload_buf + send_data_buf->payload_buf_off, tag_buf, tag_buf_len);
        send_data_buf->payload_buf_off += tag_buf_len;

		memcpy(send_data_buf->payload_buf+ send_data_buf->payload_buf_off, encrypt_data, encrypt_data_len);
		send_data_buf->payload_buf_off += encrypt_data_len;

	}while(0);
	
    return sdk_error;
}
static mkernel_internal_error send_authentication_i(ezdev_sdk_kernel *sdk_kernel, lbs_affair *authi_affair, void* ctx_client)
{
	mkernel_internal_error sdk_error = mkernel_internal_succ;
	unsigned char pubkey[128]= {0};
    unsigned char pubkeyhex[256] = { 0 };
	EZDEV_SDK_UINT32 pubkey_len = 0; 
    unsigned char encrypt_data[256] = {0};
	EZDEV_SDK_UINT32 encrypt_data_len = 0;
    unsigned char tag_data[tag_len] = { 0 };
    EZDEV_SDK_UINT32 tag_data_len = tag_len;
	do
	{
		sdk_error = common_serialize(&authi_affair->global_out_packet, DEV_PROTOCOL_LBS_FORM_VERSION, DEV_PROTOCOL_LBS_LOW_TYPE_VERSION_LICENSE,\
		                            DEV_PROTOCOL_LBS_HIGH_TYPE_VERSION);
		if (sdk_error != mkernel_internal_succ)
		{
			break;
		}
		sdk_error = authentication_i_serialize(authi_affair);
		if (sdk_error != mkernel_internal_succ)
		{
			break;
		}

        switch (sdk_kernel->dev_cur_auth_type)
        {
        case sdk_dev_auth_protocol_ecdh:
            sdk_error = ezdev_generate_publickey((bscomptls_ecdh_context*)ctx_client, pubkey, &pubkey_len);
            if (sdk_error != mkernel_internal_succ)
            {
                ezdev_sdk_kernel_log_debug(0, 0, "generate_public_key error!\n");
                break;
            }
            bscomptls_hexdump(pubkey, pubkey_len, 1, pubkeyhex);

            sdk_error = aes_128_encrypt_pubkey(authi_affair, pubkey, pubkey_len, encrypt_data, &encrypt_data_len, tag_data, tag_data_len);
            if (sdk_error != mkernel_internal_succ)
            {
                ezdev_sdk_kernel_log_debug(0, 0, "aes_128_encrypt_pubkey error!\n");
                break;
            }
            sdk_error = append_encrypt_data(&authi_affair->global_out_packet, encrypt_data, encrypt_data_len, tag_data, tag_data_len);
            if (sdk_error != mkernel_internal_succ)
            {
                ezdev_sdk_kernel_log_debug(0, 0, "append_encrypt_data error!\n");
                break;
            }
            break;
        default:
            return mkernel_internal_internal_err;
            break;
        }

        sdk_error = header_serialize(sdk_kernel, &authi_affair->global_out_packet, DEV_PROTOCOL_AUTHENTICATION_I, authi_affair->global_out_packet.payload_buf_off);
		if (sdk_error != mkernel_internal_succ)
		{
			break;
		}
		sdk_error = send_lbs_msg(sdk_kernel, authi_affair);

	} while (0);

	ezdev_sdk_kernel_log_debug(sdk_error, 0, "send_authentication_i complete");
	return sdk_error;
}

static mkernel_internal_error aes_128_decrypt_peer_pubkey(lbs_affair *authi_affair, EZDEV_SDK_UINT32 remain_len, unsigned char* out_buf, EZDEV_SDK_UINT32* out_len, \
                                                        unsigned char* intput_tag_buf, EZDEV_SDK_UINT32 tag_buf_len)
{
    mkernel_internal_error sdk_error = mkernel_internal_succ;
    EZDEV_SDK_UINT8 recv_plat_key_len = 0;
    unsigned char aes_encrypt_key[16];
    EZDEV_SDK_UINT32 peer_pubkey_len = 0;

    if (authi_affair == NULL || remain_len == 0 || out_buf == NULL || out_len == NULL || intput_tag_buf == NULL || tag_buf_len == 0)
    {
        return mkernel_internal_input_param_invalid;
    }

    recv_plat_key_len = remain_len - authi_affair->global_in_packet.payload_buf_off;
    ezdev_sdk_kernel_log_error(0, 0, "recv_plat_key_len: is :%d \n", recv_plat_key_len);

    memcpy(aes_encrypt_key, authi_affair->share_key, 16);

    sdk_error = aes_gcm_128_dec_padding(aes_encrypt_key, authi_affair->global_in_packet.payload_buf + authi_affair->global_in_packet.payload_buf_off, recv_plat_key_len, \
                                out_buf, &peer_pubkey_len, intput_tag_buf, tag_buf_len);
    if (sdk_error != mkernel_internal_succ)
    {
        ezdev_sdk_kernel_log_error(mkernel_internal_platform_appoint_error, 0, "aes_128_decrypt_peer_pubkey: aes_gcm_128_dec_padding error :%d\n", sdk_error);
        return sdk_error;
    }

    *out_len = peer_pubkey_len;

    return sdk_error;
}


/*****************************************************************/

/********************************************************************/
/****************************Authentication_II********************************/
/********************************************************************/
static mkernel_internal_error parse_authentication_ii(ezdev_sdk_kernel *sdk_kernel, lbs_affair *authi_affair, EZDEV_SDK_UINT32 remain_len, void* ctx_client)
{
	mkernel_internal_error sdk_error = mkernel_internal_succ;
	char result_code = 0;
	unsigned char peer_pubkey[ezdev_sdk_total_len]= {0};
    EZDEV_SDK_UINT32 peer_pubkey_len = 0;
    unsigned char input_tag_buf[tag_len] = { 0 };
    EZDEV_SDK_UINT32 tag_buf_len = tag_len;
	unsigned char masterkey[48]={0};
	char maskerkeykey[32]= {0};
	EZDEV_SDK_UINT32 masterkey_len  = 0;
	unsigned char md5_masterkey[16]={0};
	int nIndex = 0;
    unsigned char in_packet_flag = 0;
    unsigned char out_packet_flag = 0;

    unsigned char in_packet_auth_type = 0;
    unsigned char out_packet_auth_type = 0;
    EZDEV_SDK_UINT8 var_head_buf_off = 0;
    EZDEV_SDK_UINT8 dev_support_auth_type = 0;
	authi_affair->global_in_packet.payload_buf_off += 3;

	ezdev_sdk_kernel_log_debug(0, 0, "parse_authentication_ii remain_len:%d", remain_len);

    in_packet_flag = (authi_affair->global_in_packet.head_buf[0] & 0x08) >> 3;
    out_packet_flag = (authi_affair->global_out_packet.head_buf[0] & 0x08) >> 3;
    if (0x01 != out_packet_flag)
    {
        return mkernel_internal_internal_err;
    }

    if (0x01 == in_packet_flag && 0x01 == out_packet_flag)
    {
        in_packet_auth_type = authi_affair->global_in_packet.var_head_buf[0];
        out_packet_auth_type = authi_affair->global_out_packet.var_head_buf[0];
        if (in_packet_auth_type != out_packet_auth_type)
        {
            var_head_buf_off = 1;
            do 
            {
                var_head_buf_off++;
                dev_support_auth_type = authi_affair->global_out_packet.var_head_buf[var_head_buf_off];
                if (dev_support_auth_type == in_packet_auth_type)
                {
                    return mkernel_internal_platform_lbs_auth_type_need_rematch;
                }
            } while (var_head_buf_off < authi_affair->global_out_packet.var_head_buf_Len - 2);

            return mkernel_internal_platform_lbs_auth_type_match_fail;
        }
    }
    else
    {
        return mkernel_internal_platform_invalid_data;
    }

	memcpy(&result_code, authi_affair->global_in_packet.payload_buf + authi_affair->global_in_packet.payload_buf_off, 1);
	authi_affair->global_in_packet.payload_buf_off++;

	if (result_code != 0)
	{
		ezdev_sdk_kernel_log_debug(0, result_code, "parse_authentication_ii platform return error code:%d", result_code);
		return mkernel_internal_platform_error + result_code;
	}

    switch (sdk_kernel->dev_cur_auth_type)
    {
    case sdk_dev_auth_protocol_ecdh:
        memcpy(input_tag_buf, authi_affair->global_in_packet.payload_buf + authi_affair->global_in_packet.payload_buf_off, tag_buf_len);
        authi_affair->global_in_packet.payload_buf_off += tag_buf_len;

        sdk_error = aes_128_decrypt_peer_pubkey(authi_affair, remain_len, peer_pubkey, &peer_pubkey_len, input_tag_buf, tag_buf_len);
        if (sdk_error != mkernel_internal_succ)
        {
            ezdev_sdk_kernel_log_error(0, 0, "aes_128_decrypt_peer_pubkeyerror\n");
            return sdk_error;
        }
        memset(masterkey, 0, 48);
        sdk_error = ezdev_generate_masterkey((bscomptls_ecdh_context*)ctx_client, peer_pubkey, peer_pubkey_len, masterkey, &masterkey_len);
        if (sdk_error != mkernel_internal_succ)
        {
            ezdev_sdk_kernel_log_error(0, 0, "generate_master_key error\n");
            return sdk_error;
        }
        memset(md5_masterkey, 0, 16);
        bscomptls_md5(masterkey, masterkey_len, md5_masterkey);
        break;
    default:
        return mkernel_internal_internal_err;
        break;
    }

	memset(authi_affair->master_key, 0, ezdev_sdk_masterkey_len);

	for (; nIndex < 8; nIndex++)
	{
		sprintf(maskerkeykey + nIndex * 2, "%02X", md5_masterkey[nIndex]);

		memcpy(authi_affair->master_key, maskerkeykey, ezdev_sdk_masterkey_len);
	}
	return sdk_error;
}

static mkernel_internal_error wait_authentication_ii(ezdev_sdk_kernel *sdk_kernel, lbs_affair *authi_affair, void* ctx_client)
{
	EZDEV_SDK_UINT32 rev_cmd = 0;
	EZDEV_SDK_UINT32 remain_len = 0;
	mkernel_internal_error sdk_error = mkernel_internal_succ;
	sdk_error = wait_assign_response(sdk_kernel, authi_affair, &rev_cmd, &remain_len);
	if (sdk_error != mkernel_internal_succ)
	{
		return sdk_error;
	}

	if (rev_cmd != DEV_PROTOCOL_AUTHENTICATION_II)
	{
		return mkernel_internal_net_read_error_request;
	}
    sdk_error = parse_authentication_ii(sdk_kernel, authi_affair, remain_len, ctx_client);
    if (mkernel_internal_platform_lbs_auth_type_need_rematch == sdk_error)
    {
        sdk_kernel->dev_cur_auth_type = authi_affair->global_in_packet.var_head_buf[0];
    }

	return sdk_error;
}

static mkernel_internal_error authentication_iii_serialize_serial_sha384(lbs_affair *authi_affair)
{
	mkernel_internal_error sdk_error = mkernel_internal_succ;
	unsigned char sign_input[ezdev_sdk_devserial_maxlen];
	memset(sign_input, 0, ezdev_sdk_devserial_maxlen);
	memcpy(sign_input, authi_affair->dev_subserial, strlen(authi_affair->dev_subserial));
	sdk_error = digital_sign_serialize_sha384(&authi_affair->global_out_packet, sign_input, strlen(authi_affair->dev_subserial), authi_affair->master_key, ezdev_sdk_masterkey_len);
	return sdk_error;
}

static mkernel_internal_error send_sessionkey_req_serialize(lbs_affair *authi_affair)
{
	mkernel_internal_error sdk_error = mkernel_internal_succ;

	*(EZDEV_SDK_UINT8 *)(authi_affair->global_out_packet.payload_buf + authi_affair->global_out_packet.payload_buf_off) = ezdev_sdk_devid_len;
	authi_affair->global_out_packet.payload_buf_off++;

	memcpy(authi_affair->global_out_packet.payload_buf + authi_affair->global_out_packet.payload_buf_off, authi_affair->dev_id, ezdev_sdk_devid_len);
	authi_affair->global_out_packet.payload_buf_off += ezdev_sdk_devid_len;

    sdk_error = authentication_iii_serialize_serial_sha384(authi_affair);

	return sdk_error;
}


static mkernel_internal_error send_update_sessionkey_req(ezdev_sdk_kernel *sdk_kernel, lbs_affair *authi_affair)
{
	mkernel_internal_error sdk_error = mkernel_internal_succ;

	sdk_error = common_serialize(&authi_affair->global_out_packet, DEV_PROTOCOL_LBS_FORM_VERSION, DEV_PROTOCOL_LBS_LOW_TYPE_VERSION, DEV_PROTOCOL_LBS_HIGH_TYPE_VERSION);
	if (sdk_error != mkernel_internal_succ)
	{
		return sdk_error;
	}

	sdk_error = send_sessionkey_req_serialize(authi_affair);

	if (sdk_error != mkernel_internal_succ)
	{
		return sdk_error;
	}
    sdk_error = header_serialize(sdk_kernel, &authi_affair->global_out_packet, DEV_PROTOCOL_REFRESHSESSIONKEY_REQ, authi_affair->global_out_packet.payload_buf_off);
	if (sdk_error != mkernel_internal_succ)
	{
		return sdk_error;
	}

	sdk_error = send_lbs_msg(sdk_kernel, authi_affair);
	ezdev_sdk_kernel_log_debug(sdk_error, 0, "send_authentication_iii complete");
	return mkernel_internal_succ;
}


static mkernel_internal_error wait_update_sessionkey_rsp(ezdev_sdk_kernel *sdk_kernel, lbs_affair *authi_affair)
{
	EZDEV_SDK_UINT32 rev_cmd = 0;
	EZDEV_SDK_UINT32 remain_len = 0;
	mkernel_internal_error sdk_error = mkernel_internal_succ;
	sdk_error = wait_assign_response(sdk_kernel, authi_affair, &rev_cmd, &remain_len);
	if (sdk_error != mkernel_internal_succ)
	{
		return sdk_error;
	}

	if (rev_cmd != DEV_PROTOCOL_REFRESHSESSIONKEY_RSP)
	{
		return mkernel_internal_net_read_error_request;
	}
	sdk_error = parse_authentication_create_dev_id(authi_affair, remain_len);

	return sdk_error;
}

static mkernel_internal_error send_authentication_creat_dev_id(ezdev_sdk_kernel *sdk_kernel, lbs_affair *authi_affair)
{
	mkernel_internal_error sdk_error = mkernel_internal_succ;

	sdk_error = common_serialize(&authi_affair->global_out_packet, DEV_PROTOCOL_LBS_FORM_VERSION, DEV_PROTOCOL_LBS_LOW_TYPE_VERSION, DEV_PROTOCOL_LBS_HIGH_TYPE_VERSION);
	if (sdk_error != mkernel_internal_succ)
	{
		return sdk_error;
	}

	sdk_error = authentication_iii_serialize_serial_sha384(authi_affair);
	if (sdk_error != mkernel_internal_succ)
	{
		return sdk_error;
	}
    sdk_error = header_serialize(sdk_kernel, &authi_affair->global_out_packet, DEV_PROTOCOL_REQUEST_DEVID, authi_affair->global_out_packet.payload_buf_off);
	if (sdk_error != mkernel_internal_succ)
	{
		return sdk_error;
	}

	sdk_error = send_lbs_msg(sdk_kernel, authi_affair);
	ezdev_sdk_kernel_log_debug(sdk_error, 0, "send_authentication_iii complete");
	return mkernel_internal_succ;
}

static mkernel_internal_error parse_authentication_create_dev_id(lbs_affair *authi_affair, EZDEV_SDK_UINT32 remain_len)
{
    mkernel_internal_error sdk_error = mkernel_internal_succ;
    char result_code = 0;
    EZDEV_SDK_UINT8 en_sessionkey_len = 0;
    unsigned char sessionkey[32] = { 0 };
    EZDEV_SDK_UINT32 sessionkey_len = 0;

    EZDEV_SDK_UINT8 en_dev_id_len = 0;
    unsigned char dev_id[48] = { 0 };
    EZDEV_SDK_UINT32 dev_id_len = 0;
    unsigned char sign_input[ezdev_sdk_total_len];

    unsigned char devid_tag_buf[tag_len] = { 0 };
    EZDEV_SDK_UINT32 devid_tag_buf_len = tag_len;
    unsigned char sessionkey_tag_buf[tag_len] = { 0 };
    EZDEV_SDK_UINT32 sessionkey_tag_buf_len = tag_len;

    authi_affair->global_in_packet.payload_buf_off += 3;
    memcpy(&result_code, authi_affair->global_in_packet.payload_buf + authi_affair->global_in_packet.payload_buf_off, 1);
    authi_affair->global_in_packet.payload_buf_off++;

    if (result_code != 0)
    {
        ezdev_sdk_kernel_log_debug(mkernel_internal_platform_error, result_code, "parse_authentication_iv_create_devid platform return err code:%d\n", result_code);
        return mkernel_internal_platform_error + result_code;
    }
    memcpy(devid_tag_buf, authi_affair->global_in_packet.payload_buf + authi_affair->global_in_packet.payload_buf_off, devid_tag_buf_len);
    authi_affair->global_in_packet.payload_buf_off += devid_tag_buf_len;
    en_dev_id_len = *(authi_affair->global_in_packet.payload_buf + authi_affair->global_in_packet.payload_buf_off);
    authi_affair->global_in_packet.payload_buf_off++;
    if (en_dev_id_len != 32)
    {
        ezdev_sdk_kernel_log_debug(0, 0, "parse_authentication_create_dev_id en_dev_id_len is not 32\n");
        return mkernel_internal_platform_appoint_error;
    }
    ezdev_sdk_kernel_log_debug(0, 0, "get en_dev_id_len :%d\n", en_dev_id_len);
    sdk_error = aes_gcm_128_dec_padding(authi_affair->master_key, \
                                authi_affair->global_in_packet.payload_buf + authi_affair->global_in_packet.payload_buf_off, en_dev_id_len, \
                                dev_id, &dev_id_len, devid_tag_buf, devid_tag_buf_len);
    if (sdk_error != mkernel_internal_succ)
    {
        ezdev_sdk_kernel_log_debug(sdk_error, sdk_error, "aes_gcm_128_dec_padding err!\n");
        return sdk_error;
    }
    if (dev_id_len != 32)
    {
        ezdev_sdk_kernel_log_debug(0, 0, "parse_authentication_iv sessionkey_len is not 32\n");
        return mkernel_internal_platform_appoint_error;
    }
    memcpy(authi_affair->dev_id, dev_id, dev_id_len);
    authi_affair->global_in_packet.payload_buf_off += en_dev_id_len;
    memcpy(sessionkey_tag_buf, authi_affair->global_in_packet.payload_buf + authi_affair->global_in_packet.payload_buf_off, sessionkey_tag_buf_len);
    authi_affair->global_in_packet.payload_buf_off += sessionkey_tag_buf_len;
    en_sessionkey_len = *(authi_affair->global_in_packet.payload_buf + authi_affair->global_in_packet.payload_buf_off);
    authi_affair->global_in_packet.payload_buf_off++;
    if (en_sessionkey_len != 16)
    {
        ezdev_sdk_kernel_log_debug(0, 0, "parse_authentication_createdevid_iv en_sessionkey_len is not 32\n");
        return mkernel_internal_platform_appoint_error;
    }
    ezdev_sdk_kernel_log_debug(0, 0, "get en_sessionkey_len :%d\n", en_sessionkey_len);
    sdk_error = aes_gcm_128_dec_padding(authi_affair->master_key, \
                                authi_affair->global_in_packet.payload_buf + authi_affair->global_in_packet.payload_buf_off, en_sessionkey_len, \
                                sessionkey, &sessionkey_len, sessionkey_tag_buf, sessionkey_tag_buf_len);
    if (sdk_error != mkernel_internal_succ)
    {
        ezdev_sdk_kernel_log_debug(sdk_error, sdk_error, "aes_gcm_128_dec_padding err!\n");
        return sdk_error;
    }
    if (sessionkey_len != 16)
    {
        ezdev_sdk_kernel_log_debug(0, 0, "parse_authentication_iv sessionkey_len is not 16\n");
        return mkernel_internal_platform_appoint_error;
    }
    memcpy(authi_affair->session_key, sessionkey, sessionkey_len);
    authi_affair->global_in_packet.payload_buf_off += en_sessionkey_len;
    memset(sign_input, 0, ezdev_sdk_total_len);
    memcpy(sign_input, authi_affair->dev_subserial, strlen(authi_affair->dev_subserial));
    sdk_error = digital_sign_serialize_and_check_sha384(authi_affair->global_in_packet.payload_buf + authi_affair->global_in_packet.payload_buf_off, \
        remain_len - authi_affair->global_in_packet.payload_buf_off, sign_input, strlen(authi_affair->dev_subserial), authi_affair->master_key, 16);
    if (sdk_error != mkernel_internal_succ)
    {
        ezdev_sdk_kernel_log_debug(sdk_error, 0, "digital_sign_serialize_and_check_sha384 err!\n");
    }

    return sdk_error;
}



static mkernel_internal_error wait_authentication_creat_dev_id(ezdev_sdk_kernel *sdk_kernel, lbs_affair *authi_affair)
{
	EZDEV_SDK_UINT32 rev_cmd = 0;
	EZDEV_SDK_UINT32 remain_len = 0;
	mkernel_internal_error sdk_error = mkernel_internal_succ;
	sdk_error = wait_assign_response(sdk_kernel, authi_affair, &rev_cmd, &remain_len);
	if (sdk_error != mkernel_internal_succ)
	{
		return sdk_error;
	}

	if (rev_cmd != DEV_PROTOCOL_RESPONSE_DEVID)
	{
		return mkernel_internal_net_read_error_request;
	}

	sdk_error = parse_authentication_create_dev_id(authi_affair, remain_len);

	return sdk_error;
}

static mkernel_internal_error refreshsessionkey_i_serialize(lbs_affair *auth_affair)
{
	mkernel_internal_error sdk_error = mkernel_internal_succ;
	EZDEV_SDK_UINT8 dev_serial_len = 0;
	unsigned char en_src[16];
	unsigned char en_dst[16];
	EZDEV_SDK_UINT32 en_dst_len = 0;
	dev_serial_len = strlen(auth_affair->dev_subserial);

	*(EZDEV_SDK_UINT8 *)(auth_affair->global_out_packet.payload_buf + auth_affair->global_out_packet.payload_buf_off) = dev_serial_len;
	auth_affair->global_out_packet.payload_buf_off++;

	memcpy(auth_affair->global_out_packet.payload_buf + auth_affair->global_out_packet.payload_buf_off, auth_affair->dev_subserial, dev_serial_len);
	auth_affair->global_out_packet.payload_buf_off += dev_serial_len;

	*(EZDEV_SDK_UINT8 *)(auth_affair->global_out_packet.payload_buf + auth_affair->global_out_packet.payload_buf_off) = ezdev_sdk_devid_len;
	auth_affair->global_out_packet.payload_buf_off++;

	memcpy(auth_affair->global_out_packet.payload_buf + auth_affair->global_out_packet.payload_buf_off, auth_affair->dev_id, ezdev_sdk_devid_len);
	auth_affair->global_out_packet.payload_buf_off += ezdev_sdk_devid_len;

	memset(en_src, 0, 16);
	memset(en_dst, 0, 16);
	memset(en_src, auth_affair->random_1, 1);

	sdk_error = aes_cbc_128_enc_padding(auth_affair->master_key, (unsigned char*)en_src, 1, 16, en_dst, &en_dst_len);
	if (sdk_error != mkernel_internal_succ)
	{
		return sdk_error;
	}

	memcpy(auth_affair->global_out_packet.payload_buf + auth_affair->global_out_packet.payload_buf_off, en_dst, en_dst_len);
	auth_affair->global_out_packet.payload_buf_off += en_dst_len;

	return mkernel_internal_succ;
}

static mkernel_internal_error send_refreshsessionkey_i(ezdev_sdk_kernel *sdk_kernel, lbs_affair *authi_affair)
{
	mkernel_internal_error sdk_error = mkernel_internal_succ;

	sdk_error = common_serialize(&authi_affair->global_out_packet, DEV_PROTOCOL_LBS_FORM_VERSION, DEV_PROTOCOL_LBS_LOW_TYPE_VERSION, DEV_PROTOCOL_LBS_HIGH_TYPE_VERSION);
	if (sdk_error != mkernel_internal_succ)
	{
		return sdk_error;
	}

	sdk_error = refreshsessionkey_i_serialize(authi_affair);
	if (sdk_error != mkernel_internal_succ)
	{
		return sdk_error;
	}

	sdk_error = header_serialize_old(&authi_affair->global_out_packet, DEV_PROTOCOL_REQUEST_DEVID, authi_affair->global_out_packet.payload_buf_off);
	if (sdk_error != mkernel_internal_succ)
	{
		return sdk_error;
	}

	sdk_error = send_lbs_msg(sdk_kernel, authi_affair);
	ezdev_sdk_kernel_log_debug(sdk_error, 0, "send_refreshsessionkey_i complete");
	return mkernel_internal_succ;
}

static mkernel_internal_error send_stun_refreshsessionkey_i(ezdev_sdk_kernel *sdk_kernel, lbs_affair *authi_affair)
{
	mkernel_internal_error sdk_error = mkernel_internal_succ;

	sdk_error = common_serialize(&authi_affair->global_out_packet, DEV_PROTOCOL_LBS_FORM_VERSION, DEV_PROTOCOL_LBS_LOW_TYPE_VERSION, DEV_PROTOCOL_LBS_HIGH_TYPE_VERSION);
	if (sdk_error != mkernel_internal_succ)
	{
		return sdk_error;
	}

	sdk_error = refreshsessionkey_i_serialize(authi_affair);
	if (sdk_error != mkernel_internal_succ)
	{
		return sdk_error;
	}
	sdk_error = header_serialize_old(&authi_affair->global_out_packet, DEV_PROTOCOL_STUN_REFRESHSESSIONKEY_I, authi_affair->global_out_packet.payload_buf_off);
	if (sdk_error != mkernel_internal_succ)
	{
		return sdk_error;
	}

	sdk_error = send_lbs_msg(sdk_kernel, authi_affair);
	ezdev_sdk_kernel_log_debug(sdk_error, 0, "send_stun_refreshsessionkey_i complete");
	return mkernel_internal_succ;
}

static mkernel_internal_error parse_refreshsessionkey_ii(lbs_affair *authi_affair, EZDEV_SDK_UINT32 remain_len)
{
    mkernel_internal_error sdk_error = mkernel_internal_succ;
    char result_code = 0;
    EZDEV_SDK_UINT8 en_sessionkey_len = 0;
    EZDEV_SDK_UINT8 return_random_1 = 0;
    unsigned char ase_dst[32];
    EZDEV_SDK_UINT32 ase_dst_len = 0;
    authi_affair->global_in_packet.payload_buf_off += 3;
    memcpy(&result_code, authi_affair->global_in_packet.payload_buf + authi_affair->global_in_packet.payload_buf_off, 1);
    authi_affair->global_in_packet.payload_buf_off++;

    if (result_code != 0)
    {
        ezdev_sdk_kernel_log_debug(mkernel_internal_platform_error, result_code, "parse_refreshsessionkey_ii platform return error code:%d", result_code);
        return mkernel_internal_platform_error + result_code;
    }
    en_sessionkey_len = remain_len - authi_affair->global_in_packet.payload_buf_off;
    if (en_sessionkey_len != 32)
    {
        ezdev_sdk_kernel_log_debug(mkernel_internal_platform_appoint_error, 0, "parse_refreshsessionkey_ii en_sessionkey_len is:%d not 32");
        return mkernel_internal_platform_appoint_error;
    }

	sdk_error = aes_cbc_128_dec_padding(authi_affair->master_key, authi_affair->global_in_packet.payload_buf + authi_affair->global_in_packet.payload_buf_off, en_sessionkey_len, ase_dst, &ase_dst_len);
	if (sdk_error != mkernel_internal_succ)
	{
		ezdev_sdk_kernel_log_debug(mkernel_internal_platform_appoint_error, 0, "parse_refreshsessionkey_ii aes_cbc_128_dec error :%d", sdk_error);
		return sdk_error;
	}

    authi_affair->global_in_packet.payload_buf_off += en_sessionkey_len;

    return_random_1 = ase_dst[0];
    if (return_random_1 != authi_affair->random_1)
    {
        ezdev_sdk_kernel_log_debug(mkernel_internal_platform_appoint_error, 0, "parse_refreshsessionkey_ii random_1 error, my:%d, get:%d", authi_affair->random_1, return_random_1);
        return mkernel_internal_random1_check_error;
    }

    authi_affair->random_2 = ase_dst[1];
    memcpy(authi_affair->session_key, ase_dst + 2, ezdev_sdk_sessionkey_len);

    return mkernel_internal_succ;
}


static mkernel_internal_error wait_refreshsessionkey_ii(ezdev_sdk_kernel *sdk_kernel, lbs_affair *authi_affair)
{
	EZDEV_SDK_UINT32 rev_cmd = 0;
	EZDEV_SDK_UINT32 remain_len = 0;
	mkernel_internal_error sdk_error = mkernel_internal_succ;
	sdk_error = wait_assign_response(sdk_kernel, authi_affair, &rev_cmd, &remain_len);
	if (sdk_error != mkernel_internal_succ)
	{
		return sdk_error;
	}

	if (rev_cmd != DEV_PROTOCOL_RESPONSE_DEVID)
	{
		return mkernel_internal_net_read_error_request;
	}
	sdk_error = parse_refreshsessionkey_ii(authi_affair, remain_len);
	ezdev_sdk_kernel_log_debug(sdk_error, sdk_error, "parse_refreshsessionkey_ii end\n");

	return sdk_error;
}

static mkernel_internal_error wait_stun_refreshsessionkey_ii(ezdev_sdk_kernel *sdk_kernel, lbs_affair *authi_affair)
{
	EZDEV_SDK_UINT32 rev_cmd = 0;
	EZDEV_SDK_UINT32 remain_len = 0;
	mkernel_internal_error sdk_error = mkernel_internal_succ;
	sdk_error = wait_assign_response(sdk_kernel, authi_affair, &rev_cmd, &remain_len);
	if (sdk_error != mkernel_internal_succ)
	{
		return sdk_error;
	}

	if (rev_cmd != DEV_PROTOCOL_STUN_REFRESHSESSIONKEY_II)
	{
		return mkernel_internal_net_read_error_request;
	}

	sdk_error = parse_refreshsessionkey_ii(authi_affair, remain_len);

	return sdk_error;
}
mkernel_internal_error refreshsessionkey_iii_serialize(lbs_affair *auth_affair)
{
	mkernel_internal_error sdk_error = mkernel_internal_succ;
	unsigned char en_src[16];
	unsigned char en_dst[16];
	EZDEV_SDK_UINT32 en_dst_len = 0;
	
	memset(en_src, 0, 16);
	memset(en_dst, 0, 16);
	memset(en_src, auth_affair->random_2, 1);

    sdk_error = aes_cbc_128_enc_padding(auth_affair->master_key, (unsigned char *)en_src, 1, 16, en_dst, &en_dst_len);
	if (sdk_error != mkernel_internal_succ)
	{
		return sdk_error;
	}

	memcpy(auth_affair->global_out_packet.payload_buf + auth_affair->global_out_packet.payload_buf_off, en_dst, en_dst_len);
	auth_affair->global_out_packet.payload_buf_off += en_dst_len;

	return mkernel_internal_succ;
}

mkernel_internal_error send_refreshsessionkey_iii(ezdev_sdk_kernel *sdk_kernel, lbs_affair *authi_affair)
{
	//payload
	mkernel_internal_error sdk_error = mkernel_internal_succ;

	sdk_error = common_serialize(&authi_affair->global_out_packet, DEV_PROTOCOL_LBS_FORM_VERSION, DEV_PROTOCOL_LBS_LOW_TYPE_VERSION, DEV_PROTOCOL_LBS_HIGH_TYPE_VERSION);
	if (sdk_error != mkernel_internal_succ)
	{
		return sdk_error;
	}

	sdk_error = refreshsessionkey_iii_serialize(authi_affair);
	if (sdk_error != mkernel_internal_succ)
	{
		return sdk_error;
	}

	sdk_error = header_serialize_old(&authi_affair->global_out_packet, DEV_PROTOCOL_APPLY_DEVID_CFM, authi_affair->global_out_packet.payload_buf_off);
	if (sdk_error != mkernel_internal_succ)
	{
		return sdk_error;
	}

	sdk_error = send_lbs_msg(sdk_kernel, authi_affair);
	ezdev_sdk_kernel_log_debug(sdk_error, 0, "send_refreshsessionkey_iii complete");
	return mkernel_internal_succ;
}

mkernel_internal_error send_stun_refreshsessionkey_iii(ezdev_sdk_kernel *sdk_kernel, lbs_affair *authi_affair)
{
	//payload
	mkernel_internal_error sdk_error = mkernel_internal_succ;

	sdk_error = common_serialize(&authi_affair->global_out_packet, DEV_PROTOCOL_LBS_FORM_VERSION, DEV_PROTOCOL_LBS_LOW_TYPE_VERSION, DEV_PROTOCOL_LBS_HIGH_TYPE_VERSION);
	if (sdk_error != mkernel_internal_succ)
	{
		return sdk_error;
	}

	sdk_error = refreshsessionkey_iii_serialize(authi_affair);
	if (sdk_error != mkernel_internal_succ)
	{
		return sdk_error;
	}

	//header
	sdk_error = header_serialize_old(&authi_affair->global_out_packet, DEV_PROTOCOL_STUN_REFRESHSESSIONKEY_III, authi_affair->global_out_packet.payload_buf_off);
	if (sdk_error != mkernel_internal_succ)
	{
		return sdk_error;
	}

	sdk_error = send_lbs_msg(sdk_kernel, authi_affair);
	ezdev_sdk_kernel_log_debug(sdk_error, 0, "send_stun_refreshsessionkey_iii complete");
	return mkernel_internal_succ;
}
static mkernel_internal_error crypto_data_req_das_serialize(lbs_affair *auth_affair)
{
	mkernel_internal_error sdk_error = mkernel_internal_succ;
	bscJSON *pJsonRoot = NULL;

	char *json_buf = NULL;
	EZDEV_SDK_UINT32 json_len = 0;
	char *json_buf_padding = NULL;
	EZDEV_SDK_UINT32 json_len_padding = 0;
	unsigned char *out_buf = NULL;
	EZDEV_SDK_UINT32 out_buf_len = 0;

	do
	{
		pJsonRoot = bscJSON_CreateObject();
		if (pJsonRoot == NULL)
		{
			sdk_error = mkernel_internal_malloc_error;
			break;
		}

		bscJSON_AddStringToObject(pJsonRoot, "DevSerial", auth_affair->dev_subserial);
		bscJSON_AddStringToObject(pJsonRoot, "Type", "DAS");
		bscJSON_AddNumberToObject(pJsonRoot, "Mode", auth_affair->dev_access_mode);

		json_buf = bscJSON_PrintBuffered(pJsonRoot, ezdev_sdk_json_default_size, 0);
		if (json_buf == NULL)
		{
			sdk_error = mkernel_internal_json_format_error;
			break;
		}
		json_len = strlen(json_buf);
		json_len_padding = calculate_padding_len(json_len);
		if (ezdev_sdk_json_default_size > json_len_padding)
		{
			json_buf_padding = json_buf;
			memset(json_buf + json_len, 0, ezdev_sdk_json_default_size - json_len);
		}
		else
		{
			json_buf_padding = malloc(json_len_padding);
			if (NULL == json_buf_padding)
			{
				sdk_error = mkernel_internal_malloc_error;
				break;
			}
			memset(json_buf_padding, 0, json_len_padding);
			memcpy(json_buf_padding, json_buf, json_len);

			free(json_buf);
			json_buf = NULL;
		}

		out_buf = malloc(json_len_padding);
		if (NULL == out_buf)
		{
			sdk_error = mkernel_internal_malloc_error;
			break;
		}
		out_buf_len = json_len_padding;

		sdk_error = aes_cbc_128_enc_padding(auth_affair->session_key, (unsigned char *)json_buf_padding, json_len, json_len_padding, out_buf, &out_buf_len);
		if (sdk_error != mkernel_internal_succ)
		{
			break;
		}

		memcpy(auth_affair->global_out_packet.payload_buf + auth_affair->global_out_packet.payload_buf_off, out_buf, out_buf_len);
		auth_affair->global_out_packet.payload_buf_off += out_buf_len;

	} while (0);

	if (pJsonRoot != NULL)
	{
		bscJSON_Delete(pJsonRoot);
		pJsonRoot = NULL;
	}
	if (json_buf_padding != NULL)
	{
		free(json_buf_padding);
		json_buf_padding = NULL;
	}
	if (out_buf != NULL)
	{
		free(out_buf);
		out_buf = NULL;
	}

	return sdk_error;
}

static mkernel_internal_error crypto_data_req_stun_serialize(lbs_affair *auth_affair)
{
	mkernel_internal_error sdk_error = mkernel_internal_succ;

	bscJSON *pJsonRoot = NULL;

	char *json_buf = NULL;
	EZDEV_SDK_UINT32 json_len = 0;
	char *json_buf_padding = NULL;
	EZDEV_SDK_UINT32 json_len_padding = 0;

	unsigned char *en_dst = NULL;
	EZDEV_SDK_UINT32 en_dst_len = 0;

	do
	{
		pJsonRoot = bscJSON_CreateObject();
		if (pJsonRoot == NULL)
		{
			sdk_error = mkernel_internal_malloc_error;
			break;
		}

		bscJSON_AddStringToObject(pJsonRoot, "DevSerial", auth_affair->dev_subserial);
		bscJSON_AddStringToObject(pJsonRoot, "Type", "STUN");

		json_buf = bscJSON_PrintBuffered(pJsonRoot, ezdev_sdk_json_default_size, 0);
		if (json_buf == NULL)
		{
			sdk_error = mkernel_internal_json_format_error;
			break;
		}

		json_len = strlen(json_buf);
		json_len_padding = calculate_padding_len(json_len);
		if (ezdev_sdk_json_default_size > json_len_padding)
		{
			json_buf_padding = json_buf;
			memset(json_buf + json_len, 0, ezdev_sdk_json_default_size - json_len);
		}
		else
		{
			json_buf_padding = malloc(json_len_padding);
			if (NULL == json_buf_padding)
			{
				sdk_error = mkernel_internal_malloc_error;
				break;
			}
			memset(json_buf_padding, 0, json_len_padding);
			memcpy(json_buf_padding, json_buf, json_len);

			free(json_buf);
			json_buf = NULL;
		}

		en_dst = malloc(json_len_padding);
		if (NULL == en_dst)
		{
			sdk_error = mkernel_internal_malloc_error;
			break;
		}
		memset(en_dst, 0, json_len_padding);
		en_dst_len = json_len_padding;

		sdk_error = aes_cbc_128_enc_padding(auth_affair->session_key, (unsigned char *)json_buf_padding, json_len, json_len_padding, en_dst, &en_dst_len);
		if (sdk_error != mkernel_internal_succ)
		{
			break;
		}

		memcpy(auth_affair->global_out_packet.payload_buf + auth_affair->global_out_packet.payload_buf_off, en_dst, en_dst_len);
		auth_affair->global_out_packet.payload_buf_off += en_dst_len;
	} while (0);

	if (pJsonRoot != NULL)
	{
		bscJSON_Delete(pJsonRoot);
		pJsonRoot = NULL;
	}
	if (json_buf_padding != NULL)
	{
		free(json_buf_padding);
		json_buf_padding = NULL;
	}
	if (en_dst != NULL)
	{
		free(en_dst);
		en_dst = NULL;
	}
	return sdk_error;
}

static mkernel_internal_error send_crypto_data_req(ezdev_sdk_kernel *sdk_kernel, lbs_affair *authi_affair, EZDEV_SDK_UINT8 server_type)
{
	//payload
	mkernel_internal_error sdk_error = mkernel_internal_succ;

	sdk_error = common_serialize(&authi_affair->global_out_packet, DEV_PROTOCOL_LBS_FORM_VERSION, DEV_PROTOCOL_LBS_LOW_TYPE_VERSION, DEV_PROTOCOL_LBS_HIGH_TYPE_VERSION);
	if (sdk_error != mkernel_internal_succ)
	{
		return sdk_error;
	}
	if (server_type == 0)
	{
		sdk_error = crypto_data_req_stun_serialize(authi_affair);
	}
	else
	{
		sdk_error = crypto_data_req_das_serialize(authi_affair);
	}

	if (sdk_error != mkernel_internal_succ)
	{
		return sdk_error;
	}


	sdk_error = header_serialize_old(&authi_affair->global_out_packet, DEV_PROTOCOL_CRYPTO_DATA_REQ, authi_affair->global_out_packet.payload_buf_off);
	if (sdk_error != mkernel_internal_succ)
	{
		return sdk_error;
	}

	sdk_error = send_lbs_msg(sdk_kernel, authi_affair);
	ezdev_sdk_kernel_log_debug(sdk_error, 0, "send_crypto_data_req complete");
	return mkernel_internal_succ;
}

static mkernel_internal_error parse_crypto_data_rsp_das(lbs_affair *authi_affair, EZDEV_SDK_UINT32 remain_len, das_info *rev_das_info)
{
	mkernel_internal_error sdk_error = mkernel_internal_succ;

	char result_code = 0;
	EZDEV_SDK_UINT32 en_src_len = 0;
	unsigned char *de_dst = NULL;
	EZDEV_SDK_UINT32 de_dst_len = 0;

	authi_affair->global_in_packet.payload_buf_off += 3;
	memcpy(&result_code, authi_affair->global_in_packet.payload_buf + authi_affair->global_in_packet.payload_buf_off, 1);
	authi_affair->global_in_packet.payload_buf_off++;

	if (result_code != 0)
	{
		ezdev_sdk_kernel_log_debug(mkernel_internal_platform_error, result_code, "parse_crypto_data_rsp_das platform return error code:%d", result_code);
		return mkernel_internal_platform_error + result_code;
	}

	en_src_len = remain_len - authi_affair->global_in_packet.payload_buf_off;

	de_dst = (unsigned char *)malloc(en_src_len);
	if (de_dst == NULL)
	{
		return mkernel_internal_malloc_error;
	}
	memset(de_dst, 0, en_src_len);

	do
	{
		sdk_error = aes_cbc_128_dec_padding(authi_affair->session_key, authi_affair->global_in_packet.payload_buf + authi_affair->global_in_packet.payload_buf_off,
											en_src_len, de_dst, &de_dst_len);
		if (sdk_error != mkernel_internal_succ)
		{
			break;
		}

		sdk_error = json_parse_das_server_info((char *)de_dst, rev_das_info);
		if (sdk_error != mkernel_internal_succ)
		{
			break;
		}
	} while (0);

	if (NULL != de_dst)
	{
		free(de_dst);
		de_dst = NULL;
	}

	return sdk_error;
}

static mkernel_internal_error parse_crypto_data_rsp_stun(lbs_affair *authi_affair, EZDEV_SDK_UINT32 remain_len, stun_info *rev_stun_info)
{
	mkernel_internal_error sdk_error = mkernel_internal_succ;

	char result_code = 0;
	EZDEV_SDK_UINT32 en_src_len = 0;
	unsigned char *de_dst = NULL;
	EZDEV_SDK_UINT32 de_dst_len = 0;

	authi_affair->global_in_packet.payload_buf_off += 3;
	memcpy(&result_code, authi_affair->global_in_packet.payload_buf + authi_affair->global_in_packet.payload_buf_off, 1);
	authi_affair->global_in_packet.payload_buf_off++;

	if (result_code != 0)
	{
		ezdev_sdk_kernel_log_debug(mkernel_internal_platform_error, result_code, "parse_crypto_data_rsp_stun platform return error code:%d", result_code);
		return mkernel_internal_platform_error + result_code;
	}

	en_src_len = remain_len - authi_affair->global_in_packet.payload_buf_off;

	de_dst = (unsigned char *)malloc(en_src_len);
	if (de_dst == NULL)
	{
		return mkernel_internal_malloc_error;
	}

	memset(de_dst, 0, en_src_len);

	do
	{
		sdk_error = aes_cbc_128_dec_padding(authi_affair->session_key, authi_affair->global_in_packet.payload_buf + authi_affair->global_in_packet.payload_buf_off,
											en_src_len, de_dst, &de_dst_len);
		if (sdk_error != mkernel_internal_succ)
		{
			break;
		}

		sdk_error = json_parse_stun_server_info((char *)de_dst, rev_stun_info);
		if (sdk_error != mkernel_internal_succ)
		{
			break;
		}
	} while (0);

	if (NULL != de_dst)
	{
		free(de_dst);
		de_dst = NULL;
	}

	return sdk_error;
}

static mkernel_internal_error wait_crypto_data_rsp_das(ezdev_sdk_kernel *sdk_kernel, lbs_affair *authi_affair, das_info *rev_das_info)
{
	EZDEV_SDK_UINT32 rev_cmd = 0;
	EZDEV_SDK_UINT32 remain_len = 0;
	mkernel_internal_error sdk_error = mkernel_internal_succ;
	sdk_error = wait_assign_response(sdk_kernel, authi_affair, &rev_cmd, &remain_len);
	if (sdk_error != mkernel_internal_succ)
	{
		return sdk_error;
	}

	if (rev_cmd != DEV_PROTOCOL_CRYPTO_DATA_RSP)
	{
		return mkernel_internal_net_read_error_request;
	}

	sdk_error = parse_crypto_data_rsp_das(authi_affair, remain_len, rev_das_info);

	return sdk_error;
}

static mkernel_internal_error wait_crypto_data_rsp_stun(ezdev_sdk_kernel *sdk_kernel, lbs_affair *authi_affair, stun_info *rev_stun_info)
{
	EZDEV_SDK_UINT32 rev_cmd = 0;
	EZDEV_SDK_UINT32 remain_len = 0;
	mkernel_internal_error sdk_error = mkernel_internal_succ;
	sdk_error = wait_assign_response(sdk_kernel, authi_affair, &rev_cmd, &remain_len);
	if (sdk_error != mkernel_internal_succ)
	{
		return sdk_error;
	}

	if (rev_cmd != DEV_PROTOCOL_CRYPTO_DATA_RSP)
	{
		return mkernel_internal_net_read_error_request;
	}

	sdk_error = parse_crypto_data_rsp_stun(authi_affair, remain_len, rev_stun_info);

	return sdk_error;
}
/*********************************************************************************/

static mkernel_internal_error lbs_connect(ezdev_sdk_kernel *sdk_kernel, lbs_affair *authi_affair)
{

	mkernel_internal_error result_ = mkernel_internal_succ;
	char szRealIp[ezdev_sdk_ip_max_len] = {0};
	do
	{
		authi_affair->lbs_net_work = sdk_kernel->platform_handle.net_work_create(g_binding_nic);
		if (authi_affair->lbs_net_work == NULL)
		{
			ezdev_sdk_kernel_log_error(result_, 0, "lbs_connect net_work_create error");
			result_ = mkernel_internal_net_connect_error;
			break;
		}

		result_ = sdk_kernel->platform_handle.net_work_connect(authi_affair->lbs_net_work, sdk_kernel->server_info.server_name, sdk_kernel->server_info.server_port, 5 * 1000, szRealIp);
		if (result_ != mkernel_internal_succ)
		{
			ezdev_sdk_kernel_log_warn(result_, 0, "lbs_connect net_work_connect host error, server:%s, ip:%s, port:%d", sdk_kernel->server_info.server_name, sdk_kernel->server_info.server_ip, sdk_kernel->server_info.server_port);

			sdk_kernel->platform_handle.net_work_disconnect(authi_affair->lbs_net_work);
			sdk_kernel->platform_handle.net_work_destroy(authi_affair->lbs_net_work);
			authi_affair->lbs_net_work = NULL;

			authi_affair->lbs_net_work = sdk_kernel->platform_handle.net_work_create(g_binding_nic);
			if (authi_affair->lbs_net_work == NULL)
			{
				ezdev_sdk_kernel_log_error(result_, 0, "lbs_connect net_work_create error");
				result_ = mkernel_internal_net_connect_error;
				break;
			}

			if (0 == strlen(sdk_kernel->server_info.server_ip))
			{
				break;
			}

			result_ = sdk_kernel->platform_handle.net_work_connect(authi_affair->lbs_net_work, sdk_kernel->server_info.server_ip, sdk_kernel->server_info.server_port, 5 * 1000, szRealIp);
			if (result_ != mkernel_internal_succ)
			{
				ezdev_sdk_kernel_log_error(result_, 0, "lbs_connect net_work_connect ip error, server:%s, ip:%s, port:%d", sdk_kernel->server_info.server_name, sdk_kernel->server_info.server_ip, sdk_kernel->server_info.server_port);
				if (mkernel_internal_net_gethostbyname_error != result_)
				{
					result_ = mkernel_internal_net_connect_error;
				}

				break;
			}
		}

		ezdev_sdk_kernel_log_info(result_, 0, "lbs_connect suc, host:%s, ip:%s, parseip:%s", sdk_kernel->server_info.server_name, sdk_kernel->server_info.server_ip, szRealIp);

		memset(sdk_kernel->server_info.server_ip, 0, ezdev_sdk_ip_max_len);
		strncpy(sdk_kernel->server_info.server_ip, szRealIp, ezdev_sdk_ip_max_len - 1);
	} while (0);

	return result_;
}

static mkernel_internal_error lbs_close(ezdev_sdk_kernel *sdk_kernel, lbs_affair *authi_affair)
{
	if (authi_affair->lbs_net_work != NULL)
	{
		sdk_kernel->platform_handle.net_work_disconnect(authi_affair->lbs_net_work);
		sdk_kernel->platform_handle.net_work_destroy(authi_affair->lbs_net_work);

		authi_affair->lbs_net_work = NULL;
	}
	return mkernel_internal_succ;
}

mkernel_internal_error lbs_redirect_with_auth(ezdev_sdk_kernel *sdk_kernel, EZDEV_SDK_UINT8 nUpper)
{
	mkernel_internal_error sdk_error = mkernel_internal_succ;
	lbs_affair auth_redirect;
	das_info revc_das_info;
    void* ctx_client = NULL;

	memset(&auth_redirect, 0, sizeof(auth_redirect));
	memset(&revc_das_info, 0, sizeof(revc_das_info));

    switch (sdk_kernel->dev_cur_auth_type)
    {
    case sdk_dev_auth_protocol_ecdh:
        ctx_client = (bscomptls_ecdh_context*)malloc(sizeof(bscomptls_ecdh_context));
        if (ctx_client == NULL)
        {
            return mkernel_internal_mem_lack;
        }
        bscomptls_ecdh_init((bscomptls_ecdh_context*)ctx_client);
        sdk_kernel->dev_last_auth_type = sdk_kernel->dev_cur_auth_type;
        break;
    default:
        return mkernel_internal_internal_err;
        break;
    }

	do
	{
		sdk_error = init_lbs_affair(sdk_kernel, &auth_redirect, nUpper);
		if (sdk_error != mkernel_internal_succ)
		{
			break;
		}

		sdk_error = lbs_connect(sdk_kernel, &auth_redirect);
		if (sdk_error != mkernel_internal_succ)
		{
			break;
		}

		sdk_error = send_authentication_i(sdk_kernel, &auth_redirect, ctx_client);
		if (sdk_error != mkernel_internal_succ)
		{
			break;
		}

		sdk_error = wait_authentication_ii(sdk_kernel, &auth_redirect, ctx_client);
		if (sdk_error != mkernel_internal_succ)
		{
			break;
		}
		clear_lbs_affair_buf(&auth_redirect);
		sdk_error = send_update_sessionkey_req(sdk_kernel, &auth_redirect);
		if (sdk_error != mkernel_internal_succ)
		{
			break;
		}
		sdk_error = wait_update_sessionkey_rsp(sdk_kernel, &auth_redirect);
		if (sdk_error != mkernel_internal_succ)
		{
			break;
		}

		clear_lbs_affair_buf(&auth_redirect);
		sdk_error = send_crypto_data_req(sdk_kernel, &auth_redirect, 1);
		if (sdk_error != mkernel_internal_succ)
		{
			break;
		}

		sdk_error = wait_crypto_data_rsp_das(sdk_kernel, &auth_redirect, &revc_das_info);
		if (sdk_error != mkernel_internal_succ)
		{
			break;
		}

		save_key_value(sdk_kernel, &auth_redirect);
		save_das_info(sdk_kernel, &revc_das_info);
	} while (0);

	lbs_close(sdk_kernel, &auth_redirect);
	fini_lbs_affair(&auth_redirect);

    switch (sdk_kernel->dev_last_auth_type)
    {
    case sdk_dev_auth_protocol_ecdh:
        bscomptls_ecdh_free((bscomptls_ecdh_context* )ctx_client);
        if (ctx_client != NULL)
        {
            free(ctx_client);
            ctx_client = NULL;
        }
        break;
    default:
        return mkernel_internal_internal_err;
        break;
    }

	return sdk_error;
}

mkernel_internal_error lbs_redirect_createdevid_with_auth(ezdev_sdk_kernel *sdk_kernel, EZDEV_SDK_UINT8 nUpper)
{
	mkernel_internal_error sdk_error = mkernel_internal_succ;
	lbs_affair auth_redirect;
    void* ctx_client = NULL;
	das_info revc_das_info;
	memset(&auth_redirect, 0, sizeof(auth_redirect));
	memset(&revc_das_info, 0, sizeof(revc_das_info));

    
    switch (sdk_kernel->dev_cur_auth_type)
    {
    case sdk_dev_auth_protocol_ecdh:
        ctx_client = (bscomptls_ecdh_context*)malloc(sizeof(bscomptls_ecdh_context));
        if (ctx_client == NULL)
        {
            return mkernel_internal_mem_lack;
        }
        bscomptls_ecdh_init((bscomptls_ecdh_context* )ctx_client);
        sdk_kernel->dev_last_auth_type = sdk_kernel->dev_cur_auth_type;
    	break;
    default:
        return mkernel_internal_internal_err;
        break;
    }

	do
	{
		sdk_error = init_lbs_affair(sdk_kernel, &auth_redirect, nUpper);
		if (sdk_error != mkernel_internal_succ)
		{
			break;
		}

		sdk_error = lbs_connect(sdk_kernel, &auth_redirect);
		if (sdk_error != mkernel_internal_succ)
		{
			break;
		}

		sdk_error = send_authentication_i(sdk_kernel, &auth_redirect, ctx_client);
		if (sdk_error != mkernel_internal_succ)
		{
			break;
		}
		sdk_error = wait_authentication_ii(sdk_kernel, &auth_redirect, ctx_client);
		if (sdk_error != mkernel_internal_succ)
		{
			break;
		}
	
	    clear_lbs_affair_buf(&auth_redirect);
		sdk_error = send_authentication_creat_dev_id(sdk_kernel, &auth_redirect);
		if (sdk_error != mkernel_internal_succ)
		{
			break;
		}
		sdk_error = wait_authentication_creat_dev_id(sdk_kernel, &auth_redirect);
		if (sdk_error != mkernel_internal_succ)
		{
			break;
		}

		clear_lbs_affair_buf(&auth_redirect);
		sdk_error = send_crypto_data_req(sdk_kernel, &auth_redirect, 1);
		if (sdk_error != mkernel_internal_succ)
		{
			break;
		}

		sdk_error = wait_crypto_data_rsp_das(sdk_kernel, &auth_redirect, &revc_das_info);
		if (sdk_error != mkernel_internal_succ)
		{
			break;
		}
		save_key_value(sdk_kernel, &auth_redirect);
		save_das_info(sdk_kernel, &revc_das_info);

	} while (0);

	lbs_close(sdk_kernel, &auth_redirect);
	fini_lbs_affair(&auth_redirect);

    switch (sdk_kernel->dev_last_auth_type)
    {
    case sdk_dev_auth_protocol_ecdh:
        bscomptls_ecdh_free((bscomptls_ecdh_context*)ctx_client);
        if (ctx_client != NULL)
        {
            free(ctx_client);
            ctx_client = NULL;
        }
        break;
    default:
        return mkernel_internal_internal_err;
        break;
    }

	return sdk_error;
}

mkernel_internal_error lbs_redirect(ezdev_sdk_kernel *sdk_kernel)
{
	mkernel_internal_error sdk_error = mkernel_internal_succ;
	lbs_affair auth_redirect;

	das_info revc_das_info;

	memset(&auth_redirect, 0, sizeof(auth_redirect));
	memset(&revc_das_info, 0, sizeof(revc_das_info));

	do
	{
		sdk_error = init_lbs_affair(sdk_kernel, &auth_redirect, 1);
		if (sdk_error != mkernel_internal_succ)
		{
			break;
		}

		sdk_error = lbs_connect(sdk_kernel, &auth_redirect);
		if (sdk_error != mkernel_internal_succ)
		{
			break;
		}

		sdk_error = send_refreshsessionkey_i(sdk_kernel, &auth_redirect);
		if (sdk_error != mkernel_internal_succ)
		{
			break;
		}

		sdk_error = wait_refreshsessionkey_ii(sdk_kernel, &auth_redirect);
		if (sdk_error != mkernel_internal_succ)
		{
			break;
		}

		clear_lbs_affair_buf(&auth_redirect);
		sdk_error = send_refreshsessionkey_iii(sdk_kernel, &auth_redirect);
		if (sdk_error != mkernel_internal_succ)
		{
			break;
		}

		clear_lbs_affair_buf(&auth_redirect);
		sdk_error = send_crypto_data_req(sdk_kernel, &auth_redirect, 1);
		if (sdk_error != mkernel_internal_succ)
		{
			break;
		}

		sdk_error = wait_crypto_data_rsp_das(sdk_kernel, &auth_redirect, &revc_das_info);
		if (sdk_error != mkernel_internal_succ)
		{
			break;
		}

		save_key_value(sdk_kernel, &auth_redirect);
		save_das_info(sdk_kernel, &revc_das_info);
	} while (0);
  
	lbs_close(sdk_kernel, &auth_redirect);
	fini_lbs_affair(&auth_redirect);

	return sdk_error;
}

mkernel_internal_error lbs_getstun(ezdev_sdk_kernel *sdk_kernel, stun_info *ptr_stun)
{
	mkernel_internal_error sdk_error = mkernel_internal_succ;
	lbs_affair auth_redirect;
	memset(&auth_redirect, 0, sizeof(auth_redirect));

	do
	{
		sdk_error = init_lbs_affair(sdk_kernel, &auth_redirect, 1);
		if (sdk_error != mkernel_internal_succ)
		{
			break;
		}

		sdk_error = lbs_connect(sdk_kernel, &auth_redirect);
		if (sdk_error != mkernel_internal_succ)
		{
			break;
		}

		sdk_error = send_stun_refreshsessionkey_i(sdk_kernel, &auth_redirect);
		if (sdk_error != mkernel_internal_succ)
		{
			break;
		}

		sdk_error = wait_stun_refreshsessionkey_ii(sdk_kernel, &auth_redirect);
		if (sdk_error != mkernel_internal_succ)
		{
			break;
		}

		clear_lbs_affair_buf(&auth_redirect);
		sdk_error = send_stun_refreshsessionkey_iii(sdk_kernel, &auth_redirect);
		if (sdk_error != mkernel_internal_succ)
		{
			break;
		}

		clear_lbs_affair_buf(&auth_redirect);
		sdk_error = send_crypto_data_req(sdk_kernel, &auth_redirect, 0);
		if (sdk_error != mkernel_internal_succ)
		{
			break;
		}

		sdk_error = wait_crypto_data_rsp_stun(sdk_kernel, &auth_redirect, ptr_stun);
		if (sdk_error != mkernel_internal_succ)
		{
			break;
		}
	} while (0);

	lbs_close(sdk_kernel, &auth_redirect);
	fini_lbs_affair(&auth_redirect);

	return sdk_error;
}

static mkernel_internal_error send_get_secretkey_rsq(ezdev_sdk_kernel *hsdk_kernel, lbs_affair *hlbs_affair, EZDEV_SDK_UINT8 *aesKey, EZDEV_SDK_INT32 aesKeyLen)
{
	mkernel_internal_error sdk_rv = mkernel_internal_succ;

	char *pE = "010001";
	char *pN = "CD6E54AA0B45FEA94F03955C838C0827C9017A066A6B7FA7599FE4E1775E8E4EFD304F15630EDAAAE9C7044FC6EE94F02AD2EE44C644F55E10CD428C3D806F55C1D90D76AF395B5FA3DA155F639515EA272715238D2371A8FC0B64A098145BD0CD13D3B90FCC72B605B393C693BAE6C4BA0990BF8873A9A4CC6489960160ACDF";

	unsigned char pSendBuf[512];
	unsigned char *p = pSendBuf;	
	unsigned char buf[128];			
	unsigned int iLength = 0;		
	unsigned int uiPlainPadding = 0; 
	unsigned short netLen16 = 0;	 
	memset(buf, 0, sizeof(buf));
	memset(pSendBuf, 0, sizeof(pSendBuf));

	do
	{
		*p++ = 0x01;
		iLength = sizeof(pSendBuf) - (p - pSendBuf);
		uiPlainPadding = calculate_padding_len(strlen(hlbs_affair->dev_subserial));

		memcpy(buf, hlbs_affair->dev_subserial, strlen(hlbs_affair->dev_subserial));
		if (0 != (sdk_rv = aes_cbc_128_enc_padding(aesKey, buf, strlen((char *)buf), uiPlainPadding, p + 2, &iLength)))
		{
			ezdev_sdk_kernel_log_debug(sdk_rv, 0, "Error occur in aes_cbc_128_enc_padding\n");
			break;
		}
		netLen16 = htons(iLength);
		memcpy(p, &netLen16, sizeof(netLen16));
		p += 2;
		p += iLength;

	
		buf[0] = aesKeyLen & 0xFF; 
		memcpy(buf + 1, aesKey, aesKeyLen);
		buf[aesKeyLen + 1] = strlen(hlbs_affair->dev_subserial) & 0xFF;
		memcpy(buf + aesKeyLen + 2, hlbs_affair->dev_subserial, strlen(hlbs_affair->dev_subserial));
		iLength = sizeof(pSendBuf) - (p - pSendBuf);

		if (0 != ezRsaEncrypt(buf, aesKeyLen + strlen(hlbs_affair->dev_subserial) + 2,
							  p + 2, (int *)&iLength, pN, pE))
		{
			ezdev_sdk_kernel_log_debug(sdk_rv, 0, "Error occur in ezRsaEncrypt\n");
			sdk_rv = mkernel_internal_internal_err;
			break;
		}
		netLen16 = htons(iLength);
		memcpy(p, &netLen16, sizeof(netLen16));
		p += 2;
		p += iLength;

		sdk_rv = common_serialize(&hlbs_affair->global_out_packet, DEV_PROTOCOL_LBS_FORM_VERSION, DEV_PROTOCOL_LBS_LOW_TYPE_VERSION, DEV_PROTOCOL_LBS_HIGH_TYPE_VERSION);
		if (mkernel_internal_succ != sdk_rv)
		{
			break;
		}

		memcpy(hlbs_affair->global_out_packet.payload_buf + hlbs_affair->global_out_packet.payload_buf_off, pSendBuf, (p - pSendBuf));
		hlbs_affair->global_out_packet.payload_buf_off += (p - pSendBuf);

		sdk_rv = header_serialize_old(&hlbs_affair->global_out_packet, DEV_PROTOCOL_GET_SECRETKEY, hlbs_affair->global_out_packet.payload_buf_off);
		if (mkernel_internal_succ != sdk_rv)
		{
			break;
		}

		sdk_rv = send_lbs_msg(hsdk_kernel, hlbs_affair);
		if (mkernel_internal_succ != sdk_rv)
		{
			ezdev_sdk_kernel_log_debug(sdk_rv, 0, "Error occur in send_lbs_msg");
			break;
		}
	} while (0);

	return sdk_rv;
}

static mkernel_internal_error wait_get_secretkey_rsp(ezdev_sdk_kernel *hsdk_kernel, lbs_affair *hlbs_affair, EZDEV_SDK_UINT8 *aesKey, EZDEV_SDK_INT8 aesKeyLen, EZDEV_SDK_UINT8 *secretKey, EZDEV_SDK_INT8 *secretKeyLen, EZDEV_SDK_UINT16 *interval, EZDEV_SDK_UINT32 *duration)
{
	mkernel_internal_error sdk_rv = mkernel_internal_succ;

	EZDEV_SDK_UINT32 rev_cmd = 0;
	EZDEV_SDK_UINT32 remain_len = 0;
	EZDEV_SDK_UINT8 result_code = 0;
	EZDEV_SDK_UINT8 pPlainText[256];
	EZDEV_SDK_UINT32 iPlainTextLen = sizeof(pPlainText);
	EZDEV_SDK_UINT16 netLen16 = 0;
	EZDEV_SDK_UINT16 _interval = 0;
	EZDEV_SDK_UINT32 _duration = 0;
	EZDEV_SDK_UINT8 *p;
	memset(pPlainText, 0, iPlainTextLen);

	do
	{
		if (mkernel_internal_succ != (sdk_rv = wait_assign_response(hsdk_kernel, hlbs_affair, &rev_cmd, &remain_len)))
		{
			ezdev_sdk_kernel_log_debug(sdk_rv, 0, "Error occur in wait_assign_response");
			break;
		}

		if (DEV_PROTOCOL_GET_SECRETKEY != rev_cmd)
		{
			sdk_rv = mkernel_internal_net_read_error_request;
			break;
		}
		hlbs_affair->global_in_packet.payload_buf_off = 3;

		result_code = *(hlbs_affair->global_in_packet.payload_buf + hlbs_affair->global_in_packet.payload_buf_off++);

		memcpy(&netLen16, hlbs_affair->global_in_packet.payload_buf + hlbs_affair->global_in_packet.payload_buf_off, sizeof(short));
		hlbs_affair->global_in_packet.payload_buf_off += 2;

		if (hlbs_affair->global_in_packet.payload_buf_off + ntohs(netLen16) + 6 > remain_len)
		{
			ezdev_sdk_kernel_log_debug(sdk_rv, result_code, "rsp data len out of range, real len = %d", remain_len);
			*interval = 30;
			*duration = 3600 * 24;
			sdk_rv = mkernel_internal_rev_invalid_packet;
			break;
		}
		else
		{
			memcpy(&_interval, hlbs_affair->global_in_packet.payload_buf + hlbs_affair->global_in_packet.payload_buf_off + ntohs(netLen16), sizeof(_interval));
			memcpy(&_duration, hlbs_affair->global_in_packet.payload_buf + hlbs_affair->global_in_packet.payload_buf_off + ntohs(netLen16) + sizeof(_interval), sizeof(_duration));

			*interval = ntohs(_interval);
			*duration = ntohl(_duration);
			ezdev_sdk_kernel_log_debug(sdk_rv, result_code, "interval= %d, duration = %d", *interval, *duration);
		}

		if (0 != result_code)
		{
			sdk_rv = mkernel_internal_platform_error + result_code;

			ezdev_sdk_kernel_log_debug(sdk_rv, result_code, "wait_get_secretkey_rsp platform return error");
			break;
		}

		sdk_rv = aes_cbc_128_dec_padding(aesKey, hlbs_affair->global_in_packet.payload_buf + hlbs_affair->global_in_packet.payload_buf_off,
										 ntohs(netLen16), pPlainText, &iPlainTextLen);
		if (mkernel_internal_succ != sdk_rv)
		{
			ezdev_sdk_kernel_log_debug(sdk_rv, 0, "Error occur in aes_cbc_128_dec_padding\n");
			break;
		}
		p = pPlainText;
		if (*p > ezdev_sdk_devserial_maxlen ||
			*p > iPlainTextLen)
		{
			sdk_rv = mkernel_internal_rev_invalid_packet;
			break;
		}

		memcpy(secretKey, p + 1, *p);
		*secretKeyLen = *p;
		p += *p + 1;

		if (strlen(hlbs_affair->dev_subserial) != *p ||(p - pPlainText + *p) > iPlainTextLen ||									
			memcmp(++p, hlbs_affair->dev_subserial, strlen(hlbs_affair->dev_subserial))) 
		{
			sdk_rv = mkernel_internal_rev_invalid_packet;
			break;
		}
	} while (0);

	return sdk_rv;
}

mkernel_internal_error cnt_state_lbs_apply_serectkey(ezdev_sdk_kernel *hsdk_kernel, EZDEV_SDK_UINT16 *interval, EZDEV_SDK_UINT32 *duration)
{
	mkernel_internal_error sdk_rv = mkernel_internal_internal_err;

#ifndef _REALTEK_RTOS_
	EZDEV_SDK_UINT8 aesKey[32] = {0};							   
	EZDEV_SDK_UINT8 aesKey_hex[32] = {0};						   
	EZDEV_SDK_INT8 aesKeyLen = 16;								   
	EZDEV_SDK_UINT8 secretKey[ezdev_sdk_verify_code_maxlen] = {0};
	EZDEV_SDK_INT8 secretKeyLen8 = sizeof(secretKey);
	EZDEV_SDK_INT32 secretKeyLen32 = sizeof(secretKey);
	int i = 0;

	lbs_affair hlbs_affair;
	memset(&hlbs_affair, 0, sizeof(hlbs_affair));
	if (0 != ezRandomGen(aesKey, aesKeyLen))
		return sdk_rv;
	for (i = 0; i < aesKeyLen / 2; ++i)
	{
		sprintf((char *)aesKey_hex + 2 * i, "%02x", aesKey[i]);
	}

	do
	{
		sdk_rv = init_lbs_affair(hsdk_kernel, &hlbs_affair, 1);
		if (sdk_rv != mkernel_internal_succ)
		{
			ezdev_sdk_kernel_log_debug(sdk_rv, sdk_rv, "init_lbs_affair err\n");
			break;
		}

		if (mkernel_internal_succ != (sdk_rv = lbs_connect(hsdk_kernel, &hlbs_affair)))
		{
			ezdev_sdk_kernel_log_debug(sdk_rv, 0, "lbs_connect\n");
			break;
		}

		if (mkernel_internal_succ != (sdk_rv = send_get_secretkey_rsq(hsdk_kernel, &hlbs_affair, aesKey_hex, aesKeyLen)))
		{
			ezdev_sdk_kernel_log_debug(sdk_rv, 0, "send_get_secretkey_rsq\n");
			break;
		}

		if (mkernel_internal_succ != (sdk_rv = wait_get_secretkey_rsp(hsdk_kernel, &hlbs_affair, aesKey_hex, aesKeyLen, secretKey, &secretKeyLen8, interval, duration)))
		{
			ezdev_sdk_kernel_log_debug(sdk_rv, 0, "wait_get_secretkey_rsp\n");
			break;
		}

		hsdk_kernel->secretkey_applied = EZDEV_SDK_TRUE;
		secretKeyLen32 = (EZDEV_SDK_INT32)secretKeyLen8;
		if (mkernel_internal_succ != (sdk_rv = hsdk_kernel->platform_handle.curing_data_save(sdk_curingdata_secretkey, secretKey, secretKeyLen32)))
		{
			ezdev_sdk_kernel_log_debug(sdk_rv, 0, "Error occur in curing_data_save\n");
			sdk_rv = mkernel_internal_value_save_err;
			break;
		}

		secretKeyLen32 = sizeof(secretKey);
		memset(secretKey, 0, secretKeyLen32);

		if (mkernel_internal_succ != (sdk_rv = hsdk_kernel->platform_handle.curing_data_load(sdk_curingdata_secretkey, secretKey, &secretKeyLen32)) ||
			secretKeyLen32 > ezdev_sdk_verify_code_maxlen)
		{
			ezdev_sdk_kernel_log_error(sdk_rv, 0, "An error occur in load secretkey, return len = %d", secretKeyLen32);
			sdk_rv = mkernel_internal_value_load_err;
			break;
		}

		memcpy(hsdk_kernel->dev_info.dev_verification_code, secretKey, secretKeyLen32);
	} while (0);

	lbs_close(hsdk_kernel, &hlbs_affair);
	fini_lbs_affair(&hlbs_affair);
#endif

	return sdk_rv;
}