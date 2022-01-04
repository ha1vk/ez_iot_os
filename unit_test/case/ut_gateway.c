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
 * XuRongjun (xurongjun@ezvizlife.com) ez iot hub unit-test case
 *******************************************************************************/

#include <stdlib.h>
#include <ezos.h>
#include <utest.h>
#include <ezlog.h>
#include <kv_imp.h>

#include "ez_iot_core_def.h"
#include "ez_iot_core.h"
#include "ez_iot_hub.h"

#include "ezlog.h"
#include "ezos_kv.h"
// =================================================
#define EZ_IOT_CLOUD_ENTRY_HOST "test15.ys7.com"
#define EZ_IOT_CLOUD_ENTRY_PORT 8666
// =================================================

// =================================================
#define EZ_IOT_DEV_AUTH_MODE 1
#define EZ_IOT_DEV_UUID "4LYV8SK7UKLBOUOVS6HXVX:EF229364D"
#define EZ_IOT_DEV_LICENSE "2QaaGDXFRMwPavSdojL5Ny"
#define EZ_IOT_DEV_PRODUCT_KEY "4LYV8SK7UKLBOUOVS6HXVX"
#define EZ_IOT_DEV_NAME "EF229364D"
#define EZ_IOT_DEV_DISPLAY_NAME "IOT_UTEST_DEV"
#define EZ_IOT_DEV_ID ""
#define EZ_IOT_DEV_FWVER "V1.2.0 build 201212"
// =================================================
//#define CONFIG_EZIOT_UNIT_TEST_CASE_TIEMOUT_SECONDS 10

void ut_gateway_errcode();  ///< 测试接口返回错误码
void ut_gateway_authsap();  ///< 测试序列号验证添加
void ut_gateway_authlic();  ///< 测试license设备添加
void ut_gateway_authfail(); ///< 测试认证失败的流程
void ut_gateway_dup();      ///< 测试重复添加
void ut_gateway_update();   ///< 测试子设备信息更新(版本号、在线状态)
void ut_gateway_enum();     ///< 测试子设备遍历
void ut_gateway_clean();    ///< 测试清空功能
void ut_gateway_stress();   ///< 压力测试

static int m_event_id = -1;
static ez_char_t m_add_result_sn[32] = {0};
static int m_add_result = -1;

static long global_init();
static ez_int32_t ez_event_notice_func(ez_event_e event_type, ez_void_t *data, ez_int32_t len);
static ez_int32_t hub_recv_event_cb(ez_subdev_event_e event_type, ez_void_t *data, ez_int32_t len);
static ez_int32_t wait4add_result(ez_subdev_event_e event_type, ez_char_t *sn, ez_int32_t time_ms);

static ez_server_info_t m_lbs_addr = {CONFIG_EZIOT_UNIT_TEST_CLOUD_HOST, CONFIG_EZIOT_UNIT_TEST_CLOUD_PORT};
static ez_dev_info_t m_dev_info = {0};
static ez_hub_callbacks_t m_hub_cbs = {hub_recv_event_cb};

static ez_kv_func_t g_kv_func = {
    .ezos_kv_init = kv_init,
    .ezos_kv_raw_set = kv_raw_set,
    .ezos_kv_raw_get = kv_raw_get,
    .ezos_kv_del = kv_del,
    .ezos_kv_del_by_prefix = kv_del_by_prefix,
    .ezos_kv_print = kv_print,
    .ezos_kv_deinit = kv_deinit,
};

static ez_char_t m_test_subdev1_authm = 0;
static ez_char_t *m_test_subdev1_sn = "P20150916";
static ez_char_t *m_test_subdev1_ver = "V1.6.0 build 210118";
static ez_char_t *m_test_subdev1_type = "4LYV8SK7UKLBOUOVS6HXVX";
static ez_char_t *m_test_subdev1_vcode = "VS7ij3";
static ez_char_t *m_test_subdev1_uuid = "FFEEDDCCBBAA";

static ez_char_t m_test_subdev2_authm = 1;
static ez_char_t *m_test_subdev2_sn = "ARZCWQSRG3CW";
static ez_char_t *m_test_subdev2_ver = "V1.6.0 build 210118";
static ez_char_t *m_test_subdev2_type = "4LYV8SK7UKLBOUOVS6HXVX";
static ez_char_t *m_test_subdev2_vcode = "Jcjkn8xah1qUk5JKJFjKD2";
static ez_char_t *m_test_subdev2_uuid = "AABBCCDDEEFF";

