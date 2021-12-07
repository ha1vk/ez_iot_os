/*******************************************************************************
 * Copyright © 2017-2021 Ezviz Inc.
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
 * liwei (liwei@ezvizlife.com)
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-11-25     xurongjun    first version 
 *******************************************************************************/


/*此文件解析物模型传上来的协议，分action，属性，属性上报，此文件针对单芯片设备设计为统一格式文件.
物模型的功能点在此处以action_$(KEY)_set，property_$(KEY)_set,property_$(key)_get,event_$(KEY)_up命名
需要寻找到具体的功能点透出去
*/

/*
1.先不处理重置指令，球泡灯标准功能点暂未定义
*/

#include <stdio.h>
#include <string.h>

#include "ezlog.h"
#include "ez_iot_base.h"
#include "ez_iot_tsl.h"

#include "ez_iot_core_def.h"
#include "ez_iot_core.h"
#include "bulb_business.h"
#include "bulb_protocol_tsl_parse.h"

static ez_int32_t property_light_switch_set(ez_tsl_value_t *p_stru_key_value)
{
    if (NULL == p_stru_key_value || EZ_TSL_DATA_TYPE_BOOL != p_stru_key_value->type)
    {
        return -1;
    }

    if (p_stru_key_value->value_bool)
    {
        if (0 != turn_on_lamp())
        {
            ezlog_e(TAG_LIGHT, "turn on lamp failed.");
            return -1;
        }
    }
    else
    {
        if (turn_off_lamp())
        {
            ezlog_e(TAG_LIGHT, "turn off lamp failed.");
            return -1;
        }
    }

    return EZ_BASE_ERR_SUCC;
}

static ez_int32_t property_light_switch_up(ez_tsl_value_t *p_stru_key_value)
{
    ez_int32_t rv = -1;
    bool light_switch = get_light_switch();

    do
    {
        if (NULL == p_stru_key_value)
        {
            break;
        }

        p_stru_key_value->type = EZ_TSL_DATA_TYPE_BOOL;
        p_stru_key_value->size = sizeof(light_switch);

        p_stru_key_value->value_bool = light_switch;
        rv = EZ_BASE_ERR_SUCC;
    } while (0);

    return rv;
}

static ez_int32_t property_brightness_set(ez_tsl_value_t *p_stru_key_value)
{
    if (NULL == p_stru_key_value || EZ_TSL_DATA_TYPE_INT != p_stru_key_value->type)
    {
        return -1;
    }

    if (0 != adjust_light_brightness(p_stru_key_value->value_int))
    {
        ezlog_e(TAG_LIGHT, "adjust light brightness failed.");
        return -1;
    }

    return EZ_BASE_ERR_SUCC;
}

static ez_int32_t property_brightness_up(ez_tsl_value_t *p_stru_key_value)
{
    ez_int32_t rv = -1;
    ez_int32_t brightness = get_light_brightness();

    do
    {
        if (NULL == p_stru_key_value)
        {
            break;
        }

        p_stru_key_value->type = EZ_TSL_DATA_TYPE_INT;
        p_stru_key_value->size = sizeof(brightness);

        p_stru_key_value->value_int = brightness;
        rv = EZ_BASE_ERR_SUCC;
    } while (0);

    return rv;
}

static ez_int32_t property_color_temperature_set(ez_tsl_value_t *p_stru_key_value)
{
    if (NULL == p_stru_key_value || EZ_TSL_DATA_TYPE_INT != p_stru_key_value->type)
    {
        return -1;
    }

    if (0 != adjust_light_cct(p_stru_key_value->value_int))
    {
        ezlog_e(TAG_LIGHT, "adjust light cct failed.");
        return -1;
    }

    return EZ_BASE_ERR_SUCC;
}

static ez_int32_t property_color_temperature_up(ez_tsl_value_t *p_stru_key_value)
{
    ez_int32_t rv = -1;
    int cct = get_light_color_temperature();

    do
    {
        if (NULL == p_stru_key_value)
        {
            break;
        }

        p_stru_key_value->type = EZ_TSL_DATA_TYPE_INT;
        p_stru_key_value->size = sizeof(cct);

        p_stru_key_value->value_int = cct;
        rv = EZ_BASE_ERR_SUCC;
    } while (0);
    return rv;
}

