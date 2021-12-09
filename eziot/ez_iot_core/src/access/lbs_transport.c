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
 * Brief:
 * Device access authentication interface implementation
 * 
 * Change Logs:
 * Date           Author       Notes
 * 2021-12-01     xurongjun    Remove redundant business codes
 *******************************************************************************/

#include <ezos.h>
#include <ezlog.h>
#include "sdk_kernel_def.h"
#include "lbs_transport.h"
#include "mkernel_internal_error.h"
#include "ezdev_sdk_kernel_struct.h"
#include "dev_protocol_def.h"
#include "ezdev_ecdh_support.h"
#include "cJSON.h"
#include "mbedtls/sha256.h"
#include "mbedtls/md.h"
#include "mbedtls/aes.h"
#include "mbedtls/md5.h"
#include "mbedtls/ecdh.h"
#include "mbedtls/pkcs5.h"
#include "mbedtls/sha512.h"
#include "aes_support.h"
#include "net_module.h"
#include "utils.h"

AES_SUPPORT_INTERFACE

#define LBS_AUTH_TAG_LEN 16
#define LBS_AUTH_JSON_DEFAULT_LEN 1024
#define LBS_AUTH_INFO_LEN 128
#define LBS_AUTH_PBKDF2_HMAC_TIMES 3
#define LBS_AUTH_SHAREKEY_LEN 32
#define LBS_AUTH_SHAREKEY_SALT "www.88075998.com"
#define LBS_AUTH_MD5_LEN 32
#define LBS_AUTH_SHA256_LEN 32
#define LBS_AUTH_SHA256_OFFSET 10

typedef struct
{
    unsigned char *head_buf;
    EZDEV_SDK_UINT8 head_buf_Len;
    EZDEV_SDK_UINT8 head_buf_off;

    unsigned char *var_head_buf;
    EZDEV_SDK_UINT8 var_head_buf_Len; //可变报文头
    EZDEV_SDK_UINT8 var_head_buf_off;

    unsigned char *payload_buf;
    EZDEV_SDK_UINT32 payload_buf_Len;
    EZDEV_SDK_UINT32 payload_buf_off;
} lbs_packet;

typedef struct
{
    EZDEV_SDK_UINT8 random_1;
    EZDEV_SDK_UINT8 random_2;
    EZDEV_SDK_UINT8 random_3;
    EZDEV_SDK_UINT8 random_4;

    EZDEV_SDK_UINT16 dev_access_mode;
    sdk_dev_auth_mode dev_auth_mode;
    char dev_subserial[ezdev_sdk_devserial_maxlen];
    unsigned char master_key[ezdev_sdk_masterkey_len];
    unsigned char dev_id[ezdev_sdk_devid_len];

    unsigned char session_key[ezdev_sdk_sessionkey_len];
    unsigned char share_key[LBS_AUTH_SHAREKEY_LEN];
    EZDEV_SDK_UINT16 share_key_len;

    lbs_packet global_out_packet; ///<*	lbs 发送缓冲区
    lbs_packet global_in_packet;  ///<*	lbs 接收缓冲区

    int socket_fd;
} lbs_affair;

static mkernel_internal_error parse_authentication_create_dev_id(lbs_affair *authi_affair, EZDEV_SDK_UINT32 remain_len);
static mkernel_internal_error json_parse_das_server_info(const char *jsonstring, das_info *das_server_info);

static void generate_sharekey(ezdev_sdk_kernel *sdk_kernel, lbs_affair *redirect_affair, EZDEV_SDK_UINT8 nUpper)
{
    unsigned char sharekey_src[LBS_AUTH_INFO_LEN];
    EZDEV_SDK_UINT16 sharekey_src_len;
    EZDEV_SDK_UINT16 dev_verification_code_len;
    EZDEV_SDK_UINT16 sharekey_salt_len;

    unsigned char sharekey_dst[16];
    unsigned char sharekey_dst_hex[LBS_AUTH_MD5_LEN + 1];

    unsigned char sharekey_sha256_dst[LBS_AUTH_SHA256_LEN];
    unsigned char sharekey_sha256_dst_hex[LBS_AUTH_SHA256_LEN * 2 + 1];

    mbedtls_md_context_t sha1_ctx;
    const mbedtls_md_info_t *info_sha1 = NULL;

    ezos_memset(sharekey_src, 0, LBS_AUTH_INFO_LEN);
    ezos_memset(sharekey_dst, 0, 16);
    ezos_memset(sharekey_dst_hex, 0, LBS_AUTH_SHAREKEY_LEN + 1);

    dev_verification_code_len = ezos_strlen(sdk_kernel->dev_info.dev_verification_code);
    sharekey_salt_len = ezos_strlen(LBS_AUTH_SHAREKEY_SALT);

    sharekey_src_len = dev_verification_code_len;
    ezos_memcpy(sharekey_src, sdk_kernel->dev_info.dev_verification_code, sharekey_src_len);
    ezos_memcpy(sharekey_src + sharekey_src_len, sdk_kernel->dev_info.dev_subserial, ezos_strlen(sdk_kernel->dev_info.dev_subserial));

    sharekey_src_len += ezos_strlen(sdk_kernel->dev_info.dev_subserial);

    mbedtls_md5(sharekey_src, sharekey_src_len, sharekey_dst);

    if (nUpper != 0)
    {
        md5_hexdump(sharekey_dst, 16, 1, sharekey_dst_hex);
    }
    else
    {
        md5_hexdump(sharekey_dst, 16, 0, sharekey_dst_hex);
    }

    ezos_memset(sharekey_src, 0, LBS_AUTH_INFO_LEN);

    ezos_memcpy(sharekey_src, sharekey_dst_hex, LBS_AUTH_MD5_LEN);
    ezos_memcpy(sharekey_src + LBS_AUTH_MD5_LEN, LBS_AUTH_SHAREKEY_SALT, sharekey_salt_len);

    sharekey_src_len = LBS_AUTH_MD5_LEN + sharekey_salt_len;

    mbedtls_md5(sharekey_src, sharekey_src_len, sharekey_dst);
    md5_hexdump(sharekey_dst, 16, 1, sharekey_dst_hex);

    ezos_memset(sharekey_src, 0, LBS_AUTH_INFO_LEN);
    ezos_memcpy(sharekey_src, sharekey_dst_hex, LBS_AUTH_MD5_LEN);
    sharekey_src_len = LBS_AUTH_MD5_LEN;

    mbedtls_md5(sharekey_src, sharekey_src_len, sharekey_dst);
    md5_hexdump(sharekey_dst, 16, 1, sharekey_dst_hex);

    mbedtls_md_init(&sha1_ctx);
    info_sha1 = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
    if (info_sha1 == NULL)
    {
        return;
    }

    if (mbedtls_md_setup(&sha1_ctx, info_sha1, 1) != 0)
    {
        return;
    }

    mbedtls_pkcs5_pbkdf2_hmac(&sha1_ctx, sharekey_dst_hex, LBS_AUTH_MD5_LEN,
                              (const unsigned char *)LBS_AUTH_SHAREKEY_SALT, sharekey_salt_len, LBS_AUTH_PBKDF2_HMAC_TIMES, LBS_AUTH_SHA256_LEN, sharekey_sha256_dst);
    md5_hexdump(sharekey_sha256_dst, LBS_AUTH_SHA256_LEN, 1, sharekey_sha256_dst_hex);

    mbedtls_md_free(&sha1_ctx);

    ezos_memcpy(redirect_affair->share_key, sharekey_sha256_dst_hex + LBS_AUTH_SHA256_OFFSET, LBS_AUTH_SHAREKEY_LEN);
    redirect_affair->share_key_len = LBS_AUTH_SHAREKEY_LEN;
}

