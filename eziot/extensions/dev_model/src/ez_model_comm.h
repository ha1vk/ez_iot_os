
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
#ifndef H_EZ_MODEL_COMM_H_
#define H_EZ_MODEL_COMM_H_

#include "bscJSON.h"
#include "ez_model_def.h"

#define EZ_METHOD_LEN 128

#ifdef __cplusplus
extern "C"
{
#endif

    bscJSON *ez_value_to_json(ez_model_msg *pValue);

#ifdef __cplusplus
}
#endif

#endif