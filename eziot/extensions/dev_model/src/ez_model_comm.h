
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