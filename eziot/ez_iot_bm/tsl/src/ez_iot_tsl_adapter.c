#include "ez_iot_tsl_def.h"
#include "ez_iot_tsl.h"
#include "ez_iot_tsl_adapter.h"
#include "ez_iot_tsl_legality.h"
#include "ez_iot_tsl_schema.h"
#include "ez_iot_core_def.h"
#include "ez_iot_core_lowlvl.h"
#include "ezos.h"
#include "uuid.h"
#include "cJSON.h"
#include "webclient.h"
#include "ezlist.h"
#include "misc.h"
#include "s2j.h"
#include "mbedtls/md5.h"
#include "mbedtls/base64.h"

#define MAX_RECV_BUFFER 128
#define PARTITION_PROFILE_LABEL "log"
#define TSLMAP_JSON_KEY_SN "sn"
#define TSLMAP_JSON_KEY_HANDLE "handle"
#define EZ_TSL_KEY_TSL_PREFIX "tslpf"

typedef struct
{
    ez_char_t dev_sn[48];
    ez_char_t url[260];
    ez_char_t md5[33];
    ez_int32_t expire;
} ez_tsl_download_info_t;

typedef enum
{
    status_done = 0,        ///< 描述文件完成加载
    status_need_update = 1, ///< 需要更新描述文件
    status_query = 2,       ///< 正在查询升级包信息
    status_loading = 3,     ///< 正在下载
} dev_stauts_e;

typedef struct
{
    ez_char_t dev_sn[48];
    ez_char_t dev_type[24];
    ez_char_t dev_fw_ver[36];
    dev_stauts_e status;
    ezos_timespec_t timer;
    void *download_info;
} dev_info_t;

typedef struct
{
    ez_char_t sn[48];
    ez_char_t handle[32];
} tslmap_metadata_t;

// *tsl things to dev cbs
ez_tsl_callbacks_t g_tsl_things_cbs = {0};

static ez_int32_t g_profile_download_thread_running = 0;

static ezlist_t *g_tsl_dev_info_list = NULL;
static ez_void_t *g_profile_mutex = NULL;
static ezlist_t *g_capacities_list = NULL;
static ez_void_t *g_capacities_mutex = NULL;
static ez_bool_t g_is_online = ez_false;

/**
 * @brief 增加功能描述文件引用计数
 * 
 * @param dev_info 设备信息
 * @return ez_true 成功
 * @return ez_false 失败，找不到对应功能描述文件
 */
static ez_bool_t tsl_adapter_ref_add(ez_tsl_devinfo_t *dev_info);

/**
 * @brief 减少功能描述文件引用计数，计数为0则销毁描述文件对象
 * 
 * @param dev_info 设备信息
 * @return ez_true 成功
 * @return ez_false 失败，找不到功能点或者引用计数为0
 */
static ez_bool_t tsl_adapter_ref_del(ez_tsl_devinfo_t *dev_info);

/**
 * @brief 从加载功能描述文件
 * 
 * @param dev_info 设备信息
 * @return ez_true 成功
 * @return ez_false 失败，找不到对应功能描述文件或文件损坏
 */
static ez_bool_t tsl_adapter_profile_parse(ez_tsl_devinfo_t *dev_info, ez_char_t *profile);

/**
 * @brief 从本地加载功能描述文件
 * 
 * @param dev_info 设备信息
 * @return ez_true 成功
 * @return ez_false 失败，找不到对应功能描述文件或文件损坏
 */
static ez_bool_t tsl_adapter_profile_load(ez_tsl_devinfo_t *dev_info);

/**
 * @brief 保存功能描述文件
 * 
 * @param dev_info 设备信息
 * @param profile 功能点文件数据，如果为空表示只直接保存map
 * @param length 功能点文件数据长度
 * @return ez_true 成功
 * @return ez_false 失败，找不到对应功能描述文件或文件损坏
 */
static ez_bool_t tsl_adapter_profile_save(ez_tsl_devinfo_t *dev_info, const ez_char_t *profile, ez_int32_t length);

/**
 * @brief 在线下载功能描述文件
 * 
 * @param dev_info 设备信息
 * @return ez_true 成功
 * @return ez_false 失败，资源不足或内部错误
 */
static ez_bool_t tsl_adapter_profile_download(ez_tsl_devinfo_t *dev_info);

/**
 * @brief 设备能力集列表中新增一项
 * 
 * @param capacity 设备能力集
 * @return int32_t 
 */
static ez_bool_t tsl_adapter_capacity_add(ez_iot_tsl_capacity_t *capacity);

/**
 * @brief 根据能力集注册shadow
 * 
 * @param dev_info 设备信息
 * @param capacity 设备能力集
 */
static void tsl_adapter_shadow_inst(ez_tsl_devinfo_t *dev_info, ez_iot_tsl_capacity_t *capacity);

/**
 * @brief 增加一个设备
 * 
 * @param dev_info 设备信息
 * @param need_get 是否需要下载更新
 */
static ez_bool_t tsl_adapter_dev_add(ez_tsl_devinfo_t *dev_info, ez_bool_t need_get);

/**
 * @brief 更新设备状态
 * 
 * @param dev_info 设备信息
 * @param status 设备状态
 */
static void tsl_adapter_dev_update(ez_tsl_devinfo_t *dev_info, dev_stauts_e status);

/**
 * @brief 删除一个设备
 * 
 * @param dev_info 设备信息
 */
static void tsl_adapter_dev_del(ez_tsl_devinfo_t *dev_info);

/**
 * @brief 查询设备信息
 * 
 * @param dev_info 设备信息
 * @param status 设备状态
 */
static ez_bool_t tsl_adapter_dev_find(const int8_t *sn, ez_tsl_devinfo_t *dev_info, dev_stauts_e *status);

/**
 * @brief 启动功能点下载任务
 * 
 * @return ez_true 
 * @return ez_false 
 */
static ez_bool_t tsl_adapter_dl_task_yeild();

/**
 * @brief 设备信息文件查找索引
 * 
 * @param dev_info 
 * @param index 
 */
void devinfo2index(ez_tsl_devinfo_t *dev_info, ez_char_t index[32]);

/**
 * @brief 下载
 * 
 * @param dev_info 下载链接
 * @param buf 下载缓存
 * @param length 描述文件大小
 * @return ez_int32_t 
 */
static ez_int32_t profile_downloading(dev_info_t *dev_info, ez_char_t **buf, ez_int32_t *length);

static ez_void_t tsl_adapter_profile_query_rsp(const ez_char_t *rsp_msg);

typedef ez_void_t (*profile_query_rsp)(const ez_char_t *rsp_msg);
extern ez_err_t ez_iot_base_lowlvl_profile_query(const ez_char_t *req_msg, profile_query_rsp func_rsp);

static ez_char_t *assemble_rsp_code_msg(int code, ez_tsl_value_t *value_out)
{
    cJSON *js_root = NULL;
    ez_char_t *rv = NULL;

    do
    {
        js_root = cJSON_CreateObject();
        if (NULL == js_root)
        {
            ezlog_e(TAG_TSL, "json create object failed.");
            break;
        }

        cJSON_AddNumberToObject(js_root, "code", code);

        if (NULL != value_out)
        {
            cJSON_AddRawToObject(js_root, "data", (char *)value_out->value);
        }

        rv = cJSON_PrintUnformatted(js_root);
        if (NULL == rv)
        {
            ezlog_e(TAG_TSL, "json print error.");
            break;
        }
    } while (ez_false);

    cJSON_Delete(js_root);

    return rv;
}

static ez_tsl_data_type_e json_type_transform_dev(int type)
{
    ez_tsl_data_type_e ret_type = EZ_TSL_DATA_TYPE_MAX;
    switch (type)
    {
    case cJSON_False:
    case cJSON_True:
        ret_type = EZ_TSL_DATA_TYPE_BOOL;
        break;
    case cJSON_NULL:
        break;
    case cJSON_Number:
        ret_type = EZ_TSL_DATA_TYPE_INT;
        break;
    case cJSON_String:
        ret_type = EZ_TSL_DATA_TYPE_STRING;
        break;

    case cJSON_Array:
        ret_type = EZ_TSL_DATA_TYPE_ARRAY;
        break;

    case cJSON_Object:
        ret_type = EZ_TSL_DATA_TYPE_OBJECT;
        break;
    default:
        break;
    }

    return ret_type;
}

