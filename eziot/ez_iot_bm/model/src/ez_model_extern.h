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
#ifndef H_EZ_MODEL_EXTERN_H_
#define H_EZ_MODEL_EXTERN_H_

#include "ez_iot_model_def.h"

#ifdef __cplusplus
extern "C"
{
#endif

    int ez_model_extern_init();

    void ez_model_extern_deinit();

    int ez_get_list_size();

    int ez_reg_domain(const ez_domain_reg_t *domain_reg);

    int ez_dereg_domain(const char *domain);

    ez_domain_reg_t *ez_get_reg_domain(const char *domain);

    int ez_set_data_route_cb(ez_model_default_cb_t *ez_data_router);

#ifdef __cplusplus
}
#endif

#endif
