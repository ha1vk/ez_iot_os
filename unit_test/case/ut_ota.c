/**
 * @file ut_getway.c
 * @author xurongjun (xurongjun@ezvizlife.com)
 * @brief ceshi 
 * @version 0.1
 * @date 2021-01-28
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include <stdlib.h>
#include <string.h>

#include "ez_iot_core.h"
#include "ez_iot_core_def.h"
#include "ez_iot_ota.h"
#include <ezlog.h>
#include <kv_imp.h>
#include "utest.h"
#include <ezos.h>


static int g_progress_interval = 0;
static int test_diffs_ota = 0;
static int ota_start_flag = 0;
static int ota_over_flag = 0;
ez_ota_download_info_t download_info;

static long global_init();

void ut_ota_test();
UTEST_TC_EXPORT(ut_ota_test, global_init, NULL, 60);

static int m_event_id = -1;
static ez_server_info_t m_lbs_addr = {CONFIG_EZIOT_UNIT_TEST_CLOUD_HOST, CONFIG_EZIOT_UNIT_TEST_CLOUD_PORT};
static ez_dev_info_t m_dev_info = {0};


static ez_kv_func_t g_kv_func = {
    .ezos_kv_init = kv_init,
    .ezos_kv_raw_set = kv_raw_set,
    .ezos_kv_raw_get = kv_raw_get,
    .ezos_kv_del = kv_del,
    .ezos_kv_del_by_prefix = kv_del_by_prefix,
    .ezos_kv_print = kv_print,
    .ezos_kv_deinit = kv_deinit,
};


static ez_int32_t file_download(ez_int32_t total_len, ez_int32_t offset, ez_void_t *data, ez_int32_t len, ez_void_t* user_data)
{
    static int process = 0;
    int temp_pro;
    ez_ota_res_t res;
    ezos_memset(&res ,0, sizeof(ez_ota_res_t));

    ezos_printf("total_len:%d,offset:%d,len:%d\n", total_len, offset, len);
    temp_pro = (len+offset)/(total_len/100);
    if(temp_pro != process)
    {
        process = temp_pro;
        ez_iot_ota_progress_report(&res, (ez_int8_t*)CONFIG_EZIOT_UNIT_TEST_DEV_DISPLAY_NAME, OTA_STATE_DOWNLOADING, process);
    }

    if(len+offset >= total_len)
    {
        ota_over_flag = 1;
    }
        
    return 0;
}

static void notify_fun(ez_ota_cb_result_e result, ez_void_t* user_data)
{
    ezos_printf("result:%d\n", (int)result);
}

static int ota_sample_download_file(ez_ota_upgrade_info_t* pupgrade_info)
{
    ezos_memset(&download_info, 0, sizeof(ez_ota_download_info_t));

    download_info.block_size = 1024*10;
    download_info.timeout_s = 5;
    download_info.retry_max = pupgrade_info->retry_max;

    ezos_printf("retry_max:%d\n", pupgrade_info->retry_max);

    if(NULL!=pupgrade_info->pota_files)
    {
		if (test_diffs_ota) {
			download_info.total_size = pupgrade_info->pota_files[0].pdiffs->size;
			ezos_snprintf((char*)download_info.url, sizeof(download_info.url), "http://%s", (char*)pupgrade_info->pota_files[0].pdiffs->url);
			ezos_strncpy((char*)download_info.degist, (char*)pupgrade_info->pota_files[0].pdiffs->degist, sizeof(download_info.degist) - 1);
			test_diffs_ota = 0;
		}
		else {
			download_info.total_size = pupgrade_info->pota_files[0].size;
			ezos_snprintf((char*)download_info.url, sizeof(download_info.url), "http://%s", (char*)pupgrade_info->pota_files[0].url);
			ezos_strncpy((char*)download_info.degist, (char*)pupgrade_info->pota_files[0].degist, sizeof(download_info.degist) - 1);
		}
		ezos_printf("url:%s \n", (char*)download_info.url);
        
        ota_start_flag = 1;
    }

    return 0;
}

void start_download()
{
	ezos_printf("url:%s \n", (char*)download_info.url);
    ez_iot_ota_download(&download_info, file_download, notify_fun, NULL);
    ota_start_flag = 0;
}


static int ota_event_notify(ez_ota_res_t* pres, ez_ota_event_e event, void *data, int len)
{
    int ret = 0, i = 0;
    ezos_printf("---------------------ota event :%d----------------\n", event);
    
    switch (event)
    {
    case START_UPGRADE:
        {
            ezos_printf("--------------------upgrade packet info ----------------\n");
            ez_ota_upgrade_info_t* pupgrade_info = (ez_ota_upgrade_info_t*)data;
            ezos_printf("file_num:%d \n", pupgrade_info->file_num);
            ezos_printf("retry_max:%d \n", pupgrade_info->retry_max);
            ezos_printf("interval:%d \n", pupgrade_info->interval);
      
            g_progress_interval = pupgrade_info->interval;

            for( i = 0;i < pupgrade_info->file_num; i++)
            {
                ezos_printf("pota_files[%d]->module:%s \n", i, pupgrade_info->pota_files[i].mod_name);
                ezos_printf("pota_files[%d]->url:%s \n", i, pupgrade_info->pota_files[i].url);
                ezos_printf("pota_files[%d]->fw_ver:%s \n", i, pupgrade_info->pota_files[i].fw_ver);
                ezos_printf("pota_files[%d]->degist:%s \n", i, pupgrade_info->pota_files[i].degist);
                ezos_printf("pota_files[%d]->size:%d \n", i, pupgrade_info->pota_files[i].size);
                if(NULL!=pupgrade_info->pota_files[i].pdiffs)
                {
                    ezos_printf("pdiffs.degist: %s \n", pupgrade_info->pota_files[i].pdiffs->degist);
                    ezos_printf("pdiffs.fw_ver_src: %s \n",pupgrade_info->pota_files[i].pdiffs->fw_ver_dst);
                    ezos_printf("pdiffs.url: %s \n",pupgrade_info->pota_files[i].pdiffs->url);
                    ezos_printf("pdiffs.size: %d \n",pupgrade_info->pota_files[i].pdiffs->size);
                }
            }

            ota_sample_download_file(pupgrade_info);
        
        }
        break;
    default:
        break;
    }
    return ret;
}


int ota_sample_module_info_report()
{
	ez_err_t ret = OTA_CODE_NONE;


	ez_ota_res_t  res_t;
	ez_ota_modules_t module_list;
	ez_ota_module_t list;

	ezos_memset(&module_list, 0, sizeof(ez_ota_modules_t));
	ezos_memset(&res_t, 0, sizeof(ez_ota_res_t));
	ezos_memset(&list, 0, sizeof(ez_ota_module_t));

	ezos_printf("-------------ota_sample_module_info_report----------\n");

	list.fw_ver = (ez_int8_t*)CONFIG_EZIOT_UNIT_TEST_DEV_FIRMWARE_VERSION;
	list.mod_name = (ez_int8_t*)CONFIG_EZIOT_UNIT_TEST_DEV_DISPLAY_NAME;

	module_list.num = 1;
	module_list.plist = &list;

	ret = ez_iot_ota_modules_report(&res_t, &module_list, 5000);
	if (OTA_CODE_NONE != ret)
	{
		ezos_printf("ota report module info err:0x%#0x !\n", ret);
		return -1;
	}
	return 0;
}

static ez_int32_t ez_event_notice_func(ez_event_e event_type, ez_void_t *data, ez_int32_t len)
{
    ezos_printf("ez_event_notice_func type:%d\n", event_type);
    switch (event_type)
    {
    case EZ_EVENT_ONLINE:
        m_event_id = EZ_EVENT_ONLINE;
        break;
    case EZ_EVENT_OFFLINE:
        /* save devid */
        break;
    case EZ_EVENT_DEVID_UPDATE:
        ezos_printf("ez_event_notice_func change devid:%s\n", (char *)data);
        break;

    default:
        break;
    }

    return 0;
}