static void strip_msg_wrap(void *buf, ez_tsl_value_t *tsl_data)
{
    cJSON *js_msg = NULL;
    cJSON *js_data = NULL;

    do
    {
        js_msg = cJSON_Parse((char *)buf);
        if (NULL == js_msg)
        {
            ezlog_e(TAG_TSL, "msg parse: %s", buf);
            break;
        }

        js_data = cJSON_GetObjectItem(js_msg, "data");
        if (NULL == js_data)
        {
            ezlog_e(TAG_TSL, "msg format error: %s", (char *)buf);
            break;
        }

        tsl_data->type = json_type_transform_dev(js_data->type);

        switch (tsl_data->type)
        {
        case EZ_TSL_DATA_TYPE_BOOL:
            tsl_data->size = sizeof(ez_bool_t);
            tsl_data->value_bool = (ez_bool_t)js_data->valueint;
            break;
        case EZ_TSL_DATA_TYPE_INT:
            tsl_data->value_int = js_data->valueint;
            tsl_data->size = sizeof(js_data->valueint);
            break;
        case EZ_TSL_DATA_TYPE_DOUBLE:
            tsl_data->size = sizeof(js_data->valuedouble);
            tsl_data->value_double = js_data->valuedouble;
            break;
        case EZ_TSL_DATA_TYPE_STRING:
            tsl_data->value = (char *)ezos_malloc(strlen(js_data->valuestring) + 1);
            if (NULL == tsl_data->value)
            {
                break;
            }

            tsl_data->size = ezos_strlen(js_data->valuestring);
            ezos_strcpy(tsl_data->value, js_data->valuestring);
            break;
        case EZ_TSL_DATA_TYPE_ARRAY:
        case EZ_TSL_DATA_TYPE_OBJECT:
            tsl_data->value = cJSON_PrintUnformatted(js_data);
            if (NULL == tsl_data->value)
            {
                break;
            }

            tsl_data->size = ezos_strlen(tsl_data->value);
            break;

        default:
            break;
        }
    } while (0);

    cJSON_Delete(js_msg);
}

static ez_int32_t tsl_action_process(ez_kernel_submsg_v3_t *submsg)
{
    ez_int32_t rv = -1;
    ez_char_t *rsp_buf = NULL;
    ez_char_t domain[32] = {0};
    ez_char_t identifier[32] = {0};

    ez_tsl_rsc_t rsc_info = {.res_type = submsg->resource_type, .local_index = submsg->resource_id};
    ez_tsl_key_t key_info = {.domain = domain, .key = identifier};
    ez_tsl_value_t value_in = {0};
    value_in.type = EZ_TSL_DATA_TYPE_NULL;
    ez_tsl_value_t value_out = {0};
    value_out.type = EZ_TSL_DATA_TYPE_NULL;
    ez_kernel_pubmsg_v3_t pubmsg = {0};

    ezlog_i(TAG_TSL, "seq in: %d", submsg->msg_seq);
    ezlog_d(TAG_TSL, "recv msg: %s", submsg->buf);

    // example "domain/identifier"
    ez_int32_t num = ezos_sscanf(submsg->ext_msg, "%32[^/]/%32", domain, identifier);
    if (2 != num)
    {
        ezlog_e(TAG_TSL, "tsl key is illegal., msg:%s", submsg->ext_msg);
        goto done;
    }

    strip_msg_wrap(submsg->buf, &value_in);
    rv = g_tsl_things_cbs.action2dev(submsg->sub_serial, &rsc_info, &key_info, &value_in, &value_out);
    CHECK_RV_DONE(rv);

    rsp_buf = assemble_rsp_code_msg(rv, &value_out);
    CHECK_COND_DONE(NULL == rsp_buf, EZ_TSL_ERR_MEMORY);

    pubmsg.msg_response = 1;
    pubmsg.msg_qos = QOS_T1;
    pubmsg.msg_seq = submsg->msg_seq;
    pubmsg.msg_body = rsp_buf;
    pubmsg.msg_body_len = ezos_strlen(rsp_buf);

    ezos_strncpy(pubmsg.resource_type, rsc_info.res_type, sizeof(pubmsg.resource_type) - 1);
    ezos_strncpy(pubmsg.resource_id, rsc_info.local_index, sizeof(pubmsg.resource_id) - 1);
    ezos_strncpy(pubmsg.module, TSL_MODULE_NAME, sizeof(pubmsg.module) - 1);
    ezos_strncpy(pubmsg.method, TSL_SERVICE_METHOD_NAME, sizeof(pubmsg.module) - 1);
    ezos_strncpy(pubmsg.msg_type, TSL_MSG_TYPE_OPERATE_REPLY, sizeof(pubmsg.msg_type) - 1);
    ezos_strncpy(pubmsg.ext_msg, submsg->ext_msg, sizeof(pubmsg.ext_msg) - 1);
    ezos_strncpy(pubmsg.sub_serial, submsg->sub_serial, sizeof(pubmsg.sub_serial) - 1);

    rv = ez_kernel_send_v3(&pubmsg);
    CHECK_COND_DONE(ez_kernel_send_v3(&pubmsg), EZ_TSL_ERR_GENERAL);

done:
    SAFE_FREE(rsp_buf);

    if (EZ_TSL_DATA_TYPE_STRING == value_in.type ||
        EZ_TSL_DATA_TYPE_ARRAY == value_in.type ||
        EZ_TSL_DATA_TYPE_OBJECT == value_in.type)
    {
        SAFE_FREE(value_in.value);
    }

    if (EZ_TSL_DATA_TYPE_STRING == value_out.type ||
        EZ_TSL_DATA_TYPE_ARRAY == value_out.type ||
        EZ_TSL_DATA_TYPE_OBJECT == value_out.type)
    {
        SAFE_FREE(value_out.value);
    }

    return rv;
}

static ez_err_t business2dev_imp(const ez_shadow_value_t *pvalue, ez_shadow_business2dev_param_t *ppram)
{
    int32_t rv = -1;
    ez_tsl_value_t tsl_value = {0};
    ez_char_t local_index[8] = {0};
    ezos_snprintf(local_index, sizeof(local_index), "%d", ppram->pres.local_index);

    ez_tsl_rsc_t rsc_info = {.res_type = ppram->pres.res_type, .local_index = local_index};
    ez_tsl_key_t key_info = {.domain = (ez_char_t *)ppram->pdomain, .key = (ez_char_t *)ppram->pkey};
    tsl_value.type = pvalue->type;
    tsl_value.size = pvalue->length;
    tsl_value.value_double = pvalue->value_double;

    rv = g_tsl_things_cbs.property2dev(ppram->pres.dev_serial, &rsc_info, &key_info, &tsl_value);
    if (0 != rv)
    {
        ezlog_e(TAG_TSL, "dev process failed.");
    }

    return rv;
}

static ez_err_t business2cloud_imp(ez_shadow_value_t *pvalue, ez_shadow_business2cloud_param_t *ppram)
{
    int32_t rv = -1;
    ez_tsl_value_t *tsl_value = (ez_tsl_value_t *)pvalue;
    ez_char_t local_index[8] = {0};
    ezos_snprintf(local_index, sizeof(local_index), "%d", ppram->pres.local_index);

    ez_tsl_rsc_t rsc_info = {.res_type = ppram->pres.res_type, .local_index = local_index};
    ez_tsl_key_t key_info = {.domain = (ez_char_t *)ppram->pdomain, .key = (ez_char_t *)ppram->pkey};

    ezlog_d(TAG_TSL, "busi to cloud. dev_sn: %s", ppram->pres.dev_serial);
    rv = g_tsl_things_cbs.property2cloud(ppram->pres.dev_serial, &rsc_info, &key_info, tsl_value);
    if (0 != rv)
    {
        ezlog_e(TAG_TSL, "property2cloud failed.");
    }

    return rv;
}

static void tsl_adapter_shadow_inst(ez_tsl_devinfo_t *dev_info, ez_iot_tsl_capacity_t *capacity)
{
    ez_int32_t rsc_num = capacity->rsc_num;
    ezlog_i(TAG_TSL, "resource num: %d", rsc_num);

    for (size_t i = 0; i < rsc_num; i++)
    {
        ez_int32_t index_num = capacity->resource[i].index_num;
        ezlog_i(TAG_TSL, "index num: %d", index_num);

        for (size_t j = 0; j < index_num; j++)
        {
            ez_int32_t domain_num = capacity->resource[i].domain_num;
            ezlog_i(TAG_TSL, "domain num: %d", domain_num);
            for (size_t k = 0; k < domain_num; k++)
            {
                ez_int32_t prop_num = capacity->resource[i].domain[k].prop_num;
                ezlog_i(TAG_TSL, "prop num: %d", prop_num);
                for (size_t l = 0; l < prop_num; l++)
                {
                    tsl_domain_prop *prop = capacity->resource[i].domain[k].prop + l;

                    ez_shadow_business_t shadow_busi = {0};
                    ezos_memset(&shadow_busi, 0, sizeof(ez_shadow_business_t));
                    ezos_strncpy((char *)shadow_busi.key, prop->identifier, sizeof(shadow_busi.key) - 1);

                    if (prop->access & ACCESS_READ)
                    {
                        shadow_busi.business2cloud = business2cloud_imp;
                    }

                    if (prop->access & ACCESS_WRITE)
                    {
                        shadow_busi.business2dev = business2dev_imp;
                    }

                    ezos_strncpy((char *)shadow_busi.key, prop->identifier, sizeof(shadow_busi.key) - 1);

                    ez_shadow_res_t shadow_res = {0};
                    ez_shadow_module_t shadow_module = {0};
                    ezos_strncpy((char *)shadow_res.dev_serial, (char *)dev_info->dev_subserial, sizeof(shadow_res.dev_serial) - 1);
                    ezos_strncpy((char *)shadow_res.res_type, (char *)capacity->resource[i].rsc_category, sizeof(shadow_res.res_type) - 1);
                    shadow_res.local_index = ezos_atoi(capacity->resource[i].index + MAX_LOCAL_INDEX_LENGTH * j);

                    shadow_module.num = 1;
                    shadow_module.business = &shadow_busi;
                    ez_int32_t ret = ez_iot_shadow_register(&shadow_res, capacity->resource[i].domain[k].identifier, &shadow_module);
                    if (0 != ret)
                    {
                        ezlog_e(TAG_TSL, "shadow register failed.");
                        continue;
                    }
                }
            }
        }
    }
}