static ez_subdev_info_t m_subdev1 = {0};
static ez_subdev_info_t m_subdev2 = {0};

static void eziot_ut_gateway(void)
{
    UTEST_UNIT_RUN(ut_gateway_errcode);  ///< 测试接口返回错误码
    UTEST_UNIT_RUN(ut_gateway_authsap); ///< 测试序列号验证添加
    UTEST_UNIT_RUN(ut_gateway_authlic);  ///< 测试license设备添加
    UTEST_UNIT_RUN(ut_gateway_authfail); ///< 测试认证失败的流程
    UTEST_UNIT_RUN(ut_gateway_dup);      ///< 测试重复添加
    UTEST_UNIT_RUN(ut_gateway_update);   ///< 测试子设备信息更新(版本号、在线状态)
    UTEST_UNIT_RUN(ut_gateway_enum);     ///< 测试子设备遍历
    UTEST_UNIT_RUN(ut_gateway_clean);    ///< 测试清空功能
    UTEST_UNIT_RUN(ut_gateway_stress);   ///< 压力测试
}
UTEST_TC_EXPORT(eziot_ut_gateway, "eziot.ut_gateway", global_init, NULL, CONFIG_EZIOT_UNIT_TEST_CASE_TIEMOUT_SECONDS);

void ut_gateway_errcode()
{
    ez_byte_t devid[32] = {0};
    ez_subdev_info_t subdev_temp = {0};

    //sdk core not init
    uassert_int_equal(EZ_HUB_ERR_NOT_INIT, ez_iot_hub_add(&m_subdev1));
    uassert_int_equal(EZ_HUB_ERR_NOT_INIT, ez_iot_hub_del(m_test_subdev1_sn));
    uassert_int_equal(EZ_HUB_ERR_NOT_INIT, ez_iot_hub_ver_update(m_test_subdev1_sn, m_test_subdev2_sn));
    uassert_int_equal(EZ_HUB_ERR_NOT_INIT, ez_iot_hub_status_update(m_test_subdev1_sn, false));
    uassert_int_equal(EZ_HUB_ERR_NOT_INIT, ez_iot_hub_subdev_query(m_test_subdev1_sn, &subdev_temp));
    uassert_int_equal(EZ_HUB_ERR_NOT_INIT, ez_iot_hub_subdev_next(&subdev_temp));
    uassert_int_equal(EZ_HUB_ERR_NOT_INIT, ez_iot_hub_clean());
    uassert_int_equal(EZ_HUB_ERR_NOT_INIT, ez_iot_hub_deinit());

    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_ctrl(EZ_CMD_DEVID_SET, (ez_void_t *)devid));
    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_ctrl(EZ_CMD_KVIMPL_SET, (ez_void_t *)&g_kv_func));
    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_init(&m_lbs_addr, &m_dev_info, ez_event_notice_func));

    uassert_int_equal(EZ_HUB_ERR_SUCC, ez_iot_hub_init(&m_hub_cbs));
    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_start());

    uassert_int_equal(EZ_HUB_ERR_PARAM_INVALID, ez_iot_hub_add(NULL));
    uassert_int_equal(EZ_HUB_ERR_PARAM_INVALID, ez_iot_hub_del(NULL));
    uassert_int_equal(EZ_HUB_ERR_PARAM_INVALID, ez_iot_hub_ver_update(NULL, m_test_subdev2_sn));
    uassert_int_equal(EZ_HUB_ERR_PARAM_INVALID, ez_iot_hub_ver_update(m_test_subdev1_sn, NULL));
    uassert_int_equal(EZ_HUB_ERR_PARAM_INVALID, ez_iot_hub_status_update(NULL, false));
    uassert_int_equal(EZ_HUB_ERR_PARAM_INVALID, ez_iot_hub_subdev_query(NULL, &subdev_temp));
    uassert_int_equal(EZ_HUB_ERR_PARAM_INVALID, ez_iot_hub_subdev_query(m_test_subdev1_sn, NULL));
    uassert_int_equal(EZ_HUB_ERR_PARAM_INVALID, ez_iot_hub_subdev_next(NULL));

    uassert_int_equal(EZ_HUB_ERR_SUBDEV_NOT_FOUND, ez_iot_hub_del(m_test_subdev1_sn));
    uassert_int_equal(EZ_HUB_ERR_SUBDEV_NOT_FOUND, ez_iot_hub_ver_update(m_test_subdev1_sn, m_test_subdev2_sn));
    uassert_int_equal(EZ_HUB_ERR_SUBDEV_NOT_FOUND, ez_iot_hub_status_update(m_test_subdev1_sn, false));
    uassert_int_equal(EZ_HUB_ERR_SUBDEV_NOT_FOUND, ez_iot_hub_subdev_query(m_test_subdev1_sn, &subdev_temp));
    uassert_int_equal(EZ_HUB_ERR_ENUM_END, ez_iot_hub_subdev_next(&subdev_temp));

    uassert_int_equal(EZ_HUB_ERR_SUCC, ez_iot_hub_deinit());
    uassert_int_equal(EZ_HUB_ERR_SUCC, ez_iot_core_stop());
    ez_iot_core_deinit();
}

