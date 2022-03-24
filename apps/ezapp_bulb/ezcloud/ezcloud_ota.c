/*******************************************************************************
 * Copyright © 2017-2022 Ezviz Inc.
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
 * Contributors:
 * XuRongjun (xurongjun@ezvizlife.com)
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-03-20     xurongjun    first version 
 *******************************************************************************/

#include "ezos.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ezcloud_ota.h"
#include "ez_iot_ota.h"
#include "ezlog.h"
#include "ezos_time.h" //延迟需要
#include "device_info.h"
#include "hal_config.h"
#define OK 0
static ez_int32_t bAppUpgrading = ez_false;
ez_uint16_t last_progress = -1;
ez_size_t g_image_size = 0;

static ez_int32_t download_data_cb(ez_int32_t total_len, ez_int32_t offset, ez_void_t *data, ez_int32_t len, ez_void_t *user_data)
{
    ez_int32_t rv = 0;
    ez_uint16_t progress = 0;

    ez_ota_data_t *pStruUserOtaData = (ez_ota_data_t *)user_data;

    //ezlog_e(TAG_APP, "download_data_cb, total_len:%d, offset:%d, length:%d,bSbudevUpgrade=%d,interval=%d"
    //, total_len, offset, len,pStruUserOtaData->bSubdevUpgrade,pStruUserOtaData->interval); //打印会刷屏
    if (!pStruUserOtaData->bSubdevUpgrade)
    {
        /*第一次往flash 写数据，先清空数据，这里需要消耗一定时间*/
        if (0 == offset) //
        {
            ezlog_i(TAG_OTA, "starting ota to partition ,first erase partition");
            ez_iot_ota_progress_report(NULL, pStruUserOtaData->mod_name, OTA_STATE_DOWNLOADING, 0);
            //擦除flash会假死8到10s，这里先slepp 2s ，让系统正常运转将升级response 发出去
            ezos_delay_ms(2000);
            rv = hal_ota_begin(g_image_size);
            if (rv != OK)
            {
                ezlog_e(TAG_OTA, "ota_begin failed, error=%d", rv);
                return ota_code_genneral;
            }
            ezlog_i(TAG_OTA, "starting ota to partition");
        }

        /* 1. downloading  & dynamic burning
		   模组自身升级，由于模组内存少，这里实时写flash
		*/
        rv = hal_ota_write(data, len);
        if (rv != OK)
        {
            ezlog_e(TAG_OTA, "ota_write failed!");
            return ota_code_genneral;
        }

        if (total_len == offset + len)
        {
            /* 2 . integrity check and signature check
					这里主要保证写入flash的升级数据没有出错，数据写完后，模组内部会做一个校验，比如烧录了一个模组的程序，这里会出错
					升级包本身的数据校验在ota 模块内部file_download 下载过程中边下载边校验已完成。
			*/
            rv = hal_ota_end();
            if (rv != OK)
            {
                rv = ota_code_genneral;
                ezlog_e(TAG_OTA, "ota_end failed!");
            }
        }
    }
    else
    {
        if (0 == offset) //
        {
            ezlog_i(TAG_OTA, "starting ota to mcu ,first ymodem send header,totallen %d", total_len);
            if (rv == -1)
            {
                ezlog_e(TAG_OTA, "can't receive flag from remote device. ota break!");
            }
        }
        if (total_len == offset + len)
        {
            //mcu 数据通过ymodem方式全部发送成功，这里wifi 模组要求mcu进行切分区操作。
            ezlog_i(TAG_OTA, "ota send all date to uart finished.");
        }
    }
    progress = (offset + len) * 100 / total_len;
    ezlog_e(TAG_OTA, "ota progress= %d", progress);
    if (progress != last_progress && (progress % 10 == 0))
    {
        ez_iot_ota_progress_report(NULL, pStruUserOtaData->mod_name, OTA_STATE_DOWNLOADING, progress);
    }
    if (progress == 100)
    {
        rv = 0;
    }
    last_progress = progress; //保证不会因为progress值不变重复上报
    return rv;
}

static ez_void_t download_result_cb(ez_ota_cb_result_e result, ez_void_t *user_data)
{
    ez_int32_t rv = -1;
    ez_ota_data_t *pStruUserOtaData = (ez_ota_data_t *)user_data;
    ez_ota_res_t pres = {0};
    ez_ota_t ota_info = {0};

    ezlog_w(TAG_APP, "download_result_cb");

    if (RESULT_FAILED == result)
    {
        ez_iot_ota_status_fail(&pres, pStruUserOtaData->mod_name, "", OTA_STATE_DOWNLOADING); //上报下载过程中升级错误的
    }
    else if (RESULT_SUC == result)
    {
        rv = 0;
    }

    if (OK != rv)
    {
        ota_info.ota_code = REBOOT_OTA_FAILED;
        ezlog_e(TAG_OTA, "one module update,ota_set_boot_partition failed! err=0x%x", rv);
    }
    else
    {
        ota_info.ota_code = REBOOT_OTA_SUCCEED;
        ezlog_i(TAG_OTA, "one module update,wifidev change partition scucess..");
    }
    hal_config_set_int("ota_code",ota_info.ota_code);
    ez_iot_ota_progress_report(&pres, pStruUserOtaData->mod_name, OTA_STATE_REBOOTING, 100); //第二次进来，wifi 下载完毕，设备需要重启
    ez_iot_ota_progress_report(&pres, pStruUserOtaData->mod_name, OTA_STATE_REBOOTING, 100); //第二次进来，wifi 下载完毕，设备需要重启
    ez_iot_ota_deinit();
    ezos_delay_ms(2000); //待升级状态都上报成功后，在实际重启
    hal_ota_action();
}

