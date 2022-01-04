#include "ezos.h"
#include "ezlog.h"
#include "eztimer.h"
#include "ez_iot_base.h"

extern int ez_cloud_init();
int ez_cloud_base_init();
static ez_int32_t ez_base_notice_func(ez_base_event_e event_type, ez_void_t *data, ez_int32_t len);
static ez_bool_t g_is_inited = ez_false;
static int m_challenge_code = -1;

int example_bind(int argc, char **argv)
{
    ezlog_init();
    ezlog_start();
    ezlog_filter_lvl(CONFIG_EZIOT_EXAMPLES_SDK_LOGLVL);

    ez_cloud_init();
    ez_cloud_base_init();
    if (argc > 1)
    {
        ez_iot_base_bind_near(argv[1]);
    }

    return 0;
}

static void example_bind_sta(void)
{
    ezlog_init();
    ezlog_start();
    ezlog_filter_lvl(CONFIG_EZIOT_EXAMPLES_SDK_LOGLVL);

    /* 查询设备绑定关系 */
    ez_cloud_init();
    ez_cloud_base_init();

    ez_iot_base_bind_query();
}

#ifdef FINSH_USING_MSH
MSH_CMD_EXPORT(example_bind, eziot example bind param : <token> e.g example_bind cf08393f8581407fad8c3d55dae434ff);
MSH_CMD_EXPORT(example_bind_sta, eziot example bind status query);
#else
// int main(int argc, char **argv)
// {
//     return example_kv(argc, argv);
// }
#endif

void wait_for_button_confirm(void)
{
    //TODO 检测用户已按下按键

    /* Step 3: 发回挑战码，完成添加响应 */
    ez_iot_base_bind_response(m_challenge_code);
}

static ez_int32_t ez_base_notice_func(ez_base_event_e event_type, ez_void_t *data, ez_int32_t len)
{
    ez_bind_challenge_t *bind_chanllenge;

    switch (event_type)
    {
    case EZ_EVENT_BINDING:
    {
        /* 设备已绑定 */
        ezlog_d(TAG_APP, "Device is bound");
    }
    break;
    case EZ_EVENT_UNBINDING:
    {
        /* 设备未绑定 */
        ezlog_d(TAG_APP, "The device is not bound");
    }
    break;
    case EZ_EVENT_BINDING_CHALLENGE:
    {
        /* APP发起接触式绑定 */

        /* Step 1: 记录本次绑定挑战码 */
        bind_chanllenge = (ez_bind_challenge_t *)data;
        m_challenge_code = bind_chanllenge->challenge_code;

        /* Step 2: 等待用户按键确认, 这里用定时器来模拟 */
        eztimer_create("confirm_timer", (10 * 1000), ez_false, wait_for_button_confirm);
    }
    break;
    default:
        break;
    }

    return 0;
}

int ez_cloud_base_init()
{
    if (g_is_inited)
    {
        return 0;
    }

    ez_iot_base_init(ez_base_notice_func);
    g_is_inited = ez_true;

    return 0;
}