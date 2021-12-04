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

void ut_gateway_errcode();
void ut_gateway_rw();
void ut_gateway_update();
void ut_gateway_clean();
void ut_gateway_enum();
void ut_gateway_full();

static long global_init();

UTEST_TC_EXPORT(ut_gateway_errcode, global_init, NULL, CONFIG_EZIOT_UNIT_TEST_CASE_TIEMOUT_SECONDS);
UTEST_TC_EXPORT(ut_gateway_rw, global_init, NULL, CONFIG_EZIOT_UNIT_TEST_CASE_TIEMOUT_SECONDS);
UTEST_TC_EXPORT(ut_gateway_update, global_init, NULL, CONFIG_EZIOT_UNIT_TEST_CASE_TIEMOUT_SECONDS);
UTEST_TC_EXPORT(ut_gateway_enum, global_init, NULL, CONFIG_EZIOT_UNIT_TEST_CASE_TIEMOUT_SECONDS);
UTEST_TC_EXPORT(ut_gateway_clean, global_init, NULL, CONFIG_EZIOT_UNIT_TEST_CASE_TIEMOUT_SECONDS);
UTEST_TC_EXPORT(ut_gateway_full, global_init, NULL, CONFIG_EZIOT_UNIT_TEST_CASE_TIEMOUT_SECONDS);

static int m_event_id = -1;
static int m_base_event_id = -1;
static int m_challenge_code = -1;
static ez_char_t m_add_result_sn[32] = {0};

static ez_int32_t ez_event_notice_func(ez_event_e event_type, ez_void_t *data, ez_int32_t len);
static int32_t hub_recv_event_cb(ez_subdev_event_e event_type, void *data, int len);

static int dev_event_waitfor(int event_id, int time_ms);
static int wait4add_result(ez_subdev_event_e event_type, ez_char_t *sn, int time_ms);

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

static ez_char_t *m_test_subdev_sn1 = "E11111111";
static ez_char_t *m_test_subdev_ver1 = "V1.0.0 build 210118";
static ez_char_t *m_test_subdev_type1 = "TYPE_TEST1";
static ez_char_t *m_test_subdev_sn2 = "E22222222";
static ez_char_t *m_test_subdev_ver2 = "V2.0.0 build 210118";
static ez_char_t *m_test_subdev_type2 = "TYPE_TEST2";
static ez_subdev_info_t m_subdev1 = {0};
static ez_subdev_info_t m_subdev2 = {0};