static int dev_event_waitfor(int event_id, int time_ms)
{
    int ret = -1;
    int index = 0;
    m_event_id = -1;

    do
    {
        if (event_id == m_event_id)
        {
            ret = 0;
            break;
        }

        ezos_delay_ms(10);
        index += 10;
    } while (index < time_ms);

    return ret;
}

static int ota_start_waitfor(int time_ms)
{
    int index = 0;

    do
    {
        if (ota_start_flag)
        {
            break;
        }

        ezos_delay_ms(10);
        index += 10;
    } while (index < time_ms);

    return ota_start_flag;
}

static int ota_over_waitfor(int time_ms)
{
    int index = 0;

    do
    {
        if (ota_over_flag)
        {
            break;
        }

        ezos_delay_ms(10);
        index += 10;
    } while (index < time_ms);

    return ota_over_flag;
}

void ut_ota_test(void)
{
    ez_err_t rv;
    ez_byte_t devid[33] = "";
    ez_ota_init_t init;
    ez_ota_res_t res;
    ezos_memset(&res ,0, sizeof(ez_ota_res_t));

    init.cb.ota_recv_msg = ota_event_notify;   // 设置回调函数，用于接收平台下发的OTA升级消息 

    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_ctrl(EZ_CMD_DEVID_SET, (ez_void_t *)devid));
    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_ctrl(EZ_CMD_KVIMPL_SET, (ez_void_t *)&g_kv_func));
    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_init(&m_lbs_addr, &m_dev_info, ez_event_notice_func));

    uassert_int_equal(OTA_CODE_NONE, ez_iot_ota_init(&init));
    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_start());

    uassert_int_equal(0, dev_event_waitfor(EZ_EVENT_ONLINE, CONFIG_EZIOT_UNIT_TEST_CASE_TIEMOUT_SECONDS * 1000));
    
    uassert_int_equal(OTA_CODE_NONE, ota_sample_module_info_report());
    ezos_delay_ms(3000);
    uassert_int_equal(OTA_CODE_NONE, ez_iot_ota_status_ready(&res, (ez_int8_t*)CONFIG_EZIOT_UNIT_TEST_DEV_DISPLAY_NAME));
    
    
    rv = ota_start_waitfor(60000);
    if(rv != 1){
        uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_stop());
        uassert_int_equal(OTA_CODE_NONE, ez_iot_ota_deinit());
        return;
    }
    start_download();

    rv = ota_over_waitfor(60000);
    if(rv != 1){
        uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_stop());
        uassert_int_equal(OTA_CODE_NONE, ez_iot_ota_deinit());
        return;
    }
    ezos_delay_ms(1000);
    ez_iot_ota_progress_report(&res, (ez_int8_t*)CONFIG_EZIOT_UNIT_TEST_DEV_DISPLAY_NAME, OTA_STATE_DOWNLOAD_COMPLETED, OTA_PROGRESS_MAX);
    ezos_delay_ms(1000);
    ez_iot_ota_progress_report(&res, (ez_int8_t*)CONFIG_EZIOT_UNIT_TEST_DEV_DISPLAY_NAME, OTA_STATE_BURNING, OTA_PROGRESS_MAX);
    ezos_delay_ms(1000);
    ez_iot_ota_progress_report(&res, (ez_int8_t*)CONFIG_EZIOT_UNIT_TEST_DEV_DISPLAY_NAME, OTA_STATE_BURNING_COMPLETED, OTA_PROGRESS_MAX);
    ezos_delay_ms(1000);
    ez_iot_ota_progress_report(&res, (ez_int8_t*)CONFIG_EZIOT_UNIT_TEST_DEV_DISPLAY_NAME, OTA_STATE_REBOOTING, OTA_PROGRESS_MAX);
    ezos_delay_ms(1000);
    ez_iot_ota_status_succ(NULL, (ez_int8_t *)CONFIG_EZIOT_UNIT_TEST_DEV_DISPLAY_NAME);
    ezos_delay_ms(1000);
    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_stop());
    uassert_int_equal(OTA_CODE_NONE, ez_iot_ota_deinit());
}