static ez_int32_t property_color_rgb_set(ez_tsl_value_t *p_stru_key_value)
{
    if (NULL == p_stru_key_value || EZ_TSL_DATA_TYPE_STRING != p_stru_key_value->type)
    {
        return -1;
    }

    int r, g, b;
    printf("\n to_do DEBUG in line (%d) and function (%s)):%s \n ", __LINE__, __func__, p_stru_key_value->value);

    sscanf((char *)p_stru_key_value->value, "#%2x%2x%2x", &r, &g, &b);
    int rgb = r * 256 * 256 + g * 256 + b;
    if (0 != adjust_light_rgb(rgb))
    {
        ezlog_e(TAG_LIGHT, "adjust light rgb error.");
        return -1;
    }

    return EZ_BASE_ERR_SUCC;
}

static ez_int32_t property_color_rgb_up(ez_tsl_value_t *p_stru_key_value)
{
    ez_int32_t rv = -1;
    int rgb = get_light_color_rgb();
    char str_rgb[16] = {0};
    sprintf(str_rgb, "#%06X", rgb);

    do
    {
        if (NULL == p_stru_key_value || NULL == p_stru_key_value->value)
        {
            break;
        }

        p_stru_key_value->type = EZ_TSL_DATA_TYPE_STRING;
        p_stru_key_value->size = strlen(str_rgb);

        memcpy(p_stru_key_value->value, str_rgb, p_stru_key_value->size);
        rv = EZ_BASE_ERR_SUCC;
    } while (0);

    return rv;
}

static ez_int32_t property_light_mode_set(ez_tsl_value_t *p_stru_key_value)
{
    int cct = 2700;
    int rgb = 0xFF;

    if (NULL == p_stru_key_value || EZ_TSL_DATA_TYPE_STRING != p_stru_key_value->type)
    {
        return -1;
    }

    if (0 == strcmp(p_stru_key_value->value, "white"))
    {
        cct = get_light_color_temperature();

        if (0 != adjust_light_cct(cct))
        {
            ezlog_e(TAG_LIGHT, "adjust light cct failed.");
            return -1;
        }
    }
    else if (0 == strcmp(p_stru_key_value->value, "color"))
    {
        rgb = get_light_color_rgb();

        if (0 != adjust_light_rgb(rgb))
        {
            ezlog_e(TAG_LIGHT, "adjust light rgb error.");
            return -1;
        }
    }
    else if (0 == strcmp(p_stru_key_value->value, "scene"))
    {
    }
    else if (0 == strcmp(p_stru_key_value->value, "music"))
    {
    }

    return EZ_BASE_ERR_SUCC;
}

static ez_int32_t property_light_mode_up(ez_tsl_value_t *p_stru_key_value)
{
    ez_int32_t rv = -1;
    int light_mode = get_light_mode();
    char mode[32] = {0};
    switch (light_mode)
    {
    case LIGHT_WHITE:
        strcpy(mode, "white");
        break;
    case LIGHT_COLOR:
        strcpy(mode, "color");
        break;
    case LIGHT_SCENE:
        strcpy(mode, "scene");
        break;
    case LIGHT_MUSIC:
        strcpy(mode, "music");
        break;
    default:
        break;
    }

    do
    {
        if (NULL == p_stru_key_value || NULL == p_stru_key_value->value)
        {
            break;
        }

        p_stru_key_value->type = EZ_TSL_DATA_TYPE_STRING;
        p_stru_key_value->size = strlen(mode);

        memcpy(p_stru_key_value->value, mode, p_stru_key_value->size);
        rv = EZ_BASE_ERR_SUCC;
    } while (0);

    return rv;
}

static ez_int32_t property_plan_set(ez_tsl_value_t *p_stru_key_value)
{
    ez_int32_t rv = -1;

    if (NULL == p_stru_key_value || EZ_TSL_DATA_TYPE_ARRAY != p_stru_key_value->type)
    {
        return -1;
    }
    ezlog_v(TAG_LIGHT, "value=%d, type=%d", p_stru_key_value->type, p_stru_key_value->value);
    printf("\n to_do DEBUG in line (%d) and function (%s)): \n ", __LINE__, __func__);

    return rv;
}

static ez_int32_t property_plan_up(ez_tsl_value_t *p_stru_key_value)
{
    ez_int32_t rv = -1;

    do
    {
        if (NULL == p_stru_key_value || NULL == p_stru_key_value->value)
        {
            break;
        }

        //todo: get the plan string
        printf("\n to_do DEBUG in line (%d) and function (%s)): \n ", __LINE__, __func__);

        ezlog_v(TAG_LIGHT, "plan value=%d, type=%d", p_stru_key_value->type, p_stru_key_value->value);

        rv = EZ_BASE_ERR_SUCC;
    } while (0);

    return rv;
}