void ut_gateway_authsap()
{
    ez_byte_t devid[32] = {0};
    ez_subdev_info_t subdev_temp = {0};

    ez_iot_core_ctrl(EZ_CMD_DEVID_SET, (ez_void_t *)devid);
    ez_iot_core_ctrl(EZ_CMD_KVIMPL_SET, (ez_void_t *)&g_kv_func);

    uassert_int_equal(EZ_HUB_ERR_SUCC, ez_iot_core_init(&m_lbs_addr, &m_dev_info, ez_event_notice_func));
    uassert_int_equal(EZ_HUB_ERR_SUCC, ez_iot_hub_init(&m_hub_cbs));
    uassert_int_equal(EZ_HUB_ERR_SUCC, ez_iot_core_start());
    uassert_int_equal(EZ_HUB_ERR_SUBDEV_NOT_FOUND, ez_iot_hub_subdev_query(m_subdev1.subdev_sn, &subdev_temp));

    uassert_int_equal(EZ_HUB_ERR_SUCC, ez_iot_hub_add(&m_subdev1));
    uassert_int_equal(EZ_HUB_ERR_SUCC, wait4add_result(EZ_EVENT_SUBDEV_ADD_SUCC, m_subdev1.subdev_sn, CONFIG_EZIOT_UNIT_TEST_CASE_TIEMOUT_SECONDS * 1000));

    uassert_int_equal(EZ_HUB_ERR_SUCC, ez_iot_hub_subdev_query(m_subdev1.subdev_sn, &subdev_temp));
    uassert_int_equal(0, ezos_memcmp((void *)&m_subdev1, (void *)&subdev_temp, sizeof(subdev_temp)));

    uassert_int_equal(EZ_HUB_ERR_SUCC, ez_iot_hub_del(m_subdev1.subdev_sn));
    ezos_memset((void *)&subdev_temp, 0, sizeof(subdev_temp));
    uassert_int_equal(EZ_HUB_ERR_SUBDEV_NOT_FOUND, ez_iot_hub_subdev_query(m_subdev1.subdev_sn, &subdev_temp));

    uassert_int_equal(EZ_HUB_ERR_SUCC, ez_iot_hub_deinit());
    uassert_int_equal(EZ_HUB_ERR_SUCC, ez_iot_core_stop());
    ez_iot_core_deinit();
}

