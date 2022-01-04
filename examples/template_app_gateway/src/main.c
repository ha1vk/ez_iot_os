#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ezcloud_access.h"
#include "hub_mgr.h"
#include "ez_iot_hub.h"
#include "ez_iot_core.h"



#define DEFAULT_TIMEOUT_S                     20




static ez_int32_t ez_hub_event_notice_func(ez_subdev_event_e event_type, void *data, EZ_INT len)
{
    switch (event_type)
    {
    case EZ_EVENT_SUBDEV_ADD_SUCC:

        break;
    case EZ_EVENT_SUBDEV_ADD_FAIL:

        break;

    default:
        break;
    }

    return 0;
}


int main(int argc, char **argv)
{
    ez_cloud_init();
    ez_tsl_init();
    ez_hub_callbacks_t hub_callbacks_cbs = {ez_hub_event_notice_func};
    ez_iot_hub_init(&hub_callbacks_cbs);
    ez_cloud_start();

    subdev_alive_init(); //子设备心跳管理，上电初始化
    
    timer_deal(); //定时器处理。包含 子设备心跳处理函数、子设备添加超时处理          --//每5秒执行一次

    dev_event_waitfor(EZ_EVENT_ONLINE, DEFAULT_TIMEOUT_S * 1000);//等待设备上线

    //APP端使能子设备添加窗口后，才可添加子设备   （使能窗口示例程序在TSL回调函数中）
    //使能子设备添加窗口，示例函数                  open_add_subdev
    
    add_subdev_example();//添加子设备示例
    
    subdev_alive_update_example();//子设备心跳更新示例

    del_subdev_example();//删除子设备示例

    //5分钟超时 ， 禁用子设备添加窗口，不再添加子设备
    // 超时处理关闭窗口  ，示例函数           adddev_overtime_close_window
    
    while (1)
    {
        char message[100];
        fputs("exit           :q\n", stdout);
        fgets(message, 100, stdin);

        if (0 != strcmp(message, "q\n") || 0 != strcmp(message, "Q\n"))
        {
            break;
        }
    }

    return 0;
}