static int property_netstatus_up(ez_tsl_value_t *p_stru_key_value)
{
    ez_int32_t rv = -1;

    do
    {
        if (NULL == p_stru_key_value || NULL == p_stru_key_value->value)
        {
            break;
        }

        //todo: get netstaus string
        printf("\n to_do DEBUG in line (%d) and function (%s)): \n ", __LINE__, __func__);

        ezlog_v(TAG_LIGHT, "plan value=%d, type=%d", p_stru_key_value->type, p_stru_key_value->value);

        rv = EZ_BASE_ERR_SUCC;
    } while (0);

    return rv;
}

static ez_int32_t property_timezone_set(ez_tsl_value_t *p_stru_key_value)
{
    if (NULL == p_stru_key_value || EZ_TSL_DATA_TYPE_OBJECT != p_stru_key_value->type)
    {
        return -1;
    }

    //todo:
    printf("\n to_do DEBUG in line (%d) and function (%s)): \n ", __LINE__, __func__);

    //correct_time_zone((char *)p_stru_key_value->value);

    return EZ_BASE_ERR_SUCC;
}

static int property_timezone_up(ez_tsl_value_t *p_stru_key_value)
{
    ez_int32_t rv = -1;

    do
    {
        if (NULL == p_stru_key_value || NULL == p_stru_key_value->value)
        {
            break;
        }
        printf("\n to_do DEBUG in line (%d) and function (%s)): \n ", __LINE__, __func__);

        //todo: get netstaus string
        ezlog_v(TAG_LIGHT, "timzone value=%d, type=%d", p_stru_key_value->type, p_stru_key_value->value);
        rv = EZ_BASE_ERR_SUCC;
    } while (0);

    return rv;
}

static ez_int32_t property_scene_set(ez_tsl_value_t *p_stru_key_value)
{
    if (NULL == p_stru_key_value || EZ_TSL_DATA_TYPE_OBJECT != p_stru_key_value->type)
    {
        return -1;
    }
    char *light_scene = (char *)p_stru_key_value->value;
    //todo
    printf("\n to_do DEBUG in line (%d) and function (%s)): %s\n ", __LINE__, __func__, light_scene);

    //set_light_scene(light_scene);

    return EZ_BASE_ERR_SUCC;
}

static int property_scene_up(ez_tsl_value_t *p_stru_key_value)
{
    ez_int32_t rv = -1;

    do
    {
        if (NULL == p_stru_key_value || NULL == p_stru_key_value->value)
        {
            break;
        }

        //todo: get scene string
        printf("\n to_do DEBUG in line (%d) and function (%s)): \n ", __LINE__, __func__);

        ezlog_v(TAG_LIGHT, "scene value=%d, type=%d", p_stru_key_value->type, p_stru_key_value->value);

        rv = EZ_BASE_ERR_SUCC;
    } while (0);

    return rv;
}

static ez_int32_t property_scene_conf_set(ez_tsl_value_t *p_stru_key_value)
{
    if (NULL == p_stru_key_value || EZ_TSL_DATA_TYPE_ARRAY != p_stru_key_value->type)
    {
        return -1;
    }

    char *light_scene_conf = (char *)p_stru_key_value->value;
    //todo
    //set_light_scene_conf(light_scene_conf);
    printf("\n to_do DEBUG in line (%d) and function (%s)): %s\n ", __LINE__, __func__, light_scene_conf);

    return EZ_BASE_ERR_SUCC;
}

static int property_scene_conf_up(ez_tsl_value_t *p_stru_key_value)
{
    ez_int32_t rv = -1;

    do
    {
        if (NULL == p_stru_key_value || NULL == p_stru_key_value->value)
        {
            break;
        }

        //todo: get scene_conf string
        printf("\n to_do DEBUG in line (%d) and function (%s)): \n ", __LINE__, __func__);

        ezlog_v(TAG_LIGHT, "scene_conf value=%d, type=%d", p_stru_key_value->type, p_stru_key_value->value);

        rv = EZ_BASE_ERR_SUCC;
    } while (0);

    return rv;
}

static ez_int32_t action_countdown_on_enable(ez_tsl_value_t *p_stru_key_value, ez_tsl_value_t *value_out)
{
    if (NULL == p_stru_key_value)
    {
        return -1;
    }
    /* 
    if (0 != countdown_light_on(p_stru_key_value->value_int))
    {
        ezlog_e(TAG_LIGHT, "set countdown_on failed.");
        return -1;
    }
    */

    printf("\n to_do DEBUG in line (%d) and function (%s)): \n ", __LINE__, __func__);

    return EZ_BASE_ERR_SUCC;
}

