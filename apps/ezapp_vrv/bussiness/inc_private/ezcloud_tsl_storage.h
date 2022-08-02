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
 * XuRongjun (xurongjun@ezvizlife.com) - Device Thing Specification Language data storage wrapper
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-07-27     xurongjun    first version 
 *******************************************************************************/

#include <ezos.h>
#include <ezcloud_tsl_provider.h>

#ifndef _EZCLOUD_TSL_STORAGE_H_
#define _EZCLOUD_TSL_STORAGE_H_

#include "ezos.h"
#include "ez_iot_tsl.h"

#ifdef __cplusplus
extern "C"
{
#endif

    ez_void_t property_get_wrapper(tsl_prop_impl_t *thiz, const ez_tsl_rsc_t *rsc_info, ez_tsl_value_t *tsl_value, ez_tsl_data_type_e value_type, ez_void_t *default_value);

    ez_bool_t property_set_wrapper(tsl_prop_impl_t *thiz, const ez_tsl_rsc_t *rsc_info, const ez_tsl_value_t *tsl_value);

#ifdef __cplusplus
}
#endif

#endif /* _EZCLOUD_TSL_STORAGE_H_ */
