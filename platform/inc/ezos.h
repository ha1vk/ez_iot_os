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
*******************************************************************************/

#ifndef _EZOS_H_
#define _EZOS_H_

#ifdef CONFIG_EZIOT_OS_RT
#include <rtthread.h>
#else
#include <ezos_gconfig.h>
#endif

#include <ezos_def.h>
#include <ezos_thread.h>
#include <ezos_time.h>
#include <ezos_sem.h>
#include <ezos_socket.h>
#include <ezos_libc.h>
#include <ezos_kv.h>
#include <ezos_mem.h> 
#include <ezos_system.h>
#endif