void ut_gateway_authlic()
{
    ez_byte_t devid[32] = {0};
    ez_subdev_info_t subdev_temp = {0};

    ez_iot_core_ctrl(EZ_CMD_DEVID_SET, (ez_void_t *)devid);
    ez_iot_core_ctrl(EZ_CMD_KVIMPL_SET, (ez_void_t *)&g_kv_func);

    uassert_int_equal(EZ_HUB_ERR_SUCC, ez_iot_core_init(&m_lbs_addr, &m_dev_info, ez_event_notice_func));
    uassert_int_equal(EZ_HUB_ERR_SUCC, ez_iot_hub_init(&m_hub_cbs));
    uassert_int_equal(EZ_HUB_ERR_SUCC, ez_iot_core_start());
    uassert_int_equal(EZ_HUB_ERR_SUBDEV_NOT_FOUND, ez_iot_hub_subdev_query(m_subdev2.subdev_sn, &subdev_temp));

    uassert_int_equal(EZ_HUB_ERR_SUCC, ez_iot_hub_add(&m_subdev2));
    uassert_int_equal(EZ_HUB_ERR_SUCC, wait4add_result(EZ_EVENT_SUBDEV_ADD_SUCC, m_subdev2.subdev_sn, CONFIG_EZIOT_UNIT_TEST_CASE_TIEMOUT_SECONDS * 1000));

    uassert_int_equal(EZ_HUB_ERR_SUCC, ez_iot_hub_subdev_query(m_subdev2.subdev_sn, &subdev_temp));
    uassert_int_equal(0, ezos_memcmp((void *)&m_subdev2, (void *)&subdev_temp, sizeof(subdev_temp)));

    uassert_int_equal(EZ_HUB_ERR_SUCC, ez_iot_hub_del(m_subdev2.subdev_sn));
    ezos_memset((void *)&subdev_temp, 0, sizeof(subdev_temp));
    uassert_int_equal(EZ_HUB_ERR_SUBDEV_NOT_FOUND, ez_iot_hub_subdev_query(m_subdev2.subdev_sn, &subdev_temp));

    uassert_int_equal(EZ_HUB_ERR_SUCC, ez_iot_hub_deinit());
    uassert_int_equal(EZ_HUB_ERR_SUCC, ez_iot_core_stop());
    ez_iot_core_deinit();
}

void ut_gateway_authfail()
{
    ez_byte_t devid[32] = {0};
    ez_subdev_info_t subdev_temp = {0};
    ez_subdev_info_t subdev_temp1 = {0};
    ez_subdev_info_t subdev_temp2 = {0};

    ez_iot_core_ctrl(EZ_CMD_DEVID_SET, (ez_void_t *)devid);
    ez_iot_core_ctrl(EZ_CMD_KVIMPL_SET, (ez_void_t *)&g_kv_func);
    ezos_memcpy(&subdev_temp1, &m_subdev1, sizeof(m_subdev1));
    ezos_memcpy(&subdev_temp2, &m_subdev2, sizeof(m_subdev2));

    uassert_int_equal(EZ_HUB_ERR_SUCC, ez_iot_core_init(&m_lbs_addr, &m_dev_info, ez_event_notice_func));
    uassert_int_equal(EZ_HUB_ERR_SUCC, ez_iot_hub_init(&m_hub_cbs));
    uassert_int_equal(EZ_HUB_ERR_SUCC, ez_iot_core_start());
    uassert_int_equal(EZ_HUB_ERR_SUBDEV_NOT_FOUND, ez_iot_hub_subdev_query(subdev_temp1.subdev_sn, &subdev_temp));
    uassert_int_equal(EZ_HUB_ERR_SUBDEV_NOT_FOUND, ez_iot_hub_subdev_query(subdev_temp2.subdev_sn, &subdev_temp));

    ezos_strncpy((char *)subdev_temp1.subdev_vcode, "ABCDEF", sizeof(subdev_temp1.subdev_vcode) - 1);
    ezos_strncpy((char *)subdev_temp2.subdev_vcode, "ABCDEF", sizeof(subdev_temp2.subdev_vcode) - 1);

    uassert_int_equal(EZ_HUB_ERR_SUCC, ez_iot_hub_add(&subdev_temp1));
    uassert_int_equal(EZ_HUB_ERR_SUCC, wait4add_result(EZ_EVENT_SUBDEV_ADD_FAIL, subdev_temp1.subdev_sn, CONFIG_EZIOT_UNIT_TEST_CASE_TIEMOUT_SECONDS * 1000));

    uassert_int_equal(EZ_HUB_ERR_SUCC, ez_iot_hub_add(&subdev_temp2));
    uassert_int_equal(EZ_HUB_ERR_SUCC, wait4add_result(EZ_EVENT_SUBDEV_ADD_FAIL, subdev_temp2.subdev_sn, CONFIG_EZIOT_UNIT_TEST_CASE_TIEMOUT_SECONDS * 1000));

    /* 添加失败，自动删除 */
    uassert_int_equal(EZ_HUB_ERR_SUBDEV_NOT_FOUND, ez_iot_hub_subdev_query(subdev_temp1.subdev_sn, &subdev_temp));
    uassert_int_equal(EZ_HUB_ERR_SUBDEV_NOT_FOUND, ez_iot_hub_subdev_query(subdev_temp2.subdev_sn, &subdev_temp));

    uassert_int_equal(EZ_HUB_ERR_SUCC, ez_iot_hub_deinit());
    uassert_int_equal(EZ_HUB_ERR_SUCC, ez_iot_core_stop());
    ez_iot_core_deinit();
}

