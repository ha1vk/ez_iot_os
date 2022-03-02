#ifndef _EZVIZ_DISTRIBUTION_H_
#ifdef __cplusplus
extern "C"
{
#endif

#include "ezos_def.h"


#define _EZVIZ_DISTRIBUTION_H_

    /**
     * @brief 设置是否需要进行ap配网
     * 
     * @param enable 
     * @param timeout   ap配网超时时间 
     * @return bool 
     */
    ez_bool_t ap_distribution_set(ez_bool_t enable,ez_int_t timeout);

    /**
     * @brief 开启ap配网
     * 
     * @return bool 
     */
    void ap_distribution_do(void);

    /**
     * @brief 是否需要进入配网
     * 两种情况下：设备开关开关开；2、手机APP重置
     */
    ez_bool_t ap_distribution_check(void);

    /**
     * @brief 连接wifi
     * 
     */
    void wifi_connect_do(void);

    
    /**
     * @brief ap 配网标志位检测和更新
     * 
     * @return int 
     */
    int ap_config_checkupdate(void);


#ifdef __cplusplus
}
#endif
#endif /* _EZVIZ_DISTRIBUTION_H_ */
