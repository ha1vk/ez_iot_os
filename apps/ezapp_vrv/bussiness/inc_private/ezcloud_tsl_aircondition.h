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
 * XuRongjun (xurongjun@ezvizlife.com) - 空调物模型协议接口声明
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-07-27     xurongjun    first version
 *******************************************************************************/

#include <ezos.h>
#include <ezcloud_tsl_provider.h>

#ifndef _EZCLOUD_TSL_AIR_CONDITION_H_
#define _EZCLOUD_TSL_AIR_CONDITION_H_

#include "ezos.h"
#include "ez_iot_tsl.h"

#ifdef __cplusplus
extern "C"
{
#endif

    // 属性获取
    ez_int32_t aircondition_property_PowerSwitch_get(tsl_prop_impl_t *thiz, const ez_tsl_rsc_t *rsc_info, ez_tsl_value_t *tsl_value);
    ez_int32_t aircondition_property_CountdownCfg_get(tsl_prop_impl_t *thiz, const ez_tsl_rsc_t *rsc_info, ez_tsl_value_t *tsl_value);
    ez_int32_t aircondition_property_Temperature_get(tsl_prop_impl_t *thiz, const ez_tsl_rsc_t *rsc_info, ez_tsl_value_t *tsl_value);
    ez_int32_t aircondition_property_ACException_get(tsl_prop_impl_t *thiz, const ez_tsl_rsc_t *rsc_info, ez_tsl_value_t *tsl_value);
    ez_int32_t aircondition_property_RoomTemperature_get(tsl_prop_impl_t *thiz, const ez_tsl_rsc_t *rsc_info, ez_tsl_value_t *tsl_value);
    ez_int32_t aircondition_property_WindSpeedLevel_get(tsl_prop_impl_t *thiz, const ez_tsl_rsc_t *rsc_info, ez_tsl_value_t *tsl_value);
    ez_int32_t aircondition_property_WorkMode_get(tsl_prop_impl_t *thiz, const ez_tsl_rsc_t *rsc_info, ez_tsl_value_t *tsl_value);

    // 属性下发
    ez_int32_t aircondition_property_PowerSwitch_set(tsl_prop_impl_t *thiz, const ez_tsl_rsc_t *rsc_info, const ez_tsl_value_t *tsl_value);
    ez_int32_t aircondition_property_CountdownCfg_set(tsl_prop_impl_t *thiz, const ez_tsl_rsc_t *rsc_info, const ez_tsl_value_t *tsl_value);
    ez_int32_t aircondition_property_Temperature_set(tsl_prop_impl_t *thiz, const ez_tsl_rsc_t *rsc_info, const ez_tsl_value_t *tsl_value);
    ez_int32_t aircondition_property_RoomTemperature_set(tsl_prop_impl_t *thiz, const ez_tsl_rsc_t *rsc_info, const ez_tsl_value_t *tsl_value);
    ez_int32_t aircondition_property_WindSpeedLevel_set(tsl_prop_impl_t *thiz, const ez_tsl_rsc_t *rsc_info, const ez_tsl_value_t *tsl_value);
    ez_int32_t aircondition_property_WorkMode_set(tsl_prop_impl_t *thiz, const ez_tsl_rsc_t *rsc_info, const ez_tsl_value_t *tsl_value);

#ifdef __cplusplus
}
#endif

#endif /* _EZCLOUD_TSL_AIR_CONDITION_H_ */
