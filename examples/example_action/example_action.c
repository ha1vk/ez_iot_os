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

static ez_bool_t g_is_inited = ez_false;

static void example_action(char *buf, int len, int argc, char **argv)
{
    ezlog_init();
    ezlog_start();
    ezlog_filter_lvl(CONFIG_EZIOT_EXAMPLES_SDK_LOGLVL);

    ez_cloud_init();
    ez_cloud_base_init();
    ez_cloud_tsl_init();
}

#ifdef FINSH_USING_MSH
MSH_CMD_EXPORT(example_action, eziot example action);
#else
EZOS_CLI_EXPORT("example_action", "action test", example_action);
#endif

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

/**
 * @brief 处理云端下发的操作命令
 * 
 */
static ez_int32_t tsl_action2dev(const ez_char_t *sn, const ez_tsl_rsc_t *rsc_info, const ez_tsl_key_t *key_info,
                                 const ez_tsl_value_t *value_in, ez_tsl_value_t *value_out)
{
    ez_int32_t rv = 0;

    ezlog_w(TAG_APP, "recv action");
    ezlog_d(TAG_APP, "serial number:%s", sn);                  //global表示单品或网关，子设备为对应序列号
    ezlog_d(TAG_APP, "resource type:%s", rsc_info->res_type);  //资源类型，多用于复合类设备，如灯+视频，插座+灯
    ezlog_d(TAG_APP, "local index:%s", rsc_info->local_index); //通道号，如多开关0\1\2\3
    ezlog_d(TAG_APP, "domain:%s", key_info->domain);           //功能点领域，功能点的分类或域名空间
    ezlog_d(TAG_APP, "identifier:%s", key_info->key);          //具体功能点

    if (0 == ezos_strcmp("action_null", key_info->key))
    {
        //TODO 设备执行操作, 成功rv=0, 失败rv=-1
        ezlog_d(TAG_APP, "value:null");
    }
    else if (0 == ezos_strcmp("action_int", key_info->key))
    {
        ezlog_d(TAG_APP, "value:%d", value_in->value_int);
        //TODO 设备执行操作, 成功rv=0, 失败rv=-1
    }

    return rv;
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