struct fetch_data
{
    ez_char_t *buf;
    ez_int32_t len;
    ez_int32_t code, closed;
};

static ez_int32_t profile_downloading(dev_info_t *dev_info, ez_char_t **buf, ez_int32_t *length)
{
    ez_int32_t rv = -1;

    ez_char_t md5_hex_up[16 * 2 + 1] = {0};
    ez_char_t md5_hex[16 * 2 + 1] = {0};
    ez_int32_t already_len = 0;
    ez_char_t md5_output[16] = {0};
    mbedtls_md5_context md5_ctx = {0};

    struct webclient_session *session = NULL;
    mbedtls_md5_init(&md5_ctx);
    mbedtls_md5_starts(&md5_ctx);

    session = webclient_session_create(512);
    ez_int32_t rsp_status = 0;
    do
    {
        ez_tsl_download_info_t *download_info = (ez_tsl_download_info_t *)dev_info->download_info;
        ezlog_w(TAG_TSL, "url: %s", download_info->url);
        rsp_status = webclient_get(session, download_info->url);
        if (200 != rsp_status)
        {
            ezlog_e(TAG_TSL, "webclient get request failed. http_code: %d", rsp_status);
            break;
        }

        if (0 >= session->content_length)
        {
            ezlog_e(TAG_TSL, "content length illegal: %d", session->content_length);
            break;
        }

        *buf = (char *)ezos_malloc(session->content_length + 1);
        if (NULL == *buf)
        {
            ezlog_e(TAG_TSL, "memory not enough.");
            break;
        }
        ezos_memset(*buf, 0, session->content_length + 1);

        ez_int32_t read_len = 0;
        do
        {
            read_len = webclient_read(session, *buf + already_len, session->content_length - already_len);
            if (0 >= read_len)
            {
                break;
            }
            if (already_len == session->content_length)
            {
                break;
            }
            already_len += read_len;

        } while (ez_true);

        mbedtls_md5_update(&md5_ctx, (*buf), session->content_length);
        mbedtls_md5_finish(&md5_ctx, md5_output);
        bin2hexstr(md5_output, sizeof(md5_output), 1, md5_hex_up);
        bin2hexstr(md5_output, sizeof(md5_output), 0, md5_hex);

        if (0 != strcmp((char *)md5_hex, download_info->md5) && 0 != strcmp((char *)md5_hex_up, download_info->md5))
        {
            ezlog_e(TAG_TSL, "check_sum mismatch:%s, profile md5:%s", md5_hex, download_info->md5);
            break;
        }
        *length = session->content_length;
        rv = 0;
    } while (0);

    webclient_close(session);

    if (0 != rv)
    {
        ezos_free(*buf);
    }

    return rv;
}

/**
 * @brief 查询下载功能
 *
 * @param
 */
static void ez_profile_get_thread(void *user_data)
{
    ezlog_w(TAG_TSL, "profile thread enter.");

    dev_info_t *dev = NULL;
    ez_tsl_devinfo_t tsl_dev = {0};
    g_profile_download_thread_running = 1;

#ifndef COMPONENT_TSL_PROFILE_STRIP
    ez_bool_t need_schema = ez_true;
#else
    ez_bool_t need_schema = ez_false;
#endif

    do
    {
        if (!g_profile_download_thread_running)
        {
            break;
        }

        if (!g_is_online)
        {
            ezos_delay_ms(500);
            continue;
        }

        ezos_mutex_lock(g_profile_mutex);
        dev = NULL;
        size_t obj_size = 0;
        for (size_t i = 0; i < ezlist_size(g_tsl_dev_info_list); i++)
        {
            dev = ezlist_getat(g_tsl_dev_info_list, i, NULL, ez_false);
            if (status_done != dev->status)
            {
                dev = ezlist_getat(g_tsl_dev_info_list, i, &obj_size, ez_true);
                break;
            }

            dev = NULL;
        }
        ezos_mutex_unlock(g_profile_mutex);

        if (NULL == dev)
        {
            /* 下载完成，任务退出 */
            break;
        }

        tsl_dev.dev_type = dev->dev_type;
        tsl_dev.dev_firmwareversion = dev->dev_fw_ver;
        tsl_dev.dev_subserial = dev->dev_sn;

        if (status_loading == dev->status)
        {
            ezlog_d(TAG_TSL, "profile thread get url");
            /* 已拿到下载链接，开始下载 */
            ez_char_t *buf = NULL;
            ez_int32_t length = 0;
            if (0 != profile_downloading(dev, &buf, &length))
            {
                ezos_free(dev);
                ezos_delay_ms(100);
                continue;
            }

            ezlog_w(TAG_TSL, "%s profile dl succ.", dev->dev_sn);

            if (!tsl_adapter_profile_save(&tsl_dev, buf, length))
            {
                ezos_free(buf);
                ezos_free(dev);
                ezos_delay_ms(100);
                continue;
            }

            ezlog_w(TAG_TSL, "%s profile save succ.", dev->dev_sn);

            if (!tsl_adapter_profile_load(&tsl_dev))
            {
                ezos_free(buf);
                ezos_free(dev);
                ezos_delay_ms(100);
                continue;
            }

            tsl_adapter_dev_update(&tsl_dev, status_done);

            ezlog_w(TAG_TSL, "%s profile load succ.", dev->dev_sn);

            ezos_free(buf);
            ezos_free(dev);
        }
        else if (status_query == dev->status && !time_isexpired(&dev->timer))
        {
            /* 正在查询下载链接 */
            ezlog_v(TAG_TSL, "profile thread wait url");
            ezos_free(dev);
            ezos_delay_ms(100);
        }
        else
        {
            /* 查询下载链接 */
            cJSON *json_query = cJSON_CreateObject();
            if (NULL == json_query)
            {
                continue;
            }

            cJSON_AddStringToObject(json_query, "devSerial", dev->dev_sn);
            cJSON_AddStringToObject(json_query, "pid", dev->dev_type);
            cJSON_AddStringToObject(json_query, "version", dev->dev_fw_ver);
            cJSON_AddStringToObject(json_query, "profileVersion", "3.0");
            cJSON_AddBoolToObject(json_query, "requireSchema", need_schema);
            char *json_str = cJSON_PrintUnformatted(json_query);
            if (NULL == json_str)
            {
                cJSON_Delete(json_query);
                continue;
            }

            if (0 == ez_iot_base_lowlvl_profile_query(json_str, tsl_adapter_profile_query_rsp))
            {
                ezlog_d(TAG_TSL, "profile thread status_query");
                tsl_adapter_dev_update(&tsl_dev, status_query);
            }
            else
            {
                tsl_adapter_dev_update(&tsl_dev, status_need_update);
            }

            ezos_free(dev);
            ezos_free(json_str);
            cJSON_Delete(json_query);
        }
    } while (1);

    g_profile_download_thread_running = 0;

    return;
}

static ez_void_t iot_core_event_route(ez_kernel_event_t *ptr_event);
static void tsl_data_route_cb(ez_kernel_submsg_v3_t *psub_msg);

ez_err_t ez_iot_tsl_adapter_init(ez_tsl_callbacks_t *things_cbs)
{
    ez_err_t rv = EZ_TSL_ERR_SUCC;
    ez_kernel_extend_v3_t extend_info = {0};

    ezos_memcpy(&g_tsl_things_cbs, things_cbs, sizeof(ez_tsl_callbacks_t));

    extend_info.ez_kernel_event_route = iot_core_event_route;
    extend_info.ez_kernel_data_route = tsl_data_route_cb;
    ezos_strncpy(extend_info.module, TSL_MODULE_NAME, sizeof(extend_info.module) - 1);

    rv = ez_kernel_extend_load_v3(&extend_info);
    CHECK_COND_DONE(EZ_CORE_ERR_NOT_INIT == rv, EZ_TSL_ERR_NOT_READY);
    CHECK_COND_DONE(EZ_CORE_ERR_MEMORY == rv, EZ_TSL_ERR_MEMORY);
    CHECK_COND_DONE(EZ_CORE_ERR_SUCC != rv, EZ_TSL_ERR_GENERAL);

    g_tsl_dev_info_list = ezlist(ezlist_THREADSAFE);
    CHECK_COND_DONE(!g_tsl_dev_info_list, EZ_TSL_ERR_MEMORY);

    g_capacities_list = ezlist(ezlist_THREADSAFE);
    CHECK_COND_DONE(!g_capacities_list, EZ_TSL_ERR_MEMORY);

    g_profile_mutex = ezos_mutex_create();
    CHECK_COND_DONE(!g_capacities_list, EZ_TSL_ERR_MEMORY);

    g_capacities_mutex = ezos_mutex_create();
    CHECK_COND_DONE(!g_capacities_list, EZ_TSL_ERR_MEMORY);

done:

    return rv;
}

