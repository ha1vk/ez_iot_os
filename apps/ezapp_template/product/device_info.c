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
 * XuRongjun (xurongjun@ezvizlife.com) - Device information related interface implement
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-03-15     xurongjun    first version 
 *******************************************************************************/

#include "device_info.h"
#include "ezlog.h"
#include "cJSON.h"

#define SOFT_MAJOR_VERSION 2
#define SOFT_MINOR_VERSION 0
#define SOFT_REVISION 1
#define SAP_MAGIC_NUMBER 0x53574B48
#define SAP_MAGIC_NUMBER_INVERT 0x484B5753
#define SAP_MAGIC_LENGTH 206

typedef struct
{
    ez_char_t dev_subserial[48];     //设备序列号
    ez_char_t dev_productKey[33];    //产品PID
    ez_char_t dev_deviceName[13];    //产品序列号
    ez_char_t dev_deviceLicense[48]; //产品验证码
    ez_char_t dev_auth_mode;         // 1 for license, 0 for sap
} dev_info_t;

static dev_info_t g_dev_info = {0};

static ez_int32_t parse_sap_config(ez_char_t *buf, ez_int32_t buf_size)
{
    ez_int32_t magicNumber = 0;
    if (buf_size < SAP_MAGIC_LENGTH)
    {
        ezlog_e(TAG_APP, "not a sap data 1");
        return -1;
    }

    ezos_memcpy((void *)&magicNumber, (void *)buf, sizeof(magicNumber));
    if (magicNumber != SAP_MAGIC_NUMBER && magicNumber != SAP_MAGIC_NUMBER_INVERT)
    {
        ezlog_e(TAG_APP, "not a sap data 2");
        return -1;
    }

    ezos_memset(&g_dev_info, 0, sizeof(dev_info_t));
    ezos_strncpy(g_dev_info.dev_productKey, CONFIG_EZIOT_COMPONENT_APP_PRODUCTKEY, sizeof(g_dev_info.dev_productKey) - 1);
    ezos_memcpy(g_dev_info.dev_subserial, buf + 73, 9);
    ezos_memcpy(g_dev_info.dev_deviceLicense, buf + 30, 6);
    g_dev_info.dev_auth_mode = 0;

    return 0;
}

ez_int32_t parse_lic_config(ez_char_t *buf, ez_int32_t buf_size)
{
    ez_int32_t rv = -1;
    cJSON *cjson_lic = NULL;
    cJSON *cjson_node = NULL;

    if (NULL == (cjson_lic = cJSON_Parse(buf)))
    {
        ezlog_e(TAG_APP, "cjson_lic parse!");
        goto done;
    }

    cjson_node = cJSON_GetObjectItem(cjson_lic, "dev_productKey");
    if (NULL == cjson_node || cJSON_String != cjson_node->type)
    {
        goto done;
    }

    ezos_strncpy(g_dev_info.dev_productKey, cjson_node->valuestring, sizeof(g_dev_info.dev_productKey) - 1);

    cjson_node = cJSON_GetObjectItem(cjson_lic, "dev_deviceName");
    if (NULL == cjson_node || cJSON_String != cjson_node->type)
    {
        goto done;
    }

    ezos_strncpy(g_dev_info.dev_deviceName, cjson_node->valuestring, sizeof(g_dev_info.dev_deviceName) - 1);
    ezos_snprintf(g_dev_info.dev_subserial, sizeof(g_dev_info.dev_subserial), "%s:%s", g_dev_info.dev_productKey, g_dev_info.dev_deviceName);

    cjson_node = cJSON_GetObjectItem(cjson_lic, "dev_deviceLicense");
    if (NULL == cjson_node || cJSON_String != cjson_node->type)
    {
        goto done;
    }

    ezos_strncpy(g_dev_info.dev_deviceLicense, cjson_node->valuestring, sizeof(g_dev_info.dev_deviceLicense) - 1);
    g_dev_info.dev_auth_mode = 1;
    rv = 0;

done:
    cJSON_Delete(cjson_lic);

    return rv;
}

ez_bool_t dev_info_init(ez_char_t *buf, ez_int32_t buf_size)
{
    if (0 == parse_sap_config(buf, buf_size))
    {
        ezlog_i(TAG_APP, "it is sap auth");
        return ez_true;
    }

    if (0 == parse_lic_config(buf, buf_size))
    {
        ezlog_i(TAG_APP, "it is lic auth");
        return ez_true;
    }

    return ez_false;
}

const ez_char_t *dev_info_get_type()
{
    return g_dev_info.dev_productKey;
}

const ez_char_t *dev_info_get_fwver()
{
    ez_int32_t year = 0;
    ez_int32_t month = 0;
    ez_int32_t day = 0;
    ez_char_t month_name[4]; // 编译日期的月份
    ez_char_t *all_mon_names[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
    static ez_char_t dev_fwver[32] = {0};

    ezos_memset(month_name, 0, sizeof(month_name));
    ezos_sscanf(__DATE__, "%s%d%d", month_name, &day, &year);

    for (month = 0; month < 12; month++)
    {
        if (ezos_strcmp(month_name, all_mon_names[month]) == 0)
        {
            break;
        }
    }

    month++;
    year -= 2000;
    ezos_sprintf(dev_fwver, "V%d.%d.%d build %02d%02d%02d", CONFIG_EZIOT_COMPONENT_APP_FWV_MAJOR, CONFIG_EZIOT_COMPONENT_APP_FWV_MINOR, CONFIG_EZIOT_COMPONENT_APP_FWV_MICRO, year, month, day);

    return dev_fwver;
}

const ez_char_t *dev_info_get_sn()
{
    return g_dev_info.dev_subserial;
}

const ez_char_t *dev_info_get_vcode()
{
    return g_dev_info.dev_deviceLicense;
}

ez_int32_t dev_info_auth_mode()
{
    return g_dev_info.dev_auth_mode;
}