static ez_int32_t action_countdown_off_enable(ez_tsl_value_t *p_stru_key_value, ez_tsl_value_t *p_stru_retrun_value)
{
    if (NULL == p_stru_key_value)
    {
        return -1;
    }

    /*
    if (0 != countdown_light_off(p_stru_key_value->value_int))
    {
        ezlog_e(TAG_LIGHT, "set countdown_off failed.");
        return -1;
    }
	*/
    printf("\n to_do DEBUG in line (%d) and function (%s)): \n ", __LINE__, __func__);

    return EZ_BASE_ERR_SUCC;
}

static ez_int32_t action_countdown_on_disable(ez_tsl_value_t *p_stru_key_value, ez_tsl_value_t *p_stru_retrun_value)
{
    if (NULL == p_stru_key_value)
    {
        return -1;
    }

    /*
    if (0 != countdown_light_on_cancel()
    {
        ezlog_e(TAG_LIGHT, "cancel countdown_on failed.");
        return -1;
    }
	*/
    printf("\n to_do DEBUG in line (%d) and function (%s)): \n ", __LINE__, __func__);

    return EZ_BASE_ERR_SUCC;
}

static ez_int32_t action_countdown_off_disable(ez_tsl_value_t *p_stru_key_value, ez_tsl_value_t *p_stru_retrun_value)
{
    if (NULL == p_stru_key_value)
    {
        return -1;
    }
    /*
    if (0 != countdown_light_off_cancel()
    {
        ezlog_e(TAG_LIGHT, "cancel countdown_off failed.");
        return -1;
    }
	*/
    printf("\n to_do DEBUG in line (%d) and function (%s)): \n ", __LINE__, __func__);

    return EZ_BASE_ERR_SUCC;
}

static ez_int32_t action_countdown_query(ez_tsl_value_t *p_stru_key_value, ez_tsl_value_t *p_stru_retrun_value)
{
    if (NULL == p_stru_key_value)
    {
        return -1;
    }

    /*
    if (0 != countdown_light_query(p_stru_retrun_value)
    {
        ezlog_e(TAG_LIGHT, "cancel countdown_off failed.");
        return -1;
    }
	*/
    printf("\n to_do DEBUG in line (%d) and function (%s)): \n ", __LINE__, __func__);

    return EZ_BASE_ERR_SUCC;
}

static ez_int32_t action_restart(ez_tsl_value_t *p_stru_key_value, ez_tsl_value_t *p_stru_retrun_value)
{
    if (NULL == p_stru_key_value)
    {
        return -1;
    }

    if (0 != restart_light())
    {
        ezlog_e(TAG_LIGHT, "restart failed.");
        return -1;
    }

    return EZ_BASE_ERR_SUCC;
}

property_cmd_t property_cmd[] = {
    {"light_switch", property_light_switch_set, property_light_switch_up},
    {"brightness", property_brightness_set, property_brightness_up},
    {"color_temperature", property_color_temperature_set, property_color_temperature_up},
    {"color_rgb", property_color_rgb_set, property_color_rgb_up},
    {"light_mode", property_light_mode_set, property_light_mode_up},
    {"plan", property_plan_set, property_plan_up},
    {"scene", property_scene_set, property_scene_up},
    {"scene_conf", property_scene_conf_set, property_scene_conf_up},
    {"TimeZoneCompose", property_timezone_set, property_timezone_up},
    {NULL, NULL, NULL}};

action_cmd_t action_cmd[] =
    {
        {"countdown_on_enable", action_countdown_on_enable},
        {"countdown_off_enable", action_countdown_off_enable},
        {"countdown_on_disable", action_countdown_on_disable},
        {"countdown_off_disable", action_countdown_off_disable},
        {"countdown_query", action_countdown_query},
        {"restart", action_restart},
        {NULL, NULL}};

/**
 * @brief 物模型操作回调函数
 * @param[in] sn :sdk 回调出来资源通道信息，灯类可以不处理。
 * @param[in] rsc_info :sdk 回调出来资源通道信息，灯类可以不处理。
 * @param[in] key_info :sdk 回调出来领域功能点信息，必须处理。
 * @param[in] value_in :sdk 回调出来功能点具体值信息，必须处理
 * @param[out] value_out :函数处理后返回给sdk 的值，可以不处理。无值返回时，sdk action 底层会补充一个0或者-1 code 给平台
 * @return :成功SUCCESS/失败返回:ERROR
 * @note: 
 */