ez_err_t ez_iot_tsl_adapter_add(ez_tsl_devinfo_t *dev_info, ez_char_t *profile)
{
    ez_err_t rv = EZ_TSL_ERR_SUCC;

    do
    {
        if (tsl_adapter_dev_find(dev_info->dev_subserial, NULL, NULL))
        {
            break;
        }

        ezlog_d(TAG_TSL, "try ram");
        if (tsl_adapter_ref_add(dev_info))
        {
            tsl_adapter_dev_add(dev_info, ez_false);
            break;
        }

        if (profile)
        {
            ezlog_d(TAG_TSL, "try param");
            if (tsl_adapter_profile_parse(dev_info, profile))
            {
                tsl_adapter_dev_add(dev_info, ez_false);
                break;
            }
            else
            {
                rv = EZ_TSL_ERR_PROFILE_LOADING;
                break;
            }
        }
        else
        {
            ezlog_d(TAG_TSL, "try flash");
            if (tsl_adapter_profile_load(dev_info))
            {
                tsl_adapter_dev_add(dev_info, ez_false);
                break;
            }

            ezlog_d(TAG_TSL, "try download");
            if (tsl_adapter_profile_download(dev_info))
            {
                rv = EZ_TSL_ERR_MEMORY;
            }
        }
    } while (0);

    return rv;
}

ez_err_t ez_iot_tsl_adapter_del(ez_tsl_devinfo_t *dev_info)
{
    if (!tsl_adapter_dev_find(dev_info->dev_subserial, NULL, NULL))
    {
        return EZ_TSL_ERR_SUCC;
    }

    tsl_adapter_ref_del(dev_info);
    tsl_adapter_dev_del(dev_info);

    return EZ_TSL_ERR_SUCC;
}

void ez_iot_tsl_adapter_deinit()
{
    ezlog_w(TAG_TSL, "tsl adapter deinit.");

    ezos_mutex_lock(g_profile_mutex);
    size_t size = ezlist_size(g_tsl_dev_info_list);
    for (size_t i = 0; i < size; i++)
    {
        ezlist_removefirst(g_tsl_dev_info_list);
    }
    ezos_mutex_unlock(g_profile_mutex);

    g_profile_download_thread_running = 0;

    ezos_mutex_destroy(g_profile_mutex);
    g_profile_mutex = NULL;

    ezos_mutex_destroy(g_capacities_mutex);
    g_capacities_mutex = NULL;
}

ez_err_t ez_iot_tsl_action_value_legal(const ez_char_t *sn, const ez_tsl_rsc_t *rsc_info, const ez_tsl_key_t *key_info, const ez_tsl_value_t *value)
{
    ez_err_t rv = EZ_TSL_ERR_SUCC;

    ez_char_t dev_sn[48] = {0};
    ez_char_t dev_type[24] = {0};
    ez_char_t dev_fw_ver[36] = {0};
    dev_stauts_e dev_status = 0;
    ez_tsl_devinfo_t dev_info = {.dev_subserial = dev_sn, .dev_type = dev_type, .dev_firmwareversion = dev_fw_ver};
    ez_iot_tsl_capacity_t *capacity = NULL;
    tsl_domain_action *action = NULL;
    size_t i = 0;
    size_t j = 0;
    size_t k = 0;

    ezos_mutex_lock(g_capacities_mutex);

    CHECK_COND_DONE(!tsl_adapter_dev_find(sn, &dev_info, &dev_status), EZ_TSL_ERR_DEV_NOT_FOUND);
    CHECK_COND_DONE(status_done != dev_status, EZ_TSL_ERR_PROFILE_LOADING);

    /* find capacity */
    size_t num = ezlist_size(g_capacities_list);
    for (i = 0; i < num; i++)
    {
        ez_iot_tsl_capacity_t *capacity_temp = (ez_iot_tsl_capacity_t *)ezlist_getat(g_capacities_list, i, NULL, ez_false);
        CHECK_COND_DONE(!capacity_temp, EZ_TSL_ERR_PROFILE_LOADING);

        if (0 != strcmp((const ez_char_t *)dev_info.dev_type, capacity_temp->dev_type) ||
            0 != strcmp((const ez_char_t *)dev_info.dev_firmwareversion, capacity_temp->dev_fw_ver))
        {
            continue;
        }

        capacity = capacity_temp;
        break;
    }

    CHECK_COND_DONE(!capacity, EZ_TSL_ERR_PROFILE_LOADING);

    /* find  resource*/
    size_t rsc_num = capacity->rsc_num;
    for (i = 0; i < rsc_num; i++)
    {
        if (0 == strcmp((const ez_char_t *)rsc_info->res_type, capacity->resource[i].rsc_category))
        {
            break;
        }
    }

    CHECK_COND_DONE(i == rsc_num, EZ_TSL_ERR_DEV_NOT_FOUND);

    /* find local index*/
    size_t index_num = capacity->resource[i].index_num;
    for (j = 0; j < index_num; j++)
    {
        if (0 == strcmp(rsc_info->local_index, capacity->resource[i].index))
        {
            break;
        }
    }

    CHECK_COND_DONE(j == index_num, EZ_TSL_ERR_INDEX_NOT_FOUND);

    /*find domain*/
    size_t domain_num = capacity->resource[i].domain_num;
    for (j = 0; j < domain_num; j++)
    {
        if (0 == strcmp(capacity->resource[i].domain[j].identifier, (char *)key_info->domain))
        {
            break;
        }
    }

    CHECK_COND_DONE(j == domain_num, EZ_TSL_ERR_DOMAIN_NOT_FOUND);

    /*find key*/
    size_t action_num = capacity->resource[i].domain[j].action_num;
    for (k = 0; k < action_num; k++)
    {
        tsl_domain_action *action_temp = capacity->resource[i].domain[j].action + k;

        if (0 == strcmp(action->identifier, (char *)key_info->key))
        {
            action = action_temp;
            break;
        }
    }

    CHECK_COND_DONE(k == action_num, EZ_TSL_ERR_KEY_NOT_FOUND);

#ifndef COMPONENT_TSL_PROFILE_STRIP
    if (value)
    {
        rv = check_schema_value(&action->input_schema, value);
    }
#endif

done:
    ezos_mutex_unlock(g_capacities_mutex);

    return rv;
}

ez_err_t ez_iot_tsl_property_value_legal(const ez_char_t *sn, const ez_tsl_rsc_t *rsc_info, const ez_tsl_key_t *key_info, const ez_tsl_value_t *value)
{
    ez_err_t rv = EZ_TSL_ERR_SUCC;

    ez_char_t dev_sn[48] = {0};
    ez_char_t dev_type[24] = {0};
    ez_char_t dev_fw_ver[36] = {0};
    dev_stauts_e dev_status = 0;
    ez_tsl_devinfo_t dev_info = {.dev_subserial = dev_sn, .dev_type = dev_type, .dev_firmwareversion = dev_fw_ver};
    ez_iot_tsl_capacity_t *capacity = NULL;
    tsl_domain_prop *prop = NULL;

    size_t i = 0;
    size_t j = 0;
    size_t k = 0;

    ezos_mutex_lock(g_capacities_mutex);

    CHECK_COND_DONE(!tsl_adapter_dev_find(sn, &dev_info, &dev_status), EZ_TSL_ERR_DEV_NOT_FOUND);
    CHECK_COND_DONE(status_done != dev_status, EZ_TSL_ERR_PROFILE_LOADING);

    /* find capacity */
    size_t num = ezlist_size(g_capacities_list);
    for (i = 0; i < num; i++)
    {
        ez_iot_tsl_capacity_t *capacity_temp = (ez_iot_tsl_capacity_t *)ezlist_getat(g_capacities_list, i, NULL, ez_false);
        CHECK_COND_DONE(!capacity_temp, EZ_TSL_ERR_PROFILE_LOADING);

        if (0 != strcmp((const ez_char_t *)dev_info.dev_type, capacity_temp->dev_type) ||
            0 != strcmp((const ez_char_t *)dev_info.dev_firmwareversion, capacity_temp->dev_fw_ver))
        {
            continue;
        }

        capacity = capacity_temp;
        break;
    }

    CHECK_COND_DONE(!capacity, EZ_TSL_ERR_PROFILE_LOADING);

    /* find  resource*/
    size_t rsc_num = capacity->rsc_num;
    for (i = 0; i < rsc_num; i++)
    {
        if (0 == strcmp((const ez_char_t *)rsc_info->res_type, capacity->resource[i].rsc_category))
        {
            break;
        }
    }

    CHECK_COND_DONE(i == rsc_num, EZ_TSL_ERR_DEV_NOT_FOUND);

    /* find local index*/
    size_t index_num = capacity->resource[i].index_num;
    for (j = 0; j < index_num; j++)
    {
        if (0 == strcmp(rsc_info->local_index, capacity->resource[i].index))
        {
            break;
        }
    }

    CHECK_COND_DONE(j == index_num, EZ_TSL_ERR_INDEX_NOT_FOUND);

    /*find domain*/
    size_t domain_num = capacity->resource[i].domain_num;
    for (j = 0; j < domain_num; j++)
    {
        if (0 == strcmp(capacity->resource[i].domain[j].identifier, (char *)key_info->domain))
        {
            break;
        }
    }

    CHECK_COND_DONE(j == domain_num, EZ_TSL_ERR_DOMAIN_NOT_FOUND);

    /*find key*/
    size_t prop_num = capacity->resource[i].domain[j].prop_num;
    for (k = 0; k < prop_num; k++)
    {
        tsl_domain_prop *prop_temp = capacity->resource[i].domain[j].prop + k;
        if (0 == strcmp(prop_temp->identifier, (char *)key_info->key))
        {
            prop = prop_temp;
            break;
        }
    }

    CHECK_COND_DONE(k == prop_num, EZ_TSL_ERR_KEY_NOT_FOUND);

#ifndef COMPONENT_TSL_PROFILE_STRIP
    if (value)
    {
        rv = check_schema_value(&prop->prop_desc, value);
    }
#endif

done:
    ezos_mutex_unlock(g_capacities_mutex);

    return rv;
}