static mkernel_internal_error init_lbs_affair(ezdev_sdk_kernel *sdk_kernel, lbs_affair *redirect_affair, EZDEV_SDK_UINT8 nUpper)
{
    redirect_affair->random_1 = ezos_rand() % 256;
    redirect_affair->random_2 = ezos_rand() % 256;
    redirect_affair->random_3 = ezos_rand() % 256;
    redirect_affair->random_4 = ezos_rand() % 256;

    redirect_affair->global_out_packet.head_buf = ezos_malloc(16);
    if (NULL == redirect_affair->global_out_packet.head_buf)
    {
        ezlog_e(TAG_CORE, "malloc out_packet head_buf err");
        return mkernel_internal_mem_lack;
    }
    ezos_memset(redirect_affair->global_out_packet.head_buf, 0, 16);
    redirect_affair->global_out_packet.head_buf_off = 0;
    redirect_affair->global_out_packet.head_buf_Len = 16;

    redirect_affair->global_out_packet.var_head_buf = ezos_malloc(LBS_AUTH_TAG_LEN);
    if (NULL == redirect_affair->global_out_packet.var_head_buf)
    {
        ezlog_e(TAG_CORE, "malloc out_packet head_buf err");
        return mkernel_internal_mem_lack;
    }
    ezos_memset(redirect_affair->global_out_packet.var_head_buf, 0, LBS_AUTH_TAG_LEN);
    redirect_affair->global_out_packet.var_head_buf_off = 0;
    redirect_affair->global_out_packet.var_head_buf_Len = LBS_AUTH_TAG_LEN;
    redirect_affair->global_out_packet.payload_buf = ezos_malloc(CONFIG_EZIOT_CORE_MESSAGE_SIZE_MAX);
    if (NULL == redirect_affair->global_out_packet.payload_buf)
    {
        ezlog_e(TAG_CORE, "malloc out_packet payload_buf err");
        return mkernel_internal_mem_lack;
    }
    ezos_memset(redirect_affair->global_out_packet.payload_buf, 0, CONFIG_EZIOT_CORE_MESSAGE_SIZE_MAX);
    redirect_affair->global_out_packet.payload_buf_off = 0;
    redirect_affair->global_out_packet.payload_buf_Len = CONFIG_EZIOT_CORE_MESSAGE_SIZE_MAX;
    redirect_affair->global_in_packet.head_buf = ezos_malloc(16);
    if (NULL == redirect_affair->global_in_packet.head_buf)
    {
        ezlog_e(TAG_CORE, "malloc in_packet head_buf err ");
        return mkernel_internal_mem_lack;
    }
    ezos_memset(redirect_affair->global_in_packet.head_buf, 0, 16);
    redirect_affair->global_in_packet.head_buf_off = 0;
    redirect_affair->global_in_packet.head_buf_Len = 16;
    redirect_affair->global_in_packet.var_head_buf = ezos_malloc(LBS_AUTH_TAG_LEN);
    if (NULL == redirect_affair->global_in_packet.var_head_buf)
    {
        ezlog_e(TAG_CORE, "malloc out_packet head_buf err");
        return mkernel_internal_mem_lack;
    }
    ezos_memset(redirect_affair->global_in_packet.var_head_buf, 0, LBS_AUTH_TAG_LEN);
    redirect_affair->global_in_packet.var_head_buf_off = 0;
    redirect_affair->global_in_packet.var_head_buf_Len = LBS_AUTH_TAG_LEN;
    redirect_affair->global_in_packet.payload_buf = ezos_malloc(CONFIG_EZIOT_CORE_MESSAGE_SIZE_MAX);
    if (NULL == redirect_affair->global_in_packet.payload_buf)
    {
        ezlog_e(TAG_CORE, "malloc in_packet payload_buf err ");
        return mkernel_internal_mem_lack;
    }
    ezos_memset(redirect_affair->global_in_packet.payload_buf, 0, CONFIG_EZIOT_CORE_MESSAGE_SIZE_MAX);
    redirect_affair->global_in_packet.payload_buf_off = 0;
    redirect_affair->global_in_packet.payload_buf_Len = CONFIG_EZIOT_CORE_MESSAGE_SIZE_MAX;

    redirect_affair->dev_auth_mode = sdk_kernel->dev_info.dev_auth_mode;
    ezos_memcpy(redirect_affair->dev_subserial, sdk_kernel->dev_info.dev_subserial, ezdev_sdk_devserial_maxlen);
    ezos_memcpy(redirect_affair->dev_id, sdk_kernel->dev_id, ezdev_sdk_devid_len);
    ezos_memcpy(redirect_affair->master_key, sdk_kernel->master_key, ezdev_sdk_masterkey_len);

    redirect_affair->dev_access_mode = sdk_kernel->dev_info.dev_access_mode;

    generate_sharekey(sdk_kernel, redirect_affair, nUpper);

    redirect_affair->socket_fd = -1;

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
        ezos_free(redirect_affair->global_out_packet.head_buf);
        redirect_affair->global_out_packet.head_buf = NULL;
    }
    if (redirect_affair->global_out_packet.var_head_buf != NULL)
    {
        ezos_free(redirect_affair->global_out_packet.var_head_buf);
        redirect_affair->global_out_packet.var_head_buf = NULL;
    }
    if (redirect_affair->global_out_packet.payload_buf != NULL)
    {
        ezos_free(redirect_affair->global_out_packet.payload_buf);
        redirect_affair->global_out_packet.payload_buf = NULL;
    }
    if (redirect_affair->global_in_packet.head_buf != NULL)
    {
        ezos_free(redirect_affair->global_in_packet.head_buf);
        redirect_affair->global_in_packet.head_buf = NULL;
    }
    if (redirect_affair->global_in_packet.var_head_buf != NULL)
    {
        ezos_free(redirect_affair->global_in_packet.var_head_buf);
        redirect_affair->global_in_packet.var_head_buf = NULL;
    }
    if (redirect_affair->global_in_packet.payload_buf != NULL)
    {
        ezos_free(redirect_affair->global_in_packet.payload_buf);
        redirect_affair->global_in_packet.payload_buf = NULL;
    }
    ezos_memset(redirect_affair, 0, sizeof(lbs_affair));
}

static void clear_lbs_affair_buf(lbs_affair *redirect_affair)
{
    ezos_memset(redirect_affair->global_out_packet.head_buf, 0, 16);
    redirect_affair->global_out_packet.head_buf_off = 0;

    ezos_memset(redirect_affair->global_out_packet.payload_buf, 0, CONFIG_EZIOT_CORE_MESSAGE_SIZE_MAX);
    redirect_affair->global_out_packet.payload_buf_off = 0;

    ezos_memset(redirect_affair->global_in_packet.head_buf, 0, 16);
    redirect_affair->global_in_packet.head_buf_off = 0;

    ezos_memset(redirect_affair->global_in_packet.payload_buf, 0, CONFIG_EZIOT_CORE_MESSAGE_SIZE_MAX);
    redirect_affair->global_in_packet.payload_buf_off = 0;
}

static void save_key_value(ezdev_sdk_kernel *sdk_kernel, lbs_affair *affair)
{
    ezos_memcpy(sdk_kernel->dev_id, affair->dev_id, ezdev_sdk_devid_len);
    ezos_memcpy(sdk_kernel->master_key, affair->master_key, ezdev_sdk_masterkey_len);
    ezos_memcpy(sdk_kernel->session_key, affair->session_key, ezdev_sdk_sessionkey_len);

    sdk_kernel->key_value_save(sdk_keyvalue_devid, affair->dev_id, ezdev_sdk_devid_len);
    sdk_kernel->key_value_save(sdk_keyvalue_masterkey, affair->master_key, ezdev_sdk_masterkey_len);
}

