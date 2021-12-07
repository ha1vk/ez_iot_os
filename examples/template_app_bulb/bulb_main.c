/*******************************************************************************
 * Copyright © 2017-2021 Ezviz Inc.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 * 
 * Contributors:
 * liwei (liwei@ezvizlife.com)
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-11-25     xurongjun    first version 
 *******************************************************************************/

#include <stdio.h>

#include "ez_iot_core_def.h"
#include "ez_iot_core.h"
#include "ez_iot_base.h"
#include "ez_iot_tsl.h"

#include "ezlog.h"
#include "kv_imp.h"
#include "ezos_time.h" //延迟需要
#include "signal.h"

#include "bulb_protocol_tsl_parse.h"
#include "bulb_business.h"

static int g_event_id = -1;
static int g_last_err = 0;

static ez_char_t *g_profile_bulb = "{\"version\":\"V1.0.0 build 201231\",\"resources\":[{\"identifier\":\"global\",\"title\":\"global\",\"resourceCategory\":\"global\",\"localIndex\":[\"0\"],\"dynamic\":false,\"global\":true,\"domains\":[{\"identifier\":\"global\",\"title\":\"iot生态灯泡领域\",\"props\":[{\"identifier\":\"brightness\",\"version\":\"v2.0\",\"access\":\"rw\",\"title\":\"亮度\",\"schema\":{\"type\":\"int\",\"title\":\"亮度\",\"maximum\":100,\"minimum\":1}},{\"identifier\":\"color_rgb\",\"version\":\"v2.0\",\"access\":\"rw\",\"title\":\"颜色控制\",\"schema\":{\"type\":\"str\",\"title\":\"彩光\"}},{\"identifier\":\"color_temperature\",\"version\":\"v2.0\",\"access\":\"rw\",\"title\":\"色温控制\",\"schema\":{\"multipleOf\":10,\"type\":\"int\",\"title\":\"色温\",\"maximum\":6500,\"minimum\":2700}},{\"identifier\":\"light_mode\",\"version\":\"v2.0\",\"access\":\"rw\",\"title\":\"亮灯模式\",\"schema\":{\"type\":\"str\",\"title\":\"亮灯模式\",\"enum\":[\"white\",\"color\",\"music\"]}},{\"identifier\":\"light_scene_mode\",\"version\":\"v2.0\",\"access\":\"rw\",\"title\":\"场景模式\",\"schema\":{\"type\":\"str\",\"title\":\"场景模式\",\"enum\":[\"read\",\"warm\",\"night\",\"cinema\",\"sleeping\",\"awaken\"]}},{\"identifier\":\"light_switch\",\"version\":\"v2.0\",\"access\":\"rw\",\"title\":\"开关\",\"schema\":{\"type\":\"boolean\",\"title\":\"开关\"}},{\"identifier\":\"plan\",\"version\":\"v2.0\",\"access\":\"rw\",\"title\":\"定时计划\",\"schema\":{\"type\":\"array\",\"required\":[\"startTime\"],\"items\":{\"type\":\"object\",\"properties\":{\"weekDays\":{\"type\":\"str\"},\"enable\":{\"type\":\"int\"},\"action\":{\"type\":\"str\"},\"startTime\":{\"type\":\"str\"},\"endTime\":{\"type\":\"str\"}}}}},{\"identifier\":\"scene\",\"version\":\"v2.0\",\"access\":\"rw\",\"title\":\"场景状态\",\"schema\":{\"type\":\"object\",\"title\":\"场景状态\",\"properties\":{}}},{\"identifier\":\"scene_conf\",\"version\":\"v2.0\",\"access\":\"rw\",\"title\":\"场景配置\",\"schema\":{\"type\":\"object\",\"title\":\"场景配置\",\"properties\":{}}},{\"identifier\":\"timezone_setting\",\"version\":\"v2.0\",\"access\":\"rw\",\"title\":\"时区设置\",\"schema\":{\"type\":\"object\",\"title\":\"时区设置\",\"properties\":{}}}],\"actions\":[{\"identifier\":\"countdown_off_disable\",\"version\":\"v2.0\",\"direction\":\"Plt2Dev\",\"title\":\"停止倒计时关\"},{\"identifier\":\"countdown_off_enable\",\"version\":\"v2.0\",\"direction\":\"Plt2Dev\",\"title\":\"倒计时-关\",\"input\":{\"schema\":{\"type\":\"int\",\"title\":\"倒计时-关\",\"maximum\":86340,\"minimum\":60}}},{\"identifier\":\"countdown_on_disable\",\"version\":\"v2.0\",\"direction\":\"Plt2Dev\",\"title\":\"停止倒计时开\"},{\"identifier\":\"countdown_on_enable\",\"version\":\"v2.0\",\"direction\":\"Plt2Dev\",\"title\":\"倒计时-开\",\"input\":{\"schema\":{\"type\":\"int\",\"title\":\"倒计时-开\",\"maximum\":86340,\"minimum\":60}}},{\"identifier\":\"countdown_query\",\"version\":\"v2.0\",\"direction\":\"Plt2Dev\",\"title\":\"查询倒计时\",\"output\":{\"schema\":{\"type\":\"object\",\"title\":\"查询 倒计时\",\"properties\":{}}}},{\"identifier\":\"restart\",\"version\":\"v2.0\",\"direction\":\"Plt2Dev\",\"title\":\"重启\"},{\"identifier\":\"timestamp\",\"version\":\"v2.0\",\"direction\":\"Plt2Dev\",\"title\":\"查询设备时间戳\",\"output\":{\"schema\":{\"type\":\"str\",\"title\":\"查询设备时间戳\"}}}],\"events\":[]}]}]}";
static ez_char_t *g_profile_bulbV3 = "{\"version\":\"V1.0.0 build 201231\",\"resources\":[{\"identifier\":\"global\",\"title\":\"全局资源\",\"resourceCategory\":\"global\",\"localIndex\":[\"0\"],\"dynamic\":false,\"global\":true,\"domains\":[{\"identifier\":\"customDomain\",\"title\":\"自定义领域\",\"props\":[{\"identifier\":\"brightness\",\"version\":\"v3.0\",\"access\":\"rw\",\"title\":\"brightness\",\"schema\":{\"type\":\"int\",\"maximum\":100,\"minimum\":1}},{\"identifier\":\"color_rgb\",\"version\":\"v3.0\",\"access\":\"rw\",\"title\":\"color_rgb\",\"schema\":{\"type\":\"str\",\"maxLength\":32}},{\"identifier\":\"color_temperature\",\"version\":\"v3.0\",\"access\":\"rw\",\"title\":\"color_temperature\",\"schema\":{\"type\":\"int\"}},{\"identifier\":\"light_mode\",\"version\":\"v3.0\",\"access\":\"rw\",\"title\":\"light_mode\",\"schema\":{\"type\":\"str\",\"maxLength\":32}},{\"identifier\":\"plan\",\"version\":\"v3.0\",\"access\":\"rw\",\"title\":\"plan\",\"schema\":{\"maxItems\":8,\"type\":\"array\",\"minItems\":1,\"items\":{\"type\":\"str\",\"maxLength\":100}}},{\"identifier\":\"sence\",\"version\":\"v3.0\",\"access\":\"rw\",\"title\":\"sence\",\"schema\":{\"type\":\"str\"}}],\"actions\":[],\"events\":[]}]}]}";

