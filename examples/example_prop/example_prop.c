#include "ezos.h"
#include "ez_iot_core.h"
#include "ez_iot_tsl.h"
#include "ezlog.h"
#include "cli.h"

#define KV_RSCC "PetDryerRes"
#define KV_LOCALINDEX "0"
#define KV_DOMAIN "attr_test"

#define KV_NAME1 "attr_r_bool"
#define KV_NAME2 "attr_rw_bool"
#define KV_NAME3 "attr_rw_int"
#define KV_NAME4 "attr_r_int"
#define KV_NAME5 "attr_rw_str"
#define KV_NAME6 "attr_r_str"
#define KV_NAME7 "attr_rw_num"
#define KV_NAME8 "attr_r_num"
#define KV_NAME9 "attr_rw_obj"
#define KV_NAME10 "attr_r_obj"
#define KV_NAME11 "attr_r_array"
#define KV_NAME12 "attr_rw_array"
#define KV_NAME13 "attr_rw_int_multi"
#define KV_NAME14 "attr_rw_int_enum"
#define KV_NAME15 "attr_rw_str_enum"

extern int ez_cloud_init();
extern int ez_cloud_base_init();
static int ez_cloud_tsl_init();
static void example_prop_changed();

static ez_int32_t tsl_notice(ez_tsl_event_e event_type, ez_void_t *data, ez_int32_t len);
static ez_int32_t tsl_action2dev(const ez_char_t *sn, const ez_tsl_rsc_t *rsc_info, const ez_tsl_key_t *key_info, const ez_tsl_value_t *value_in, ez_tsl_value_t *value_out);
static ez_int32_t tsl_property2cloud(const ez_char_t *sn, const ez_tsl_rsc_t *rsc_info, const ez_tsl_key_t *key_info, ez_tsl_value_t *value_out);
static ez_int32_t tsl_property2dev(const ez_char_t *sn, const ez_tsl_rsc_t *rsc_info, const ez_tsl_key_t *key_info, const ez_tsl_value_t *value);

static ez_bool_t g_is_inited = ez_false;
static ez_char_t g_dev_subserial[48];

typedef struct
{
    ez_kv_default_node_t kv;
    ez_tsl_data_type_e type;
} kv_type_t;

static int bvalue = 0;
static int ivalue = 99;
static double dvalue = 9.99;
static int evalue1 = 2;
static int evalue2 = 30;

static int bvalue_chg1 = 1;
static int ivalue_chg1 = 88;
static double dvalue_chg1 = 8.88;
static int evalue1_chg1 = 2;
static int evalue2_chg1 = 20;

/**
 * @brief 初始数据
 * 
 */
static kv_type_t default_kv_set[] = {
    {{KV_NAME1, &bvalue, sizeof(bvalue)}, EZ_TSL_DATA_TYPE_BOOL},
    {{KV_NAME2, &bvalue, sizeof(bvalue)}, EZ_TSL_DATA_TYPE_BOOL},
    {{KV_NAME3, &ivalue, sizeof(ivalue)}, EZ_TSL_DATA_TYPE_INT},
    {{KV_NAME4, &ivalue, sizeof(ivalue)}, EZ_TSL_DATA_TYPE_INT},
    {{KV_NAME5, "99", 2}, EZ_TSL_DATA_TYPE_STRING},
    {{KV_NAME6, "99", 2}, EZ_TSL_DATA_TYPE_STRING},
    {{KV_NAME7, &dvalue, sizeof(dvalue)}, EZ_TSL_DATA_TYPE_DOUBLE},
    {{KV_NAME8, &dvalue, sizeof(dvalue)}, EZ_TSL_DATA_TYPE_DOUBLE},
    {{KV_NAME9, "{\"value\":\"99\"}", 14}, EZ_TSL_DATA_TYPE_OBJECT},
    {{KV_NAME10, "{\"value\":\"99\"}", 14}, EZ_TSL_DATA_TYPE_OBJECT},
    {{KV_NAME11, "[{\"p_int\":99,\"p_enum\":30},{\"p_int\":88,\"p_enum\":30}]", 51}, EZ_TSL_DATA_TYPE_ARRAY},
    {{KV_NAME12, "[{\"p_int\":99,\"p_enum\":30},{\"p_int\":88,\"p_enum\":30}]", 51}, EZ_TSL_DATA_TYPE_ARRAY},
    {{KV_NAME13, &evalue2, sizeof(evalue2)}, EZ_TSL_DATA_TYPE_INT},
    {{KV_NAME14, &evalue1, sizeof(evalue1)}, EZ_TSL_DATA_TYPE_INT},
    {{KV_NAME15, "enum3", 5}, EZ_TSL_DATA_TYPE_STRING},
};

/**
 * @brief 变更的数据
 * 
 */