void ut_gateway_errcode()
{
    ez_byte_t devid[32] = {0};
    ez_subdev_info_t subdev_temp = {0};

    //sdk core not init
    uassert_int_equal(EZ_HUB_ERR_NOT_INIT, ez_iot_hub_add(&m_subdev1));
    uassert_int_equal(EZ_HUB_ERR_NOT_INIT, ez_iot_hub_del(m_test_subdev_sn1));
    uassert_int_equal(EZ_HUB_ERR_NOT_INIT, ez_iot_hub_ver_update(m_test_subdev_sn1, m_test_subdev_ver2));
    uassert_int_equal(EZ_HUB_ERR_NOT_INIT, ez_iot_hub_status_update(m_test_subdev_sn1, false));
    uassert_int_equal(EZ_HUB_ERR_NOT_INIT, ez_iot_hub_subdev_query(m_test_subdev_sn1, &subdev_temp));
    uassert_int_equal(EZ_HUB_ERR_NOT_INIT, ez_iot_hub_subdev_next(&subdev_temp));
    uassert_int_equal(EZ_HUB_ERR_NOT_INIT, ez_iot_hub_clean());
    uassert_int_equal(EZ_HUB_ERR_NOT_INIT, ez_iot_hub_deinit());

    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_ctrl(EZ_CMD_DEVID_SET, (ez_void_t *)devid));
    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_ctrl(EZ_CMD_KVIMPL_SET, (ez_void_t *)&g_kv_func));
    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_init(&m_lbs_addr, &m_dev_info, ez_event_notice_func));

    uassert_int_equal(EZ_HUB_ERR_SUCC, ez_iot_hub_init(NULL));
    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_start());

    uassert_int_equal(EZ_HUB_ERR_PARAM_INVALID, ez_iot_hub_add(NULL));
    uassert_int_equal(EZ_HUB_ERR_PARAM_INVALID, ez_iot_hub_del(NULL));
    uassert_int_equal(EZ_HUB_ERR_PARAM_INVALID, ez_iot_hub_ver_update(NULL, m_test_subdev_ver2));
    uassert_int_equal(EZ_HUB_ERR_PARAM_INVALID, ez_iot_hub_ver_update(m_test_subdev_sn1, NULL));
    uassert_int_equal(EZ_HUB_ERR_PARAM_INVALID, ez_iot_hub_status_update(NULL, false));
    uassert_int_equal(EZ_HUB_ERR_PARAM_INVALID, ez_iot_hub_subdev_query(NULL, &subdev_temp));
    uassert_int_equal(EZ_HUB_ERR_PARAM_INVALID, ez_iot_hub_subdev_query(m_test_subdev_sn1, NULL));
    uassert_int_equal(EZ_HUB_ERR_PARAM_INVALID, ez_iot_hub_subdev_next(NULL));

    uassert_int_equal(EZ_HUB_ERR_SUBDEV_NOT_FOUND, ez_iot_hub_del(m_test_subdev_sn1));
    uassert_int_equal(EZ_HUB_ERR_SUBDEV_NOT_FOUND, ez_iot_hub_ver_update(m_test_subdev_sn1, m_test_subdev_ver2));
    uassert_int_equal(EZ_HUB_ERR_SUBDEV_NOT_FOUND, ez_iot_hub_status_update(m_test_subdev_sn1, false));
    uassert_int_equal(EZ_HUB_ERR_SUBDEV_NOT_FOUND, ez_iot_hub_subdev_query(m_test_subdev_sn1, &subdev_temp));
    uassert_int_equal(EZ_HUB_ERR_ENUM_END, ez_iot_hub_subdev_next(&subdev_temp));

    uassert_int_equal(ez_errno_succ, ez_iot_hub_deinit());
    uassert_int_equal(ez_errno_succ, ez_iot_stop());
    ez_iot_fini();
}

void ut_gateway_rw()
{
    ez_subdev_info_t subdev_temp = {0};
    uassert_int_equal(ez_errno_succ, ez_iot_init(&m_lbs_addr, &m_dev_info, &m_cbs, &m_kv_cbs));
    uassert_int_equal(ez_errno_succ, ez_iot_hub_init(&m_hub_cbs));
    uassert_int_equal(ez_errno_succ, ez_iot_start());
    uassert_int_equal(EZ_HUB_ERR_SUBDEV_NOT_FOUND, ez_iot_hub_subdev_query(m_subdev1.subdev_sn, &subdev_temp));

    uassert_int_equal(ez_errno_succ, ez_iot_hub_add(&m_subdev1));
    uassert_int_equal(ez_errno_succ, ez_iot_hub_subdev_query(m_subdev1.subdev_sn, &subdev_temp));
    uassert_int_equal(0, memcmp((void *)&m_subdev1, (void *)&subdev_temp, sizeof(subdev_temp)));
    uassert_int_equal(ez_errno_hub_subdev_existed, ez_iot_hub_add(&m_subdev1));

    uassert_int_equal(ez_errno_succ, ez_iot_hub_del(m_subdev1.subdev_sn));
    memset((void *)&subdev_temp, 0, sizeof(subdev_temp));
    uassert_int_equal(EZ_HUB_ERR_SUBDEV_NOT_FOUND, ez_iot_hub_subdev_query(m_subdev1.subdev_sn, &subdev_temp));

    uassert_int_equal(ez_errno_succ, ez_iot_hub_deinit());
    uassert_int_equal(ez_errno_succ, ez_iot_stop());
    ez_iot_fini();
}