ez_err_t ez_iot_tsl_event_value_legal(const ez_char_t *sn, const ez_tsl_rsc_t *rsc_info, const ez_tsl_key_t *key_info, const ez_tsl_value_t *value)
{
    ez_err_t rv = EZ_TSL_ERR_SUCC;

    ez_char_t dev_sn[48] = {0};
    ez_char_t dev_type[24] = {0};
    ez_char_t dev_fw_ver[36] = {0};
    dev_stauts_e dev_status = 0;
    ez_tsl_devinfo_t dev_info = {.dev_subserial = dev_sn, .dev_type = dev_type, .dev_firmwareversion = dev_fw_ver};
    ez_iot_tsl_capacity_t *capacity = NULL;
    tsl_domain_event *event = NULL;
    size_t i = 0;
    size_t j = 0;
    size_t k = 0;

    ezos_mutex_lock(g_capacities_mutex);

    CHECK_COND_DONE(!tsl_adapter_dev_find(sn, &dev_info, &dev_status), EZ_TSL_ERR_DEV_NOT_FOUND);
    CHECK_COND_DONE(status_done != dev_status, EZ_TSL_ERR_PROFILE_LOADING);

    /* find capacity */
    size_t num = ezlist_size(g_capacities_list);
    for (i = 0; i < num; i++)
    {
        ez_iot_tsl_capacity_t *capacity_temp = (ez_iot_tsl_capacity_t *)ezlist_getat(g_capacities_list, i, NULL, ez_false);
        CHECK_COND_DONE(!capacity_temp, EZ_TSL_ERR_PROFILE_LOADING);

        if (0 != strcmp((const ez_char_t *)dev_info.dev_type, capacity_temp->dev_type) ||
            0 != strcmp((const ez_char_t *)dev_info.dev_firmwareversion, capacity_temp->dev_fw_ver))
        {
            continue;
        }

        capacity = capacity_temp;
        break;
    }

    CHECK_COND_DONE(!capacity, EZ_TSL_ERR_PROFILE_LOADING);

    /* find  resource*/
    size_t rsc_num = capacity->rsc_num;

    for (i = 0; i < rsc_num; i++)
    {
        if (0 == strcmp((const ez_char_t *)rsc_info->res_type, capacity->resource[i].rsc_category))
        {
            break;
        }
    }

    CHECK_COND_DONE(i == rsc_num, EZ_TSL_ERR_DEV_NOT_FOUND);

    /* find local index*/
    size_t index_num = capacity->resource[i].index_num;
    for (j = 0; j < index_num; j++)
    {
        if (0 == strcmp(rsc_info->local_index, capacity->resource[i].index))
        {
            break;
        }
    }

    CHECK_COND_DONE(j == index_num, EZ_TSL_ERR_INDEX_NOT_FOUND);

    /*find domain*/
    size_t domain_num = capacity->resource[i].domain_num;
    for (j = 0; j < domain_num; j++)
    {
        if (0 == strcmp(capacity->resource[i].domain[j].identifier, (char *)key_info->domain))
        {
            break;
        }
    }

    CHECK_COND_DONE(j == domain_num, EZ_TSL_ERR_DOMAIN_NOT_FOUND);

    /*find key*/
    size_t event_num = capacity->resource[i].domain[j].event_num;
    for (k = 0; k < event_num; k++)
    {
        tsl_domain_event *event_temp = capacity->resource[i].domain[j].event + k;

        if (0 == strcmp(event_temp->identifier, (char *)key_info->key))
        {
            event = event_temp;
            break;
        }
    }

    CHECK_COND_DONE(k == event_num, EZ_TSL_ERR_KEY_NOT_FOUND);

#ifndef COMPONENT_TSL_PROFILE_STRIP
    if (value)
    {
        rv = check_schema_value(&event->input_schema, value);
    }
#endif

done:
    ezos_mutex_unlock(g_capacities_mutex);

    return rv;
}

ez_int32_t tsl_find_property_rsc_by_keyinfo(const ez_char_t *sn, const ez_tsl_key_t *key_info, ez_char_t *res_type, ez_int32_t len)
{
    int32_t rv = EZ_TSL_ERR_DEV_NOT_FOUND;

    ez_char_t dev_sn[48] = {0};
    ez_char_t dev_type[24] = {0};
    ez_char_t dev_fw_ver[36] = {0};
    dev_stauts_e dev_status = 0;
    ez_tsl_devinfo_t dev_info = {.dev_subserial = dev_sn, .dev_type = dev_type, .dev_firmwareversion = dev_fw_ver};
    ez_iot_tsl_capacity_t *capacity = NULL;

    size_t i = 0;
    size_t j = 0;
    size_t k = 0;

    ezos_mutex_lock(g_capacities_mutex);

    CHECK_COND_DONE(!tsl_adapter_dev_find(sn, &dev_info, &dev_status), EZ_TSL_ERR_DEV_NOT_FOUND);
    CHECK_COND_DONE(status_done != dev_status, EZ_TSL_ERR_PROFILE_LOADING);

    /* find capacity */
    size_t num = ezlist_size(g_capacities_list);
    for (i = 0; i < num; i++)
    {
        ez_iot_tsl_capacity_t *capacity_temp = (ez_iot_tsl_capacity_t *)ezlist_getat(g_capacities_list, i, NULL, ez_false);
        CHECK_COND_DONE(!capacity_temp, EZ_TSL_ERR_PROFILE_LOADING);

        if (0 != strcmp((const ez_char_t *)dev_info.dev_type, capacity_temp->dev_type) ||
            0 != strcmp((const ez_char_t *)dev_info.dev_firmwareversion, capacity_temp->dev_fw_ver))
        {
            continue;
        }

        capacity = capacity_temp;
        break;
    }

    for (i = 0; i < capacity->rsc_num; i++)
    {
        /* 查找domain */
        for (j = 0; j < capacity->resource[i].domain_num; j++)
        {
            if (0 == strcmp(capacity->resource[i].domain[j].identifier, (char *)key_info->domain))
            {
                break;
            }
        }

        CHECK_COND_DONE(j == capacity->rsc_num, EZ_TSL_ERR_RSCTYPE_NOT_FOUND);

        /* 查找key */
        for (k = 0; k < capacity->resource[i].domain[j].prop_num; k++)
        {
            tsl_domain_prop *prop_temp = capacity->resource[i].domain[j].prop + k;
            if (0 == strcmp(prop_temp->identifier, (char *)key_info->key))
            {
                break;
            }
        }

        CHECK_COND_DONE(k == capacity->rsc_num, EZ_TSL_ERR_RSCTYPE_NOT_FOUND);

        ezos_strncpy(res_type, capacity->resource[i].rsc_category, len - 1);
        rv = 0;
    }

done:
    ezos_mutex_unlock(g_capacities_mutex);

    return rv;
}

ez_err_t tsl_find_event_rsc_by_keyinfo(const ez_char_t *sn, const ez_tsl_key_t *key_info, ez_char_t *res_type, ez_int32_t len)
{
    ez_err_t rv = EZ_TSL_ERR_DEV_NOT_FOUND;

    ez_char_t dev_sn[48] = {0};
    ez_char_t dev_type[24] = {0};
    ez_char_t dev_fw_ver[36] = {0};
    dev_stauts_e dev_status = 0;
    ez_tsl_devinfo_t dev_info = {.dev_subserial = dev_sn, .dev_type = dev_type, .dev_firmwareversion = dev_fw_ver};
    ez_iot_tsl_capacity_t *capacity = NULL;

    size_t i = 0;
    size_t j = 0;
    size_t k = 0;

    ezos_mutex_lock(g_capacities_mutex);

    CHECK_COND_DONE(!tsl_adapter_dev_find(sn, &dev_info, &dev_status), EZ_TSL_ERR_DEV_NOT_FOUND);
    CHECK_COND_DONE(status_done != dev_status, EZ_TSL_ERR_PROFILE_LOADING);

    /* find capacity */
    size_t num = ezlist_size(g_capacities_list);
    for (i = 0; i < num; i++)
    {
        ez_iot_tsl_capacity_t *capacity_temp = (ez_iot_tsl_capacity_t *)ezlist_getat(g_capacities_list, i, NULL, ez_false);
        CHECK_COND_DONE(!capacity_temp, EZ_TSL_ERR_PROFILE_LOADING);

        if (0 != strcmp((const ez_char_t *)dev_info.dev_type, capacity_temp->dev_type) ||
            0 != strcmp((const ez_char_t *)dev_info.dev_firmwareversion, capacity_temp->dev_fw_ver))
        {
            continue;
        }

        capacity = capacity_temp;
        break;
    }

    for (i = 0; i < capacity->rsc_num; i++)
    {
        /* 查找domain */
        for (j = 0; j < capacity->resource[i].domain_num; j++)
        {
            if (0 != strcmp(capacity->resource[i].domain[j].identifier, (char *)key_info->domain))
            {
                continue;
            }
        }

        /* 查找key */
        for (k = 0; k < capacity->resource[i].domain[j].event_num; k++)
        {
            tsl_domain_event *event = capacity->resource[i].domain[j].event + k;
            if (0 != strcmp(event->identifier, (char *)key_info->key))
            {
                continue;
            }
        }

        ezos_strncpy(res_type, capacity->resource[i].rsc_category, len - 1);
        rv = 0;
    }

done:
    ezos_mutex_unlock(g_capacities_mutex);

    return rv;
}

