#include "ezos.h"
#include "ez_iot_core.h"
#include "ez_iot_core_def.h"
#include "ezlog.h"

#define EZAPP_DEVID_KEY "ezapp_devid"

static ez_int32_t ez_event_notice_func(ez_event_e event_type, ez_void_t *data, ez_int32_t len);
static ez_server_info_t m_lbs_addr = {CONFIG_EZIOT_EXAMPLES_CLOUD_HOST, CONFIG_EZIOT_EXAMPLES_CLOUD_PORT};
static ez_dev_info_t m_dev_info = {0};
static ez_bool_t g_is_inited = ez_false;

extern int example_kv_init(const void *default_kv);
extern int example_kv_raw_set(const char *key, const void *value, size_t length);
extern int example_kv_raw_get(const char *key, void *value, size_t *length);
extern int example_kv_del(const char *key);
extern int example_kv_del_by_prefix(const char *key_prefix);
extern void example_kv_print(void);
extern int example_kv_deinit(void);

static ez_kv_func_t g_kv_func = {
    .ezos_kv_init = example_kv_init,
    .ezos_kv_raw_set = example_kv_raw_set,
    .ezos_kv_raw_get = example_kv_raw_get,
    .ezos_kv_del = example_kv_del,
    .ezos_kv_del_by_prefix = example_kv_del_by_prefix,
    .ezos_kv_print = example_kv_print,
    .ezos_kv_deinit = example_kv_deinit,
};

int ez_cloud_init()
{
    ez_byte_t devid[32] = {0};

    if (g_is_inited)
        return 0;

    ezos_strncpy(m_dev_info.dev_typedisplay, CONFIG_EZIOT_EXAMPLES_DEV_DISPLAY_NAME, sizeof(m_dev_info.dev_typedisplay) - 1);
    ezos_strncpy(m_dev_info.dev_firmwareversion, CONFIG_EZIOT_EXAMPLES_DEV_FIRMWARE_VERSION, sizeof(m_dev_info.dev_firmwareversion) - 1);

#if defined(CONFIG_EZIOT_EXAMPLES_DEV_AUTH_MODE_SAP)
    m_dev_info.auth_mode = 0;
    ezos_strncpy(m_dev_info.dev_type, CONFIG_EZIOT_EXAMPLES_DEV_TYPE, sizeof(m_dev_info.dev_type) - 1);
    ezos_strncpy(m_dev_info.dev_subserial, CONFIG_EZIOT_EXAMPLES_DEV_SERIAL_NUMBER, sizeof(m_dev_info.dev_subserial) - 1);
    ezos_strncpy(m_dev_info.dev_verification_code, CONFIG_EZIOT_EXAMPLES_DEV_VERIFICATION_CODE, sizeof(m_dev_info.dev_verification_code) - 1);
#elif defined(CONFIG_EZIOT_EXAMPLES_DEV_AUTH_MODE_LICENCE)
    m_dev_info.auth_mode = 1;
    ezos_strncpy(m_dev_info.dev_type, CONFIG_EZIOT_EXAMPLES_DEV_PRODUCT_KEY, sizeof(m_dev_info.dev_type) - 1);
    ezos_snprintf(m_dev_info.dev_subserial, sizeof(m_dev_info.dev_subserial) - 1, "%s:%s", CONFIG_EZIOT_EXAMPLES_DEV_PRODUCT_KEY, CONFIG_EZIOT_EXAMPLES_DEV_NAME);
    ezos_strncpy(m_dev_info.dev_verification_code, CONFIG_EZIOT_EXAMPLES_DEV_LICENSE, sizeof(m_dev_info.dev_verification_code) - 1);
#endif

    /* Step 1: 读取首次上线后固化的devid */
    //TODO

    /* Step 2: 设置devid及kv回调函数 */
    ez_iot_core_ctrl(EZ_CMD_DEVID_SET, (ez_void_t *)devid);
    ez_iot_core_ctrl(EZ_CMD_KVIMPL_SET, (ez_void_t *)&g_kv_func);

    /* Step 3: 初始化SDK */
    ez_iot_core_init(&m_lbs_addr, &m_dev_info, ez_event_notice_func);
    ez_iot_core_start();
    g_is_inited = ez_true;

    return 0;
}

static ez_int32_t ez_event_notice_func(ez_event_e event_type, ez_void_t *data, ez_int32_t len)
{
    switch (event_type)
    {
    case EZ_EVENT_ONLINE:
        ezlog_d(TAG_APP, "dev online!");
        break;
    case EZ_EVENT_OFFLINE:
        ezlog_d(TAG_APP, "dev offline!");
        break;
    case EZ_EVENT_DEVID_UPDATE:
        ezlog_i(TAG_APP, "devid update, new devid:");
        ezlog_hexdump(TAG_APP, 16, data, len);
        //TODO 需要固化devid
        break;
    default:
        break;
    }

    return 0;
}

static void example_online(void)
{
    ezlog_init();
    ezlog_start();
    ezlog_filter_lvl(CONFIG_EZIOT_EXAMPLES_SDK_LOGLVL);

    if (0 != ez_cloud_init())
    {
        ezlog_e(TAG_APP, "example online init err");
    }
}

#ifdef FINSH_USING_MSH
MSH_CMD_EXPORT(example_online, eziot example online);
#else
// int main(int argc, char **argv)
// {
//     return example_kv(argc, argv);
// }
#endif