static long global_init()
{
    ezlog_init();
    ezlog_start();
    ezlog_filter_lvl(CONFIG_EZIOT_UNIT_TEST_SDK_LOGLVL);

    ezos_strncpy(m_dev_info.dev_typedisplay, CONFIG_EZIOT_UNIT_TEST_DEV_DISPLAY_NAME, sizeof(m_dev_info.dev_typedisplay) - 1);
    ezos_strncpy(m_dev_info.dev_firmwareversion, CONFIG_EZIOT_UNIT_TEST_DEV_FIRMWARE_VERSION, sizeof(m_dev_info.dev_firmwareversion) - 1);

#if defined(CONFIG_EZIOT_UNIT_TEST_DEV_AUTH_MODE_SAP)
    m_dev_info.auth_mode = 0;
    ezos_strncpy(m_dev_info.dev_type, CONFIG_EZIOT_UNIT_TEST_DEV_TYPE, sizeof(m_dev_info.dev_type) - 1);
    ezos_strncpy(m_dev_info.dev_subserial, CONFIG_EZIOT_UNIT_TEST_DEV_SERIAL_NUMBER, sizeof(m_dev_info.dev_subserial) - 1);
    ezos_strncpy(m_dev_info.dev_verification_code, CONFIG_EZIOT_UNIT_TEST_DEV_VERIFICATION_CODE, sizeof(m_dev_info.dev_verification_code) - 1);
#elif defined(CONFIG_EZIOT_UNIT_TEST_DEV_AUTH_MODE_LICENCE)
    m_dev_info.auth_mode = 1;
    ezos_strncpy(m_dev_info.dev_type, CONFIG_EZIOT_UNIT_TEST_DEV_PRODUCT_KEY, sizeof(m_dev_info.dev_type) - 1);
    ezos_snprintf(m_dev_info.dev_subserial, sizeof(m_dev_info.dev_subserial) - 1, "%s:%s", CONFIG_EZIOT_UNIT_TEST_DEV_PRODUCT_KEY, CONFIG_EZIOT_UNIT_TEST_DEV_NAME);
    ezos_strncpy(m_dev_info.dev_verification_code, CONFIG_EZIOT_UNIT_TEST_DEV_LICENSE, sizeof(m_dev_info.dev_verification_code) - 1);
#endif

    return 0;
}