static ez_int32_t ota_download_fun(ota_upgrade_info_t *upgrade_infos, ez_int32_t file_index)
{
    ez_int32_t rv = -1;

    ezlog_i(TAG_OTA, "wifi dev process upgrade  start\n ");
    ez_ota_data_t *pstruUserOtaData = (ez_ota_data_t *)ezos_malloc(sizeof(ez_ota_data_t));
    if (NULL == pstruUserOtaData)
    {
        ezlog_e(TAG_OTA, "malloc download userdate failed!");
        return ota_code_mem;
    }
    memset(pstruUserOtaData, 0, sizeof(ez_ota_data_t));
    ez_ota_download_info_t download_info = {0};
    snprintf(download_info.url, sizeof(download_info.url) - 1, "http://%s", upgrade_infos->pota_files[file_index].url);
    strncpy(download_info.degist, upgrade_infos->pota_files[file_index].degist, sizeof(download_info.degist) - 1);
    download_info.block_size = 1024; //这个大小可以改成512字节的倍数
    download_info.timeout_s = 60 * 5;
    download_info.retry_max = upgrade_infos->retry_max;
    download_info.total_size = upgrade_infos->pota_files[file_index].size;
    pstruUserOtaData->interval = upgrade_infos->interval;
    pstruUserOtaData->bSubdevUpgrade = ez_false;
    pstruUserOtaData->file_num = upgrade_infos->file_num;
    strncpy(pstruUserOtaData->mod_name, upgrade_infos->pota_files[file_index].mod_name, sizeof(pstruUserOtaData->mod_name));

    rv = ez_iot_ota_download(&download_info, download_data_cb, download_result_cb, (void *)pstruUserOtaData);

    return rv;
}

static ez_void_t show_upgrade_info(ez_ota_upgrade_info_t *upgrade_infos)
{
    ezlog_e(TAG_APP, "file_num:%d", upgrade_infos->file_num);
    ezlog_e(TAG_APP, "retry_max:%d", upgrade_infos->retry_max);
    ezlog_e(TAG_APP, "interval:%d", upgrade_infos->interval);

    for (ez_int32_t i = 0; i < upgrade_infos->file_num; i++)
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

static ez_int32_t ota_event_notify(ez_ota_res_t *pres, ez_ota_event_e event, ez_void_t *data, ez_int32_t len)
{
    ez_int32_t rv = -1;
    ez_ota_t ota_info = {0};
    switch (event)
    {
    case START_UPGRADE:
    {
        //bulb_ctrl_deinit();
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
        if (bAppUpgrading) //0 != strlen(g_devsn)
        {
            break;
        }
        bAppUpgrading = ez_true;
        show_upgrade_info(upgrade_infos);
        g_image_size = upgrade_infos->pota_files->size;
        ezlog_e(TAG_APP, "g_image_size:%d", g_image_size);
        rv = ota_download_fun((ota_upgrade_info_t *)upgrade_infos, 0);
        if (0 != rv)
        {
            bAppUpgrading = ez_false;
            ezlog_i(TAG_OTA, "upgrade file error. mod_name:%s ", upgrade_infos->pota_files->mod_name);
        }
        else
        {
            /* 0. ota begin */
            ezlog_i(TAG_OTA, "upgrade file created. mod_name:%s ", upgrade_infos->pota_files->mod_name);
            //ez_iot_ota_progress_report(pres, upgrade_infos->pota_files[iIndexWifiDev].mod_name, ota_state_starting, 0);
        }
    }
    break;
    default:
        break;
    }
    ota_info.ota_code = REBOOT_OTA_FAILED; //初始状态先写成失败
    hal_config_set_int("ota_code",ota_info.ota_code);
    return rv;
}

ez_void_t ezcloud_ota_init(ez_void_t)
{
    ez_ota_init_t init_info = {.cb.ota_recv_msg = ota_event_notify};
    ez_ota_res_t ota_res = {0};
    ez_ota_module_t module = {dev_info_get_type(), dev_info_get_fwver()};
    ez_ota_modules_t modules = {1, &module};

    ez_iot_ota_init(&init_info);
    ez_iot_ota_modules_report(&ota_res, &modules, 5000);
    //ez_iot_ota_status_ready(&ota_res, module.mod_name);
}

ez_int32_t ez_ota_reboot_report_result()
{
    ez_ota_res_t pres = {0};
    ez_ota_t ota_info = {0};
    ez_int32_t ota_code_len = sizeof(ota_info.ota_code);
    if (0 != hal_config_get_int("ota_code", &ota_info.ota_code, 2))
    {
        ezlog_e(TAG_APP, "config_read ota_code error!");
        return -1;
    }
    if (REBOOT_NORMAL == ota_info.ota_code)
    {
        ezlog_i(TAG_OTA, "not uppdate reboot, start report ready status");
        ez_iot_ota_status_ready(&pres, get_dev_productKey()); //上报状态0，清除服务器状态
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
        hal_config_set_int("ota_code",ota_info.ota_code);
        ezos_delay_ms(10000);
        ez_iot_ota_status_ready(&pres, get_dev_productKey()); //上报状态0，清除服务器状态
    }

    return 0;
}