void ut_gateway_update()
{
    ez_subdev_info_t subdev_temp = {0};
    static ez_char_t *temp_subdev_ver = "V3.0.0 build 210118";
    static bool temp_subdev_sta = false;

    uassert_int_equal(ez_errno_succ, ez_iot_init(&m_lbs_addr, &m_dev_info, &m_cbs, &m_kv_cbs));
    uassert_int_equal(ez_errno_succ, ez_iot_hub_init(&m_hub_cbs));
    uassert_int_equal(ez_errno_succ, ez_iot_start());

    uassert_int_equal(ez_errno_succ, ez_iot_hub_add(&m_subdev1));
    uassert_int_equal(ez_errno_succ, ez_iot_hub_subdev_query(m_subdev1.subdev_sn, &subdev_temp));
    uassert_int_equal(0, memcmp((void *)&m_subdev1, (void *)&subdev_temp, sizeof(subdev_temp)));

    uassert_int_equal(ez_errno_succ, ez_iot_hub_ver_update(m_subdev1.subdev_sn, temp_subdev_ver));
    memset((void *)&subdev_temp, 0, sizeof(subdev_temp));
    uassert_int_equal(ez_errno_succ, ez_iot_hub_subdev_query(m_subdev1.subdev_sn, &subdev_temp));
    uassert_int_equal(0, strncmp((char *)temp_subdev_ver, (char *)subdev_temp.subdev_ver, sizeof(subdev_temp.subdev_ver) - 1));

    uassert_int_equal(ez_errno_succ, ez_iot_hub_status_update(m_subdev1.subdev_sn, temp_subdev_sta));
    memset((void *)&subdev_temp, 0, sizeof(subdev_temp));
    uassert_int_equal(ez_errno_succ, ez_iot_hub_subdev_query(m_subdev1.subdev_sn, &subdev_temp));
    uassert_false(subdev_temp.sta);

    uassert_int_equal(ez_errno_succ, ez_iot_hub_clean());
    uassert_int_equal(ez_errno_succ, ez_iot_hub_deinit());
    uassert_int_equal(ez_errno_succ, ez_iot_stop());
    ez_iot_fini();
}

void ut_gateway_clean()
{
    ez_subdev_info_t subdev_temp = {0};

    uassert_int_equal(ez_errno_succ, ez_iot_init(&m_lbs_addr, &m_dev_info, &m_cbs, &m_kv_cbs));
    uassert_int_equal(ez_errno_succ, ez_iot_hub_init(&m_hub_cbs));
    uassert_int_equal(ez_errno_succ, ez_iot_start());

    uassert_int_equal(ez_errno_succ, ez_iot_hub_add(&m_subdev1));
    uassert_int_equal(ez_errno_succ, ez_iot_hub_add(&m_subdev2));
    uassert_int_equal(ez_errno_succ, ez_iot_hub_clean());

    uassert_int_equal(EZ_HUB_ERR_SUBDEV_NOT_FOUND, ez_iot_hub_subdev_query(m_subdev1.subdev_sn, &subdev_temp));
    uassert_int_equal(EZ_HUB_ERR_SUBDEV_NOT_FOUND, ez_iot_hub_subdev_query(m_subdev1.subdev_sn, &subdev_temp));
    uassert_int_equal(EZ_HUB_ERR_ENUM_END, ez_iot_hub_subdev_next(&subdev_temp));

    uassert_int_equal(ez_errno_succ, ez_iot_hub_deinit());
    uassert_int_equal(ez_errno_succ, ez_iot_stop());
    ez_iot_fini();
}

void ut_gateway_enum()
{
    ez_subdev_info_t subdev_temp = {0};
    const int subdev_max = 64;
    int subdev_count = 0;

    uassert_int_equal(ez_errno_succ, ez_iot_init(&m_lbs_addr, &m_dev_info, &m_cbs, &m_kv_cbs));
    uassert_int_equal(ez_errno_succ, ez_iot_hub_init(&m_hub_cbs));
    uassert_int_equal(ez_errno_succ, ez_iot_start());

    uassert_int_equal(ez_errno_succ, ez_iot_hub_add(&m_subdev1));
    uassert_int_equal(ez_errno_succ, ez_iot_hub_add(&m_subdev2));

    uassert_int_equal(ez_errno_succ, ez_iot_hub_subdev_next(&subdev_temp));
    uassert_int_equal(0, memcmp((void *)&m_subdev1, (void *)&subdev_temp, sizeof(subdev_temp)));

    uassert_int_equal(ez_errno_succ, ez_iot_hub_subdev_next(&subdev_temp));
    uassert_int_equal(0, memcmp((void *)&m_subdev2, (void *)&subdev_temp, sizeof(subdev_temp)));
    uassert_int_equal(EZ_HUB_ERR_ENUM_END, ez_iot_hub_subdev_next(&subdev_temp));

    uassert_int_equal(ez_errno_succ, ez_iot_hub_clean());
    uassert_int_equal(ez_errno_succ, ez_iot_hub_deinit());
    uassert_int_equal(ez_errno_succ, ez_iot_stop());
    ez_iot_fini();
}

