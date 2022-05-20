#include "ezos.h"
#include "ez_iot_ota.h"
#include "ezlog.h"
#include "cli.h"

extern int ez_cloud_init();
static int ez_cloud_ota_init();

static ez_int32_t ota_event_notify(ez_ota_res_t *pres, ez_ota_event_e event, ez_void_t *data, ez_int32_t len);
static ez_int32_t download_data_cb(ez_uint32_t total_len, ez_uint32_t offset, ez_void_t *data, ez_uint32_t len, ez_void_t *user_data);
static ez_void_t download_result_cb(ez_ota_cb_result_e result, ez_void_t *user_data);
static ez_void_t show_upgrade_info(ez_ota_upgrade_info_t *upgrade_infos);

static char g_devsn[72] = {0};
static ez_bool_t g_is_inited = ez_false;
#if defined(CONFIG_EZIOT_EXAMPLES_DEV_AUTH_MODE_SAP)
ez_int8_t *g_module_name = (ez_int8_t *)CONFIG_EZIOT_EXAMPLES_DEV_TYPE;
#elif defined(CONFIG_EZIOT_EXAMPLES_DEV_AUTH_MODE_LICENCE)
ez_int8_t *g_module_name = (ez_int8_t *)CONFIG_EZIOT_EXAMPLES_DEV_PRODUCT_KEY;
#endif

void example_ota(char *buf, int len, int argc, char **argv)
{
    ezlog_init();
    ezlog_start();
    ezlog_filter_lvl(CONFIG_EZIOT_EXAMPLES_SDK_LOGLVL);

    ez_cloud_init();
    ez_cloud_ota_init();
}

#ifdef FINSH_USING_MSH
MSH_CMD_EXPORT(example_ota, eziot example ota);
#else
EZOS_CLI_EXPORT("example_ota", "ota test", example_ota);
#endif

static int ez_cloud_ota_init()
{

    ez_ota_init_t init_info = {.cb.ota_recv_msg = ota_event_notify};
    ez_ota_res_t ota_res = {0};
    ez_ota_module_t module = {g_module_name, (ez_int8_t *)CONFIG_EZIOT_EXAMPLES_DEV_FIRMWARE_VERSION};
    ez_ota_modules_t modules = {1, &module};

    if (g_is_inited)
    {
        return 0;
    }

    ez_iot_ota_init(&init_info);
    ez_iot_ota_modules_report(&ota_res, &modules, 5000);
    ez_iot_ota_status_ready(&ota_res, g_module_name);
    g_is_inited = ez_true;

    return 0;
}

static ez_int32_t ota_event_notify(ez_ota_res_t *pres, ez_ota_event_e event, ez_void_t *data, ez_int32_t len)
{
    ez_int32_t rv = -1;

    switch (event)
    {
    case START_UPGRADE:
    {
        ez_ota_upgrade_info_t *upgrade_infos = (ez_ota_upgrade_info_t *)data;
        if (NULL == upgrade_infos || sizeof(ez_ota_upgrade_info_t) != len)
        {
            break;
        }

        /* 异常情况 */
        if (upgrade_infos->file_num <= 0)
        {
            break;
        }

        /* 正在升级中 */
        if (0 != ezos_strlen(g_devsn))
        {
            break;
        }

        show_upgrade_info(upgrade_infos);
        ezos_strncpy(g_devsn, (char *)pres->dev_serial, sizeof(g_devsn) - 1);

        ez_ota_download_info_t download_info = {0};
        ezos_snprintf((ez_char_t *)download_info.url, sizeof(download_info.url) - 1, "http://%s", (ez_char_t *)upgrade_infos->pota_files[0].url);
        ezos_strncpy((ez_char_t *)download_info.degist, (ez_char_t *)upgrade_infos->pota_files[0].degist, sizeof(download_info.degist) - 1);
        download_info.block_size = 1024 * 5;
        download_info.timeout_s = 60 * 5;
        download_info.retry_max = upgrade_infos->retry_max;
        download_info.total_size = upgrade_infos->pota_files[0].size;

        /* 0. 开始下载 */
        rv = ez_iot_ota_download(&download_info, download_data_cb, download_result_cb, (void *)g_devsn);
        if (0 == rv)
        {
            ez_ota_res_t pres = {0};
            ez_iot_ota_progress_report(&pres, g_module_name, OTA_STATE_STARTING, 0);
        }
    }
    break;
    default:
        break;
    }

    return rv;
}

