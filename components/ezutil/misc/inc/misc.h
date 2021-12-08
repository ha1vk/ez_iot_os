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
 * 2021-11-22     xurongjun    first version 
 *******************************************************************************/

#ifndef _UTIL_MISC_H_
#define _UTIL_MISC_H_

#include <ezos.h>

void bin2hexstr(const unsigned char* src, int len, int upper, unsigned char* dst);

ez_bool_t time_isexpired(ezos_timespec_t *assign_timer);

ez_void_t time_countdown(ezos_timespec_t *assign_timer, ezos_time_t timeout_ms);

#endif