void ut_gateway_dup()
{
    ez_byte_t devid[32] = {0};
    ez_subdev_info_t subdev_temp = {0};

    ez_iot_core_ctrl(EZ_CMD_DEVID_SET, (ez_void_t *)devid);
    ez_iot_core_ctrl(EZ_CMD_KVIMPL_SET, (ez_void_t *)&g_kv_func);

    uassert_int_equal(EZ_HUB_ERR_SUCC, ez_iot_core_init(&m_lbs_addr, &m_dev_info, ez_event_notice_func));
    uassert_int_equal(EZ_HUB_ERR_SUCC, ez_iot_hub_init(&m_hub_cbs));
    uassert_int_equal(EZ_HUB_ERR_SUCC, ez_iot_core_start());
    uassert_int_equal(EZ_HUB_ERR_SUBDEV_NOT_FOUND, ez_iot_hub_subdev_query(m_subdev1.subdev_sn, &subdev_temp));

    uassert_int_equal(EZ_HUB_ERR_SUCC, ez_iot_hub_add(&m_subdev1));

    uassert_int_equal(EZ_HUB_ERR_SUCC, ez_iot_hub_subdev_query(m_subdev1.subdev_sn, &subdev_temp));
    uassert_int_equal(0, ezos_memcmp((void *)&m_subdev1, (void *)&subdev_temp, sizeof(subdev_temp)));
    uassert_int_equal(EZ_HUB_ERR_SUBDEV_EXISTED, ez_iot_hub_add(&m_subdev1));

    uassert_int_equal(EZ_HUB_ERR_SUCC, ez_iot_hub_del(m_subdev1.subdev_sn));
    ezos_memset((void *)&subdev_temp, 0, sizeof(subdev_temp));
    uassert_int_equal(EZ_HUB_ERR_SUBDEV_NOT_FOUND, ez_iot_hub_subdev_query(m_subdev1.subdev_sn, &subdev_temp));

    uassert_int_equal(EZ_HUB_ERR_SUCC, ez_iot_hub_deinit());
    uassert_int_equal(EZ_HUB_ERR_SUCC, ez_iot_core_stop());
    ez_iot_core_deinit();
}