static ez_bool_t tsl_adapter_capacity_add(ez_iot_tsl_capacity_t *capacity)
{
    ezos_mutex_lock(g_capacities_mutex);
    ezlist_addlast(g_capacities_list, (void *)capacity, sizeof(ez_iot_tsl_capacity_t));
    ezos_mutex_unlock(g_capacities_mutex);

    return ez_true;
}

static ez_bool_t tsl_adapter_ref_add(ez_tsl_devinfo_t *dev_info)
{
    if (NULL == dev_info || NULL != g_capacities_list)
    {
        return ez_false;
    }

    ezos_mutex_lock(g_capacities_mutex);

    for (size_t i = 0; i < ezlist_size(g_capacities_list); i++)
    {
        ez_iot_tsl_capacity_t *capacity = (ez_iot_tsl_capacity_t *)ezlist_getat(g_capacities_list, i, NULL, ez_false);
        if (0 == strcmp((char *)dev_info->dev_type, capacity->dev_type) && 0 == strcmp((char *)dev_info->dev_firmwareversion, capacity->dev_fw_ver))
        {
            capacity->ref++;
            tsl_adapter_shadow_inst(dev_info, capacity);
            ezlog_w(TAG_TSL, "profile found, ++ref:%d.", capacity->ref);
            ezos_mutex_unlock(g_capacities_mutex);
            return ez_true;
        }
    }

    ezos_mutex_unlock(g_capacities_mutex);

    return ez_false;
}

static ez_int32_t find_dev_by_sn(cJSON *json_obj, ez_char_t *sn)
{
    ez_int32_t index = -1;

    for (int i = 0; i < cJSON_GetArraySize(json_obj); i++)
    {
        cJSON *js_item = cJSON_GetArrayItem(json_obj, i);
        if (NULL == js_item)
        {
            continue;
        }

        cJSON *js_sn = cJSON_GetObjectItem(js_item, TSLMAP_JSON_KEY_SN);
        if (NULL == js_sn)
        {
            continue;
        }

        if (0 == strcmp(js_sn->valuestring, (char *)sn))
        {
            index = i;
            break;
        }
    }

    return index;
}

static ez_int32_t get_tsl_handle_count(cJSON *json_obj, ez_char_t *handle)
{
    ez_int32_t count = 0;

    for (int i = 0; i < cJSON_GetArraySize(json_obj); i++)
    {
        cJSON *js_item = cJSON_GetArrayItem(json_obj, i);
        if (NULL == js_item)
        {
            continue;
        }

        cJSON *js_sn = cJSON_GetObjectItem(js_item, TSLMAP_JSON_KEY_HANDLE);
        if (NULL == js_sn)
        {
            continue;
        }

        if (0 == strcmp(js_sn->valuestring, (char *)handle))
        {
            count++;
            break;
        }
    }

    return count;
}

static cJSON *tslmap_metadata_to_json(tslmap_metadata_t *struct_obj)
{
    s2j_create_json_obj(metadata_json);
    s2j_json_set_basic_element(metadata_json, struct_obj, string, sn);
    s2j_json_set_basic_element(metadata_json, struct_obj, string, handle);

    return metadata_json;
}

static void json_to_tslmap_metadata(cJSON *json_obj, tslmap_metadata_t *struct_obj)
{
    cJSON *json_temp = NULL;

    s2j_struct_get_basic_element_ex(struct_obj, json_obj, string, sn, "");
    s2j_struct_get_basic_element_ex(struct_obj, json_obj, string, handle, "");
}

static ez_bool_t tsl_adapter_profile_parse(ez_tsl_devinfo_t *dev_info, ez_char_t *profile)
{
    ez_bool_t rv = ez_false;
    ez_iot_tsl_capacity_t capacity = {0};

    CHECK_COND_DONE(0 != profile_parse((char *)profile, ezos_strlen(profile), &capacity), ez_false);

    ezos_strncpy(capacity.dev_fw_ver, (char *)dev_info->dev_firmwareversion, sizeof(capacity.dev_fw_ver) - 1);
    ezos_strncpy(capacity.dev_type, (char *)dev_info->dev_type, sizeof(capacity.dev_type) - 1);

    tsl_adapter_capacity_add(&capacity);
    tsl_adapter_shadow_inst(dev_info, &capacity);

    rv = ez_true;
done:

    return rv;
}

static ez_bool_t tsl_adapter_profile_load(ez_tsl_devinfo_t *dev_info)
{
    size_t length = 0;
    ez_bool_t rv = ez_false;
    ez_iot_tsl_capacity_t capacity = {0};
    tslmap_metadata_t tsl_metadata = {0};
    size_t tslmap_len = 0;
    ez_char_t *pbuf = NULL;
    cJSON *js_root = NULL;
    ez_int32_t index = -1;

    CHECK_COND_DONE(ezos_kv_raw_get(EZ_KV_DEFALUT_KEY_TSLMAP, NULL, &tslmap_len), ez_false);
    CHECK_COND_DONE(0 == tslmap_len, ez_false);

    pbuf = (char *)ezos_malloc(tslmap_len + 1);
    CHECK_COND_DONE(!pbuf, ez_false);

    ezos_memset(pbuf, 0, tslmap_len + 1);
    CHECK_COND_DONE(ezos_kv_raw_get(EZ_KV_DEFALUT_KEY_TSLMAP, pbuf, &tslmap_len), ez_false);
    CHECK_COND_DONE(!(js_root = cJSON_Parse(pbuf)), ez_false);
    SAFE_FREE(pbuf);

    CHECK_COND_DONE(-1 == (index = find_dev_by_sn(js_root, (char *)dev_info->dev_subserial)), ez_false);

    cJSON *js_item = cJSON_GetArrayItem(js_root, index);
    json_to_tslmap_metadata(js_item, &tsl_metadata);

    CHECK_COND_DONE(ezos_kv_raw_get(tsl_metadata.handle, NULL, &length), ez_false);

    pbuf = ezos_malloc(length + 1);
    CHECK_COND_DONE(!pbuf, ez_false);

    ezos_memset(pbuf, 0, length + 1);
    CHECK_COND_DONE(ezos_kv_raw_get(tsl_metadata.handle, pbuf, &length), ez_false);
    ezlog_d(TAG_TSL, "length: %d,read_buf: %s, ", length, pbuf);

    CHECK_COND_DONE(ezos_kv_raw_get(tsl_metadata.handle, pbuf, &length), ez_false);
    CHECK_COND_DONE(0 != profile_parse((char *)pbuf, length, &capacity), ez_false);

    ezos_strncpy(capacity.dev_fw_ver, (char *)dev_info->dev_firmwareversion, sizeof(capacity.dev_fw_ver) - 1);
    ezos_strncpy(capacity.dev_type, (char *)dev_info->dev_type, sizeof(capacity.dev_type) - 1);

    tsl_adapter_capacity_add(&capacity);

    tsl_adapter_shadow_inst(dev_info, &capacity);

    rv = ez_true;
done:
    SAFE_FREE(pbuf);
    cJSON_Delete(js_root);

    return rv;
}