static void save_das_info(ezdev_sdk_kernel *sdk_kernel, das_info *recv_das_info)
{
    ezos_memcpy(&sdk_kernel->redirect_das_info, recv_das_info, sizeof(das_info));
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
    byte_1 = ((cmd & 0x0F) << 4) | 0x2;
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
    unsigned char sign_input[LBS_AUTH_INFO_LEN];
    unsigned char sign_output[64];
    EZDEV_SDK_INT32 mbedtls_result;

    if ((lbs_pack->payload_buf_off + 64) > lbs_pack->payload_buf_Len)
    {
        return mkernel_internal_mem_lack;
    }

    if (sign_src_len > LBS_AUTH_INFO_LEN)
    {
        return mkernel_internal_input_param_invalid;
    }
    ezos_memset(sign_input, 0, LBS_AUTH_INFO_LEN);
    ezos_memset(sign_output, 0, 64);
    ezos_memcpy(sign_input, sign_src, sign_src_len);
    sign_input_len = sign_src_len;
    mbedtls_sha512(sign_input, sign_input_len, sign_output, 1);

    ezos_memset(sign_input, 0, LBS_AUTH_INFO_LEN);
    ezos_memcpy(sign_input, sign_output, 64);
    sign_input_len = ezos_strlen((const char *)sign_input);

    ezos_memset(sign_output, 0, 64);
    mbedtls_result = mbedtls_md_hmac(mbedtls_md_info_from_string("SHA384"), master_key, master_key_len, sign_input, 48, sign_output);
    if (mbedtls_result != 0)
    {
        return mkernel_internal_hmac_error;
    }

    ezos_memcpy(lbs_pack->payload_buf + lbs_pack->payload_buf_off, sign_output, 48);

    lbs_pack->payload_buf_off += 48;

    return mkernel_internal_succ;
}