void ut_gateway_update()
{
    ez_subdev_info_t subdev_temp = {0};
    static ez_char_t *temp_subdev_ver = "V3.0.0 build 210118";
    static bool temp_subdev_sta = false;

    ez_byte_t devid[32] = {0};

    ez_iot_core_ctrl(EZ_CMD_DEVID_SET, (ez_void_t *)devid);
    ez_iot_core_ctrl(EZ_CMD_KVIMPL_SET, (ez_void_t *)&g_kv_func);

    uassert_int_equal(EZ_HUB_ERR_SUCC, ez_iot_core_init(&m_lbs_addr, &m_dev_info, ez_event_notice_func));

    uassert_int_equal(EZ_HUB_ERR_SUCC, ez_iot_hub_init(&m_hub_cbs));
    uassert_int_equal(EZ_HUB_ERR_SUCC, ez_iot_core_start());

    uassert_int_equal(EZ_HUB_ERR_SUCC, ez_iot_hub_add(&m_subdev1));
    uassert_int_equal(EZ_HUB_ERR_SUCC, ez_iot_hub_subdev_query(m_subdev1.subdev_sn, &subdev_temp));
    uassert_int_equal(0, ezos_memcmp((void *)&m_subdev1, (void *)&subdev_temp, sizeof(subdev_temp)));

    uassert_int_equal(EZ_HUB_ERR_SUCC, ez_iot_hub_ver_update(m_subdev1.subdev_sn, temp_subdev_ver));
    ezos_memset((void *)&subdev_temp, 0, sizeof(subdev_temp));
    uassert_int_equal(EZ_HUB_ERR_SUCC, ez_iot_hub_subdev_query(m_subdev1.subdev_sn, &subdev_temp));
    uassert_int_equal(0, ezos_strncmp((char *)temp_subdev_ver, (char *)subdev_temp.subdev_ver, sizeof(subdev_temp.subdev_ver) - 1));

    uassert_int_equal(EZ_HUB_ERR_SUCC, ez_iot_hub_status_update(m_subdev1.subdev_sn, temp_subdev_sta));
    ezos_memset((void *)&subdev_temp, 0, sizeof(subdev_temp));
    uassert_int_equal(EZ_HUB_ERR_SUCC, ez_iot_hub_subdev_query(m_subdev1.subdev_sn, &subdev_temp));
    uassert_false(subdev_temp.sta);

    uassert_int_equal(EZ_HUB_ERR_SUCC, ez_iot_hub_clean());
    uassert_int_equal(EZ_HUB_ERR_SUCC, ez_iot_hub_deinit());
    uassert_int_equal(EZ_HUB_ERR_SUCC, ez_iot_core_stop());
    ez_iot_core_deinit();
}

void ut_gateway_clean()
{
    ez_subdev_info_t subdev_temp = {0};

    ez_byte_t devid[32] = {0};

    ez_iot_core_ctrl(EZ_CMD_DEVID_SET, (ez_void_t *)devid);
    ez_iot_core_ctrl(EZ_CMD_KVIMPL_SET, (ez_void_t *)&g_kv_func);

    uassert_int_equal(EZ_HUB_ERR_SUCC, ez_iot_core_init(&m_lbs_addr, &m_dev_info, ez_event_notice_func));

    uassert_int_equal(EZ_HUB_ERR_SUCC, ez_iot_hub_init(&m_hub_cbs));
    uassert_int_equal(EZ_HUB_ERR_SUCC, ez_iot_core_start());

    uassert_int_equal(EZ_HUB_ERR_SUCC, ez_iot_hub_add(&m_subdev1));
    uassert_int_equal(EZ_HUB_ERR_SUCC, ez_iot_hub_add(&m_subdev2));
    uassert_int_equal(EZ_HUB_ERR_SUCC, ez_iot_hub_clean());

    uassert_int_equal(EZ_HUB_ERR_SUBDEV_NOT_FOUND, ez_iot_hub_subdev_query(m_subdev1.subdev_sn, &subdev_temp));
    uassert_int_equal(EZ_HUB_ERR_SUBDEV_NOT_FOUND, ez_iot_hub_subdev_query(m_subdev2.subdev_sn, &subdev_temp));
    uassert_int_equal(EZ_HUB_ERR_ENUM_END, ez_iot_hub_subdev_next(&subdev_temp));

    uassert_int_equal(EZ_HUB_ERR_SUCC, ez_iot_hub_deinit());
    uassert_int_equal(EZ_HUB_ERR_SUCC, ez_iot_core_stop());
    ez_iot_core_deinit();
}

