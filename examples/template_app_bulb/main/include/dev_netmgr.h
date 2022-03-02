
#include <stdbool.h>

typedef enum
{
    net_sta_dile = 0x00000000,         ///< 未配网
    net_sta_distribution = 0x00000001, ///< 正在配网
    net_sta_local = 0x00000002,        ///< 连上wifi
    net_sta_cloud = 0x00000004,        ///< 连上云服务器
    net_sta_ota = 0x00000008           ///< 升级中
} netmgr_sta_flag_t;

/**
 * @brief 更新模组的网络状态
 * 
 * @param sta 要更新的状态值
 * @param reset 是否复位此状态值 
 */
void netmgr_sta_update(netmgr_sta_flag_t sta, int reset);

/**
 * @brief 是否已经完成配网
 * 
 * @return true 已完成
 * @return false 未完成
 */
bool netmgr_is_wd_done();

/**
 * @brief wifi链接状态
 * 
 * @return true 已连接
 * @return false 未连接
 */
bool netmgr_is_wifi_connected();

/**
 * @brief 萤石云连接状态
 * 
 * @return true 已连接
 * @return false 未连接
 */
bool netmgr_is_cloud_connected();

/**
 * @brief 升级状态
 * 
 * @return true 升级中
 * @return false 没在升级状态
 */
bool netmgr_is_ota();

/**
 * @brief 是否处于配网状态
 * 
 * @return true 在处于配网状态
 * @return false 不在配网状态
 */
bool netmgr_is_distribution();

/**
 * @brief 更新ip地址
 * 
 * @param ip ip地址
 */
void netmgr_ip_update(char *ip);

/**
 * @brief 更新ssid
 * 
 * @param ssid 
 */
void netmgr_ssid_update(char *ssid);

/**
 * @brief 获取ip
 * 
 * @return char* 
 */
char *netmgr_get_ip(void);

/**
 * @brief 获取mac
 * 
 * @param mac 
 */
char *netmgr_get_mac(void);

/**
 * @brief 获取ssid
 * 
 * @param ssid 
 */
char *netmgr_get_ssid(void);
