
#include <stdio.h>

#include <string.h>
#include "dev_netmgr.h"


static netmgr_sta_flag_t g_net_sta = 0;
static char g_net_ip[20] = {0};
static char g_net_mac[18] = {0};
static char g_net_ssid[33] = {0};

/**@fn
 * @brief         低三位表示联网三个过程：1,配网，2.连接路由器热点，3.连上平台
 * @param[in]     sta：过程标记,true表示这一过程出现问题,false 表示联网过程正常
 * @param[out]
 * @return
 */

void netmgr_sta_update(netmgr_sta_flag_t sta, int reset)
{
    int cache = g_net_sta;

    if (net_sta_dile == sta)
    {
        g_net_sta = sta;

        return;
    }
    
    if (reset)
    {
        g_net_sta &= ~sta;
    }
    else
    {
        g_net_sta |= sta;
    }

}

/**@fn
 * @brief         判断是否已经配过网，从配置文件中读取是否有ssid 信息来决定
 * @param[in]     
 * @param[out]
 * @return
 */

bool netmgr_is_wd_done(void)
{
    bool ret = true;
    printf("\n to_do DEBUG in line (%d) and function (%s)): \n ",__LINE__, __func__);
    

    return ret;
}

bool netmgr_is_wifi_connected()
{
    return (g_net_sta & net_sta_local);
}

bool netmgr_is_cloud_connected()
{
    return (g_net_sta & net_sta_cloud);
}

bool netmgr_is_ota()
{
    return (g_net_sta & net_sta_ota);
}

bool netmgr_is_distribution()
{
    return (g_net_sta & net_sta_distribution);
}

void netmgr_ip_update(char *ip)
{
    strncpy(g_net_ip, ip, sizeof(g_net_ip) - 1);
}

void netmgr_mac_update(char *mac)
{
    strncpy(g_net_mac, mac, sizeof(g_net_mac) - 1);
}

void netmgr_ssid_update(char *ssid)
{
    strncpy(g_net_ssid, ssid, sizeof(g_net_ssid) - 1);
}

char *netmgr_get_ip(void)
{
    return g_net_ip;
}

char *netmgr_get_ssid(void)
{
    return g_net_ssid;
}