#define BULB_EZIOT_TEST_DEV_TYPE "G1FC47D8"
//线上序列号
#define BULB_EZIOT_TEST_DEV_SERIAL_NUMBER "P20151771"
#define BULB_EZIOT_TEST_DEV_VERIFICATION_CODE "LGYmzx"

#define BULB_EZIOT_TEST_CLOUD_HOST "devcn.eziot.com"
#define BULB_EZIOT_TEST_CLOUD_PORT 8666

static ez_kv_func_t g_kv_func = {
    .ezos_kv_init = kv_init,
    .ezos_kv_raw_set = kv_raw_set,
    .ezos_kv_raw_get = kv_raw_get,
    .ezos_kv_del = kv_del,
    .ezos_kv_del_by_prefix = kv_del_by_prefix,
    .ezos_kv_print = kv_print,
    .ezos_kv_deinit = kv_deinit,
};

static ez_int32_t ez_event_notice_func(ez_event_e event_type, ez_void_t *data, ez_int32_t len)
{
    ezlog_w(TAG_APP, "receive event from sdk: event_type[%d]", event_type);
    ///< 通过ap配网或者蓝牙配网从app获取token
    ez_char_t *dev_token = "11fc0ed37d874d1f8fc58016495db19a";

    switch (event_type)
    {
    case EZ_EVENT_ONLINE:
        ezlog_w(TAG_APP, "dev online");

        if (EZ_CORE_ERR_SUCC != ez_iot_base_bind_near(dev_token))
        {
            ezlog_w(TAG_APP, "bind request send error");
        }
        break;

    case EZ_EVENT_OFFLINE:
        ezlog_w(TAG_APP, "dev offline");

        break;
    case EZ_EVENT_DISCONNECT:
        break;
    case EZ_EVENT_RECONNECT:
        break;
    case EZ_EVENT_DEVID_UPDATE:
        ezlog_w(TAG_APP, "because first regist to ezcloud, you recv event devid update");
        /*you should write the devid to your flash to permanent save */
        break;

    case EZ_EVENT_SERVER_UPDATE:
    {
        ezlog_w(TAG_APP, "recv event svrname update,usually it is not happend");

        /* 遇到服务器迁移时，服务器会会发出重定向的请求，这里需要记录服务器域名，以及发起重新绑定 */
        ezlog_w(TAG_APP, "new domain is %s \n", (char *)data);

        if (EZ_CORE_ERR_SUCC != ez_iot_base_bind_near(dev_token))
        {
            ezlog_w(TAG_APP, "bind request send error");
        }
        break;
    }

    default:
        ezlog_w(TAG_APP, "unknow event type. %d", event_type);
        break;
    }

    return 0;
}