static ez_bool_t tsl_adapter_profile_save(ez_tsl_devinfo_t *dev_info, const ez_char_t *profile, ez_int32_t length)
{
    ez_bool_t rv = ez_false;
    cJSON *js_root = NULL;
    cJSON *js_tslmap_item = NULL;
    ez_char_t *pbuf = NULL;
    ez_char_t *pbuf_save = NULL;
    int8_t handle_curr[32] = {0};
    ez_int32_t index = -1;
    tslmap_metadata_t tsl_metadata = {0};
    size_t tslmap_len = 0;

    ezos_kv_print();
    devinfo2index(dev_info, (char *)handle_curr);
    CHECK_COND_DONE(ezos_kv_raw_get(EZ_KV_DEFALUT_KEY_TSLMAP, NULL, &tslmap_len), ez_false);

    if (0 == tslmap_len)
    {
        ezlog_w(TAG_TSL, "tslmap null, try clear all!");
        ezos_kv_del_by_prefix(EZ_TSL_KEY_TSL_PREFIX);
        js_root = cJSON_CreateArray();
        CHECK_COND_DONE(!js_root, ez_false);
    }
    else
    {
        pbuf = (char *)ezos_malloc(tslmap_len + 1);
        CHECK_COND_DONE(!pbuf, ez_false);
        ezos_memset(pbuf, 0, tslmap_len + 1);

        CHECK_COND_DONE(ezos_kv_raw_get(EZ_KV_DEFALUT_KEY_TSLMAP, pbuf, &tslmap_len), ez_false);
        CHECK_COND_DONE(!(js_root = cJSON_Parse(pbuf)), ez_false);
        index = find_dev_by_sn(js_root, (char *)dev_info->dev_subserial);
    }

    do
    {
        if (-1 == index)
        {
            /* 新增设备，tslmap增加元数据 */
            ezlog_d(TAG_TSL, "add handle 2 tslmap:%s", handle_curr);
            ezos_strncpy(tsl_metadata.sn, (char *)dev_info->dev_subserial, sizeof(tsl_metadata.sn) - 1);
            ezos_strncpy(tsl_metadata.handle, (char *)handle_curr, sizeof(tsl_metadata.handle) - 1);

            CHECK_COND_DONE(!(js_tslmap_item = tslmap_metadata_to_json(&tsl_metadata)), ez_false);
            cJSON_AddItemToArray(js_root, js_tslmap_item);
            CHECK_COND_DONE(!(pbuf_save = cJSON_PrintUnformatted(js_root)), EZ_TSL_ERR_MEMORY);
            CHECK_COND_DONE(ezos_kv_raw_set(EZ_KV_DEFALUT_KEY_TSLMAP, pbuf_save, ezos_strlen(pbuf_save)), EZ_TSL_ERR_STORAGE);
            ezlog_d(TAG_TSL, "add handle succ!!");
        }
        else
        {
            cJSON *js_item = cJSON_GetArrayItem(js_root, index);
            json_to_tslmap_metadata(js_item, &tsl_metadata);

            if (0 == strcmp(tsl_metadata.handle, handle_curr))
            {
                /* 元数据相同，无需更新tslmap */
                break;
            }

            if (1 == get_tsl_handle_count(js_root, tsl_metadata.handle))
            {
                /* 功能描述文件只有当前设备在使用，设备型号或者版本号已发生变更，tslpf文件删除 */
                ezlog_w(TAG_TSL, "ref = 0, del old pf");
                ezos_kv_del(tsl_metadata.handle);
            }

            ezlog_w(TAG_TSL, "tslmap chg, update");
            cJSON_ReplaceItemInObject(js_item, TSLMAP_JSON_KEY_HANDLE, cJSON_CreateString(handle_curr));
            CHECK_COND_DONE(!(pbuf_save = cJSON_PrintUnformatted(js_root)), EZ_TSL_ERR_MEMORY);
            CHECK_COND_DONE(ezos_kv_raw_set(EZ_KV_DEFALUT_KEY_TSLMAP, pbuf_save, ezos_strlen(pbuf_save)), EZ_TSL_ERR_STORAGE);
            ezlog_w(TAG_TSL, "update succ!");
        }
    } while (ez_false);

    /* 只有在tslmap中找不到索引句柄情况下才会保存功能描述文件 */
    if (NULL != profile && 1 == get_tsl_handle_count(js_root, handle_curr))
    {
        ezlog_i(TAG_TSL, "save pf:%s", handle_curr);
        CHECK_COND_DONE(EZ_KV_ERR_SUCC != ezos_kv_raw_set(handle_curr, profile, length), ez_false);
        ezlog_i(TAG_TSL, "save pf succ!");
    }

    rv = ez_true;
done:
    SAFE_FREE(pbuf);
    SAFE_FREE(pbuf_save);
    cJSON_Delete(js_root);

    return rv;
}

ez_bool_t ez_iot_tsl_adapter_profile_del(const ez_char_t *dev_subserial)
{
    ez_bool_t rv = ez_false;
    cJSON *js_root = NULL;
    ez_char_t *pbuf = NULL;
    ez_char_t *pbuf_save = NULL;
    ez_int32_t index = -1;
    tslmap_metadata_t tsl_metadata = {0};
    size_t tslmap_len = 0;

    ezos_kv_print();
    CHECK_COND_DONE(ezos_kv_raw_get(EZ_KV_DEFALUT_KEY_TSLMAP, NULL, &tslmap_len), ez_false);
    CHECK_COND_DONE(0 == tslmap_len, ez_true);

    pbuf = (char *)ezos_malloc(tslmap_len + 1);
    CHECK_COND_DONE(!pbuf, ez_false);
    ezos_memset(pbuf, 0, tslmap_len + 1);

    CHECK_COND_DONE(ezos_kv_raw_get(EZ_KV_DEFALUT_KEY_TSLMAP, pbuf, &tslmap_len), ez_false);
    CHECK_COND_DONE(!(js_root = cJSON_Parse(pbuf)), ez_false);

    CHECK_COND_DONE(-1 == (index = find_dev_by_sn(js_root, (char *)dev_subserial)), ez_false);
    cJSON *js_item = cJSON_GetArrayItem(js_root, index);
    json_to_tslmap_metadata(js_item, &tsl_metadata);

    if (1 == get_tsl_handle_count(js_root, tsl_metadata.handle))
    {
        /* 功能描述文件只有当前设备在使用，设备型号或者版本号已发生变更，tslpf文件删除 */
        ezlog_w(TAG_TSL, "ref = 0, del");
        CHECK_COND_DONE(ezos_kv_del(tsl_metadata.handle), ez_false);
        ezlog_w(TAG_TSL, "del pf succ!");
    }

    ezlog_w(TAG_TSL, "tslmap chg, update");
    cJSON_DeleteItemFromArray(js_root, index);
    CHECK_COND_DONE(!(pbuf_save = cJSON_PrintUnformatted(js_root)), ez_false);
    CHECK_COND_DONE(ezos_kv_raw_set(EZ_KV_DEFALUT_KEY_TSLMAP, pbuf_save, (uint32_t)strlen(pbuf_save)), ez_false);
    ezlog_w(TAG_TSL, "update succ!");

done:
    SAFE_FREE(pbuf);
    SAFE_FREE(pbuf_save);
    cJSON_Delete(js_root);

    return rv;
}

static ez_bool_t tsl_adapter_profile_download(ez_tsl_devinfo_t *dev_info)
{
    if (!tsl_adapter_dev_add(dev_info, ez_true))
    {
        return ez_false;
    }

    if (!tsl_adapter_dl_task_yeild())
    {
        return ez_false;
    }

    return ez_true;
}

static ez_bool_t tsl_adapter_ref_del(ez_tsl_devinfo_t *dev_info)
{
    ez_bool_t rv = ez_false;

    if (NULL == dev_info || NULL == g_capacities_list)
    {
        return ez_false;
    }

    ezos_mutex_lock(g_capacities_mutex);

    for (size_t i = 0; i < ezlist_size(g_capacities_list); i++)
    {
        ez_iot_tsl_capacity_t *capacity = (ez_iot_tsl_capacity_t *)ezlist_getat(g_capacities_list, i, NULL, ez_false);
        if (0 == strcmp((char *)dev_info->dev_type, capacity->dev_type) && 0 == strcmp((char *)dev_info->dev_firmwareversion, capacity->dev_fw_ver))
        {
            ezlog_w(TAG_TSL, "profile found, --ref:%d.", capacity->ref - 1);

            if (--capacity->ref <= 0)
            {
                free_profile_memory(capacity);
                ezlist_removeat(g_capacities_list, i);
                rv = ez_false;
            }
            else
            {
                rv = ez_true;
            }

            break;
        }
    }

    ezos_mutex_unlock(g_capacities_mutex);

    return rv;
}

static ez_bool_t tsl_adapter_dev_add(ez_tsl_devinfo_t *dev_info, ez_bool_t need_get)
{
    dev_info_t tsl_devinfo = {0};

    ezos_mutex_lock(g_profile_mutex);

    ezos_strncpy(tsl_devinfo.dev_fw_ver, (char *)dev_info->dev_firmwareversion, sizeof(tsl_devinfo.dev_fw_ver) - 1);
    ezos_strncpy(tsl_devinfo.dev_type, (char *)dev_info->dev_type, sizeof(tsl_devinfo.dev_type) - 1);
    ezos_strncpy(tsl_devinfo.dev_sn, (char *)dev_info->dev_subserial, sizeof(tsl_devinfo.dev_sn) - 1);
    tsl_devinfo.status = need_get ? status_need_update : status_done;

    ezlist_addlast(g_tsl_dev_info_list, (void *)&tsl_devinfo, sizeof(dev_info_t));

    ezos_mutex_unlock(g_profile_mutex);

    tsl_adapter_profile_save(dev_info, NULL, 0);

    return ez_true;
}

static void tsl_adapter_dev_update(ez_tsl_devinfo_t *dev_info, dev_stauts_e status)
{
    ezos_mutex_lock(g_profile_mutex);

    dev_info_t *dev = NULL;
    size_t size = ezlist_size(g_tsl_dev_info_list);
    for (size_t i = 0; i < size; i++)
    {
        dev = ezlist_getat(g_tsl_dev_info_list, i, NULL, ez_false);
        if (0 == strcmp(dev->dev_sn, (char *)dev_info->dev_subserial))
        {
            break;
        }

        dev = NULL;
    }

    if (NULL == dev)
    {
        goto done;
    }

    switch (status)
    {
    case status_done:
    {
        if (dev->download_info)
        {
            ezos_free(dev->download_info);
            dev->download_info = NULL;
        }
        dev->status = status;
    }
    break;
    case status_need_update:
    {
        time_countdown(&dev->timer, 30 * 1000);
    }
    break;
    case status_query:
    {
        time_countdown(&dev->timer, 30 * 1000);
        dev->status = status;
    }
    break;
    case status_loading:
    {
        dev->status = status;
    }
    break;
    default:
        break;
    }

done:
    ezos_mutex_unlock(g_profile_mutex);
}