void ut_gateway_full()
{
    ez_subdev_info_t subdev_temp = {0};
    const int subdev_max = 64;
    int i = 0;

    uassert_int_equal(ez_errno_succ, ez_iot_init(&m_lbs_addr, &m_dev_info, &m_cbs, &m_kv_cbs));
    uassert_int_equal(ez_errno_succ, ez_iot_hub_init(&m_hub_cbs));
    uassert_int_equal(ez_errno_succ, ez_iot_hub_clean());
    uassert_int_equal(ez_errno_succ, ez_iot_start());
    memcpy((void *)&subdev_temp, (void *)&m_subdev1, sizeof(subdev_temp));

    for (i = 0; i < subdev_max; i++)
    {
        snprintf((char *)subdev_temp.subdev_sn, sizeof(subdev_temp.subdev_sn), "E11111111_%d", i);
        uassert_int_equal(ez_errno_succ, ez_iot_hub_add(&subdev_temp));
    }

    uassert_int_equal(EZ_HUB_ERR_OUT_OF_RANGE, ez_iot_hub_add(&m_subdev1));
    uassert_int_equal(ez_errno_succ, ez_iot_hub_clean());
    uassert_int_equal(ez_errno_succ, ez_iot_hub_deinit());
    uassert_int_equal(ez_errno_succ, ez_iot_stop());
    ez_iot_fini();
}

static int dev_event_waitfor(int event_id, int time_ms)
{
    int ret = -1;
    int index = 0;
    m_event_id = -1;

    do
    {
        if (event_id == m_event_id)
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

static int32_t hub_recv_event_cb(hub_event_t event_type, void *data, int len)
{
    hub_subdev_info_t *sub_dev_info = NULL;

    switch (event_type)
    {
    case EZ_EVENT_SUBDEV_ADD_SUCC:
        strncpy(m_add_result_sn, (char *)data, sizeof(m_add_result_sn) - 1);
        break;
    case EZ_EVENT_SUBDEV_ADD_FAIL:
        sub_dev_info = (hub_subdev_info_t *)data;
        strncpy(m_add_result_sn, (char *)sub_dev_info->subdev_sn, sizeof(m_add_result_sn) - 1);
        break;

    default:
        break;
    }

    m_add_result = event_type;

    return 0;
}

static int wait4add_result(hub_event_t event_type, ez_char_t *sn, int time_ms)
{
    int ret = -1;
    int index = 0;
    m_add_result = -1;
    memset(m_add_result_sn, 0, sizeof(m_add_result_sn));

    do
    {
        if (event_type == m_add_result &&
            0 == strcmp(m_add_result_sn, sn))
        {
            ret = 0;
            break;
        }

        hal_thread_sleep(10);
        index += 10;
    } while (index < time_ms);

    return ret;
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

    m_subdev1.sta = true;
    strncpy((char *)m_subdev1.subdev_sn, (char *)m_test_subdev_sn1, sizeof(m_subdev1.subdev_sn) - 1);
    strncpy((char *)m_subdev1.subdev_ver, (char *)m_test_subdev_ver1, sizeof(m_subdev1.subdev_ver) - 1);
    strncpy((char *)m_subdev1.subdev_type, (char *)m_test_subdev_type1, sizeof(m_subdev1.subdev_type) - 1);

    m_subdev2.sta = true;
    strncpy((char *)m_subdev2.subdev_sn, (char *)m_test_subdev_sn2, sizeof(m_subdev2.subdev_sn) - 1);
    strncpy((char *)m_subdev2.subdev_ver, (char *)m_test_subdev_ver2, sizeof(m_subdev2.subdev_ver) - 1);
    strncpy((char *)m_subdev2.subdev_type, (char *)m_test_subdev_type2, sizeof(m_subdev2.subdev_type) - 1);

    return 0;
}