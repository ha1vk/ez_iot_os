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
 * 2021-11-25     liwei    first version 
 *******************************************************************************/


/*此文件解析物模型传上来的协议，分action，属性，属性上报，此文件针对单芯片设备设计为统一格式文件.
物模型的功能点在此处以

property_$(key)_up,   设备主动上报的属性值调用此函数
property_$(KEY)_set,  sdk 底层接收到的属性协议处理函数
action_$(KEY)_set，    sdk 底层接收到的操作协议处理函数 
event_$(KEY)_up     设备主动上报事件调用此函数

可以通过python 脚本将控制台下载的物模型文件转化为此文件
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

#include "product_config.h"    //获取设备序列号用
static ez_int32_t property_brightness_up(ez_tsl_value_t *p_stru_key_value)
{
	ez_int32_t rv = -1;

	do
	{
		if (NULL == p_stru_key_value || NULL == p_stru_key_value->value)
		{
			break;
		}
		//todo:获取实际值上传
		p_stru_key_value->type = EZ_TSL_DATA_TYPE_INT;
		//完善此部分...
		printf("\n to_do DEBUG in line (%d) and function (%s)): \n ", __LINE__, __func__);
		rv = EZ_BASE_ERR_SUCC;
	}while(0);
	return rv;
}

static ez_int32_t property_colortemperature_up(ez_tsl_value_t *p_stru_key_value)
{
	ez_int32_t rv = -1;

	do
	{
		if (NULL == p_stru_key_value || NULL == p_stru_key_value->value)
		{
			break;
		}
		//todo:获取实际值上传
		p_stru_key_value->type = EZ_TSL_DATA_TYPE_INT;
		//完善此部分...
		printf("\n to_do DEBUG in line (%d) and function (%s)): \n ", __LINE__, __func__);
		rv = EZ_BASE_ERR_SUCC;
	}while(0);
	return rv;
}

static ez_int32_t property_countdowncfg_up(ez_tsl_value_t *p_stru_key_value)
{
	ez_int32_t rv = -1;

	do
	{
		if (NULL == p_stru_key_value || NULL == p_stru_key_value->value)
		{
			break;
		}
		//todo:获取实际值上传
		p_stru_key_value->type = EZ_TSL_DATA_TYPE_OBJECT;
		//完善此部分...
		printf("\n to_do DEBUG in line (%d) and function (%s)): \n ", __LINE__, __func__);
		rv = EZ_BASE_ERR_SUCC;
	}while(0);
	return rv;
}

static ez_int32_t property_lightswitchplan_up(ez_tsl_value_t *p_stru_key_value)
{
	ez_int32_t rv = -1;

	do
	{
		if (NULL == p_stru_key_value || NULL == p_stru_key_value->value)
		{
			break;
		}
		//todo:获取实际值上传
		p_stru_key_value->type = EZ_TSL_DATA_TYPE_OBJECT;
		//完善此部分...
		printf("\n to_do DEBUG in line (%d) and function (%s)): \n ", __LINE__, __func__);
		rv = EZ_BASE_ERR_SUCC;
	}while(0);
	return rv;
}

static ez_int32_t property_powerswitch_up(ez_tsl_value_t *p_stru_key_value)
{
	ez_int32_t rv = -1;

	do
	{
		if (NULL == p_stru_key_value || NULL == p_stru_key_value->value)
		{
			break;
		}
		//todo:获取实际值上传
		p_stru_key_value->type = EZ_TSL_DATA_TYPE_BOOL;
		//完善此部分...
		printf("\n to_do DEBUG in line (%d) and function (%s)): \n ", __LINE__, __func__);
		rv = EZ_BASE_ERR_SUCC;
	}while(0);
	return rv;
}

static ez_int32_t property_biorhythm_up(ez_tsl_value_t *p_stru_key_value)
{
	ez_int32_t rv = -1;

	do
	{
		if (NULL == p_stru_key_value || NULL == p_stru_key_value->value)
		{
			break;
		}
		//todo:获取实际值上传
		p_stru_key_value->type = EZ_TSL_DATA_TYPE_OBJECT;
		//完善此部分...
		printf("\n to_do DEBUG in line (%d) and function (%s)): \n ", __LINE__, __func__);
		rv = EZ_BASE_ERR_SUCC;
	}while(0);
	return rv;
}

static ez_int32_t property_colorrgb_up(ez_tsl_value_t *p_stru_key_value)
{
	ez_int32_t rv = -1;

	do
	{
		if (NULL == p_stru_key_value || NULL == p_stru_key_value->value)
		{
			break;
		}
		//todo:获取实际值上传
		p_stru_key_value->type = EZ_TSL_DATA_TYPE_STRING;
		//完善此部分...
		printf("\n to_do DEBUG in line (%d) and function (%s)): \n ", __LINE__, __func__);
		rv = EZ_BASE_ERR_SUCC;
	}while(0);
	return rv;
}

static ez_int32_t property_helpsleep_up(ez_tsl_value_t *p_stru_key_value)
{
	ez_int32_t rv = -1;

	do
	{
		if (NULL == p_stru_key_value || NULL == p_stru_key_value->value)
		{
			break;
		}
		//todo:获取实际值上传
		p_stru_key_value->type = EZ_TSL_DATA_TYPE_ARRAY;
		//完善此部分...
		printf("\n to_do DEBUG in line (%d) and function (%s)): \n ", __LINE__, __func__);
		rv = EZ_BASE_ERR_SUCC;
	}while(0);
	return rv;
}

static ez_int32_t property_musicrhythm_up(ez_tsl_value_t *p_stru_key_value)
{
	ez_int32_t rv = -1;

	do
	{
		if (NULL == p_stru_key_value || NULL == p_stru_key_value->value)
		{
			break;
		}
		//todo:获取实际值上传
		p_stru_key_value->type = EZ_TSL_DATA_TYPE_STRING;
		//完善此部分...
		printf("\n to_do DEBUG in line (%d) and function (%s)): \n ", __LINE__, __func__);
		rv = EZ_BASE_ERR_SUCC;
	}while(0);
	return rv;
}

static ez_int32_t property_scene_up(ez_tsl_value_t *p_stru_key_value)
{
	ez_int32_t rv = -1;

	do
	{
		if (NULL == p_stru_key_value || NULL == p_stru_key_value->value)
		{
			break;
		}
		//todo:获取实际值上传
		p_stru_key_value->type = EZ_TSL_DATA_TYPE_ARRAY;
		//完善此部分...
		printf("\n to_do DEBUG in line (%d) and function (%s)): \n ", __LINE__, __func__);
		rv = EZ_BASE_ERR_SUCC;
	}while(0);
	return rv;
}

static ez_int32_t property_wakeup_up(ez_tsl_value_t *p_stru_key_value)
{
	ez_int32_t rv = -1;

	do
	{
		if (NULL == p_stru_key_value || NULL == p_stru_key_value->value)
		{
			break;
		}
		//todo:获取实际值上传
		p_stru_key_value->type = EZ_TSL_DATA_TYPE_ARRAY;
		//完善此部分...
		printf("\n to_do DEBUG in line (%d) and function (%s)): \n ", __LINE__, __func__);
		rv = EZ_BASE_ERR_SUCC;
	}while(0);
	return rv;
}

static ez_int32_t property_workmode_up(ez_tsl_value_t *p_stru_key_value)
{
	ez_int32_t rv = -1;

	do
	{
		if (NULL == p_stru_key_value || NULL == p_stru_key_value->value)
		{
			break;
		}
		//todo:获取实际值上传
		p_stru_key_value->type = EZ_TSL_DATA_TYPE_STRING;
		//完善此部分...
		printf("\n to_do DEBUG in line (%d) and function (%s)): \n ", __LINE__, __func__);
		rv = EZ_BASE_ERR_SUCC;
	}while(0);
	return rv;
}

static ez_int32_t property_timezonecompose_up(ez_tsl_value_t *p_stru_key_value)
{
	ez_int32_t rv = -1;

	do
	{
		if (NULL == p_stru_key_value || NULL == p_stru_key_value->value)
		{
			break;
		}
		//todo:获取实际值上传
		p_stru_key_value->type = EZ_TSL_DATA_TYPE_OBJECT;
		//完善此部分...
		printf("\n to_do DEBUG in line (%d) and function (%s)): \n ", __LINE__, __func__);
		rv = EZ_BASE_ERR_SUCC;
	}while(0);
	return rv;
}

static ez_int32_t property_netstatus_up(ez_tsl_value_t *p_stru_key_value)
{
	ez_int32_t rv = -1;

	do
	{
		if (NULL == p_stru_key_value || NULL == p_stru_key_value->value)
		{
			break;
		}
		//todo:获取实际值上传
		p_stru_key_value->type = EZ_TSL_DATA_TYPE_OBJECT;
		//完善此部分...
		printf("\n to_do DEBUG in line (%d) and function (%s)): \n ", __LINE__, __func__);
		rv = EZ_BASE_ERR_SUCC;
	}while(0);
	return rv;
}

static ez_int32_t property_scene1_up(ez_tsl_value_t *p_stru_key_value)
{
	ez_int32_t rv = -1;

	do
	{
		if (NULL == p_stru_key_value || NULL == p_stru_key_value->value)
		{
			break;
		}
		//todo:获取实际值上传
		p_stru_key_value->type = EZ_TSL_DATA_TYPE_ARRAY;
		//完善此部分...
		printf("\n to_do DEBUG in line (%d) and function (%s)): \n ", __LINE__, __func__);
		rv = EZ_BASE_ERR_SUCC;
	}while(0);
	return rv;
}



//用户可在这些函数中让设备执行//
static ez_int32_t property_brightness_set(ez_tsl_value_t *p_stru_key_value)
{
	if (NULL == p_stru_key_value || EZ_TSL_DATA_TYPE_INT != p_stru_key_value->type)
	{
		return -1;
	}
	//todo:应用层业务处理
	printf("\n to_do DEBUG in line (%d) and function (%s)): \n ", __LINE__, __func__);
	//完善此部分...
	return EZ_BASE_ERR_SUCC;
}

static ez_int32_t property_colortemperature_set(ez_tsl_value_t *p_stru_key_value)
{
	if (NULL == p_stru_key_value || EZ_TSL_DATA_TYPE_INT != p_stru_key_value->type)
	{
		return -1;
	}
	//todo:应用层业务处理
	printf("\n to_do DEBUG in line (%d) and function (%s)): \n ", __LINE__, __func__);
	//完善此部分...
	return EZ_BASE_ERR_SUCC;
}

static ez_int32_t property_countdowncfg_set(ez_tsl_value_t *p_stru_key_value)
{
	if (NULL == p_stru_key_value || EZ_TSL_DATA_TYPE_OBJECT != p_stru_key_value->type)
	{
		return -1;
	}
	//todo:应用层业务处理
	printf("\n to_do DEBUG in line (%d) and function (%s)): \n ", __LINE__, __func__);
	//完善此部分...
	return EZ_BASE_ERR_SUCC;
}

static ez_int32_t property_lightswitchplan_set(ez_tsl_value_t *p_stru_key_value)
{
	if (NULL == p_stru_key_value || EZ_TSL_DATA_TYPE_OBJECT != p_stru_key_value->type)
	{
		return -1;
	}
	//todo:应用层业务处理
	printf("\n to_do DEBUG in line (%d) and function (%s)): \n ", __LINE__, __func__);
	//完善此部分...
	return EZ_BASE_ERR_SUCC;
}

static ez_int32_t property_powerswitch_set(ez_tsl_value_t *p_stru_key_value)
{
	if (NULL == p_stru_key_value || EZ_TSL_DATA_TYPE_BOOL != p_stru_key_value->type)
	{
		return -1;
	}
	//todo:应用层业务处理
	printf("\n to_do DEBUG in line (%d) and function (%s)): \n ", __LINE__, __func__);
	//完善此部分...
	return EZ_BASE_ERR_SUCC;
}

static ez_int32_t property_biorhythm_set(ez_tsl_value_t *p_stru_key_value)
{
	if (NULL == p_stru_key_value || EZ_TSL_DATA_TYPE_OBJECT != p_stru_key_value->type)
	{
		return -1;
	}
	//todo:应用层业务处理
	printf("\n to_do DEBUG in line (%d) and function (%s)): \n ", __LINE__, __func__);
	//完善此部分...
	return EZ_BASE_ERR_SUCC;
}

static ez_int32_t property_colorrgb_set(ez_tsl_value_t *p_stru_key_value)
{
	if (NULL == p_stru_key_value || EZ_TSL_DATA_TYPE_STRING != p_stru_key_value->type)
	{
		return -1;
	}
	//todo:应用层业务处理
	printf("\n to_do DEBUG in line (%d) and function (%s)): \n ", __LINE__, __func__);
	//完善此部分...
	return EZ_BASE_ERR_SUCC;
}

static ez_int32_t property_helpsleep_set(ez_tsl_value_t *p_stru_key_value)
{
	if (NULL == p_stru_key_value || EZ_TSL_DATA_TYPE_ARRAY != p_stru_key_value->type)
	{
		return -1;
	}
	//todo:应用层业务处理
	printf("\n to_do DEBUG in line (%d) and function (%s)): \n ", __LINE__, __func__);
	//完善此部分...
	return EZ_BASE_ERR_SUCC;
}

static ez_int32_t property_musicrhythm_set(ez_tsl_value_t *p_stru_key_value)
{
	if (NULL == p_stru_key_value || EZ_TSL_DATA_TYPE_STRING != p_stru_key_value->type)
	{
		return -1;
	}
	//todo:应用层业务处理
	printf("\n to_do DEBUG in line (%d) and function (%s)): \n ", __LINE__, __func__);
	//完善此部分...
	return EZ_BASE_ERR_SUCC;
}

static ez_int32_t property_scene_set(ez_tsl_value_t *p_stru_key_value)
{
	if (NULL == p_stru_key_value || EZ_TSL_DATA_TYPE_ARRAY != p_stru_key_value->type)
	{
		return -1;
	}
	//todo:应用层业务处理
	printf("\n to_do DEBUG in line (%d) and function (%s)): \n ", __LINE__, __func__);
	//完善此部分...
	return EZ_BASE_ERR_SUCC;
}

static ez_int32_t property_wakeup_set(ez_tsl_value_t *p_stru_key_value)
{
	if (NULL == p_stru_key_value || EZ_TSL_DATA_TYPE_ARRAY != p_stru_key_value->type)
	{
		return -1;
	}
	//todo:应用层业务处理
	printf("\n to_do DEBUG in line (%d) and function (%s)): \n ", __LINE__, __func__);
	//完善此部分...
	return EZ_BASE_ERR_SUCC;
}

static ez_int32_t property_workmode_set(ez_tsl_value_t *p_stru_key_value)
{
	if (NULL == p_stru_key_value || EZ_TSL_DATA_TYPE_STRING != p_stru_key_value->type)
	{
		return -1;
	}
	//todo:应用层业务处理
	printf("\n to_do DEBUG in line (%d) and function (%s)): \n ", __LINE__, __func__);
	//完善此部分...
	return EZ_BASE_ERR_SUCC;
}

static ez_int32_t property_timezonecompose_set(ez_tsl_value_t *p_stru_key_value)
{
	if (NULL == p_stru_key_value || EZ_TSL_DATA_TYPE_OBJECT != p_stru_key_value->type)
	{
		return -1;
	}
	//todo:应用层业务处理
	printf("\n to_do DEBUG in line (%d) and function (%s)): \n ", __LINE__, __func__);
	//完善此部分...
	return EZ_BASE_ERR_SUCC;
}

static ez_int32_t property_netstatus_set(ez_tsl_value_t *p_stru_key_value)
{
	if (NULL == p_stru_key_value || EZ_TSL_DATA_TYPE_OBJECT != p_stru_key_value->type)
	{
		return -1;
	}
	//todo:应用层业务处理
	printf("\n to_do DEBUG in line (%d) and function (%s)): \n ", __LINE__, __func__);
	//完善此部分...
	return EZ_BASE_ERR_SUCC;
}

static ez_int32_t property_scene1_set(ez_tsl_value_t *p_stru_key_value)
{
	if (NULL == p_stru_key_value || EZ_TSL_DATA_TYPE_ARRAY != p_stru_key_value->type)
	{
		return -1;
	}
	//todo:应用层业务处理
	printf("\n to_do DEBUG in line (%d) and function (%s)): \n ", __LINE__, __func__);
	//完善此部分...
	return EZ_BASE_ERR_SUCC;
}


static void action_getcountdown(char *data,char *value)
{



}


//功能点数组用来找功能点//
property_cmd_t property_cmd[]=
{
	{"Brightness","LightCtrl","global","0",property_brightness_set,property_brightness_up},
	{"ColorTemperature","LightCtrl","global","0",property_colortemperature_set,property_colortemperature_up},
	{"CountdownCfg","LightCtrl","global","0",property_countdowncfg_set,property_countdowncfg_up},
	{"LightSwitchPlan","LightCtrl","global","0",property_lightswitchplan_set,property_lightswitchplan_up},
	{"PowerSwitch","PowerMgr","global","0",property_powerswitch_set,property_powerswitch_up},
	{"Biorhythm","RGBLightCtrl","global","0",property_biorhythm_set,property_biorhythm_up},
	{"ColorRgb","RGBLightCtrl","global","0",property_colorrgb_set,property_colorrgb_up},
	{"HelpSleep","RGBLightCtrl","global","0",property_helpsleep_set,property_helpsleep_up},
	{"MusicRhythm","RGBLightCtrl","global","0",property_musicrhythm_set,property_musicrhythm_up},
	{"Scene","RGBLightCtrl","global","0",property_scene_set,property_scene_up},
	{"WakeUp","RGBLightCtrl","global","0",property_wakeup_set,property_wakeup_up},
	{"WorkMode","RGBLightCtrl","global","0",property_workmode_set,property_workmode_up},
	{"TimeZoneCompose","TimeMgr","global","0",property_timezonecompose_set,property_timezonecompose_up},
	{"NetStatus","WifiStatus","global","0",property_netstatus_set,property_netstatus_up},
	{"Scene1","custom","global","0",property_scene1_set,property_scene1_up},
    {NULL,NULL,NULL,NULL,NULL,NULL}
};

action_cmd_t action_cmd[]=
{
    {"LightCtrl","getcountdown",action_getcountdown},
    {NULL,NULL,NULL}
};

ez_int32_t tsl_notice(ez_tsl_event_e event_type, ez_void_t *data, ez_int32_t len)
{
    return 0;
}

int user_property_report(char *key)
{
    int ret = 0;
    int i = 0;
    char dev_serial[72] = {0};
    ez_tsl_rsc_t rsc_info = {.res_type = NULL, .local_index = NULL};  
    ez_tsl_key_t key_info = {.domain = NULL,.key = NULL};

    if (NULL == key )
    {
        ezlog_e(TAG_AP, "property param error.");
        return -1;
    }

    sprintf(dev_serial, "%s:%s", get_lic_productKey(), get_lic_deviceName());

    for (i = 0; property_cmd[i].identify != NULL; i++)
    {
        if (strncmp(key, property_cmd[i].identify, IDENTIFIER_LEN_MAX) == 0) /* 匹配功能点 */
        {
            break;
        }
    }

    if (NULL != property_cmd[i].identify)
    {
              
        /**
		* @brief 云端下发属性，且设备的属性已变化，应该执行主动上报。
		* 
		*/
 
        printf("\n to_do DEBUG in line (%d) and function (%s)): \n ",__LINE__, __func__);
        rsc_info.res_type = property_cmd[i].res_type;
        rsc_info.local_index = property_cmd[i].index;

        key_info.domain = property_cmd[i].domain;
        key_info.key = key;

        ez_iot_tsl_property_report(dev_serial, &rsc_info, &key_info, NULL);
    }
    else
    {
        ret = -1;
        ezlog_w(TAG_AP, "property[%s] do not realize set to dev!!\n", key);
    }

    return ret;
    
}

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
    int ret = 0;
    int i = 0;
    if (NULL == key_info || NULL == value_out)
    {
        ezlog_e(TAG_AP, "things action2dev param error.");
        return -1;
    }

    for (i = 0; action_cmd[i].identify != NULL; i++)
    {
        if (strncmp(key_info->key, action_cmd[i].identify, IDENTIFIER_LEN_MAX) == 0) /* 匹配功能点 */
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
 * @note: 此函数执行不应该阻塞，相关业务处理若有时间较长应开启另一个线程任务处理
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

        /* @brief 云端下发属性，且设备的属性已变化，应该执行主动上报。
        ez_iot_tsl_property_report()接口传NULL 值，sdk会调用tsl_things_property2cloud上报属性值，
        采用这种方式的好处是将上报时机交由sdk来处理，sdk 不需要缓存value值，在网络拥塞的情况下更为合适。
        */
       
         ez_iot_tsl_property_report(sn, rsc_info, key_info, NULL);            

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
    int ret = 0;
    int i;
    if (NULL == key_info || NULL == value_out)
    {
        ezlog_e(TAG_AP, "things property2cloud param error.");
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
        ret = property_cmd[i].func_up(value_out);
    }
    else
    {
        ezlog_w(TAG_AP, "property[%s] do not realize up to cloude!!\n", key_info->key);
    }
    return ret;
}
