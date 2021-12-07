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
#ifndef _EZ_OTA_H_
#define _EZ_OTA_H_

#include <ezos.h>
#include "ez_iot_ota.h"

#ifdef __cplusplus
extern "C"
{
#endif

ez_err_t ez_progress_report(const ez_ota_res_t *pres, const ez_int8_t* pmodule, const ez_int8_t*perr_msg, ez_ota_errcode_e errcode, ez_int8_t status, ez_int16_t progress);

ez_err_t  ez_ota_file_download(ez_ota_download_info_t *input_info, get_file_cb file_cb, notify_cb notify, void* user_data);

#ifdef __cplusplus
}
#endif


#endif