static void tsl_adapter_dev_del(ez_tsl_devinfo_t *dev_info)
{
    ezos_mutex_lock(g_profile_mutex);

    dev_info_t *dev = NULL;
    size_t size = ezlist_size(g_tsl_dev_info_list);
    for (size_t i = 0; i < size; i++)
    {
        dev = ezlist_getat(g_tsl_dev_info_list, i, NULL, ez_false);
        if (0 != strcmp(dev->dev_sn, (char *)dev_info->dev_subserial))
        {
            continue;
        }

        ezos_bzero((void *)&dev->timer, sizeof(ezos_timespec_t));

        if (dev->download_info)
        {
            ezos_free(dev->download_info);
        }

        ezlist_removeat(g_tsl_dev_info_list, i);
        break;
    }

    ezos_mutex_unlock(g_profile_mutex);
}

static ez_bool_t tsl_adapter_dev_find(const int8_t *sn, ez_tsl_devinfo_t *dev_info, dev_stauts_e *status)
{
    dev_info_t *dev = NULL;
    ez_bool_t rv = ez_false;

    if (NULL == sn)
    {
        return ez_false;
    }

    ezos_mutex_lock(g_profile_mutex);

    size_t size = ezlist_size(g_tsl_dev_info_list);
    for (size_t i = 0; i < size; i++)
    {
        dev = ezlist_getat(g_tsl_dev_info_list, i, NULL, ez_false);
        if (0 != strcmp(dev->dev_sn, (char *)sn))
        {
            continue;
        }

        if (dev_info)
        {
            ezos_strncpy((char *)dev_info->dev_subserial, dev->dev_sn, sizeof(dev->dev_sn) - 1);
            ezos_strncpy((char *)dev_info->dev_type, dev->dev_type, sizeof(dev->dev_type) - 1);
            ezos_strncpy((char *)dev_info->dev_firmwareversion, dev->dev_fw_ver, sizeof(dev->dev_fw_ver) - 1);
        }

        if (status)
        {
            *status = dev->status;
        }
        rv = ez_true;
    }

    ezos_mutex_unlock(g_profile_mutex);

    return rv;
}

static ez_bool_t tsl_adapter_dl_task_yeild()
{
    ez_bool_t rv = ez_false;
    ezos_mutex_lock(g_profile_mutex);

    do
    {
        if (g_profile_download_thread_running)
        {
            rv = ez_true;
            break;
        }

        if (0 != ezos_thread_create(NULL, "ez_profile_get",
                                    ez_profile_get_thread, NULL,
                                    CONFIG_EZIOT_TSL_DOWNLOAD_STACK_SIZE,
                                    CONFIG_EZIOT_TSL_DOWNLOAD_TASK_PRIORITY))
        {
            ezlog_e(TAG_TSL, "create profile get thread failed.");
            break;
        }

        rv = ez_true;
    } while (0);

    ezos_mutex_unlock(g_profile_mutex);

    return rv;
}

void devinfo2index(ez_tsl_devinfo_t *dev_info, ez_char_t index[32])
{
    ez_char_t md5_output[16] = {0};
    ez_char_t base64_output[24 + 1] = {0};
    size_t olen = sizeof(base64_output);

    mbedtls_md5_context md5_ctx = {0};

    mbedtls_md5_init(&md5_ctx);
    mbedtls_md5_starts(&md5_ctx);

    mbedtls_md5_update(&md5_ctx, dev_info->dev_type, ezos_strlen((char *)dev_info->dev_type));
    mbedtls_md5_update(&md5_ctx, dev_info->dev_firmwareversion, ezos_strlen((char *)dev_info->dev_firmwareversion));
    mbedtls_md5_finish(&md5_ctx, md5_output);

    mbedtls_base64_encode(base64_output, sizeof(base64_output), &olen, md5_output, sizeof(md5_output));
    ezos_memset(index, 0, 32);
    ezos_snprintf(index, 32, "%s_%s", EZ_TSL_KEY_TSL_PREFIX, base64_output);
}

static ez_void_t iot_core_event_route(ez_kernel_event_t *ptr_event)
{
    if (!ptr_event)
    {
        return;
    }

    switch (ptr_event->event_type)
    {
    case SDK_KERNEL_EVENT_ONLINE:
    case SDK_KERNEL_EVENT_SWITCHOVER:
        g_is_online = ez_true;
        break;
    case SDK_KERNEL_EVENT_BREAK:
        g_is_online = ez_false;
        break;
    default:
        break;
    }
}

static void tsl_data_route_cb(ez_kernel_submsg_v3_t *psub_msg)
{
    if (0 == ezos_strcmp(psub_msg->method, TSL_SERVICE_METHOD_NAME))
    {
        tsl_action_process(psub_msg);
    }
    else if (0 == ezos_strcmp(psub_msg->method, TSL_ATTRIBUTE_METHOD_NAME))
    {
        ezlog_e(TAG_TSL, "attr not support.");
    }
}

ez_void_t tsl_adapter_set_profile_url(char *dev_sn, char *url, char *md5, int expire)
{
    ez_tsl_download_info_t *download_info = NULL;
    int8_t dev_subserial[48] = {0};
    int8_t dev_type[24] = {0};
    int8_t dev_firmwareversion[36] = {0};
    ez_tsl_devinfo_t tsl_dev = {dev_subserial, dev_type, dev_firmwareversion};

    ezos_mutex_lock(g_profile_mutex);
    for (size_t i = 0; i < ezlist_size(g_tsl_dev_info_list); i++)
    {
        dev_info_t *dev = ezlist_getat(g_tsl_dev_info_list, i, NULL, ez_false);
        if (0 == strcmp(dev->dev_sn, dev_sn) && status_done != dev->status)
        {
            if (NULL != dev->download_info)
            {
                download_info = (ez_tsl_download_info_t *)dev->download_info;
            }
            else
            {
                download_info = (void *)ezos_malloc(sizeof(ez_tsl_download_info_t));
                ezos_memset(download_info, 0, sizeof(ez_tsl_download_info_t));
                dev->download_info = (void *)download_info;
            }

            if (NULL == download_info)
            {
                ezlog_e(TAG_TSL, "memory not enough.");
                break;
            }

            ezos_memset((void *)download_info, 0, sizeof(ez_tsl_download_info_t));
            ezos_snprintf(download_info->url, sizeof(download_info->url) - 1, "http://%s", url);
            download_info->expire = expire;
            ezos_strncpy(download_info->md5, md5, sizeof(download_info->md5) - 1);
            ezos_strncpy(download_info->dev_sn, dev_sn, sizeof(download_info->dev_sn) - 1);

            ezos_strncpy((char *)dev_subserial, (char *)dev->dev_sn, sizeof(dev_subserial) - 1);
            ezos_strncpy((char *)dev_type, (char *)dev->dev_type, sizeof(dev_type) - 1);
            ezos_strncpy((char *)dev_firmwareversion, (char *)dev->dev_fw_ver, sizeof(dev_firmwareversion) - 1);

            break;
        }
    }

    ezos_mutex_unlock(g_profile_mutex);

    tsl_adapter_dev_update(&tsl_dev, status_loading);
}

static ez_void_t tsl_adapter_profile_query_rsp(const ez_char_t *rsp_msg)
{
    ez_err_t rv = EZ_TSL_ERR_SUCC;
    (ez_void_t) rv;

    cJSON *root = NULL;
    cJSON *url = NULL;
    cJSON *expire = NULL;
    cJSON *md5 = NULL;
    cJSON *dev_sn = NULL;

    root = cJSON_Parse(rsp_msg);
    CHECK_COND_DONE(!root, EZ_TSL_ERR_MEMORY);

    url = cJSON_GetObjectItem(root, "url");
    CHECK_COND_DONE(!url, EZ_TSL_ERR_MEMORY);
    CHECK_COND_DONE(cJSON_String != url->type, EZ_TSL_ERR_GENERAL);

    expire = cJSON_GetObjectItem(root, "expire");
    CHECK_COND_DONE(!expire, EZ_TSL_ERR_MEMORY);
    CHECK_COND_DONE(cJSON_Number != expire->type, EZ_TSL_ERR_MEMORY);

    md5 = cJSON_GetObjectItem(root, "md5");
    CHECK_COND_DONE(!md5, EZ_TSL_ERR_MEMORY);
    CHECK_COND_DONE(cJSON_String != md5->type, EZ_TSL_ERR_MEMORY);

    dev_sn = cJSON_GetObjectItem(root, "devSerial");
    CHECK_COND_DONE(!dev_sn, EZ_TSL_ERR_MEMORY);
    CHECK_COND_DONE(cJSON_String != dev_sn->type, EZ_TSL_ERR_MEMORY);

    tsl_adapter_set_profile_url(dev_sn->valuestring, url->valuestring, md5->valuestring, expire->valueint);

done:
    cJSON_Delete(root);
}