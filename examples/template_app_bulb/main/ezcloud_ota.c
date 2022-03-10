#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ezcloud_ota.h"
#include "ez_iot_ota.h"
#include "ezlog.h"
#include "ezos_time.h" //延迟需要
#include "esp_system.h"
#include "dev_init.h"
#include "dev_info.h"
#include "product_config.h"
#include "bulb_business.h"
#include "config_implement.h"

static int ota_event_notify(ez_ota_res_t *pres, ez_ota_event_e event, void *data, int len);
extern ez_int32_t eztimer_create(ez_char_t *name, ez_int32_t time_out, ez_bool_t reload, void (*fun)(void));
static int download_data_cb(uint32_t total_len, uint32_t offset, void *data, uint32_t len, void *user_data);
static void download_result_cb(ez_ota_cb_result_e result, void *user_data);
static void show_upgrade_info(ez_ota_upgrade_info_t *upgrade_infos);
char g_PTID[72]={0};
static int bAppUpgrading = ez_false;
EZ_INT16 last_progress = -1;

int ez_ota_init()
{
    int rv	= -1;
    ez_ota_init_t init_info = {.cb.ota_recv_msg = ota_event_notify};
    rv=ez_iot_ota_init(&init_info);
    return rv;
}

void ez_ota_start()
{
    ez_ota_res_t ota_res = {0};
    char dev_firmwareversion[64] = {0};
    ez_ota_module_t szModules[1] =  {0};
    memset(&szModules,0,sizeof(szModules));
    ez_ota_modules_t modules = {1, &szModules};
    mk_soft_version(dev_firmwareversion);
    szModules[0].mod_name = get_dev_productKey();
	szModules[0].fw_ver = dev_firmwareversion;

    ez_iot_ota_modules_report(&ota_res, &modules, 5000);                      //上报升级模块的PID和版本信息
}

int ez_ota_reboot_report_reuslt()
{
	int rv  = -1;
	ez_ota_res_t pres = {0};
	
	ez_ota_t ota_info = {0};
	int ota_code_len=sizeof(ota_info.ota_code);
	if(0 != config_get_value(K_OTA_CODE,&ota_info.ota_code,&ota_code_len))
	{
		ezlog_e(TAG_APP, "config_read WIFI_CC error!");
		return -1;
	}
	if(REBOOT_NORMAL == ota_info.ota_code)
	{
 		ezlog_i(TAG_OTA, "not uppdate reboot,start report ready status ",rv);
		ez_iot_ota_status_ready(&pres, get_dev_productKey());            //上报状态0，清除服务器状态
	}
	else
	{

		/* 报升级成功与否*/    	
		if (REBOOT_OTA_SUCCEED == ota_info.ota_code)
		{
 			ezlog_i(TAG_OTA, "uppdate reboot,start report success status ");
			ez_iot_ota_status_succ(&pres, get_dev_productKey());
		}
		else
		{		
 			ezlog_i(TAG_OTA, "uppdate reboot,start report failed status ");
			ez_iot_ota_status_fail(&pres, get_dev_productKey(), "", OTA_CODE_BURN); //烧录过程中升级错误
		}

		ota_info.ota_code = REBOOT_NORMAL;
        config_set_value(K_OTA_CODE,&ota_info.ota_code,sizeof(ota_info.ota_code));
        ezos_delay_ms(10000); 
		ez_iot_ota_status_ready(&pres, get_dev_productKey());            //上报状态0，清除服务器状态
	}
    
    return 0;
}

static int ota_download_fun(ota_upgrade_info_t *upgrade_infos,int file_index)
{
	int rv = -1;
	
	ezlog_i(TAG_OTA, "wifi dev process upgrade  start\n ");
	OTA_USER_DATA_T *pstruUserOtaData = (OTA_USER_DATA_T *)malloc(sizeof(OTA_USER_DATA_T));
	if (NULL == pstruUserOtaData)
	{
		ezlog_e(TAG_OTA, "malloc download userdate failed!");
		return ota_code_mem;
	}
	memset(pstruUserOtaData, 0, sizeof(OTA_USER_DATA_T));
	ez_ota_download_info_t download_info = {0};
    snprintf(download_info.url, sizeof(download_info.url) - 1, "http://%s", upgrade_infos->pota_files[file_index].url);
    strncpy(download_info.degist, upgrade_infos->pota_files[file_index].degist, sizeof(download_info.degist) - 1);
    download_info.block_size = 1024;    //这个大小可以改成512字节的倍数
    download_info.timeout_s = 60 * 5;
    download_info.retry_max = upgrade_infos->retry_max;
    download_info.total_size = upgrade_infos->pota_files[file_index].size;

	pstruUserOtaData->pUpdate_partition = esp_ota_get_next_update_partition(NULL);
	if (!pstruUserOtaData->pUpdate_partition)
	{
		ezlog_e(TAG_OTA, "get next partition failed");
		free(pstruUserOtaData);
		pstruUserOtaData = NULL;
		return ota_code_genneral;
	}
	pstruUserOtaData->interval =  upgrade_infos->interval; 
	pstruUserOtaData->bSubdevUpgrade = false;
	pstruUserOtaData->file_num = upgrade_infos->file_num;
	strncpy(pstruUserOtaData->mod_name,upgrade_infos->pota_files[file_index].mod_name,sizeof(pstruUserOtaData->mod_name));

    rv = ez_iot_ota_download(&download_info, download_data_cb, download_result_cb, (void *)pstruUserOtaData);
   
    return rv;
}

