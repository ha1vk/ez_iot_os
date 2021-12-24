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
 * XuRongjun (xurongjun@ezvizlife.com) ez iot core unit-test case
 *******************************************************************************/

#include <stdlib.h>
#include <ezos.h>
#include <utest.h>
#include <ezlog.h>
#include <kv_imp.h>

#include "ez_iot_core_def.h"
#include "ez_iot_core.h"

/**
 * @brief Test interface compatibility and error codes
 * 
 */
void ut_online_errcode();
void ut_online_access();
void ut_online_restart();

static long global_init();
static ez_int32_t ez_event_notice_func(ez_event_e event_type, ez_void_t *data, ez_int32_t len);
static int dev_event_waitfor(int event_id, int time_ms);

static int m_event_id = -1;
static int m_last_err = 0;
static ez_server_info_t m_lbs_addr = {CONFIG_EZIOT_UNIT_TEST_CLOUD_HOST, CONFIG_EZIOT_UNIT_TEST_CLOUD_PORT};
static ez_dev_info_t m_dev_info = {0};

static ez_kv_func_t g_kv_func = {
    .ezos_kv_init = kv_init,
    .ezos_kv_raw_set = kv_raw_set,
    .ezos_kv_raw_get = kv_raw_get,
    .ezos_kv_del = kv_del,
    .ezos_kv_del_by_prefix = kv_del_by_prefix,
    .ezos_kv_print = kv_print,
    .ezos_kv_deinit = kv_deinit,
};

static void testcase(void)
{
    UTEST_UNIT_RUN(ut_online_errcode);
    UTEST_UNIT_RUN(ut_online_access);
    UTEST_UNIT_RUN(ut_online_restart);
}
UTEST_TC_EXPORT(testcase, "eziot.ut_online", global_init, NULL, CONFIG_EZIOT_UNIT_TEST_CASE_TIEMOUT_SECONDS);

void ut_online_errcode()
{
    ez_byte_t devid[32] = {0};

    uassert_int_equal(EZ_CORE_ERR_NOT_INIT, ez_iot_core_start());
    uassert_int_equal(EZ_CORE_ERR_NOT_INIT, ez_iot_core_stop());

    uassert_int_equal(EZ_CORE_ERR_PARAM_INVALID, ez_iot_core_init(NULL, NULL, NULL));
    uassert_int_equal(EZ_CORE_ERR_PARAM_INVALID, ez_iot_core_init(&m_lbs_addr, NULL, NULL));
    uassert_int_equal(EZ_CORE_ERR_PARAM_INVALID, ez_iot_core_init(&m_lbs_addr, &m_dev_info, NULL));

    uassert_int_equal(EZ_CORE_ERR_DEVID, ez_iot_core_init(&m_lbs_addr, &m_dev_info, ez_event_notice_func));
    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_ctrl(EZ_CMD_DEVID_SET, (ez_void_t *)devid));
    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_ctrl(EZ_CMD_KVIMPL_SET, (ez_void_t *)&g_kv_func));

    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_init(&m_lbs_addr, &m_dev_info, ez_event_notice_func));

    uassert_int_equal(EZ_CORE_ERR_NOT_READY, ez_iot_core_stop());
    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_start());
    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_stop());

    ez_iot_core_deinit();
}

void ut_online_access()
{
    ez_byte_t devid[32] = {0};

    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_ctrl(EZ_CMD_DEVID_SET, (ez_void_t *)devid));
    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_ctrl(EZ_CMD_KVIMPL_SET, (ez_void_t *)&g_kv_func));

    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_init(&m_lbs_addr, &m_dev_info, ez_event_notice_func));
    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_start());
    uassert_int_equal(EZ_CORE_ERR_SUCC, dev_event_waitfor(EZ_EVENT_ONLINE, CONFIG_EZIOT_UNIT_TEST_CASE_TIEMOUT_SECONDS * 1000));
    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_stop());

    ez_iot_core_deinit();
}

void ut_online_restart()
{
    ez_byte_t devid[32] = {0};

    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_ctrl(EZ_CMD_DEVID_SET, (ez_void_t *)devid));
    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_ctrl(EZ_CMD_KVIMPL_SET, (ez_void_t *)&g_kv_func));

    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_init(&m_lbs_addr, &m_dev_info, ez_event_notice_func));
    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_start());
    uassert_int_equal(EZ_CORE_ERR_SUCC, dev_event_waitfor(EZ_EVENT_ONLINE, CONFIG_EZIOT_UNIT_TEST_CASE_TIEMOUT_SECONDS * 1000));
    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_stop());

    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_start());
    uassert_int_equal(EZ_CORE_ERR_SUCC, dev_event_waitfor(EZ_EVENT_ONLINE, CONFIG_EZIOT_UNIT_TEST_CASE_TIEMOUT_SECONDS * 1000));
    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_stop());

    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_start());
    uassert_int_equal(EZ_CORE_ERR_SUCC, dev_event_waitfor(EZ_EVENT_ONLINE, CONFIG_EZIOT_UNIT_TEST_CASE_TIEMOUT_SECONDS * 1000));
    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_stop());

    ez_iot_core_deinit();
}

static int dev_event_waitfor(int event_id, int time_ms)
{
    int ret = -1;
    int index = 0;
    m_event_id = -1;
    m_last_err = 0;

    do
    {
        if (event_id == m_event_id)
        {
            m_last_err = 0;
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

    return 0;
}