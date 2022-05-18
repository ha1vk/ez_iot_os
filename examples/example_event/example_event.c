#include "ezos.h"
#include "ez_iot_core.h"
#include "ez_iot_tsl.h"
#include "ezlog.h"
#include "cli.h"

extern int ez_cloud_init();
extern int ez_cloud_base_init();
static int ez_cloud_tsl_init();
static ez_int32_t tsl_notice(ez_tsl_event_e event_type, ez_void_t *data, ez_int32_t len);
static ez_int32_t tsl_action2dev(const ez_char_t *sn, const ez_tsl_rsc_t *rsc_info, const ez_tsl_key_t *key_info, const ez_tsl_value_t *value_in, ez_tsl_value_t *value_out);
static ez_int32_t tsl_property2cloud(const ez_char_t *sn, const ez_tsl_rsc_t *rsc_info, const ez_tsl_key_t *key_info, ez_tsl_value_t *value_out);
static ez_int32_t tsl_property2dev(const ez_char_t *sn, const ez_tsl_rsc_t *rsc_info, const ez_tsl_key_t *key_info, const ez_tsl_value_t *value);
static void example_event_report_null();
static void example_event_report_obj();

static ez_bool_t g_is_inited = ez_false;

static void example_event(char *buf, int len, int argc, char **argv)
{
    ezlog_init();
    ezlog_start();
    ezlog_filter_lvl(CONFIG_EZIOT_EXAMPLES_SDK_LOGLVL);

    ez_cloud_init();
    ez_cloud_base_init();
    ez_cloud_tsl_init();

    example_event_report_null();
    example_event_report_obj();
}

#ifdef FINSH_USING_MSH
MSH_CMD_EXPORT(example_event, eziot example event);
#else
EZOS_CLI_EXPORT("example_event", "eziot example event", example_event);
#endif

static void example_event_report_null()
{
    ez_err_t rv;
    ez_char_t dev_subserial[48];
#if defined(CONFIG_EZIOT_EXAMPLES_DEV_AUTH_MODE_SAP)
    ezos_strncpy(dev_subserial, CONFIG_EZIOT_EXAMPLES_DEV_SERIAL_NUMBER, sizeof(dev_subserial) - 1);
#elif defined(CONFIG_EZIOT_EXAMPLES_DEV_AUTH_MODE_LICENCE)
    ezos_snprintf(dev_subserial, sizeof(dev_subserial) - 1, "%s:%s", CONFIG_EZIOT_EXAMPLES_DEV_PRODUCT_KEY, CONFIG_EZIOT_EXAMPLES_DEV_NAME);
#endif

    /**
     * @brief "event_test"、"event_null"、"PetDryerRes"等功能点均为设备在开发控制台定义的功能点，
     * 该模拟设备功能点描述参考device_feautre->4LYV8SK7UKLBOUOVS6HXVX_V1.2.0_build_201201.bin
     */
    ez_tsl_key_t tsl_keyinfo = {.domain = "event_test", .key = "event_null"};
    ez_tsl_rsc_t rsc_info = {.res_type = "PetDryerRes", .local_index = "0"};

    rv = ez_iot_tsl_event_report(dev_subserial, &rsc_info, &tsl_keyinfo, NULL);
    if (EZ_TSL_ERR_PROFILE_LOADING == rv)
    {
        ezos_delay_ms(3000);
        rv = ez_iot_tsl_event_report(dev_subserial, &rsc_info, &tsl_keyinfo, NULL);
    }
}

static void example_event_report_obj()
{
    ez_err_t rv;
    ez_char_t dev_subserial[48];
#if defined(CONFIG_EZIOT_EXAMPLES_DEV_AUTH_MODE_SAP)
    ezos_strncpy(dev_subserial, CONFIG_EZIOT_EXAMPLES_DEV_SERIAL_NUMBER, sizeof(dev_subserial) - 1);
#elif defined(CONFIG_EZIOT_EXAMPLES_DEV_AUTH_MODE_LICENCE)
    ezos_snprintf(dev_subserial, sizeof(dev_subserial) - 1, "%s:%s", CONFIG_EZIOT_EXAMPLES_DEV_PRODUCT_KEY, CONFIG_EZIOT_EXAMPLES_DEV_NAME);
#endif

    /**
     * @brief "event_test"、"event_ext"、"PetDryerRes"等功能点均为设备在开发控制台定义的功能点，
     * 该模拟设备功能点描述参考device_feautre->4LYV8SK7UKLBOUOVS6HXVX_V1.2.0_build_201201.bin
     */
    static char *js_rep = "{\"ext\":{\"psd\":\"consequat sit in\"}}";
    ez_tsl_key_t tsl_keyinfo = {.domain = "event_test", .key = "event_ext"};
    ez_tsl_rsc_t rsc_info = {.res_type = "PetDryerRes", .local_index = "0"};
    ez_tsl_value_t tsl_value = {.type = EZ_TSL_DATA_TYPE_OBJECT, .size = ezos_strlen(js_rep), .value = js_rep};

    rv = ez_iot_tsl_event_report(dev_subserial, &rsc_info, &tsl_keyinfo, &tsl_value);
    if (EZ_TSL_ERR_PROFILE_LOADING == rv)
    {
        ezos_delay_ms(3000);
        rv = ez_iot_tsl_event_report(dev_subserial, &rsc_info, &tsl_keyinfo, &tsl_value);
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
    ez_iot_tsl_reg(NULL);

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
    return -1;
}

static ez_int32_t tsl_property2dev(const ez_char_t *sn, const ez_tsl_rsc_t *rsc_info, const ez_tsl_key_t *key_info, const ez_tsl_value_t *value)
{
    return -1;
}

static ez_int32_t tsl_notice(ez_tsl_event_e event_type, ez_void_t *data, ez_int32_t len)
{
    return 0;
}