static kv_type_t default_kv_set_1[] = {
    {{KV_NAME1, &bvalue_chg1, sizeof(bvalue_chg1)}, EZ_TSL_DATA_TYPE_BOOL},
    {{KV_NAME2, &bvalue_chg1, sizeof(bvalue_chg1)}, EZ_TSL_DATA_TYPE_BOOL},
    {{KV_NAME3, &ivalue_chg1, sizeof(ivalue_chg1)}, EZ_TSL_DATA_TYPE_INT},
    {{KV_NAME4, &ivalue_chg1, sizeof(ivalue_chg1)}, EZ_TSL_DATA_TYPE_INT},
    {{KV_NAME5, "88", 2}, EZ_TSL_DATA_TYPE_STRING},
    {{KV_NAME6, "88", 2}, EZ_TSL_DATA_TYPE_STRING},
    {{KV_NAME7, &dvalue_chg1, sizeof(dvalue_chg1)}, EZ_TSL_DATA_TYPE_DOUBLE},
    {{KV_NAME8, &dvalue_chg1, sizeof(dvalue_chg1)}, EZ_TSL_DATA_TYPE_DOUBLE},
    {{KV_NAME9, "{\"value\":\"88\"}", 14}, EZ_TSL_DATA_TYPE_OBJECT},
    {{KV_NAME10, "{\"value\":\"88\"}", 14}, EZ_TSL_DATA_TYPE_OBJECT},
    {{KV_NAME11, "[{\"p_int\":88,\"p_enum\":20},{\"p_int\":88,\"p_enum\":20}]", 51}, EZ_TSL_DATA_TYPE_ARRAY},
    {{KV_NAME12, "[{\"p_int\":88,\"p_enum\":20},{\"p_int\":88,\"p_enum\":20}]", 51}, EZ_TSL_DATA_TYPE_ARRAY},
    {{KV_NAME13, &evalue2_chg1, sizeof(evalue2_chg1)}, EZ_TSL_DATA_TYPE_INT},
    {{KV_NAME14, &evalue1_chg1, sizeof(evalue1_chg1)}, EZ_TSL_DATA_TYPE_INT},
    {{KV_NAME15, "enum2", 5}, EZ_TSL_DATA_TYPE_STRING},
};

void example_prop(char *buf, int len, int argc, char **argv)
{
    ezlog_init();
    ezlog_start();
    ezlog_filter_lvl(CONFIG_EZIOT_EXAMPLES_SDK_LOGLVL);

    ez_cloud_init();
    ez_cloud_base_init();
    ez_cloud_tsl_init();
}

void example_prop_chg(char *buf, int len, int argc, char **argv)
{
    ezlog_init();
    ezlog_start();
    ezlog_filter_lvl(CONFIG_EZIOT_EXAMPLES_SDK_LOGLVL);

    ez_cloud_init();
    ez_cloud_base_init();
    ez_cloud_tsl_init();

    example_prop_changed();
}

#ifdef FINSH_USING_MSH
MSH_CMD_EXPORT(example_prop, eziot example property sync);
MSH_CMD_EXPORT(example_prop_chg, eziot example property changed and report);
#else
EZOS_CLI_EXPORT("example_prop", "prop test", example_prop);
EZOS_CLI_EXPORT("example_prop_chg", "prop change test", example_prop_chg);

#endif

static int find_index_by_key(const char *key)
{
    int index = -1;

    for (size_t i = 0; i < sizeof(default_kv_set) / sizeof(kv_type_t); i++)
    {
        if (0 == ezos_strcmp(default_kv_set[i].kv.key, key))
        {
            index = i;
            break;
        }
    }

    return index;
}

/**
 * @brief 属性上报方式1：直接发送
 * 
 */
void example_prop_changed()
{
    ez_tsl_rsc_t rsc_info = {.res_type = KV_RSCC, .local_index = KV_LOCALINDEX};
    ez_tsl_key_t tsl_keyinfo = {.domain = KV_DOMAIN, .key = NULL};
    ez_tsl_value_t tsl_value = {0};

    for (size_t i = 0; i < sizeof(default_kv_set_1) / sizeof(kv_type_t); i++)
    {
        tsl_keyinfo.key = default_kv_set_1[i].kv.key;
        tsl_value.type = default_kv_set_1[i].type;

        if (EZ_TSL_DATA_TYPE_BOOL == default_kv_set_1[i].type ||
            EZ_TSL_DATA_TYPE_INT == default_kv_set_1[i].type)
        {
            tsl_value.size = sizeof(tsl_value.value_int);
            ezos_memcpy(&tsl_value.value_int, default_kv_set_1[i].kv.value, tsl_value.size);
        }
        else if (EZ_TSL_DATA_TYPE_DOUBLE == default_kv_set_1[i].type)
        {
            tsl_value.size = sizeof(tsl_value.value_double);
            ezos_memcpy(&tsl_value.value_double, default_kv_set_1[i].kv.value, tsl_value.size);
        }
        else
        {
            tsl_value.value = default_kv_set_1[i].kv.value;
            tsl_value.size = default_kv_set_1[i].kv.value_len;
        }

        ez_iot_tsl_property_report(g_dev_subserial, &rsc_info, &tsl_keyinfo, &tsl_value);
    }
}

