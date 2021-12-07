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
 * xurongjun (xurongjun@ezvizlife.com)
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-12-05     xurongjun    first version 
 *******************************************************************************/

#ifndef _TSL_PROFILE_H_
#define _TSL_PROFILE_H_

#include <ezos.h>

#ifdef __cplusplus
extern "C"
{
#endif

    ez_err_t tsl_profile_init(ez_void_t);

    ez_err_t tsl_profile_deinit(ez_void_t);

    ez_bool_t tsl_profile_check(ez_char_t *dev_sn);

    ez_err_t tsl_profile_reg(ez_char_t *dev_sn, ez_char_t *dev_type, ez_char_t *dev_fwver, ez_char_t *profile);

    ez_bool_t tsl_profile_ref_add(ez_char_t *dev_sn, ez_char_t *dev_type, ez_char_t *dev_fwver);

    ez_void_t tsl_profile_ref_del(ez_char_t *dev_sn);

    const tsl_capacity_t *tsl_profile_get_lock(ez_char_t *dev_sn);

    ez_void_t tsl_profile_get_unlock(ez_void_t);


#ifdef __cplusplus
}
#endif

#endif