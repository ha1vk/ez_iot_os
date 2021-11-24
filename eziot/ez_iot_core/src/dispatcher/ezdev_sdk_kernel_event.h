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

#ifndef H_EZDEV_SDK_KERNEL_EVENT_H_
#define H_EZDEV_SDK_KERNEL_EVENT_H_

#define EZDEV_SDK_KERNEL_EVENT_INTERFACE                                                                                                 \
    extern mkernel_internal_error broadcast_user_start();                                                                                \
    extern mkernel_internal_error broadcast_user_stop();                                                                                 \
    extern mkernel_internal_error broadcast_user_event(sdk_kernel_event_type event_type, void *ctx, EZDEV_SDK_UINT32 ctx_size);          \
    extern mkernel_internal_error broadcast_user_event_reconnect_success();

#endif