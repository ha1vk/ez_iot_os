#include <string.h>
#include <math.h>
#include <stdbool.h>

#include "product_test.h"
#include "config_implement.h"
#include "product_config.h"
#include "pt_light_mode.h"

#include "ezlog.h"
#include "ezos_def.h"
#include "ezconn.h"
#include "ezhal_wifi.h"

#define TAG_PROTEST "T_PRODUCT" // 日志打印TAG

#define AP_LIST_MAX_NUM 20 // 每次扫描wifi列表的最大个数
#define AP_SCAN_MAX_TIME 2 // 每次扫描wifi列表的最大个数

#define TEST_WIFI_SSID_PREFIX   "EzvizTest001_" // 产测环境路由前缀，如moduletest001_XX，XX表示老练测试要进行的时间

#define TEST2_WIFI_SSID_PREFIX  "EzvizTest002_" // 产测环境路由前缀，如moduletest001_XX，XX表示老练测试要进行的时间
#define MS_PER_MIN (60 * 1000)
#define TEST_WEEK_SIGNAL (-60)

#define STAGE_1_TIME 120 // 第一阶段产测时间，单位s
#define PT_ENDLESS_LOOP(ms) \
    do                      \
    {                       \
        ezos_delay_ms(ms);  \
    } while(1)

bool g_stage_1_flag = false;
bool g_stage_2_flag = false;

int g_stage_1_time = 0; // 根据用户配置文件获取
int g_stage_2_time = 0; // 根据wifi信息获取

typedef enum
{
    STATE_NO_ROUTE,
    STATE_WEAK_SIGNAL,
    STATE_NORMAL,
} pt_state_e;

static pt_state_e check_test_environment()
{
    pt_state_e pt_state = STATE_NO_ROUTE;
    ezconn_wifi_config(EZCONN_WIFI_MODE_STA);
    ezos_delay_ms(2000);
    ezhal_wifi_list_t ap_list[AP_LIST_MAX_NUM] = {0};
    int wifi_list_num = 0;
    
    for (int j = 0; j < AP_SCAN_MAX_TIME; j++)
    {
        memset(&ap_list, 0, sizeof(ap_list));

        wifi_list_num = ezhal_sta_get_scan_list(AP_LIST_MAX_NUM, ap_list);
        if (0 == wifi_list_num)
        {
            ezlog_e(TAG_PROTEST, "wifi scan failed.");
            break;
        }

        for (int i = 0; i < wifi_list_num; ++i)
        {
        	//如果产测二未成功，就开始扫描产测二的路由器
            if(!g_stage_2_flag)
            {
	            if (0 == strncmp((char *)ap_list[i].ssid, TEST_WIFI_SSID_PREFIX, strlen((char *)TEST_WIFI_SSID_PREFIX)))
                {
	                sscanf((char *)ap_list[i].ssid, "EzvizTest001_%d", &g_stage_2_time);
                    if (1 > g_stage_2_time)
                    {
                        g_stage_2_time = 50;
                    }

                    ezlog_i(TAG_PROTEST, "stage 2 test %d min.", g_stage_2_time);
                    ezlog_d(TAG_PROTEST, "scaned ssid: %s,rssi=%d", ap_list[i].ssid, ap_list[i].rssi);
                    if (ap_list[i].rssi < TEST_WEEK_SIGNAL)
                    {
                        pt_state = STATE_WEAK_SIGNAL;
                        continue;
                    }
                    else
                    {
                        //搜索到强信号则退出来
                        pt_light_stage2_time(g_stage_2_time);
                        pt_state = STATE_NORMAL;
                        break;
                    }
                }
            }
			else//如果产测二已经成功，则扫描产测三的路由器
			{
	            if (0 == strncmp((char *)ap_list[i].ssid, TEST2_WIFI_SSID_PREFIX, strlen((char *)TEST2_WIFI_SSID_PREFIX)))
                {
                    ezlog_d(TAG_PROTEST, "scaned ssid: %s,rssi=%d", ap_list[i].ssid, ap_list[i].rssi);
                    if (ap_list[i].rssi < TEST_WEEK_SIGNAL)
                    {
                        pt_state = STATE_WEAK_SIGNAL;
                        continue;
                    }
                    else
                    {
                        pt_state = STATE_NORMAL;
                        break;
                    }
                }
			}
        }

        if (g_stage_2_flag || STATE_NORMAL == pt_state)  // 包装测试 只扫描一次就退出来
        {
            break;
        }
    }

    ezlog_d(TAG_PROTEST, "check test environment exit.");
    return pt_state;
}