static mkernel_internal_error digital_sign_serialize_and_check_sha384(unsigned char *target_sign, EZDEV_SDK_UINT16 target_sign_len,
                                                                      unsigned char *sign_src, EZDEV_SDK_UINT16 sign_src_len, unsigned char *master_key, EZDEV_SDK_UINT16 master_key_len)
{
    EZDEV_SDK_UINT32 sign_input_len = 0;
    unsigned char sign_input[LBS_AUTH_INFO_LEN];
    unsigned char sign_output[64];
    EZDEV_SDK_INT32 mbedtls_result = 0;

    if (sign_src_len > 128)
    {
        ezlog_d(TAG_CORE, "digital_sign_serialize_and_check_sha384 ,sign_src_len > 128");
        return mkernel_internal_input_param_invalid;
    }
    if (target_sign_len != 48)
    {
        ezlog_d(TAG_CORE, "digital_sign_serialize_and_check_sha384 ,target_sign_len != 64");
        return mkernel_internal_sign_check_error;
    }
    ezos_memset(sign_input, 0, LBS_AUTH_INFO_LEN);
    ezos_memset(sign_output, 0, 64);

    ezos_memcpy(sign_input, sign_src, sign_src_len);
    sign_input_len = sign_src_len;
    mbedtls_sha512(sign_input, sign_input_len, sign_output, 1);

    ezos_memset(sign_input, 0, LBS_AUTH_INFO_LEN);
    ezos_memcpy(sign_input, sign_output, 48);
    ezos_memset(sign_output, 0, 64);

    mbedtls_result = mbedtls_md_hmac(mbedtls_md_info_from_string("SHA384"), master_key, master_key_len, sign_input, 48, sign_output);
    if (mbedtls_result != 0)
    {
        ezlog_d(TAG_CORE, "mbedtls_md_hmac error");
        return mkernel_internal_hmac_error;
    }

    mbedtls_result = ezos_memcmp(target_sign, sign_output, 48);
    if (mbedtls_result != 0)
    {
        ezlog_d(TAG_CORE, "mbedtls_md_hmac check error");
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
    mkernel_internal_error sdk_error = net_read(authi_affair->socket_fd, &byte_1, 1, 5 * 1000);
    if (sdk_error != mkernel_internal_succ)
    {
        ezlog_d(TAG_CORE, "wait_assign_response revc byte_1 error");
        return sdk_error;
    }

    *rev_cmd = 0;
    *rev_cmd = (byte_1 & 0xF0) >> 4;

    *remain_len = 0;
    do
    {
        byte_2 = 0;
        sdk_error = net_read(authi_affair->socket_fd, &byte_2, 1, 5 * 1000);
        if (sdk_error != mkernel_internal_succ)
        {
            ezlog_d(TAG_CORE, "wait_assign_response revc byte_2 error");
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

        sdk_error = net_read(authi_affair->socket_fd, authi_affair->global_in_packet.var_head_buf, 1, 5 * 1000);
        if (sdk_error != 0)
        {
            ezlog_d(TAG_CORE, "wait_assign_response revc var head error");
            return sdk_error;
        }
        authi_affair->global_in_packet.var_head_buf_off += 1;
        *remain_len -= authi_affair->global_in_packet.var_head_buf_off;
        if (*remain_len > authi_affair->global_in_packet.payload_buf_Len)
        {
            ezlog_d(TAG_CORE, "wait_assign_response payload_buf size %d not enough for %d", authi_affair->global_in_packet.payload_buf_Len, *remain_len);
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
    sdk_error = net_read(authi_affair->socket_fd, authi_affair->global_in_packet.payload_buf, *remain_len, 5 * 1000);

    if (sdk_error != 0)
    {
        ezlog_d(TAG_CORE, "wait_assign_response revc data error");
        return sdk_error;
    }

    return mkernel_internal_succ;
}

static mkernel_internal_error send_lbs_msg(ezdev_sdk_kernel *sdk_kernel, lbs_affair *authi_affair)
{
    mkernel_internal_error result_ = mkernel_internal_succ;
    EZDEV_SDK_INT32 real_sendlen = 0;
    unsigned char cmd = 0;
    unsigned char flag = 0;
    result_ = net_write(authi_affair->socket_fd, authi_affair->global_out_packet.head_buf, authi_affair->global_out_packet.head_buf_off, 5 * 1000, &real_sendlen);
    if (result_ != mkernel_internal_succ)
    {
        return result_;
    }
    cmd = (authi_affair->global_out_packet.head_buf[0] & 0xf0) >> 4;
    flag = (authi_affair->global_out_packet.head_buf[0] & 0x08) >> 3;
    if (DEV_PROTOCOL_AUTHENTICATION_I == cmd && 0x01 == flag)
    {
        result_ = net_write(authi_affair->socket_fd, authi_affair->global_out_packet.var_head_buf, authi_affair->global_out_packet.var_head_buf_off, 5 * 1000, &real_sendlen);
        if (result_ != mkernel_internal_succ)
        {
            return result_;
        }
    }

    result_ = net_write(authi_affair->socket_fd, authi_affair->global_out_packet.payload_buf, authi_affair->global_out_packet.payload_buf_off, 5 * 1000, &real_sendlen);
    if (result_ != mkernel_internal_succ)
    {
        return result_;
    }

    return mkernel_internal_succ;
}
static mkernel_internal_error authentication_i_serialize(lbs_affair *authi_affair)
{
    EZDEV_SDK_UINT8 dev_serial_len = ezos_strlen(authi_affair->dev_subserial);

    *(EZDEV_SDK_UINT8 *)(authi_affair->global_out_packet.payload_buf + authi_affair->global_out_packet.payload_buf_off) = authi_affair->dev_auth_mode;
    authi_affair->global_out_packet.payload_buf_off++;

    *(EZDEV_SDK_UINT8 *)(authi_affair->global_out_packet.payload_buf + authi_affair->global_out_packet.payload_buf_off) = dev_serial_len;
    authi_affair->global_out_packet.payload_buf_off++;

    ezos_memcpy(authi_affair->global_out_packet.payload_buf + authi_affair->global_out_packet.payload_buf_off, authi_affair->dev_subserial, dev_serial_len);
    authi_affair->global_out_packet.payload_buf_off += dev_serial_len;

    return mkernel_internal_succ;
}

static mkernel_internal_error aes_128_encrypt_pubkey(lbs_affair *authi_affair, unsigned char *input_buf, EZDEV_SDK_UINT32 input_buf_len,
                                                     unsigned char *output_buf, EZDEV_SDK_UINT32 *output_buf_len,
                                                     unsigned char *output_tag_buf, EZDEV_SDK_UINT32 tag_buf_len)
{
    mkernel_internal_error sdk_error = mkernel_internal_succ;
    unsigned char *out_buf = NULL;
    EZDEV_SDK_UINT32 out_buf_len = 0;
    unsigned char aes_encrypt_key[16] = {0};
    unsigned char temp[512] = {0};
    EZDEV_SDK_UINT32 templen = 0;
    if (authi_affair == NULL || input_buf == NULL || input_buf_len == 0)
    {
        return mkernel_internal_input_param_invalid;
    }
    out_buf = (unsigned char *)ezos_malloc(input_buf_len + 1);
    if (NULL == out_buf)
    {
        sdk_error = mkernel_internal_malloc_error;
        return sdk_error;
    }
    ezos_memset(out_buf, 0, input_buf_len + 1);
    out_buf_len = input_buf_len;
    ezos_memcpy(aes_encrypt_key, authi_affair->share_key, 16);

    sdk_error = aes_gcm_128_enc_padding(aes_encrypt_key, input_buf, input_buf_len, out_buf, &out_buf_len, output_tag_buf, tag_buf_len);
    if (sdk_error != mkernel_internal_succ)
    {
        ezlog_w(TAG_CORE, "aes_gcm_128_enc_padding err!");
        ezos_free(out_buf);
        return sdk_error;
    }

    sdk_error = aes_gcm_128_dec_padding(aes_encrypt_key, out_buf, out_buf_len, temp, &templen, output_tag_buf, tag_buf_len);
    ezos_memcpy(output_buf, out_buf, out_buf_len);
    *output_buf_len = out_buf_len;
    if (out_buf)
    {
        ezos_free(out_buf);
    }
    ezlog_d(TAG_CORE, "aes_128_encrypt_pubkey end!");
    return mkernel_internal_succ;
}

static mkernel_internal_error append_encrypt_data(lbs_packet *send_data_buf, unsigned char *encrypt_data, EZDEV_SDK_UINT32 encrypt_data_len,
                                                  unsigned char *tag_buf, EZDEV_SDK_UINT32 tag_buf_len)
{
    mkernel_internal_error sdk_error = mkernel_internal_succ;
    do
    {
        if (send_data_buf == NULL || encrypt_data == NULL)
        {
            sdk_error = mkernel_internal_input_param_invalid;
            break;
        }

        ezos_memcpy(send_data_buf->payload_buf + send_data_buf->payload_buf_off, tag_buf, tag_buf_len);
        send_data_buf->payload_buf_off += tag_buf_len;

        ezos_memcpy(send_data_buf->payload_buf + send_data_buf->payload_buf_off, encrypt_data, encrypt_data_len);
        send_data_buf->payload_buf_off += encrypt_data_len;

    } while (0);

    return sdk_error;
}
static mkernel_internal_error send_authentication_i(ezdev_sdk_kernel *sdk_kernel, lbs_affair *authi_affair, void *ctx_client)
{
    mkernel_internal_error sdk_error = mkernel_internal_succ;
    unsigned char pubkey[128] = {0};
    unsigned char pubkeyhex[256] = {0};
    EZDEV_SDK_UINT32 pubkey_len = 0;
    unsigned char encrypt_data[256] = {0};
    EZDEV_SDK_UINT32 encrypt_data_len = 0;
    unsigned char tag_data[LBS_AUTH_TAG_LEN] = {0};
    EZDEV_SDK_UINT32 tag_data_len = LBS_AUTH_TAG_LEN;
    do
    {
        sdk_error = common_serialize(&authi_affair->global_out_packet, DEV_PROTOCOL_LBS_FORM_VERSION, DEV_PROTOCOL_LBS_LOW_TYPE_VERSION_LICENSE,
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
            sdk_error = ezdev_generate_publickey((mbedtls_ecdh_context *)ctx_client, pubkey, &pubkey_len);
            if (sdk_error != mkernel_internal_succ)
            {
                ezlog_d(TAG_CORE, "generate_public_key error!");
                break;
            }
            md5_hexdump(pubkey, pubkey_len, 1, pubkeyhex);

            sdk_error = aes_128_encrypt_pubkey(authi_affair, pubkey, pubkey_len, encrypt_data, &encrypt_data_len, tag_data, tag_data_len);
            if (sdk_error != mkernel_internal_succ)
            {
                ezlog_d(TAG_CORE, "aes_128_encrypt_pubkey error!");
                break;
            }
            sdk_error = append_encrypt_data(&authi_affair->global_out_packet, encrypt_data, encrypt_data_len, tag_data, tag_data_len);
            if (sdk_error != mkernel_internal_succ)
            {
                ezlog_d(TAG_CORE, "append_encrypt_data error!");
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

    ezlog_d(TAG_CORE, "send_authentication_i complete, code:%d", sdk_error);
    return sdk_error;
}

static mkernel_internal_error aes_128_decrypt_peer_pubkey(lbs_affair *authi_affair, EZDEV_SDK_UINT32 remain_len, unsigned char *out_buf, EZDEV_SDK_UINT32 *out_len,
                                                          unsigned char *intput_tag_buf, EZDEV_SDK_UINT32 tag_buf_len)
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
    ezlog_v(TAG_CORE, "recv_plat_key_len: is :%d ", recv_plat_key_len);

    ezos_memcpy(aes_encrypt_key, authi_affair->share_key, 16);

    sdk_error = aes_gcm_128_dec_padding(aes_encrypt_key, authi_affair->global_in_packet.payload_buf + authi_affair->global_in_packet.payload_buf_off, recv_plat_key_len,
                                        out_buf, &peer_pubkey_len, intput_tag_buf, tag_buf_len);
    if (sdk_error != mkernel_internal_succ)
    {
        ezlog_e(TAG_CORE, "aes_128_decrypt_peer_pubkey: aes_gcm_128_dec_padding error :%d", sdk_error);
        return sdk_error;
    }

    *out_len = peer_pubkey_len;

    return sdk_error;
}

/*****************************************************************/

/********************************************************************/
/****************************Authentication_II********************************/
/********************************************************************/
static mkernel_internal_error parse_authentication_ii(ezdev_sdk_kernel *sdk_kernel, lbs_affair *authi_affair, EZDEV_SDK_UINT32 remain_len, void *ctx_client)
{
    mkernel_internal_error sdk_error = mkernel_internal_succ;
    char result_code = 0;
    unsigned char peer_pubkey[LBS_AUTH_INFO_LEN] = {0};
    EZDEV_SDK_UINT32 peer_pubkey_len = 0;
    unsigned char input_tag_buf[LBS_AUTH_TAG_LEN] = {0};
    EZDEV_SDK_UINT32 tag_buf_len = LBS_AUTH_TAG_LEN;
    unsigned char masterkey[48] = {0};
    char maskerkeykey[32] = {0};
    EZDEV_SDK_UINT32 masterkey_len = 0;
    unsigned char md5_masterkey[16] = {0};
    int nIndex = 0;
    unsigned char in_packet_flag = 0;
    unsigned char out_packet_flag = 0;

    unsigned char in_packet_auth_type = 0;
    unsigned char out_packet_auth_type = 0;
    EZDEV_SDK_UINT8 var_head_buf_off = 0;
    EZDEV_SDK_UINT8 dev_support_auth_type = 0;
    authi_affair->global_in_packet.payload_buf_off += 3;

    ezlog_d(TAG_CORE, "parse_authentication_ii remain_len:%d", remain_len);

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

    ezos_memcpy(&result_code, authi_affair->global_in_packet.payload_buf + authi_affair->global_in_packet.payload_buf_off, 1);
    authi_affair->global_in_packet.payload_buf_off++;

    if (result_code != 0)
    {
        ezlog_d(TAG_CORE, "parse_authentication_ii platform return error code:%d", result_code);
        return mkernel_internal_platform_error + result_code;
    }

    switch (sdk_kernel->dev_cur_auth_type)
    {
    case sdk_dev_auth_protocol_ecdh:
        ezos_memcpy(input_tag_buf, authi_affair->global_in_packet.payload_buf + authi_affair->global_in_packet.payload_buf_off, tag_buf_len);
        authi_affair->global_in_packet.payload_buf_off += tag_buf_len;

        sdk_error = aes_128_decrypt_peer_pubkey(authi_affair, remain_len, peer_pubkey, &peer_pubkey_len, input_tag_buf, tag_buf_len);
        if (sdk_error != mkernel_internal_succ)
        {
            ezlog_e(TAG_CORE, "aes_128_decrypt_peer_pubkey");
            return sdk_error;
        }
        ezos_memset(masterkey, 0, 48);
        sdk_error = ezdev_generate_masterkey((mbedtls_ecdh_context *)ctx_client, peer_pubkey, peer_pubkey_len, masterkey, &masterkey_len);
        if (sdk_error != mkernel_internal_succ)
        {
            ezlog_e(TAG_CORE, "generate_master_key error");
            return sdk_error;
        }
        ezos_memset(md5_masterkey, 0, 16);
        mbedtls_md5(masterkey, masterkey_len, md5_masterkey);
        break;
    default:
        return mkernel_internal_internal_err;
        break;
    }

    ezos_memset(authi_affair->master_key, 0, ezdev_sdk_masterkey_len);

    for (; nIndex < 8; nIndex++)
    {
        sprintf(maskerkeykey + nIndex * 2, "%02X", md5_masterkey[nIndex]);

        ezos_memcpy(authi_affair->master_key, maskerkeykey, ezdev_sdk_masterkey_len);
    }
    return sdk_error;
}

static mkernel_internal_error wait_authentication_ii(ezdev_sdk_kernel *sdk_kernel, lbs_affair *authi_affair, void *ctx_client)
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
    ezos_memset(sign_input, 0, ezdev_sdk_devserial_maxlen);
    ezos_memcpy(sign_input, authi_affair->dev_subserial, ezos_strlen(authi_affair->dev_subserial));
    sdk_error = digital_sign_serialize_sha384(&authi_affair->global_out_packet, sign_input, ezos_strlen(authi_affair->dev_subserial), authi_affair->master_key, ezdev_sdk_masterkey_len);
    return sdk_error;
}

static mkernel_internal_error send_sessionkey_req_serialize(lbs_affair *authi_affair)
{
    mkernel_internal_error sdk_error = mkernel_internal_succ;

    *(EZDEV_SDK_UINT8 *)(authi_affair->global_out_packet.payload_buf + authi_affair->global_out_packet.payload_buf_off) = ezdev_sdk_devid_len;
    authi_affair->global_out_packet.payload_buf_off++;

    ezos_memcpy(authi_affair->global_out_packet.payload_buf + authi_affair->global_out_packet.payload_buf_off, authi_affair->dev_id, ezdev_sdk_devid_len);
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
    ezlog_d(TAG_CORE, "send_authentication_iii complete, code:%d", sdk_error);
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
    ezlog_d(TAG_CORE, "send_authentication_iii complete, code:%d", sdk_error);
    return mkernel_internal_succ;
}

static mkernel_internal_error parse_authentication_create_dev_id(lbs_affair *authi_affair, EZDEV_SDK_UINT32 remain_len)
{
    mkernel_internal_error sdk_error = mkernel_internal_succ;
    char result_code = 0;
    EZDEV_SDK_UINT8 en_sessionkey_len = 0;
    unsigned char sessionkey[32] = {0};
    EZDEV_SDK_UINT32 sessionkey_len = 0;

    EZDEV_SDK_UINT8 en_dev_id_len = 0;
    unsigned char dev_id[48] = {0};
    EZDEV_SDK_UINT32 dev_id_len = 0;
    unsigned char sign_input[LBS_AUTH_INFO_LEN];

    unsigned char devid_tag_buf[LBS_AUTH_TAG_LEN] = {0};
    EZDEV_SDK_UINT32 devid_tag_buf_len = LBS_AUTH_TAG_LEN;
    unsigned char sessionkey_tag_buf[LBS_AUTH_TAG_LEN] = {0};
    EZDEV_SDK_UINT32 sessionkey_tag_buf_len = LBS_AUTH_TAG_LEN;

    authi_affair->global_in_packet.payload_buf_off += 3;
    ezos_memcpy(&result_code, authi_affair->global_in_packet.payload_buf + authi_affair->global_in_packet.payload_buf_off, 1);
    authi_affair->global_in_packet.payload_buf_off++;

    if (result_code != 0)
    {
        ezlog_d(TAG_CORE, "parse_authentication_iv_create_devid platform return err code:%d", result_code);
        return mkernel_internal_platform_error + result_code;
    }
    ezos_memcpy(devid_tag_buf, authi_affair->global_in_packet.payload_buf + authi_affair->global_in_packet.payload_buf_off, devid_tag_buf_len);
    authi_affair->global_in_packet.payload_buf_off += devid_tag_buf_len;
    en_dev_id_len = *(authi_affair->global_in_packet.payload_buf + authi_affair->global_in_packet.payload_buf_off);
    authi_affair->global_in_packet.payload_buf_off++;
    if (en_dev_id_len != 32)
    {
        ezlog_d(TAG_CORE, "parse_authentication_create_dev_id en_dev_id_len is not 32");
        return mkernel_internal_platform_appoint_error;
    }
    ezlog_d(TAG_CORE, "get en_dev_id_len :%d", en_dev_id_len);
    sdk_error = aes_gcm_128_dec_padding(authi_affair->master_key,
                                        authi_affair->global_in_packet.payload_buf + authi_affair->global_in_packet.payload_buf_off, en_dev_id_len,
                                        dev_id, &dev_id_len, devid_tag_buf, devid_tag_buf_len);
    if (sdk_error != mkernel_internal_succ)
    {
        ezlog_d(TAG_CORE, "aes_gcm_128_dec_padding err!");
        return sdk_error;
    }
    if (dev_id_len != 32)
    {
        ezlog_d(TAG_CORE, "parse_authentication_iv sessionkey_len is not 32");
        return mkernel_internal_platform_appoint_error;
    }
    ezos_memcpy(authi_affair->dev_id, dev_id, dev_id_len);
    authi_affair->global_in_packet.payload_buf_off += en_dev_id_len;
    ezos_memcpy(sessionkey_tag_buf, authi_affair->global_in_packet.payload_buf + authi_affair->global_in_packet.payload_buf_off, sessionkey_tag_buf_len);
    authi_affair->global_in_packet.payload_buf_off += sessionkey_tag_buf_len;
    en_sessionkey_len = *(authi_affair->global_in_packet.payload_buf + authi_affair->global_in_packet.payload_buf_off);
    authi_affair->global_in_packet.payload_buf_off++;
    if (en_sessionkey_len != 16)
    {
        ezlog_d(TAG_CORE, "parse_authentication_createdevid_iv en_sessionkey_len is not 32");
        return mkernel_internal_platform_appoint_error;
    }
    ezlog_d(TAG_CORE, "get en_sessionkey_len :%d", en_sessionkey_len);
    sdk_error = aes_gcm_128_dec_padding(authi_affair->master_key,
                                        authi_affair->global_in_packet.payload_buf + authi_affair->global_in_packet.payload_buf_off, en_sessionkey_len,
                                        sessionkey, &sessionkey_len, sessionkey_tag_buf, sessionkey_tag_buf_len);
    if (sdk_error != mkernel_internal_succ)
    {
        ezlog_d(TAG_CORE, "aes_gcm_128_dec_padding err!");
        return sdk_error;
    }
    if (sessionkey_len != 16)
    {
        ezlog_d(TAG_CORE, "parse_authentication_iv sessionkey_len is not 16");
        return mkernel_internal_platform_appoint_error;
    }
    ezos_memcpy(authi_affair->session_key, sessionkey, sessionkey_len);
    authi_affair->global_in_packet.payload_buf_off += en_sessionkey_len;
    ezos_memset(sign_input, 0, LBS_AUTH_INFO_LEN);
    ezos_memcpy(sign_input, authi_affair->dev_subserial, ezos_strlen(authi_affair->dev_subserial));
    sdk_error = digital_sign_serialize_and_check_sha384(authi_affair->global_in_packet.payload_buf + authi_affair->global_in_packet.payload_buf_off,
                                                        remain_len - authi_affair->global_in_packet.payload_buf_off, sign_input, ezos_strlen(authi_affair->dev_subserial), authi_affair->master_key, 16);
    if (sdk_error != mkernel_internal_succ)
    {
        ezlog_e(TAG_CORE, "digital_sign_serialize_and_check_sha384 err!, code:%d", sdk_error);
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
    dev_serial_len = ezos_strlen(auth_affair->dev_subserial);

    *(EZDEV_SDK_UINT8 *)(auth_affair->global_out_packet.payload_buf + auth_affair->global_out_packet.payload_buf_off) = dev_serial_len;
    auth_affair->global_out_packet.payload_buf_off++;

    ezos_memcpy(auth_affair->global_out_packet.payload_buf + auth_affair->global_out_packet.payload_buf_off, auth_affair->dev_subserial, dev_serial_len);
    auth_affair->global_out_packet.payload_buf_off += dev_serial_len;

    *(EZDEV_SDK_UINT8 *)(auth_affair->global_out_packet.payload_buf + auth_affair->global_out_packet.payload_buf_off) = ezdev_sdk_devid_len;
    auth_affair->global_out_packet.payload_buf_off++;

    ezos_memcpy(auth_affair->global_out_packet.payload_buf + auth_affair->global_out_packet.payload_buf_off, auth_affair->dev_id, ezdev_sdk_devid_len);
    auth_affair->global_out_packet.payload_buf_off += ezdev_sdk_devid_len;

    ezos_memset(en_src, 0, 16);
    ezos_memset(en_dst, 0, 16);
    ezos_memset(en_src, auth_affair->random_1, 1);

    sdk_error = aes_cbc_128_enc_padding(auth_affair->master_key, (unsigned char *)en_src, 1, 16, en_dst, &en_dst_len);
    if (sdk_error != mkernel_internal_succ)
    {
        return sdk_error;
    }

    ezos_memcpy(auth_affair->global_out_packet.payload_buf + auth_affair->global_out_packet.payload_buf_off, en_dst, en_dst_len);
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
    ezlog_d(TAG_CORE, "send_refreshsessionkey_i complete, code:%d", sdk_error);
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
    ezos_memcpy(&result_code, authi_affair->global_in_packet.payload_buf + authi_affair->global_in_packet.payload_buf_off, 1);
    authi_affair->global_in_packet.payload_buf_off++;

    if (result_code != 0)
    {
        ezlog_d(TAG_CORE, "parse_refreshsessionkey_ii platform return error code:%d", result_code);
        return mkernel_internal_platform_error + result_code;
    }
    en_sessionkey_len = remain_len - authi_affair->global_in_packet.payload_buf_off;
    if (en_sessionkey_len != 32)
    {
        ezlog_d(TAG_CORE, "parse_refreshsessionkey_ii en_sessionkey_len is not 32");
        return mkernel_internal_platform_appoint_error;
    }

    sdk_error = aes_cbc_128_dec_padding(authi_affair->master_key, authi_affair->global_in_packet.payload_buf + authi_affair->global_in_packet.payload_buf_off, en_sessionkey_len, ase_dst, &ase_dst_len);
    if (sdk_error != mkernel_internal_succ)
    {
        ezlog_d(TAG_CORE, "parse_refreshsessionkey_ii aes_cbc_128_dec error :%d", sdk_error);
        return sdk_error;
    }

    authi_affair->global_in_packet.payload_buf_off += en_sessionkey_len;

    return_random_1 = ase_dst[0];
    if (return_random_1 != authi_affair->random_1)
    {
        ezlog_d(TAG_CORE, "parse_refreshsessionkey_ii random_1 error, my:%d, get:%d", authi_affair->random_1, return_random_1);
        return mkernel_internal_random1_check_error;
    }

    authi_affair->random_2 = ase_dst[1];
    ezos_memcpy(authi_affair->session_key, ase_dst + 2, ezdev_sdk_sessionkey_len);

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
    ezlog_d(TAG_CORE, "parse_refreshsessionkey_ii end");

    return sdk_error;
}

mkernel_internal_error refreshsessionkey_iii_serialize(lbs_affair *auth_affair)
{
    mkernel_internal_error sdk_error = mkernel_internal_succ;
    unsigned char en_src[16];
    unsigned char en_dst[16];
    EZDEV_SDK_UINT32 en_dst_len = 0;

    ezos_memset(en_src, 0, 16);
    ezos_memset(en_dst, 0, 16);
    ezos_memset(en_src, auth_affair->random_2, 1);

    sdk_error = aes_cbc_128_enc_padding(auth_affair->master_key, (unsigned char *)en_src, 1, 16, en_dst, &en_dst_len);
    if (sdk_error != mkernel_internal_succ)
    {
        return sdk_error;
    }

    ezos_memcpy(auth_affair->global_out_packet.payload_buf + auth_affair->global_out_packet.payload_buf_off, en_dst, en_dst_len);
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
    ezlog_d(TAG_CORE, "send_refreshsessionkey_iii complete, code:%d", sdk_error);
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
    ezlog_d(TAG_CORE, "send_stun_refreshsessionkey_iii complete, code:%d", sdk_error);
    return mkernel_internal_succ;
}
static mkernel_internal_error crypto_data_req_das_serialize(lbs_affair *auth_affair)
{
    mkernel_internal_error sdk_error = mkernel_internal_succ;
    cJSON *pJsonRoot = NULL;

    char *json_buf = NULL;
    EZDEV_SDK_UINT32 json_len = 0;
    char *json_buf_padding = NULL;
    EZDEV_SDK_UINT32 json_len_padding = 0;
    unsigned char *out_buf = NULL;
    EZDEV_SDK_UINT32 out_buf_len = 0;

    do
    {
        pJsonRoot = cJSON_CreateObject();
        if (pJsonRoot == NULL)
        {
            sdk_error = mkernel_internal_malloc_error;
            break;
        }

        cJSON_AddStringToObject(pJsonRoot, "DevSerial", auth_affair->dev_subserial);
        cJSON_AddStringToObject(pJsonRoot, "Type", "DAS");
        cJSON_AddNumberToObject(pJsonRoot, "Mode", auth_affair->dev_access_mode);

        json_buf = cJSON_PrintBuffered(pJsonRoot, LBS_AUTH_JSON_DEFAULT_LEN, 0);
        if (json_buf == NULL)
        {
            sdk_error = mkernel_internal_json_format_error;
            break;
        }
        json_len = ezos_strlen(json_buf);
        json_len_padding = calculate_padding_len(json_len);
        if (LBS_AUTH_JSON_DEFAULT_LEN > json_len_padding)
        {
            json_buf_padding = json_buf;
            ezos_memset(json_buf + json_len, 0, LBS_AUTH_JSON_DEFAULT_LEN - json_len);
        }
        else
        {
            json_buf_padding = ezos_malloc(json_len_padding);
            if (NULL == json_buf_padding)
            {
                sdk_error = mkernel_internal_malloc_error;
                break;
            }
            ezos_memset(json_buf_padding, 0, json_len_padding);
            ezos_memcpy(json_buf_padding, json_buf, json_len);

            ezos_free(json_buf);
            json_buf = NULL;
        }

        out_buf = ezos_malloc(json_len_padding);
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

        ezos_memcpy(auth_affair->global_out_packet.payload_buf + auth_affair->global_out_packet.payload_buf_off, out_buf, out_buf_len);
        auth_affair->global_out_packet.payload_buf_off += out_buf_len;

    } while (0);

    if (pJsonRoot != NULL)
    {
        cJSON_Delete(pJsonRoot);
        pJsonRoot = NULL;
    }
    if (json_buf_padding != NULL)
    {
        ezos_free(json_buf_padding);
        json_buf_padding = NULL;
    }
    if (out_buf != NULL)
    {
        ezos_free(out_buf);
        out_buf = NULL;
    }

    return sdk_error;
}

static mkernel_internal_error crypto_data_req_stun_serialize(lbs_affair *auth_affair)
{
    mkernel_internal_error sdk_error = mkernel_internal_succ;

    cJSON *pJsonRoot = NULL;

    char *json_buf = NULL;
    EZDEV_SDK_UINT32 json_len = 0;
    char *json_buf_padding = NULL;
    EZDEV_SDK_UINT32 json_len_padding = 0;

    unsigned char *en_dst = NULL;
    EZDEV_SDK_UINT32 en_dst_len = 0;

    do
    {
        pJsonRoot = cJSON_CreateObject();
        if (pJsonRoot == NULL)
        {
            sdk_error = mkernel_internal_malloc_error;
            break;
        }

        cJSON_AddStringToObject(pJsonRoot, "DevSerial", auth_affair->dev_subserial);
        cJSON_AddStringToObject(pJsonRoot, "Type", "STUN");

        json_buf = cJSON_PrintBuffered(pJsonRoot, LBS_AUTH_JSON_DEFAULT_LEN, 0);
        if (json_buf == NULL)
        {
            sdk_error = mkernel_internal_json_format_error;
            break;
        }

        json_len = ezos_strlen(json_buf);
        json_len_padding = calculate_padding_len(json_len);
        if (LBS_AUTH_JSON_DEFAULT_LEN > json_len_padding)
        {
            json_buf_padding = json_buf;
            ezos_memset(json_buf + json_len, 0, LBS_AUTH_JSON_DEFAULT_LEN - json_len);
        }
        else
        {
            json_buf_padding = ezos_malloc(json_len_padding);
            if (NULL == json_buf_padding)
            {
                sdk_error = mkernel_internal_malloc_error;
                break;
            }
            ezos_memset(json_buf_padding, 0, json_len_padding);
            ezos_memcpy(json_buf_padding, json_buf, json_len);

            ezos_free(json_buf);
            json_buf = NULL;
        }

        en_dst = ezos_malloc(json_len_padding);
        if (NULL == en_dst)
        {
            sdk_error = mkernel_internal_malloc_error;
            break;
        }
        ezos_memset(en_dst, 0, json_len_padding);
        en_dst_len = json_len_padding;

        sdk_error = aes_cbc_128_enc_padding(auth_affair->session_key, (unsigned char *)json_buf_padding, json_len, json_len_padding, en_dst, &en_dst_len);
        if (sdk_error != mkernel_internal_succ)
        {
            break;
        }

        ezos_memcpy(auth_affair->global_out_packet.payload_buf + auth_affair->global_out_packet.payload_buf_off, en_dst, en_dst_len);
        auth_affair->global_out_packet.payload_buf_off += en_dst_len;
    } while (0);

    if (pJsonRoot != NULL)
    {
        cJSON_Delete(pJsonRoot);
        pJsonRoot = NULL;
    }
    if (json_buf_padding != NULL)
    {
        ezos_free(json_buf_padding);
        json_buf_padding = NULL;
    }
    if (en_dst != NULL)
    {
        ezos_free(en_dst);
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
    ezlog_d(TAG_CORE, "send_crypto_data_req complete, code:%d", sdk_error);
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
    ezos_memcpy(&result_code, authi_affair->global_in_packet.payload_buf + authi_affair->global_in_packet.payload_buf_off, 1);
    authi_affair->global_in_packet.payload_buf_off++;

    if (result_code != 0)
    {
        ezlog_d(TAG_CORE, "parse_crypto_data_rsp_das platform return error code:%d", result_code);
        return mkernel_internal_platform_error + result_code;
    }

    en_src_len = remain_len - authi_affair->global_in_packet.payload_buf_off;

    de_dst = (unsigned char *)ezos_malloc(en_src_len);
    if (de_dst == NULL)
    {
        return mkernel_internal_malloc_error;
    }
    ezos_memset(de_dst, 0, en_src_len);

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
        ezos_free(de_dst);
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
/*********************************************************************************/

static mkernel_internal_error lbs_connect(ezdev_sdk_kernel *sdk_kernel, lbs_affair *authi_affair)
{

    mkernel_internal_error result_ = mkernel_internal_succ;
    char szRealIp[ezdev_sdk_ip_max_len] = {0};
    do
    {
        authi_affair->socket_fd = net_create(NULL);
        if (authi_affair->socket_fd == -1)
        {
            ezlog_e(TAG_CORE, "lbs_connect net_work_create error");
            result_ = mkernel_internal_net_connect_error;
            break;
        }

        result_ = net_connect(authi_affair->socket_fd, sdk_kernel->server_info.server_name, sdk_kernel->server_info.server_port, 5 * 1000, szRealIp);
        if (result_ != mkernel_internal_succ)
        {
            ezlog_w(TAG_CORE, "lbs_connect net_work_connect host error, server:%s, ip:%s, port:%d", sdk_kernel->server_info.server_name, sdk_kernel->server_info.server_ip, sdk_kernel->server_info.server_port);

            net_disconnect(authi_affair->socket_fd);
            authi_affair->socket_fd = -1;

            authi_affair->socket_fd = net_create(NULL);
            if (authi_affair->socket_fd == -1)
            {
                ezlog_e(TAG_CORE, "lbs_connect net_work_create error");
                result_ = mkernel_internal_net_connect_error;
                break;
            }

            if (0 == ezos_strlen(sdk_kernel->server_info.server_ip))
            {
                break;
            }

            result_ = net_connect(authi_affair->socket_fd, sdk_kernel->server_info.server_ip, sdk_kernel->server_info.server_port, 5 * 1000, szRealIp);
            if (result_ != mkernel_internal_succ)
            {
                ezlog_e(TAG_CORE, "lbs_connect net_work_connect ip error, server:%s, ip:%s, port:%d", sdk_kernel->server_info.server_name, sdk_kernel->server_info.server_ip, sdk_kernel->server_info.server_port);
                if (mkernel_internal_net_gethostbyname_error != result_)
                {
                    result_ = mkernel_internal_net_connect_error;
                }

                break;
            }
        }

        ezlog_i(TAG_CORE, "lbs_connect suc, host:%s, ip:%s, parseip:%s", sdk_kernel->server_info.server_name, sdk_kernel->server_info.server_ip, szRealIp);

        ezos_memset(sdk_kernel->server_info.server_ip, 0, ezdev_sdk_ip_max_len);
        ezos_strncpy(sdk_kernel->server_info.server_ip, szRealIp, ezdev_sdk_ip_max_len - 1);
    } while (0);

    return result_;
}

static mkernel_internal_error lbs_close(ezdev_sdk_kernel *sdk_kernel, lbs_affair *authi_affair)
{
    if (authi_affair->socket_fd != -1)
    {
        net_disconnect(authi_affair->socket_fd);

        authi_affair->socket_fd = -1;
    }
    return mkernel_internal_succ;
}

mkernel_internal_error lbs_redirect_with_auth(ezdev_sdk_kernel *sdk_kernel, EZDEV_SDK_UINT8 nUpper)
{
    mkernel_internal_error sdk_error = mkernel_internal_succ;
    lbs_affair auth_redirect;
    das_info revc_das_info;
    void *ctx_client = NULL;

    ezos_memset(&auth_redirect, 0, sizeof(auth_redirect));
    ezos_memset(&revc_das_info, 0, sizeof(revc_das_info));

    switch (sdk_kernel->dev_cur_auth_type)
    {
    case sdk_dev_auth_protocol_ecdh:
        ctx_client = (mbedtls_ecdh_context *)ezos_malloc(sizeof(mbedtls_ecdh_context));
        if (ctx_client == NULL)
        {
            return mkernel_internal_mem_lack;
        }
        mbedtls_ecdh_init((mbedtls_ecdh_context *)ctx_client);
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
        mbedtls_ecdh_free((mbedtls_ecdh_context *)ctx_client);
        if (ctx_client != NULL)
        {
            ezos_free(ctx_client);
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
    void *ctx_client = NULL;
    das_info revc_das_info;
    ezos_memset(&auth_redirect, 0, sizeof(auth_redirect));
    ezos_memset(&revc_das_info, 0, sizeof(revc_das_info));

    switch (sdk_kernel->dev_cur_auth_type)
    {
    case sdk_dev_auth_protocol_ecdh:
        ctx_client = (mbedtls_ecdh_context *)ezos_malloc(sizeof(mbedtls_ecdh_context));
        if (ctx_client == NULL)
        {
            return mkernel_internal_mem_lack;
        }
        mbedtls_ecdh_init((mbedtls_ecdh_context *)ctx_client);
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
        mbedtls_ecdh_free((mbedtls_ecdh_context *)ctx_client);
        if (ctx_client != NULL)
        {
            ezos_free(ctx_client);
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

    ezos_memset(&auth_redirect, 0, sizeof(auth_redirect));
    ezos_memset(&revc_das_info, 0, sizeof(revc_das_info));

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

static mkernel_internal_error json_parse_das_server_info(const char *jsonstring, das_info *das_server_info)
{
    mkernel_internal_error sdk_error = mkernel_internal_succ;
    cJSON *json_item = NULL;
    cJSON *address_json_item = NULL;
    cJSON *port_json_item = NULL;
    cJSON *udpport_json_item = NULL;
    cJSON *domain_json_item = NULL;
    cJSON *serverid_json_item = NULL;
    cJSON *dasinfo_json_item = NULL;
    cJSON *das_json_item = NULL;

    do
    {
        json_item = cJSON_Parse((const char *)jsonstring);
        if (json_item == NULL)
        {
            sdk_error = mkernel_internal_json_parse_error;
            break;
        }

        das_json_item = cJSON_GetObjectItem(json_item, "Type");
        if (das_json_item == NULL)
        {
            sdk_error = mkernel_internal_json_parse_error;
            break;
        }
        if (das_json_item->type != cJSON_String || das_json_item->valuestring == NULL)
        {
            sdk_error = mkernel_internal_json_parse_error;
            break;
        }
        if (ezos_strcmp(das_json_item->valuestring, "DAS") != 0)
        {
            sdk_error = mkernel_internal_get_error_json;
            break;
        }

        dasinfo_json_item = cJSON_GetObjectItem(json_item, "DasInfo");
        if (dasinfo_json_item == NULL)
        {
            sdk_error = mkernel_internal_get_error_json;
            break;
        }

        address_json_item = cJSON_GetObjectItem(dasinfo_json_item, "Address");
        port_json_item = cJSON_GetObjectItem(dasinfo_json_item, "Port");
        udpport_json_item = cJSON_GetObjectItem(dasinfo_json_item, "UdpPort");
        domain_json_item = cJSON_GetObjectItem(dasinfo_json_item, "Domain");
        serverid_json_item = cJSON_GetObjectItem(dasinfo_json_item, "ServerID");
        if (NULL == serverid_json_item || NULL == port_json_item || NULL == domain_json_item || NULL == serverid_json_item || NULL == udpport_json_item)
        {
            sdk_error = mkernel_internal_get_error_json;
            break;
        }

        if (port_json_item->type != cJSON_Number || udpport_json_item->type != cJSON_Number ||
            serverid_json_item->type != cJSON_String || serverid_json_item->valuestring == NULL ||
            domain_json_item->type != cJSON_String || domain_json_item->valuestring == NULL ||
            address_json_item->type != cJSON_String || address_json_item->valuestring == NULL)
        {
            sdk_error = mkernel_internal_get_error_json;
            break;
        }

        if (ezos_strlen(address_json_item->valuestring) >= ezdev_sdk_ip_max_len)
        {
            ezlog_d(TAG_CORE, "parse_crypto_data_rsp_das Address >= 64");
            sdk_error = mkernel_internal_platform_appoint_error;
            break;
        }
        ezos_strncpy(das_server_info->das_address, address_json_item->valuestring, ezos_strlen(address_json_item->valuestring));

        if (ezos_strlen(domain_json_item->valuestring) >= ezdev_sdk_ip_max_len)
        {
            ezlog_d(TAG_CORE, "parse_crypto_data_rsp_das domain >= 64");
            sdk_error = mkernel_internal_platform_appoint_error;
            break;
        }
        ezos_strncpy(das_server_info->das_domain, domain_json_item->valuestring, ezos_strlen(domain_json_item->valuestring));

        if (ezos_strlen(serverid_json_item->valuestring) >= ezdev_sdk_name_len)
        {
            ezlog_d(TAG_CORE, "parse_crypto_data_rsp_das serverid >= 64");
            sdk_error = mkernel_internal_platform_appoint_error;
            break;
        }
        ezos_strncpy(das_server_info->das_serverid, serverid_json_item->valuestring, ezos_strlen(serverid_json_item->valuestring));

        das_server_info->das_port = port_json_item->valueint;
        das_server_info->das_udp_port = udpport_json_item->valueint;
        ezlog_d(TAG_CORE, "das address:%s,port:%d", das_server_info->das_address, das_server_info->das_port);
    } while (0);

    if (NULL != json_item)
    {
        cJSON_Delete(json_item);
        json_item = NULL;
    }

    return sdk_error;
}