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
 * XuRongjun (xurongjun@ezvizlife.com) - Device Thing Specification Language Protocol
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-03-22     xurongjun    first version 
 *******************************************************************************/

#ifndef _EZCLOUD_TSL_PROVIDER_H_
#define _EZCLOUD_TSL_PROVIDER_H_

#include "ezos.h"
#include "ez_iot_tsl.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct tsl_prop_impl_s tsl_prop_impl_t;
    typedef struct tsl_action_impl_s tsl_action_impl_t;

    struct tsl_prop_impl_s
    {
        ez_char_t *identify;
        ez_char_t *domain;
        ez_char_t *res_type;
        ez_char_t *index;
        ez_int32_t (*func_set)(tsl_prop_impl_t *thiz, const ez_tsl_rsc_t *rsc_info, const ez_tsl_value_t *value_in);
        ez_int32_t (*func_get)(tsl_prop_impl_t *thiz, const ez_tsl_rsc_t *rsc_info, ez_tsl_value_t *value_out);
    };

    struct tsl_action_impl_s
    {
        ez_char_t *identify;
        ez_char_t *domain;
        ez_char_t *res_type;
        ez_char_t *index;
        ez_int32_t (*func_do)(tsl_action_impl_t *thiz, const ez_tsl_rsc_t *rsc_info, const ez_tsl_value_t *value_in, ez_tsl_value_t *value_out);
    };

    ez_int32_t provider_dynamic_rsc_query(const ez_char_t *res_type, ez_char_t index_lst[][4], ez_int32_t max_count);

    extern tsl_prop_impl_t g_tsl_prop_lst[];
    extern tsl_action_impl_t g_tsl_action_lst[];
#ifdef __cplusplus
}
#endif

#endif /* _EZCLOUD_TSL_PROVIDER_H_ */
