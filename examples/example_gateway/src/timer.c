#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <stdio.h>
#include <signal.h>
#include <stddef.h>

#include "ez_iot_core.h"
#include "ez_iot_hub.h"
#include "ezlog.h"
#include "kv_imp.h"
#include "ez_iot_tsl.h"
#include "cJSON.h"
#include "flashdb.h"
#include "hub_mgr.h"
#include "ezos_thread.h"


static ez_thread_t g_subdev_alive_deal_thread;


void subdev_alive_deal_stop(void)
{
    ezlog_e(TAG_APP, "subdev_alive_deal stop");

    ezos_thread_destroy(g_subdev_alive_deal_thread);
}


void signalHandler(int signo)
{
    switch (signo){
        case SIGALRM:
             subdev_alive_func();   //子设备心跳处理函数
             adddev_overtime_close_window();//添加子设备窗口 超时处理，关闭添加窗口
             
            break;
    }
}


static void subdev_alive_deal_thread(void *user_data)
{
    int res = 1;
    //n 秒延时  ，该函数每 n 秒执行一次               // n = SUBDEV_ALIVE_DEAL_INTERVALTIME
    signal(SIGALRM, signalHandler);

    //初始化定时器 ， 5秒后启动定时器，每5秒触发一次
    struct itimerval new_value, old_value;
    new_value.it_value.tv_sec = SUBDEV_ALIVE_DEAL_INTERVALTIME;    
    new_value.it_value.tv_usec = 0;
    new_value.it_interval.tv_sec = SUBDEV_ALIVE_DEAL_INTERVALTIME;
    new_value.it_interval.tv_usec = 0;
    res = setitimer(ITIMER_REAL, &new_value, &old_value);

    if (res)
    {
        ezlog_e(TAG_APP, "Set timer failed!!/n");
    }
    
}




void timer_deal(void)
{
    ezlog_w(TAG_APP, "subdev_alive_deal start");

    ezos_thread_create(&g_subdev_alive_deal_thread , (int8_t *)"subdev_alive_deal" , subdev_alive_deal_thread , NULL ,  1024 * 4, 0);
    if (NULL == g_subdev_alive_deal_thread)
    {
        ezlog_i(TAG_APP, "subdev alive deal thread stop");
        subdev_alive_deal_stop();   
    }
}

