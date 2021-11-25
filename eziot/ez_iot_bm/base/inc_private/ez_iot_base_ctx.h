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

#ifndef _EZ_IOT_BASE_CTX_H_
#define _EZ_IOT_BASE_CTX_H_

#include "ezos.h"
#include "ez_iot_base.h"

#ifdef __cplusplus
extern "C"
{
#endif

    ez_void_t base_notice_set(const ez_base_notice pfunc);

    ez_base_notice base_notice_get(ez_void_t);

#ifdef __cplusplus
}
#endif

#endif