static ez_int32_t ez_base_notice_func(ez_base_event_e event_type, ez_void_t *data, ez_int32_t len)
{
    ez_bind_info_t *bind_info;
    ez_bind_challenge_t *bind_chanllenge;
    ezlog_d(TAG_APP, "receive notice message from sdk: event_type[%d]", event_type);

    switch (event_type)
    {
    case EZ_EVENT_BINDING:
    {
        ezlog_w(TAG_APP, "dev bound");
        bind_info = (ez_bind_info_t *)data;
        if (0 != ezos_strlen(bind_info->user_id))
        {
            /*
	            user_id 为客户端绑定时生成的一个对应id，可以用这个id 做一些业务逻辑，比如
	            一台设备被另一个用户抢占式绑定后，之前绑定保存下来的user_id会与之前的user_id 不一致，
	            此时可以恢复下设备上的默认参数比如清除定时参数
	            简单应用可以暂时不考虑处理
				*/
        }
    }
    break;
    case EZ_EVENT_UNBINDING:
    {
        ezlog_w(TAG_APP, "dev unbound");
    }
    break;
    case EZ_EVENT_BINDING_CHALLENGE:
    {
        ezlog_w(TAG_APP, "binding process......receive EZ_EVENT_BINDING_CHALLENGE ");
        //物理按键绑定
    }
    break;
    default:
        break;
    }

    return 0;
}

