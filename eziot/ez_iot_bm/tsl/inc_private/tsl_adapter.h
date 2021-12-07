#include <ezos.h>
#include "tsl_def.h"
#include "ez_iot_tsl.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief 初始化描述文件适配器
     * 
     * @param data_cbs 描述文件读写接口
     * @param things_cbs 物模型处理回调接口
     */
    ez_err_t tsl_adapter_init(ez_tsl_callbacks_t *things_cbs);

    /**
     * @brief 增加一个设备
     * 
     * @param dev_info 设备信息
     * @return ez_int32_t 0表示成功、-1表示失败
     */
    ez_err_t tsl_adapter_add(ez_tsl_devinfo_t *dev_info, ez_char_t *profile);

    /**
     * @brief 检测profile是否正在下载
     * 
     * @param dev_sn 序列化
     * @return true正在下载， false未下载
     */
    ez_bool_t tsl_adapter_status_check(ez_char_t *dev_sn);

    /**
     * @brief 删除一个设备
     * 
     * @param dev_info 设备信息
     * @return ez_int32_t 0表示成功、-1表示失败
     */
    ez_err_t tsl_adapter_del(ez_char_t *dev_sn);

    /**
     * @brief 反初始化
     * 
     */
    ez_void_t tsl_adapter_deinit();

#ifdef __cplusplus
}
#endif