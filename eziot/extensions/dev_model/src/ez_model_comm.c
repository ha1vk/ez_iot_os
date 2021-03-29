#include "stdlib.h"
#include "string.h"
#include "ez_sdk_log.h"
#include "ez_model_comm.h"


bscJSON *ez_value_to_json(ez_model_msg *pValue)
{
    bscJSON *json_value = NULL;
  
    if (!pValue)
    {
        return NULL;
    }
        
    ez_log_d(TAG_MOD, "ez_value_to_json:type:%d\n", pValue->type);
    
    switch (pValue->type)
    {
    case model_data_type_bool:
        json_value = bscJSON_CreateBool(pValue->value_bool);
        break;
    case model_data_type_int:
        json_value = bscJSON_CreateNumber(pValue->value_int);
        break;
    case model_data_type_double:
        json_value = bscJSON_CreateNumber(pValue->value_double);
        break;
    case model_data_type_string:
        json_value = bscJSON_CreateString(pValue->value);
        break;
    case model_data_type_array:
    case model_data_type_object:
        json_value = bscJSON_Parse(pValue->value);
        break;
    case model_data_type_null:
        json_value = bscJSON_CreateNull();
    default:
        break;
    }

    return json_value;
}