static ez_int32_t download_data_cb(ez_uint32_t total_len, ez_uint32_t offset, ez_void_t *data, ez_uint32_t len, ez_void_t *user_data)
{
    ez_ota_res_t pres = {0};
    ez_int16_t progress = ((offset + len) * (100 / 2) / total_len);

    /* 1. 下载中 */
    if (total_len > offset + len)
    {
        /* 防止上报太频繁，每10%上报一次 */
        if (0 == (progress % 10))
        {
            ez_iot_ota_progress_report(&pres, g_module_name, OTA_STATE_DOWNLOADING, progress);
        }

        return 0;
    }

    /* 2. 下载完成 */
    ez_iot_ota_progress_report(&pres, g_module_name, OTA_STATE_DOWNLOAD_COMPLETED, 50);

    /* 3. 完整性校验和签名校验 */
    //TODO
    ezos_delay_ms(5000);

    /* 4. 烧录 */
    ez_iot_ota_progress_report(&pres, g_module_name, OTA_STATE_BURNING, 60);
    ezos_delay_ms(5000);

    /* 5. 烧录完成 */
    ez_iot_ota_progress_report(&pres, g_module_name, OTA_STATE_BURNING_COMPLETED, 80);
    ezos_delay_ms(5000);

    /* 6. reboot */
    ez_iot_ota_progress_report(&pres, g_module_name, OTA_STATE_REBOOTING, 90);
    ezos_delay_ms(5000);

    /* 7. 重启后上报成功+新版本信息 */
    ez_iot_ota_status_succ(&pres, g_module_name);
    //TODO 上报新版本信息

    return 0;
}

static ez_void_t download_result_cb(ez_ota_cb_result_e result, ez_void_t *user_data)
{
    ez_ota_res_t pres = {0};
    ezos_memset(g_devsn, 0, sizeof(g_devsn));

    if (RESULT_FAILED == result)
    {
        ez_iot_ota_status_fail(&pres, g_module_name, (ez_int8_t *)"", OTA_CODE_DOWNLOAD);
    }
}

static void show_upgrade_info(ez_ota_upgrade_info_t *upgrade_infos)
{
    ezlog_i(TAG_APP, "file_num:%d", upgrade_infos->file_num);
    ezlog_i(TAG_APP, "retry_max:%d", upgrade_infos->retry_max);
    ezlog_i(TAG_APP, "interval:%d", upgrade_infos->interval);

    for (int i = 0; i < upgrade_infos->file_num; i++)
    {
        ezlog_i(TAG_APP, "pota_files[%d]->module:%s", i, upgrade_infos->pota_files[i].mod_name);
        ezlog_i(TAG_APP, "pota_files[%d]->url:%s", i, upgrade_infos->pota_files[i].url);
        ezlog_i(TAG_APP, "pota_files[%d]->fw_ver:%s", i, upgrade_infos->pota_files[i].fw_ver);
        ezlog_i(TAG_APP, "pota_files[%d]->degist:%s", i, upgrade_infos->pota_files[i].degist);
        ezlog_i(TAG_APP, "pota_files[%d]->size:%d", i, upgrade_infos->pota_files[i].size);
        if (upgrade_infos->pota_files[i].pdiffs)
        {
            ezlog_i(TAG_APP, "pdiffs.degist: %s", upgrade_infos->pota_files[i].pdiffs->degist);
            ezlog_i(TAG_APP, "pdiffs.fw_ver_src: %s", upgrade_infos->pota_files[i].pdiffs->fw_ver_dst);
            ezlog_i(TAG_APP, "pdiffs.url: %s", upgrade_infos->pota_files[i].pdiffs->url);
            ezlog_i(TAG_APP, "pdiffs.size: %d", upgrade_infos->pota_files[i].pdiffs->size);
        }
    }
}