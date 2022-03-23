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
 * XuRongjun (xurongjun@ezvizlife.com) - Product profile interface implement
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-03-15     xurongjun    first version 
 *******************************************************************************/

#include "product_config.h"
#include "ezlog.h"
#include "s2j.h"

#define IO_NUMS 5 // 模组对外可配的IO个数，目前暂定为5个

#define CHECK_JSOBJ_DONE(jsobj, key)                       \
    if (!(jsobj))                                          \
    {                                                      \
        ezlog_e(TAG_APP, "missing json object: %s", #key); \
        goto done;                                         \
    }

typedef struct
{
    ez_char_t brightness_upper; //亮度最大值(%)上限，调节力度最小1%
    ez_char_t brightness_lower; //亮度最大值(%)下限
    ez_char_t power_memory;     //0：空 1：断电记忆（定义成5s） 2：开关后恢复到默认状态
    ez_char_t power_on_light;   //产品上电点灯模式 0：空 1：开关过程无渐变 2：开关过程中有渐变（需要定义）
    ez_char_t memory_timval;    //记忆模式间隔时间，默认5s，单位s
    ez_char_t reset_time_lower; //进入配网重置次数下限值
    ez_char_t reset_time_upper; //进入配网重置次数上限值
    ez_int16_t default_cct;     //默认色温值，默认为3000K，单位K
} bulb_function_t;

typedef struct
{
    io_config_t io_config[IO_NUMS];       // IO引脚配置
    ez_char_t reset_switch_times;         // 表示几次，现在默认为3次
    bulb_function_t function;             //灯泡产品需要的配置信息
    bulb_test_param_t product_test_param; //灯泡产测参数// 占位符，保持字节对齐
} product_param_t;

typedef struct
{
    ez_char_t ap_prefix[23];         // AP前缀，最大22个字节，wifi的ssid = 前缀 + "_" + 序列号
    ez_char_t user_defined_type[64]; // 用户需要的展类型
} device_t;

typedef struct
{
    device_t device;            // 设备信息
    ez_char_t PTID[64];         // 产品配置信息索引
    ez_int32_t product_type;    // 0:灯，以后有新产品再增加
    ez_int32_t product_subtype; // 产品子类型
    ez_int32_t ap_timval;       // 配网窗口期，默认15分钟
    ez_char_t country_code[4];  // 产品所在的国家 //0->CN 1->EU 2->US 3->JP 4->BR 6->OTHER
    product_param_t param;      // mcu产品配置
} product_config_t;

static product_config_t g_product_config = {0};

ez_bool_t product_config_init(ez_char_t *buf, ez_int32_t buf_size)
{
    ez_bool_t rv = ez_false;
    cJSON *json_root = cJSON_Parse(buf);
    cJSON *json_temp = NULL;

    if (NULL == json_root)
    {
        ezlog_e(TAG_APP, "cjson_product parse!");
        goto done;
    }

    ezos_bzero((void *)&g_product_config, sizeof(g_product_config));

    // parse root
    s2j_struct_get_basic_element_ex(&g_product_config, json_root, int, product_type, 0);
    s2j_struct_get_basic_element_ex(&g_product_config, json_root, int, ap_timval, 15);
    s2j_struct_get_basic_element_ex(&g_product_config, json_root, string, country_code, "CN");

    s2j_struct_get_basic_element(&g_product_config, json_root, string, PTID);
    CHECK_JSOBJ_DONE(json_temp, PTID);

    s2j_struct_get_basic_element(&g_product_config, json_root, int, product_subtype);
    CHECK_JSOBJ_DONE(json_temp, product_subtype);

    // parse device item
    cJSON *json_device = cJSON_GetObjectItem(json_root, "device");
    CHECK_JSOBJ_DONE(json_device, device);
    s2j_struct_get_basic_element_ex(&g_product_config.device, json_device, string, ap_prefix, "EZVIZ");
    s2j_struct_get_basic_element(&g_product_config.device, json_device, string, user_defined_type);

    // parse product_param item
    cJSON *product_param = cJSON_GetObjectItem(json_root, "product_param");
    CHECK_JSOBJ_DONE(product_param, product_param);
    s2j_struct_get_basic_element_ex(&g_product_config.param, product_param, int, reset_switch_times, 3);

    // parse IO_config item
    cJSON *IO_config = cJSON_GetObjectItem(product_param, "IO_config");
    CHECK_JSOBJ_DONE(IO_config, IO_config);
    ez_int32_t IO_config_Num = cJSON_GetArraySize(IO_config);

    for (ez_int32_t i = 0; i < IO_config_Num; i++)
    {
        if (i >= IO_NUMS)
        {
            break;
        }

        cJSON *signal_IO_config = cJSON_GetArrayItem(IO_config, i);
        s2j_struct_get_basic_element_ex(&g_product_config.param.io_config[i], signal_IO_config, int, name, 0xff);
        s2j_struct_get_basic_element_ex(&g_product_config.param.io_config[i], signal_IO_config, int, enable, 0xff);
        s2j_struct_get_basic_element_ex(&g_product_config.param.io_config[i], signal_IO_config, int, light, 0xff);
        s2j_struct_get_basic_element_ex(&g_product_config.param.io_config[i], signal_IO_config, int, i2c, 0xff);
        s2j_struct_get_basic_element_ex(&g_product_config.param.io_config[i], signal_IO_config, int, mode, 0xff);
        s2j_struct_get_basic_element_ex(&g_product_config.param.io_config[i], signal_IO_config, int, drive_mode, 0xff);

        cJSON *drive_mode_param = cJSON_GetObjectItem(signal_IO_config, "drive_mode_param");
        if (NULL != drive_mode_param)
        {
            s2j_struct_get_basic_element(&g_product_config.param.io_config[i], drive_mode_param, int, intr_type);
            s2j_struct_get_basic_element(&g_product_config.param.io_config[i], drive_mode_param, int, pwm_frequency);
        }
    }

    // parse function item
    cJSON *function = cJSON_GetObjectItem(product_param, "function");
    CHECK_JSOBJ_DONE(function, function);
    s2j_struct_get_basic_element(&g_product_config.param.function, function, int, brightness_upper);
    s2j_struct_get_basic_element(&g_product_config.param.function, function, int, brightness_lower);
    s2j_struct_get_basic_element(&g_product_config.param.function, function, int, power_memory);
    s2j_struct_get_basic_element(&g_product_config.param.function, function, int, power_on_light);
    s2j_struct_get_basic_element(&g_product_config.param.function, function, int, memory_timval);
    s2j_struct_get_basic_element(&g_product_config, function, int, ap_timval);
    s2j_struct_get_basic_element(&g_product_config.param.function, function, int, reset_time_lower);
    s2j_struct_get_basic_element(&g_product_config.param.function, function, int, reset_time_upper);
    s2j_struct_get_basic_element(&g_product_config.param.function, function, int, default_cct);

    // parse product_test_param item
    cJSON *product_test_param = cJSON_GetObjectItem(product_param, "product_test_param");
    CHECK_JSOBJ_DONE(product_test_param, product_test_param);
    s2j_struct_get_basic_element(&g_product_config.param.product_test_param, function, string, order);
    s2j_struct_get_basic_element(&g_product_config.param.product_test_param, function, int, step1time);

    rv = ez_true;
done:

    return rv;
}

const ez_char_t *product_config_get_wd_prefix(ez_void_t)
{
    return g_product_config.device.ap_prefix;
}

ez_int32_t product_config_get_wd_period(ez_void_t)
{
    return g_product_config.ap_timval;
}

ez_void_t product_config_get_wd_condition(ez_int32_t *lower, ez_int32_t *upper)
{
    *lower = g_product_config.param.reset_switch_times;
    *upper = g_product_config.param.function.brightness_upper;
}

ez_int32_t product_config_default_cct(ez_void_t)
{
    return g_product_config.param.function.default_cct;
}

ez_int32_t product_config_io(io_config_t **io_config)
{
    *io_config = g_product_config.param.io_config;
    return IO_NUMS;
}

ez_int32_t product_config_subtype(ez_void_t)
{
    return g_product_config.product_subtype;
}

const ez_char_t *product_config_ptid(ez_void_t)
{
    return g_product_config.PTID;
}

const ez_char_t *product_config_default_nickname(ez_void_t)
{
    return g_product_config.device.user_defined_type;
}

const bulb_test_param_t *get_product_test_param(ez_void_t)
{
    return (const bulb_test_param_t *)&g_product_config.param.product_test_param;
}