void ut_gateway_enum()
{
    ez_subdev_info_t subdev_temp = {0};
    ez_byte_t devid[32] = {0};

    ez_iot_core_ctrl(EZ_CMD_DEVID_SET, (ez_void_t *)devid);
    ez_iot_core_ctrl(EZ_CMD_KVIMPL_SET, (ez_void_t *)&g_kv_func);

    ez_iot_core_init(&m_lbs_addr, &m_dev_info, ez_event_notice_func);
    uassert_int_equal(EZ_HUB_ERR_SUCC, ez_iot_hub_init(&m_hub_cbs));
    uassert_int_equal(EZ_HUB_ERR_SUCC, ez_iot_core_start());

    uassert_int_equal(EZ_HUB_ERR_SUCC, ez_iot_hub_add(&m_subdev1));
    uassert_int_equal(EZ_HUB_ERR_SUCC, ez_iot_hub_add(&m_subdev2));

    uassert_int_equal(EZ_HUB_ERR_SUCC, ez_iot_hub_subdev_next(&subdev_temp));
    uassert_int_equal(0, ezos_memcmp((void *)&m_subdev1, (void *)&subdev_temp, sizeof(subdev_temp)));

    uassert_int_equal(EZ_HUB_ERR_SUCC, ez_iot_hub_subdev_next(&subdev_temp));
    uassert_int_equal(0, ezos_memcmp((void *)&m_subdev2, (void *)&subdev_temp, sizeof(subdev_temp)));
    uassert_int_equal(EZ_HUB_ERR_ENUM_END, ez_iot_hub_subdev_next(&subdev_temp));

    uassert_int_equal(EZ_HUB_ERR_SUCC, ez_iot_hub_clean());
    uassert_int_equal(EZ_HUB_ERR_SUCC, ez_iot_hub_deinit());
    uassert_int_equal(EZ_HUB_ERR_SUCC, ez_iot_core_stop());
    ez_iot_core_deinit();
}

void ut_gateway_stress()
{
    ez_subdev_info_t subdev_temp = {0};
    const int subdev_max = 64;
    int i = 0;

    ez_byte_t devid[32] = {0};

    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_ctrl(EZ_CMD_DEVID_SET, (ez_void_t *)devid));
    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_ctrl(EZ_CMD_KVIMPL_SET, (ez_void_t *)&g_kv_func));
    uassert_int_equal(EZ_HUB_ERR_SUCC, ez_iot_core_init(&m_lbs_addr, &m_dev_info, ez_event_notice_func));
    uassert_int_equal(EZ_HUB_ERR_SUCC, ez_iot_hub_init(&m_hub_cbs));
    uassert_int_equal(EZ_HUB_ERR_SUCC, ez_iot_hub_clean());

    for (i = 0; i < subdev_max; i++)
    {
        snprintf((char *)subdev_temp.subdev_sn, sizeof(subdev_temp.subdev_sn), "E111111%02d", i);
        uassert_int_equal(EZ_HUB_ERR_SUCC, ez_iot_hub_add(&subdev_temp));
    }

    uassert_int_equal(EZ_HUB_ERR_OUT_OF_RANGE, ez_iot_hub_add(&m_subdev1));
    uassert_int_equal(EZ_HUB_ERR_SUCC, ez_iot_hub_clean());
    uassert_int_equal(EZ_HUB_ERR_SUCC, ez_iot_hub_deinit());
    ez_iot_core_deinit();
}

static ez_int32_t wait4add_result(ez_subdev_event_e event_type, ez_char_t *sn, ez_int32_t time_ms)
{
    int ret = -1;
    int index = 0;
    m_add_result = -1;
    ezos_memset(m_add_result_sn, 0, sizeof(m_add_result_sn));

    do
    {
        if (event_type == m_add_result &&
            0 == ezos_strcmp(m_add_result_sn, sn))
        {
            ret = 0;
            break;
        }

        ezos_delay_ms(10);
        index += 10;
    } while (index < time_ms);

    return ret;
}

static ez_int32_t ez_event_notice_func(ez_event_e event_type, ez_void_t *data, ez_int32_t len)
{
    switch (event_type)
    {
    case EZ_EVENT_ONLINE:
        m_event_id = EZ_EVENT_ONLINE;
        break;
    case EZ_EVENT_OFFLINE:
        /* save devid */
        break;

    default:
        break;
    }

    return 0;
}

static ez_int32_t hub_recv_event_cb(ez_subdev_event_e event_type, ez_void_t *data, ez_int32_t len)
{
    ez_subdev_info_t *sub_dev_info = NULL;

    switch (event_type)
    {
    case EZ_EVENT_SUBDEV_ADD_SUCC:
        ezos_strncpy(m_add_result_sn, (char *)data, sizeof(m_add_result_sn) - 1);
        break;
    case EZ_EVENT_SUBDEV_ADD_FAIL:
        sub_dev_info = (ez_subdev_info_t *)data;
        ezos_strncpy(m_add_result_sn, (char *)sub_dev_info->subdev_sn, sizeof(m_add_result_sn) - 1);
        break;

    default:
        break;
    }

    m_add_result = event_type;

    return 0;
}

