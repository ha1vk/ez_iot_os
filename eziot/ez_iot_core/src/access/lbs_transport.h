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
 * Brief:
 * Device access authentication interface declaration
 * 
 * Change Logs:
 * Date           Author       Notes
 * 2021-12-01     xurongjun    Remove redundant interfaces
 *******************************************************************************/

#ifndef H_LBS_TRANSPORT_H_
#define H_LBS_TRANSPORT_H_

#define LBS_TRANSPORT_INTERFACE                                                                       \
    extern ez_sdk_error lbs_redirect(ezdev_sdk_kernel *sdk_kernel);                                   \
    extern ez_sdk_error lbs_redirect_with_auth(ezdev_sdk_kernel *sdk_kernel, EZDEV_SDK_UINT8 nUpper); \
    extern ez_sdk_error lbs_redirect_createdevid_with_auth(ezdev_sdk_kernel *sdk_kernel, EZDEV_SDK_UINT8 nUpper);

#endif //H_LBS_TRANSPORT_H_