static int ez_cloud_tsl_init()
{
    ez_tsl_callbacks_t tsl_things_cbs = {tsl_notice, tsl_action2dev, tsl_property2cloud, tsl_property2dev};

    if (g_is_inited)
    {
        return 0;
    }

    ez_iot_tsl_init(&tsl_things_cbs);
    ez_iot_tsl_reg(NULL, NULL);

#if defined(CONFIG_EZIOT_EXAMPLES_DEV_AUTH_MODE_SAP)
    ezos_strncpy(g_dev_subserial, CONFIG_EZIOT_EXAMPLES_DEV_SERIAL_NUMBER, sizeof(g_dev_subserial) - 1);
#elif defined(CONFIG_EZIOT_EXAMPLES_DEV_AUTH_MODE_LICENCE)
    ezos_snprintf(g_dev_subserial, sizeof(g_dev_subserial) - 1, "%s:%s", CONFIG_EZIOT_EXAMPLES_DEV_PRODUCT_KEY, CONFIG_EZIOT_EXAMPLES_DEV_NAME);
#endif

    g_is_inited = ez_true;
    return 0;
}

static ez_int32_t tsl_action2dev(const ez_char_t *sn, const ez_tsl_rsc_t *rsc_info, const ez_tsl_key_t *key_info,
                                 const ez_tsl_value_t *value_in, ez_tsl_value_t *value_out)
{
    return 0;
}

static ez_int32_t tsl_property2cloud(const ez_char_t *sn, const ez_tsl_rsc_t *rsc_info, const ez_tsl_key_t *key_info, ez_tsl_value_t *value_out)
{
    ez_int32_t rv;
    void *p = NULL;

    ezlog_w(TAG_APP, "property sync");
    ezlog_d(TAG_APP, "serial number:%s", sn);                  //global表示单品或网关，子设备为对应序列号
    ezlog_d(TAG_APP, "resource type:%s", rsc_info->res_type);  //资源类型，多用于复合类设备，如灯+视频，插座+灯
    ezlog_d(TAG_APP, "local index:%s", rsc_info->local_index); //通道号，如多开关0\1\2\3
    ezlog_d(TAG_APP, "domain:%s", key_info->domain);           //功能点领域，功能点的分类或域名空间
    ezlog_d(TAG_APP, "identifier:%s", key_info->key);          //具体功能点

    int index = find_index_by_key(key_info->key);
    if (-1 == index)
    {
        ezlog_e(TAG_APP, "error occour, key not found:%s", key_info->key);
        return -1;
    }

    value_out->type = default_kv_set[index].type;

    if (EZ_TSL_DATA_TYPE_BOOL == value_out->type ||
        EZ_TSL_DATA_TYPE_INT == value_out->type)
    {
        p = &value_out->value_int;
    }
    else if (EZ_TSL_DATA_TYPE_DOUBLE == value_out->type)
    {
        p = &value_out->value_double;
    }
    else
    {
        p = value_out->value;
    }

    rv = ezos_kv_raw_get(key_info->key, p, (ez_size_t *)&value_out->size);
    if (EZ_KV_ERR_SUCC != rv || 0 == value_out->size)
    {
        ezos_memcpy(p, default_kv_set[index].kv.value, default_kv_set[index].kv.value_len);
        value_out->size = default_kv_set[index].kv.value_len;
        rv = 0;
    }

    return rv;
}

static ez_int32_t tsl_property2dev(const ez_char_t *sn, const ez_tsl_rsc_t *rsc_info, const ez_tsl_key_t *key_info, const ez_tsl_value_t *value)
{
    ez_int32_t rv;
    ez_void_t *p;
    ez_int_t len;

    ezlog_w(TAG_APP, "recv property");
    ezlog_d(TAG_APP, "serial number:%s", sn);                  //global表示单品或网关，子设备为对应序列号
    ezlog_d(TAG_APP, "resource type:%s", rsc_info->res_type);  //资源类型，多用于复合类设备，如灯+视频，插座+灯
    ezlog_d(TAG_APP, "local index:%s", rsc_info->local_index); //通道号，如多开关0\1\2\3
    ezlog_d(TAG_APP, "domain:%s", key_info->domain);           //功能点领域，功能点的分类或域名空间
    ezlog_d(TAG_APP, "identifier:%s", key_info->key);          //具体功能点

    int index = find_index_by_key(key_info->key);
    if (-1 == index)
    {
        ezlog_e(TAG_APP, "error occour, key not found:%s", key_info->key);

        return -1;
    }

    if (EZ_TSL_DATA_TYPE_BOOL == value->type ||
        EZ_TSL_DATA_TYPE_INT == value->type)
    {
        p = (ez_void_t *)&value->value_int;
        len = sizeof(value->value_int);
    }
    else if (EZ_TSL_DATA_TYPE_DOUBLE == value->type)
    {
        p = (ez_void_t *)&value->value_double;
        len = sizeof(value->value_double);
    }
    else
    {
        p = (ez_void_t *)value->value;
        len = value->size;
    }

    rv = ezos_kv_raw_set(key_info->key, p, len);
    ez_iot_tsl_property_report(g_dev_subserial, rsc_info, key_info, NULL);

    return rv;
}

static ez_int32_t tsl_notice(ez_tsl_event_e event_type, ez_void_t *data, ez_int32_t len)
{
    return 0;
}
