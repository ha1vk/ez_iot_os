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

#include "ezlog.h"
#include "ez_model_comm.h"


cJSON *ez_value_to_json(ez_model_msg_t *pValue)
{
    cJSON *json_value = NULL;
  
    if (!pValue)
    {
        return NULL;
    }
        
    ezlog_d(TAG_MOD, "ez_value_to_json:type:%d\n", pValue->type);
    
    switch (pValue->type)
    {
    case model_data_type_bool:
        json_value = cJSON_CreateBool(pValue->value_bool);
        break;
    case model_data_type_int:
        json_value = cJSON_CreateNumber(pValue->value_int);
        break;
    case model_data_type_double:
        json_value = cJSON_CreateNumber(pValue->value_double);
        break;
    case model_data_type_string:
        json_value = cJSON_CreateString(pValue->value);
        break;
    case model_data_type_array:
    case model_data_type_object:
        json_value = cJSON_Parse(pValue->value);
        break;
    case model_data_type_null:
        json_value = cJSON_CreateNull();
    default:
        break;
    }

    return json_value;
}