static int ota_event_notify(ez_ota_res_t *pres, ez_ota_event_e event, void *data, int len)
{
    int rv = -1;
	ez_ota_t ota_info = {0};
    switch (event)
    {
    case START_UPGRADE: {
		bulb_ctrl_deinit();
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
        if (bAppUpgrading)   //0 != strlen(g_devsn)
        {
            break;
        }
        bAppUpgrading = ez_true;
        show_upgrade_info(upgrade_infos);
        rv = ota_download_fun((ota_upgrade_info_t *)upgrade_infos, 0);
        if (0 != rv)
		{
			bAppUpgrading = ez_false;
			ezlog_i(TAG_OTA, "upgrade file error. mod_name:%s ",upgrade_infos->pota_files->mod_name);
		}
		else
		{
			/* 0. ota begin */
			ezlog_i(TAG_OTA, "upgrade file created. mod_name:%s ",upgrade_infos->pota_files->mod_name);
			//ez_iot_ota_progress_report(pres, upgrade_infos->pota_files[iIndexWifiDev].mod_name, ota_state_starting, 0);
		}
    }
    break;
    default:
        break;
    }
	ota_info.ota_code = REBOOT_OTA_FAILED;  //初始状态先写成失败
    config_set_value(K_OTA_CODE,&ota_info.ota_code,sizeof(ota_info.ota_code));
    return rv;
}

static void show_upgrade_info(ez_ota_upgrade_info_t *upgrade_infos)
{
    ezlog_e(TAG_APP, "file_num:%d", upgrade_infos->file_num);
    ezlog_e(TAG_APP, "retry_max:%d", upgrade_infos->retry_max);
    ezlog_e(TAG_APP, "interval:%d", upgrade_infos->interval);

    for (int i = 0; i < upgrade_infos->file_num; i++)
    {
        ezlog_e(TAG_APP, "pota_files[%d]->module:%s", i, upgrade_infos->pota_files[i].mod_name);
        ezlog_e(TAG_APP, "pota_files[%d]->url:%s", i, upgrade_infos->pota_files[i].url);
        ezlog_e(TAG_APP, "pota_files[%d]->fw_ver:%s", i, upgrade_infos->pota_files[i].fw_ver);
        ezlog_e(TAG_APP, "pota_files[%d]->degist:%s", i, upgrade_infos->pota_files[i].degist);
        ezlog_e(TAG_APP, "pota_files[%d]->size:%d", i, upgrade_infos->pota_files[i].size);
        if (upgrade_infos->pota_files[i].pdiffs)
        {
            ezlog_e(TAG_APP, "pdiffs.degist: %s", upgrade_infos->pota_files[i].pdiffs->degist);
            ezlog_e(TAG_APP, "pdiffs.fw_ver_src: %s", upgrade_infos->pota_files[i].pdiffs->fw_ver_dst);
            ezlog_e(TAG_APP, "pdiffs.url: %s", upgrade_infos->pota_files[i].pdiffs->url);
            ezlog_e(TAG_APP, "pdiffs.size: %d", upgrade_infos->pota_files[i].pdiffs->size);
        }
    }
}

