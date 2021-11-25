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
 * XuRongjun (xurongjun@ezvizlife.com)
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-11-25     xurongjun    first version 
 *******************************************************************************/

#ifndef _EZ_IOT_BASE_EXTERN_H_
#define _EZ_IOT_BASE_EXTERN_H_

#include <ezos.h>

#ifdef __cplusplus
extern "C"
{
#endif

    ez_err_t base_extern_init(ez_void_t);

    ez_void_t base_extern_deinit(ez_void_t);

#ifdef __cplusplus
}
#endif

#endif
