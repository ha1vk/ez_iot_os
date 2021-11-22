#include <ezos.h>
#include "ez_iot_tsl_def.h"
#include "ez_iot_tsl_schema.h"
#include "ez_iot_tsl.h"
#include "ez_iot_shadow.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct
    {
        ez_shadow_res_t shadow_res;
        ez_char_t domain[32];
    } shadow_report_key_t;

    /**
     * @brief 初始化描述文件适配器
     * 
     * @param data_cbs 描述文件读写接口
     * @param things_cbs 物模型处理回调接口
     */
    ez_err_t ez_iot_tsl_adapter_init(ez_tsl_things_callbacks_t *things_cbs);

    /**
     * @brief 增加一个设备
     * 
     * @param dev_info 设备信息
     * @return ez_int32_t 0表示成功、-1表示失败
     */
    ez_err_t ez_iot_tsl_adapter_add(ez_tsl_devinfo_t *dev_info);

    /**
     * @brief 删除一个设备
     * 
     * @param dev_info 设备信息
     * @return ez_int32_t 0表示成功、-1表示失败
     */
    ez_err_t ez_iot_tsl_adapter_del(ez_tsl_devinfo_t *pevinfo);

    /**
     * @brief 删除功能描述文件
     * 
     * @param dev_subserial 设备序列号
     * @return true 成功
     * @return false 找不到对应功能描述文件或文件损坏
     */
    ez_bool_t ez_iot_tsl_adapter_profile_del(const ez_char_t * dev_subserial);

    /**
     * @brief 反初始化
     * 
     */
    ez_void_t ez_iot_tsl_adapter_deinit();

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
    ez_err_t ez_iot_tsl_property_value_legal(const ez_char_t *sn, const ez_tsl_rsc_t *rsc_info, const ez_tsl_key_t *key_info, const ez_tsl_value_t *value);

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
    ez_err_t ez_iot_tsl_event_value_legal(const ez_char_t *sn, const ez_tsl_rsc_t *rsc_info, const ez_tsl_key_t *key_info, const ez_tsl_value_t *value);

    /**
     * @brief 校验操作功能点是否合法
     * 
     * @param sn 序列号
     * @param rsc_info 通道信息
     * @param key_info 功能点
     * @return EZ_TSL_ERR_DEV_NOT_FOUND 
     * @return EZ_TSL_ERR_DEV_NOT_FOUND 
     * @return EZ_TSL_ERR_INDEX_NOT_FOUND 
     * @return EZ_TSL_ERR_DOMAIN_NOT_FOUND 
     * @return EZ_TSL_ERR_KEY_NOT_FOUND 
     */
    ez_err_t ez_iot_tsl_action_value_legal(const ez_char_t *sn, const ez_tsl_rsc_t *rsc_info, const ez_tsl_key_t *key_info, const ez_tsl_value_t *value);

    /**
     * @brief 通过domain和key反查属性的res_type/rsc_category
     * 
     * @param sn 序列号
     * @param key_info 功能点
     * @param res_type 资源类型缓冲区
     * @param len 资源类型缓冲区长度
     * @return ez_int32_t 
     * @return ez_errno_succ 
     * @return EZ_TSL_ERR_DEV_NOT_FOUND 
     * @return EZ_TSL_ERR_PROFILE_LOADING 
     * @return EZ_TSL_ERR_DEV_NOT_FOUND 
     * @return EZ_TSL_ERR_KEY_NOT_FOUND 
     */
    ez_err_t tsl_find_property_rsc_by_keyinfo(const ez_char_t *sn, const ez_tsl_key_t *key_info, ez_char_t *res_type, ez_int32_t len);

    /**
     * @brief 通过domain和key反查事件的res_type/rsc_category
     * 
     * @param sn 序列号
     * @param key_info 功能点
     * @param res_type 资源类型缓冲区
     * @param len 资源类型缓冲区长度
     * @return ez_int32_t 
     * @return ez_errno_succ 
     * @return EZ_TSL_ERR_DEV_NOT_FOUND 
     * @return EZ_TSL_ERR_PROFILE_LOADING 
     * @return EZ_TSL_ERR_DEV_NOT_FOUND 
     * @return EZ_TSL_ERR_KEY_NOT_FOUND 
     */
    ez_err_t tsl_find_event_rsc_by_keyinfo(const ez_char_t *sn, const ez_tsl_key_t *key_info, ez_char_t *res_type, ez_int32_t len);

#ifdef __cplusplus
}
#endif