static long global_init()
{
    ezlog_init();
    ezlog_start();
    ezlog_filter_lvl(CONFIG_EZIOT_UNIT_TEST_SDK_LOGLVL);

    ezos_strncpy(m_dev_info.dev_typedisplay, CONFIG_EZIOT_UNIT_TEST_DEV_DISPLAY_NAME, sizeof(m_dev_info.dev_typedisplay) - 1);
    ezos_strncpy(m_dev_info.dev_firmwareversion, CONFIG_EZIOT_UNIT_TEST_DEV_FIRMWARE_VERSION, sizeof(m_dev_info.dev_firmwareversion) - 1);

#if defined(CONFIG_EZIOT_UNIT_TEST_DEV_AUTH_MODE_SAP)
    m_dev_info.auth_mode = 0;
    ezos_strncpy(m_dev_info.dev_type, CONFIG_EZIOT_UNIT_TEST_DEV_TYPE, sizeof(m_dev_info.dev_type) - 1);
    ezos_strncpy(m_dev_info.dev_subserial, CONFIG_EZIOT_UNIT_TEST_DEV_SERIAL_NUMBER, sizeof(m_dev_info.dev_subserial) - 1);
    ezos_strncpy(m_dev_info.dev_verification_code, CONFIG_EZIOT_UNIT_TEST_DEV_VERIFICATION_CODE, sizeof(m_dev_info.dev_verification_code) - 1);
#elif defined(CONFIG_EZIOT_UNIT_TEST_DEV_AUTH_MODE_LICENCE)
    m_dev_info.auth_mode = 1;
    ezos_strncpy(m_dev_info.dev_type, CONFIG_EZIOT_UNIT_TEST_DEV_PRODUCT_KEY, sizeof(m_dev_info.dev_type) - 1);
    ezos_snprintf(m_dev_info.dev_subserial, sizeof(m_dev_info.dev_subserial) - 1, "%s:%s", CONFIG_EZIOT_UNIT_TEST_DEV_PRODUCT_KEY, CONFIG_EZIOT_UNIT_TEST_DEV_NAME);
    ezos_strncpy(m_dev_info.dev_verification_code, CONFIG_EZIOT_UNIT_TEST_DEV_LICENSE, sizeof(m_dev_info.dev_verification_code) - 1);
#endif

    m_subdev1.auth_mode = m_test_subdev1_authm;
    ezos_strncpy((char *)m_subdev1.subdev_sn, (char *)m_test_subdev1_sn, sizeof(m_subdev1.subdev_sn) - 1);
    ezos_strncpy((char *)m_subdev1.subdev_ver, (char *)m_test_subdev1_ver, sizeof(m_subdev1.subdev_ver) - 1);
    ezos_strncpy((char *)m_subdev1.subdev_type, (char *)m_test_subdev1_type, sizeof(m_subdev1.subdev_type) - 1);
    ezos_strncpy((char *)m_subdev1.subdev_vcode, (char *)m_test_subdev1_vcode, sizeof(m_subdev1.subdev_vcode) - 1);
    ezos_strncpy((char *)m_subdev1.subdev_uuid, (char *)m_test_subdev1_uuid, sizeof(m_subdev1.subdev_uuid) - 1);
    m_subdev1.sta = 1;

    m_subdev2.auth_mode = m_test_subdev2_authm;
    ezos_strncpy((char *)m_subdev2.subdev_sn, (char *)m_test_subdev2_sn, sizeof(m_subdev2.subdev_sn) - 1);
    ezos_strncpy((char *)m_subdev2.subdev_ver, (char *)m_test_subdev2_ver, sizeof(m_subdev2.subdev_ver) - 1);
    ezos_strncpy((char *)m_subdev2.subdev_type, (char *)m_test_subdev2_type, sizeof(m_subdev2.subdev_type) - 1);
    ezos_strncpy((char *)m_subdev2.subdev_vcode, (char *)m_test_subdev2_vcode, sizeof(m_subdev2.subdev_vcode) - 1);
    ezos_strncpy((char *)m_subdev2.subdev_uuid, (char *)m_test_subdev2_uuid, sizeof(m_subdev2.subdev_uuid) - 1);
    m_subdev2.sta = 0;

    return 0;
}