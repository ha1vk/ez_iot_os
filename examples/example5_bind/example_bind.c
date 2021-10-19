#include "ez_iot.h"
#include "ez_iot_log.h"

#ifdef RT_THREAD
#include <rtthread.h>
#include <finsh.h>
#endif

//TODO 每次添加，需从debug版萤石互联app获取新的token
#define BIND_TOKEN "cf08393f8581407fad8c3d55dae434ff"

extern int ez_cloud_init();
extern const char *ez_cloud_get_sn();
extern const char *ez_cloud_get_ver();
extern const char *ez_cloud_get_type();
extern int ez_cloud_start();
extern void ez_cloud_deint();

int example_bind(int argc, char **argv)
{
    if (0 != ez_cloud_init() || 0 != ez_cloud_start())
    {
        ez_log_e(TAG_APP, "example bind init err");
    }

    ez_iot_binding(BIND_TOKEN);

    return 0;
}


#ifdef FINSH_USING_MSH
MSH_CMD_EXPORT(example_bind, run ez-iot-sdk example bind);
#else
// int main(int argc, char **argv)
// {
//     return example_kv(argc, argv);
// }
#endif