ez_int32_t tsl_things_action2dev(const ez_int8_t *sn, const ez_tsl_rsc_t *rsc_info, const ez_tsl_key_t *key_info,

                                 const ez_tsl_value_t *value_in, ez_tsl_value_t *value_out)
{
    printf("\n to_do DEBUG in line (%d) and function (%s)): \n ", __LINE__, __func__);

    int ret = 0;
    int i = 0;
    if (NULL == key_info || NULL == value_out)
    {
        ezlog_e(TAG_AP, "things report2dev param error.");
        return -1;
    }

    for (i = 0; action_cmd[i].identify != NULL; i++)
    {
        if (strncmp(key_info->key, property_cmd[i].identify, IDENTIFIER_LEN_MAX) == 0) /* 匹配功能点 */
        {
            break;
        }
    }
    if (NULL != action_cmd[i].identify)
    {
        ret = action_cmd[i].func_set(value_in, value_out);
    }
    else
    {
        ezlog_w(TAG_AP, "action[%s] do not realize!!\n", key_info->key);
    }
    return ret;
}

/**
 * @brief 物模型属性设置回调函数
 * @param[in] sn :sdk 回调出来资源通道信息，灯类可以不处理。
 * @param[in] rsc_info :sdk 回调出来资源通道信息，灯类可以不处理。
 * @param[in] key_info :sdk 回调出来领域功能点信息，必须处理。
 * @param[in] value_out :sdk 回调出来领域功能点具体值信息，必须处理
 * @return :成功SUCCESS/失败返回:ERROR
 * @note: SDK 24小时会强制执行同步设备的属性给平台，会遍历所有功能点回调此函数
 */
ez_int32_t tsl_things_property2dev(const ez_int8_t *sn, const ez_tsl_rsc_t *rsc_info, const ez_tsl_key_t *key_info, ez_tsl_value_t *value_out)
{
    int ret = 0;
    int i = 0;

    if (NULL == key_info || NULL == value_out)
    {
        ezlog_e(TAG_AP, "things report2dev param error.");
        return -1;
    }

    for (i = 0; property_cmd[i].identify != NULL; i++)
    {
        if (strncmp(key_info->key, property_cmd[i].identify, IDENTIFIER_LEN_MAX) == 0) /* 匹配功能点 */
        {
            break;
        }
    }

    if (NULL != property_cmd[i].identify)
    {
        ret = property_cmd[i].func_set(value_out);

        /**
		* @brief 云端下发属性，且设备的属性已变化，应该执行主动上报。
		* 
		*/
        if (EZ_BASE_ERR_SUCC == ret)
        {
            ret = property_cmd[i].func_up(value_out);
            if (EZ_BASE_ERR_SUCC == ret)
            {
                ez_iot_tsl_property_report(sn, rsc_info, key_info, value_out);
            }
        }
    }
    else
    {
        ret = -1;
        ezlog_w(TAG_AP, "property[%s] do not realize set to dev!!\n", key_info->key);
    }

    return ret;
}

/**
 * @brief 物模型属性同步回调函数
 * @param[in] sn :sdk 回调出来资源通道信息，灯类可以不处理。
 * @param[in] rsc_info :sdk 回调出来资源通道信息，灯类可以不处理。
 * @param[in] key_info :sdk 回调出来领域功能点信息，必须处理。
 * @param[in] value_in :sdk 回调出来功能点具体值信息，必须处理
 * @param[out] value_out :函数处理后的属性值返回给sdk，必须处理，内存由sdk 进行释放
 * @return :成功SUCCESS/失败返回:ERROR
 * @note: SDK 24小时会强制执行同步设备的属性给平台，会遍历所有功能点回调此函数
 */
ez_int32_t tsl_things_property2cloud(const ez_int8_t *sn, const ez_tsl_rsc_t *rsc_info, const ez_tsl_key_t *key_info, ez_tsl_value_t *value_out)
{
    printf("\n to_do DEBUG in line (%d) and function (%s)): \n ", __LINE__, __func__);

    int ret = 0;
    int i;
    if (NULL == key_info || NULL == value_out)
    {
        ezlog_e(TAG_AP, "things report2dev param error.");
        return -1;
    }
    printf("\n to_do DEBUG in line (%d) and function (%s)): \n ", __LINE__, __func__);

    for (i = 0; property_cmd[i].identify != NULL; i++)
    {
        if (strncmp(key_info->key, property_cmd[i].identify, IDENTIFIER_LEN_MAX) == 0) /* 匹配功能点 */
        {
            break;
        }
    }

    if (NULL != property_cmd[i].identify)
    {
        ret = property_cmd[i].func_up(value_out);
    }
    else
    {
        ezlog_w(TAG_AP, "property[%s] do not realize up to cloude!!\n", key_info->key);
    }
    return ret;
}
