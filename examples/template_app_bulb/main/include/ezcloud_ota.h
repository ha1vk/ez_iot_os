/**
 * @file ezcloud_ota.h
 * @author xurongjun (xurongjun@.com)
 * @brief 
 * @version 0.1
 * @date 2021-03-11
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#ifndef _EZCLOUD_OTA_H_
#define _EZCLOUD_OTA_H_
#ifdef HAL_ESP
#include "esp_ota_ops.h"
#include "esp_partition.h"
#endif
#ifdef __cplusplus
extern "C"
{
#endif
	typedef struct
    {
        int8_t url[270];       ///< 包下载路径,不带http/https前缀。设备根据自己的能力选择下载通道。
        int8_t fw_ver_dst[64]; ///< 差分包对应的原升级包版本号，只有这个版本的固件才能通过差分包升级至fw_ver版本的固件
        int8_t degist[32 + 1]; ///< 升级包的摘要值,包含'\0'
        int size;              ///< 软件包大小，单位Byte
    } ota_file_diff_t;

    typedef struct
    {
        int8_t mod_name[56];    ///< 模块名称,iot 互联模组：PID,e.g41X9TLMJB9GOA2ABOXKLEP, 透传模组对应的mcu 模组为PID_MCU
        int8_t url[270];        ///< 包下载路径,不带http/https前缀，iot互联模组url：ezviz-ota.cn-sh2.ufileos.com/e7135e21d0be26c483ce90e1c09a131f
        int8_t fw_ver[32];      ///< 升级包版本号 e.g V1.1.5 build 210602
        int8_t degist[32 + 1];  ///< 升级包的摘要值,包含'\0'
        int32_t size;           ///< 软件包大小，单位Byte
        ota_file_diff_t *pdiffs;///< 该版本对应的的差分包
    } ota_file_info_t;

    typedef struct
    {
        int retry_max;               ///< 下载最大重试次数
        int file_num;                ///< 升级信息数量
        int interval;                ///< 上报升级进度间隔
        ota_file_info_t *pota_files; ///< 升级信息
    } ota_upgrade_info_t;
    
    typedef struct
    {
        int8_t  url[286];   ///< 升级包的url信息,带http/https前缀，270+前缀长度
        int8_t  degist[33]; ///< 升级包摘要
        int32_t block_size; ///< 接收缓冲区大小\单次接收大小
        int32_t total_size; ///< 升级包的总大小
        int32_t timeout_s;  ///< 下载超时时间
        int32_t retry_max;  ///< 下载最大重试次数
    } ota_download_info_t;


    typedef enum
    {
        ota_code_none = 0,                     ///< 没有错误
        ota_code_file_size = 0x00500032,       ///< 下文件大小和查询的信息不匹配
        ota_code_file_size_range = 0x00500033, ///< 下文件大小超过分区大小
        ota_code_dns = 0x00500034,             ///< 下载地址域名解析失败
        ota_code_download = 0x00500035,        ///< 下载出错（超过最大重试次数）
        ota_code_digest = 0x00500036,          ///< 底层接口调用失败
        ota_code_sign = 0x00500037,            ///< 签名校验失败
        ota_code_damage = 0x00500038,          ///< 升级包损坏
        ota_code_mem = 0x00500039,             ///< 内存不足
        ota_code_burn = 0x0050003a,            ///< 烧录固件出错
        ota_code_genneral = 0x0050003b,        ///< 未知错误
        ota_code_cancel = 0x0050003c,          ///< 主动停止
        ota_code_recovery = 0x0050003d,        ///< 升级失败，通过备份系统或最小系统恢复
        ota_code_uart_err = 0x0050003e,        ///< 外挂设备升级失败
        ota_code_max = 0x0050004f
    } ota_errcode_e;


	typedef struct
		{
			int interval;                ///< 上报升级进度间隔
			int file_num;				 //V3升级协议中升级文件个数，
			char mod_name[32];            //ptid
			int bSubdevUpgrade;			 //至少包含有一个mcu 升级包程序
            #ifdef HAL_ESP
			esp_partition_t *pUpdate_partition;  //乐鑫备份升级中的分区
			esp_ota_handle_t update_handle; 		//乐鑫的升级写flash句柄
            #endif
        } OTA_USER_DATA_T;

    int ez_ota_init();

    void ez_ota_start();

#ifdef __cplusplus
}
#endif

#endif