static int download_data_cb(EZ_UINT32  total_len, EZ_UINT32  offset, void *data, EZ_UINT32  len, void *user_data)
{
    int  rv = 0;
    EZ_INT16 progress = 0;
   
	OTA_USER_DATA_T *pStruUserOtaData = (OTA_USER_DATA_T *)user_data;

    //ezlog_e(TAG_APP, "download_data_cb, total_len:%d, offset:%d, length:%d,bSbudevUpgrade=%d,interval=%d"
    //, total_len, offset, len,pStruUserOtaData->bSubdevUpgrade,pStruUserOtaData->interval); //打印会刷屏
	if (!pStruUserOtaData->bSubdevUpgrade)
	{
		/*第一次往flash 写数据，先清空数据，这里esp32 需要消耗一定时间*/
		if(0 == offset )//
		{
			ezlog_i(TAG_OTA, "starting ota to partition ,first erase partition");
			ez_iot_ota_progress_report(NULL, pStruUserOtaData->mod_name, OTA_STATE_DOWNLOADING, 0);
			//esp  擦除flash会假死8到10s，这里先slepp 2s ，让系统正常运转将升级response 发出去 
            ezos_delay_ms(2000);
			rv = esp_ota_begin(pStruUserOtaData->pUpdate_partition, OTA_SIZE_UNKNOWN, &pStruUserOtaData->update_handle);
			if (rv != ESP_OK)
			{
				ezlog_e(TAG_OTA, "esp_ota_begin failed, error=%d", rv);
				return ota_code_genneral;
			}
			ezlog_i(TAG_OTA, "starting ota to partition :%s,size=%d,adress=0x%x,handle=%d", 
			pStruUserOtaData->pUpdate_partition->label,pStruUserOtaData->pUpdate_partition->size,
			pStruUserOtaData->pUpdate_partition->address,pStruUserOtaData->update_handle);			
		}

		/* 1. downloading  & dynamic burning
		   模组自身升级，由于模组内存少，这里实时写flash
		*/	
		
		if(esp_ota_write(pStruUserOtaData->update_handle, data, len) != ESP_OK)
		{
			ezlog_e(TAG_OTA, "esp_ota_write failed! 0x%x",pStruUserOtaData->update_handle);
			return ota_code_genneral;
		}
        // ezlog_e(TAG_OTA, "total len is :%d",total_len );
        // ezlog_e(TAG_OTA, "offset is :%d",offset );
        // ezlog_e(TAG_OTA, "len is :%d",len );
		if(total_len == offset + len )
		{
			/* 2 . integrity check and signature check
					这里主要保证写入flash的升级数据没有出错，数据写完后，乐鑫模组内部会做一个校验，比如烧录了一个esp32 的程序，这里会出错
					升级包本身的数据校验在ota 模块内部file_download 下载过程中边下载边校验已完成。
			*/	   
            //ezlog_e(TAG_OTA, "total len is :%d",total_len );
			if (esp_ota_end(pStruUserOtaData->update_handle) != ESP_OK)
			{
				rv = ota_code_genneral;
				ezlog_e(TAG_OTA, "esp_ota_end failed! 0x%x",pStruUserOtaData->update_handle);
			}
		}
	}
	else
	{
		if(0 == offset )//
		{
			ezlog_i(TAG_OTA, "starting ota to mcu ,first ymodem send header,totallen %d",total_len);
			if (rv == -1)
			{
				ezlog_e(TAG_OTA, "can't receive flag from remote device. ota break!");
			}
		}
		if(total_len == offset + len )
		{
			//mcu 数据通过ymodem方式全部发送成功，这里wifi 模组要求mcu进行切分区操作。
			ezlog_i(TAG_OTA, "ota send all date to uart finished."); 
		}
	}
        // ezlog_e(TAG_OTA, "total len is :%d",total_len );
        // ezlog_e(TAG_OTA, "offset is :%d",offset );
        // ezlog_e(TAG_OTA, "len is :%d",len );
        progress =(offset+len) * 100 / total_len;
        ezlog_e(TAG_OTA, "ota progress= %d",progress);
        //g_progress=progress;
        //g_mod_name=pStruUserOtaData->mod_name;  
    if(progress!=last_progress&&(progress%10==0))   
	{
       ez_iot_ota_progress_report(NULL, pStruUserOtaData->mod_name, OTA_STATE_DOWNLOADING, progress);
	}
	if(progress==100)
	{
       rv=0;
	}
	last_progress=progress;    //保证不会因为progress值不变重复上报
    return rv ;
}

static void download_result_cb(ez_ota_cb_result_e result, void *user_data)
{
 int  rv = -1;
	 OTA_USER_DATA_T *pStruUserOtaData = (OTA_USER_DATA_T *)user_data;
	// char mcu_mod_name[32]= {0};
      ez_ota_res_t pres = {0};
	  ez_ota_t ota_info = {0};

	// bAppUpgrading = ez_false;

	 ezlog_w(TAG_APP, "download_result_cb");

    
    if (RESULT_FAILED == result)
    {		
        ez_iot_ota_status_fail(&pres, pStruUserOtaData->mod_name, "", OTA_STATE_DOWNLOADING); //上报下载过程中升级错误的 
    }
    else if (RESULT_SUC== result) //下载都成功，包括esp 写flash全部成功，以及ymodem 发数据到mcu设备都成功
    {
		rv = esp_ota_set_boot_partition(esp_ota_get_next_update_partition(NULL));		
    }
    	
    if (ESP_OK !=rv)
	{
		ota_info.ota_code = REBOOT_OTA_FAILED; 
		ezlog_e(TAG_OTA, "one module update,esp_ota_set_boot_partition failed! err=0x%x", rv);					
	}
	else
	{
		ota_info.ota_code = REBOOT_OTA_SUCCEED;
		ezlog_i(TAG_OTA, "one module update,wifidev change partition scucess..");
	}
	config_set_value(K_OTA_CODE,&ota_info.ota_code,sizeof(ota_info.ota_code));
	ez_iot_ota_progress_report(&pres, pStruUserOtaData->mod_name, OTA_STATE_REBOOTING, 100);//第二次进来，wifi 下载完毕，设备需要重启
	ez_iot_ota_progress_report(&pres, pStruUserOtaData->mod_name, OTA_STATE_REBOOTING, 100);//第二次进来，wifi 下载完毕，设备需要重启
	ez_iot_ota_deinit();
    ezos_delay_ms(2000);  //待升级状态都上报成功后，在实际重启
	esp_restart();
}