static int read_product_test_config()
{
    int ret = 0;

    int size = sizeof(g_stage_1_flag);
    ret = config_get_value(K_PT_PARA1, &g_stage_1_flag, &size);
    size = sizeof(g_stage_2_flag);
    ret |= config_get_value(K_PT_PARA2, &g_stage_2_flag, &size);
    if (0 != ret)
    {
        ezlog_e(TAG_PROTEST, "get pt param failed.");
        return -1;
    }
    
    ezlog_i(TAG_PROTEST, "stage 1 flag: %d", g_stage_1_flag);
    ezlog_i(TAG_PROTEST, "stage 2 flag: %d", g_stage_2_flag);
	
    BULB_TEST_T *bulb_test = get_product_test_param();

    g_stage_1_time = bulb_test->step1time;

    if (20 > g_stage_1_time || g_stage_1_time > 600)
    {
        g_stage_1_time = STAGE_1_TIME;
    }
    ezlog_i(TAG_PROTEST, "stage 1 time : %d s.", g_stage_1_time);

    return 0;
}

static int act_no_route()
{
    ezlog_d(TAG_PROTEST, "%s enter.", __FUNCTION__);
    pt_light_set_mode(MODE_NO_ROUTE);
    PT_ENDLESS_LOOP(100);
    return 0;
}

static int act_weak_signal()
{
    ezlog_d(TAG_PROTEST, "%s enter.", __FUNCTION__);
    pt_light_set_mode(MODE_WEAK_SIGNAL);
    PT_ENDLESS_LOOP(100);
    return 0;
}

static int act_pt1_normal()
{
    ezlog_d(TAG_PROTEST, "%s enter.", __FUNCTION__);
    int stage_2_age_time = 0;
    int size = sizeof(stage_2_age_time);
    int ret = config_get_value(K_PT_AGE_TIME, &stage_2_age_time, &size);
    if (0 != ret)
    {
        ezlog_e(TAG_PROTEST, "get pt param failed.");
        return -1;
    }

    int delay_time = 0;
    
    if (stage_2_age_time > 0)
    {
        pt_light_set_mode(MODE_PT1_RETEST);
        delay_time = 12500;
    }
    else
    {
        pt_light_set_mode(MODE_PT1_NORMAL);
        delay_time = 60000;
    }
    ezos_delay_ms(delay_time);

    ezlog_d(TAG_PROTEST, "pt2 normal enter");
    pt_light_set_mode(MODE_PT2_NORMAL);
    ezos_delay_ms((g_stage_2_time - stage_2_age_time) * 60 * 1000);

    ezlog_d(TAG_PROTEST, "pt2 end enter");
    pt_light_set_mode(MODE_PT2_END);

    g_stage_2_flag = true;
    size = sizeof(g_stage_2_flag);
    config_set_value(K_PT_PARA2, &g_stage_2_flag, size);
    config_set_value(K_PT_AGE_TIME, &g_stage_2_time, sizeof(g_stage_2_time));
    PT_ENDLESS_LOOP(100);
    return ret;
}

static int act_pt3_normal()
{
    ezlog_d(TAG_PROTEST, "%s enter.", __FUNCTION__);
    int ret = 0;
    int stage_3_count = 0;
    int size = sizeof(stage_3_count);
    ret = config_get_value(K_PT_STAGE3_COUNT, &stage_3_count, &size);
    if (0 != ret)
    {
        ezlog_e(TAG_PROTEST, "read stage3_count failed.");
        return ret;
    }
    
    stage_3_count++;
    ret = config_set_value(K_PT_STAGE3_COUNT, &stage_3_count, size);
    if (0 != ret)
    {
        ezlog_e(TAG_PROTEST, "write stage3_count failed.");
        return ret;
    }

    if (stage_3_count >= 5)
    {
        ezlog_i(TAG_PROTEST, "5 enter to stage 3,factory set");
        config_reset_factory();
        pt_light_set_mode(MODE_RESET_FACTORY);
        PT_ENDLESS_LOOP(100);
    }
    else
    {
        pt_light_set_mode(MODE_PT3_NORMAL);
        PT_ENDLESS_LOOP(100);
    }
    return ret;
}

int ez_product_test(int type)
{
    int ret = 0;

    ret = pt_light_init(type);
    if (0 != ret)
    {
        ezlog_e(TAG_PROTEST, "pt_light init failed.");
        return ret;
    }
    
    ret = read_product_test_config();
    if (0 != ret)
    {
        ezlog_e(TAG_PROTEST, "read product test failed.");
        return ret;
    }
    pt_state_e pt_state = check_test_environment();
    switch (pt_state)
    {
    case STATE_NO_ROUTE:
        if (!g_stage_2_flag)
        {
            ret = act_no_route();
        }
        else
        {
            ret = 0;
        }
        break;
    case STATE_WEAK_SIGNAL:
        ret = act_weak_signal();
        break;
    case STATE_NORMAL:
        if (!g_stage_2_flag)
        {
            ret = act_pt1_normal();
        }
        else
        {
            ret = act_pt3_normal();
        }
        
        break;
    default:
        break;
    }

    return ret;
}

int is_product_test_done()
{
    int step2 = 0;
    int size = sizeof(step2);
    config_get_value(K_PT_PARA2, &step2, &size);

    return step2;
}