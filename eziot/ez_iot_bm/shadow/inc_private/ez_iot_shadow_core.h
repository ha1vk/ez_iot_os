/**
 * @file shadow_core.h
 * @author xurongjun (xurongjun@ezvizlife.com)
 * @brief 
 * @version 0.1
 * @date 2020-08-17
 * 
 * @copyright Copyright (c) 2020
 * 
 */

#ifndef _SHADOW_CORE_H_
#define _SHADOW_CORE_H_

#ifdef __cplusplus
extern "C"
{
#endif

    typedef enum
    {
        SHADOW_EVENT_TYPE_RESET,   ///< 切换平台
        SHADOW_EVENT_TYPE_ADD,     ///< 增加模块
        SHADOW_EVENT_TYPE_ONLINE,  ///< 设备上线/断线重连
        SHADOW_EVENT_TYPE_OFFLINE, ///< 设备掉线
        SHADOW_EVENT_TYPE_REPORT,  ///< 状态上报
        SHADOW_EVENT_TYPE_RECV,    ///< 收到数据
    } shadow_event_type_e;

    /**
     * @brief 驱动shadow模块开始运行
     * 
     * @return true 
     * @return false 
     */
    ez_bool_t shadow_core_start();

    /**
     * @brief shadow模块停止运行
     * 
     */
    ez_void_t shadow_core_stop();

    /**
     * @brief 本地环境相关事件（上下线、模块增删、主动上报）发生，信号通知触发至内部做相应动作。
     * 
     * @param event_type 事件类型
     */
    ez_void_t shadow_core_event_occured(shadow_event_type_e event_type);

    /**
     * @brief v3协议模块注册接口
     * 
     * @param sn 主设备/子设备序列号
     * @param res_type 通道类型（某种类型通道的集合，视频通道集合或者报警通道集合）
     * @param index 通道号（例如报警通道0或者报警通道1）
     * @param domain_id 领域id
     * @param props_num 属性数量
     * @param props 属性列表
     * @return int 返回handle，-1表示失败 
     */
    ez_err_t shadow_core_module_addv3(ez_char_t *sn, ez_char_t *res_type, ez_int16_t index, ez_char_t *domain_id, ez_uint16_t props_num, ez_void_t *props);

    /**
     * @brief 删除领域模块
     * 
     * @param handle 领域对应句柄
     * @return int 
     */
    ez_err_t shadow_core_module_delete(ez_char_t *sn, ez_char_t *res_type, ez_int16_t index, ez_char_t *domain_id);

    /**
     * @brief key值发生更变，触发上报。
     * 
     * @param handle 领域的操作句柄
     * @param pcKey 需要上报的key值
     * @return int 
     */
    ez_err_t shadow_core_propertie_changed(ez_char_t *sn, ez_char_t *res_type, ez_int16_t index, ez_char_t *domain_id, int8_t *pkey, ez_void_t *value);

    /**
     * @brief 云端下发命令入口
     * 
     * @param shadow_res 设备资源
     * @param seq 业务时序值
     * @param payload 消息报文体
     * @return int 
     */
    int shadow_core_cloud_data_in(void *shadow_res, uint32_t seq, int8_t *business_type, int8_t *payload);

#ifdef __cplusplus
}
#endif

#endif
