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
* Contributors:
 *    shenhongyin - initial API and implementation and/or initial documentation
 *******************************************************************************/
#ifndef _H_EZ_MODEL_USER_H_
#define _H_EZ_MODEL_USER_H_

#include "ez_model_def.h"
#include "ez_model_extern.h"

#ifdef __cplusplus
extern "C"
{
#endif

    int ez_model_send_user_msg(ez_basic_info *basic_info, ez_model_msg *msg, ez_msg_attr *msg_attr);

    int ez_model_send_reply(ez_basic_info *basic_info, ez_model_msg *msg, ez_err_info *status, ez_msg_attr *msg_attr);

    int ez_model_send_origin_msg(ez_basic_info *basic_info, const char *msg, unsigned int msg_len, int msg_response, ez_msg_attr *msg_attr);

    const char *ez_model_get_version();

#ifdef __cplusplus
}
#endif

#endif