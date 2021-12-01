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
#ifndef H_EZ_OTA_EXTERN_H_
#define H_EZ_OTA_EXTERN_H_


#include "ez_iot_ota.h"
#include "ez_ota_def.h"

#ifdef __cplusplus
extern "C"
{
#endif

ez_err_t ez_ota_extern_init();

int ez_ota_extern_fini();

int ez_get_exit_status();

ez_err_t ezdev_ota_module_info_report(const ez_ota_res_t *pres, const ez_ota_modules_t* pmodules, const unsigned int timeout_ms);

#ifdef __cplusplus
}
#endif

#endif