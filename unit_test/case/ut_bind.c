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
 * XuRongjun (xurongjun@ezvizlife.com) ez iot base unit-test case
 *******************************************************************************/

#include <stdlib.h>
#include <ezos.h>
#include <utest.h>
#include <ezlog.h>
#include <kv_imp.h>

#include "ez_iot_core_def.h"
#include "ez_iot_core.h"
#include "ez_iot_base.h"

void ut_base_bind_by_near();
void ut_base_bind_by_challenge();
void ut_base_bind_status_query();

static long global_init();

UTEST_TC_EXPORT(ut_base_bind_by_near, global_init, NULL, CONFIG_EZIOT_UNIT_TEST_CASE_TIEMOUT_SECONDS);
UTEST_TC_EXPORT(ut_base_bind_by_challenge, global_init, NULL, CONFIG_EZIOT_UNIT_TEST_CASE_TIEMOUT_SECONDS);
UTEST_TC_EXPORT(ut_base_bind_status_query, global_init, NULL, CONFIG_EZIOT_UNIT_TEST_CASE_TIEMOUT_SECONDS);

static ez_int32_t ez_event_notice_func(ez_event_e event_type, ez_void_t *data, ez_int32_t len);
static ez_int32_t ez_base_notice_func(ez_base_event_e event_type, ez_void_t *data, ez_int32_t len);

static int dev_event_waitfor(int event_id, int time_ms);
static int dev_contact_bind_waitfor(int32_t *challenge_code, int time_ms);
static int dev_bind_stauts(int time_ms);

static int m_event_id = -1;
static int m_base_event_id = -1;
static int m_challenge_code = -1;

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

void ut_base_bind_by_near()
{
    ez_byte_t devid[32] = {0};

    ///< 通过ap配网或者蓝牙配网从app获取token
    ez_char_t *dev_token = "68b8efe8246f461691971c95eb8ba725";
 
    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_ctrl(EZ_CMD_DEVID_SET, (ez_void_t *)devid));
    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_ctrl(EZ_CMD_KVIMPL_SET, (ez_void_t *)&g_kv_func));
    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_init(&m_lbs_addr, &m_dev_info, ez_event_notice_func));
    uassert_int_equal(EZ_BASE_ERR_SUCC, ez_iot_base_init(ez_base_notice_func));

    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_start());
    uassert_int_equal(EZ_BASE_ERR_SUCC, ez_iot_base_bind_near(dev_token));

    uassert_int_equal(0, dev_event_waitfor(EZ_EVENT_ONLINE, CONFIG_EZIOT_UNIT_TEST_CASE_TIEMOUT_SECONDS * 1000));

    ezos_delay_ms(3000);
    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_stop());
    ez_iot_base_deinit();
    ez_iot_core_deinit();
}

void ut_base_bind_by_challenge()
{
    ez_byte_t devid[32] = {0};
    int32_t challenge_code = 0;

    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_ctrl(EZ_CMD_DEVID_SET, (ez_void_t *)devid));
    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_ctrl(EZ_CMD_KVIMPL_SET, (ez_void_t *)&g_kv_func));
    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_init(&m_lbs_addr, &m_dev_info, ez_event_notice_func));
    uassert_int_equal(EZ_BASE_ERR_SUCC, ez_iot_base_init(ez_base_notice_func));

    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_start());

    uassert_int_equal(0, dev_event_waitfor(EZ_EVENT_ONLINE, CONFIG_EZIOT_UNIT_TEST_CASE_TIEMOUT_SECONDS * 1000));
    uassert_int_equal(0, dev_contact_bind_waitfor(&challenge_code, CONFIG_EZIOT_UNIT_TEST_CASE_TIEMOUT_SECONDS * 1000 * 3));

    uassert_int_equal(EZ_BASE_ERR_SUCC, ez_iot_base_bind_response(challenge_code));

    ezos_delay_ms(3000);
    uassert_int_equal(EZ_BASE_ERR_SUCC, ez_iot_core_stop());
    ez_iot_base_deinit();
    ez_iot_core_deinit();
}

void ut_base_bind_status_query()
{
    ez_byte_t devid[32] = {0};

    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_ctrl(EZ_CMD_DEVID_SET, (ez_void_t *)devid));
    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_ctrl(EZ_CMD_KVIMPL_SET, (ez_void_t *)&g_kv_func));
    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_init(&m_lbs_addr, &m_dev_info, ez_event_notice_func));
    uassert_int_equal(EZ_BASE_ERR_SUCC, ez_iot_base_init(ez_base_notice_func));

    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_start());
    uassert_int_equal(EZ_BASE_ERR_SUCC, ez_iot_base_bind_query());

    uassert_int_equal(0, dev_bind_stauts(CONFIG_EZIOT_UNIT_TEST_CASE_TIEMOUT_SECONDS * 1000));

    ezos_delay_ms(3000);
    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_stop());
    ez_iot_base_deinit();
    ez_iot_core_deinit();
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

static int dev_contact_bind_waitfor(int32_t *challenge_code, int time_ms)
{
    int ret = -1;
    int index = 0;
    m_challenge_code = -1;

    do
    {
        if (-1 != m_challenge_code)
        {
            *challenge_code = m_challenge_code;
            ret = 0;
            break;
        }

        ezos_delay_ms(10);
        index += 10;
    } while (index < time_ms);

    return ret;
}

static int dev_bind_stauts(int time_ms)
{
    int ret = -1;
    int index = 0;
    m_base_event_id = -1;

    do
    {
        if (EZ_EVENT_BINDING == m_base_event_id ||
            EZ_EVENT_UNBINDING == m_base_event_id)
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

static ez_int32_t ez_base_notice_func(ez_base_event_e event_type, ez_void_t *data, ez_int32_t len)
{
    ez_bind_info_t *bind_info;
    ez_bind_challenge_t *bind_chanllenge;

    switch (event_type)
    {
    case EZ_EVENT_BINDING:
    {
        bind_info = (ez_bind_info_t *)data;
        if (0 != ezos_strlen(bind_info->user_id))
        {
            m_base_event_id = EZ_EVENT_ONLINE;
        }
    }
    break;
    case EZ_EVENT_UNBINDING:
    {
        m_base_event_id = EZ_EVENT_UNBINDING;
    }
    break;
    case EZ_EVENT_BINDING_CHALLENGE:
    {
        bind_chanllenge = (ez_bind_challenge_t *)data;
        m_challenge_code = bind_chanllenge->challenge_code;
    }
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
    ezlog_filter_lvl(4);

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
