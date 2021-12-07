#ifndef _TSL_LEGALITY_H_
#define _TSL_LEGALITY_H_

#include <ezos.h>
#include "ez_iot_tsl.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief 校验属性功能点数据是否合法
     * 
     * @param sn 序列号
     * @param rsc_info 通道信息
     * @param key_info 功能点
     * @param value 功能点数据
     * @return EZ_TSL_ERR_VALUE_TYPE 
     * @return EZ_TSL_ERR_VALUE_ILLEGAL 
     */
    ez_err_t tsl_legality_property_check(const ez_char_t *sn, const ez_tsl_rsc_t *rsc_info, const ez_tsl_key_t *key_info, const ez_tsl_value_t *value);

    /**
     * @brief 校验事件功能点数据是否合法
     * 
     * @param sn 序列号
     * @param rsc_info 通道信息
     * @param key_info 功能点
     * @param value 功能点数据
     * @return EZ_TSL_ERR_VALUE_TYPE 
     * @return EZ_TSL_ERR_VALUE_ILLEGAL 
     */
    ez_err_t tsl_legality_event_check(const ez_char_t *sn, const ez_tsl_rsc_t *rsc_info, const ez_tsl_key_t *key_info, const ez_tsl_value_t *value);

#ifdef __cplusplus
}
#endif

#endif //_EZ_IOT_TSL_INFO_CHECK_H_