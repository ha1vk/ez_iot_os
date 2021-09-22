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
 *******************************************************************************/
#ifndef H_TIME_PLATFORM_WRAPPER_H_
#define H_TIME_PLATFORM_WRAPPER_H_



extern ez_timespec* Platform_TimerCreater();                                            \
extern char Platform_TimeIsExpired_Bydiff(ez_timespec *time, EZDEV_SDK_UINT32 time_ms); \
extern char Platform_TimerIsExpired(ez_timespec *time);                                 \
extern void Platform_TimerCountdownMS(ez_timespec *time, unsigned int time_ms);         \
extern void Platform_TimerCountdown(ez_timespec *time, unsigned int timeout);           \
extern EZDEV_SDK_UINT32 Platform_TimerLeftMS(ez_timespec *time);                        \
extern void Platform_TimeDestroy(ez_timespec *time);   

#endif