void online_access()
{
    ez_byte_t devid[32] = {0};
    ez_dev_info_t dev_info = {0};
    ez_server_info_t lbs_addr = {BULB_EZIOT_TEST_CLOUD_HOST, BULB_EZIOT_TEST_CLOUD_PORT};

    /*First regist to ezcloud,devid is null, otherwise you shuld get the devid from your flash storage */
    if (EZ_CORE_ERR_SUCC != ez_iot_core_ctrl(EZ_CMD_DEVID_SET, (ez_void_t *)devid))
    {
        ezlog_w(TAG_APP, "set devid error");
        return;
    }

    if (EZ_CORE_ERR_SUCC != ez_iot_core_ctrl(EZ_CMD_KVIMPL_SET, (ez_void_t *)&g_kv_func))
    {
        ezlog_w(TAG_APP, "set flash kv function error");
        return;
    }

    /*you can get the device info from you flash storage or other method*/
    ezos_strncpy(dev_info.dev_typedisplay, CONFIG_EZIOT_UNIT_TEST_DEV_DISPLAY_NAME, sizeof(dev_info.dev_typedisplay) - 1);
    ezos_strncpy(dev_info.dev_firmwareversion, CONFIG_EZIOT_UNIT_TEST_DEV_FIRMWARE_VERSION, sizeof(dev_info.dev_firmwareversion) - 1);
#if 1
    dev_info.auth_mode = 0;
    ezos_strncpy(dev_info.dev_type, BULB_EZIOT_TEST_DEV_TYPE, sizeof(dev_info.dev_type) - 1);
    ezos_strncpy(dev_info.dev_subserial, BULB_EZIOT_TEST_DEV_SERIAL_NUMBER, sizeof(dev_info.dev_subserial) - 1);
    ezos_strncpy(dev_info.dev_verification_code, BULB_EZIOT_TEST_DEV_VERIFICATION_CODE, sizeof(dev_info.dev_verification_code) - 1);
#else
    dev_info.auth_mode = 0;
    ezos_strncpy(dev_info.dev_type, CONFIG_EZIOT_UNIT_TEST_DEV_TYPE, sizeof(dev_info.dev_type) - 1);
    ezos_strncpy(dev_info.dev_subserial, CONFIG_EZIOT_UNIT_TEST_DEV_SERIAL_NUMBER, sizeof(dev_info.dev_subserial) - 1);
    ezos_strncpy(dev_info.dev_verification_code, CONFIG_EZIOT_UNIT_TEST_DEV_VERIFICATION_CODE, sizeof(dev_info.dev_verification_code) - 1);
#endif
    /*you can get the lbs addres from the ap distribution or from the flash storage*/
    if (EZ_CORE_ERR_SUCC != ez_iot_core_init(&lbs_addr, &dev_info, ez_event_notice_func))
    {
        ezlog_w(TAG_APP, "ez sdk init error");
        return;
    }

    if (EZ_CORE_ERR_SUCC != ez_iot_base_init(ez_base_notice_func))
    {
    }

    if (EZ_CORE_ERR_SUCC != ez_iot_core_start())
    {
        ezlog_w(TAG_APP, "ez_sdk start error");
        return;
    }
}

void tsl_access()
{
    ez_tsl_callbacks_t tsl_things_cbs = {tsl_things_action2dev, tsl_things_property2cloud, tsl_things_property2dev};
    if (EZ_CORE_ERR_SUCC != ez_iot_tsl_init(&tsl_things_cbs))
    {
        ezlog_w(TAG_APP, "ez_iot_tsl_init  error");
        return;
    }

    if (EZ_CORE_ERR_SUCC != ez_iot_tsl_reg(NULL, g_profile_bulbV3))

    {
        ezlog_w(TAG_APP, "ez_iot_tsl_reg  error");
        return;
    }
}

int main(int argc, char **argv)
{
    ezlog_init();
    ezlog_start();
    ezlog_filter_lvl(5);

    /*mqtt 底层发数据有可能因为服务器关闭重连收到此消息，默认是杀死进程，此处忽略此信号*/
    signal(SIGPIPE, SIG_IGN);

    online_access();

    tsl_access();
#if 0
    if (0 != ez_cloud_init() 
    || 0 != ez_tsl_init() 
    || 0 != ez_cloud_start()
    )
    {
        ezlog_e(TAG_APP, "example prop init err");
    }
#endif

    bulb_ctrl_init();

    /* 主（父）线程循环,否则进程退出*/
    while (1)
    {
        ezos_delay_ms(1000);
    }
    return 0;
}
