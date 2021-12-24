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
 * XuRongjun (xurongjun@ezvizlife.com) ez iot tsl unit-test case
 *******************************************************************************/

#include <stdlib.h>
#include <ezos.h>
#include <utest.h>
#include <ezlog.h>
#include <kv_imp.h>

#include "ez_iot_core_def.h"
#include "ez_iot_core.h"
#include "ez_iot_base.h"
#include "ez_iot_tsl.h"

static long global_init();

void ut_tsl_errcode();
void ut_tsl_prop_report_bool();
void ut_tsl_prop_report_int();
void ut_tsl_prop_report_float();
void ut_tsl_prop_report_str();
void ut_tsl_prop_report_jobj();
void ut_tsl_prop_report_jarr();
void ut_tsl_event_report_null();
void ut_tsl_event_report_obj();

void ut_tsl_profile_prop_report_bool();
void ut_tsl_profile_prop_report_int();
void ut_tsl_profile_prop_report_float();
void ut_tsl_profile_prop_report_str();
void ut_tsl_profile_prop_report_jobj();
void ut_tsl_profile_prop_report_jarr();
void ut_tsl_profile_event_report_null();
void ut_tsl_profile_event_report_obj();

static ez_int32_t ez_event_notice_func(ez_event_e event_type, ez_void_t *data, ez_int32_t len);
static ez_int32_t ez_base_notice_func(ez_base_event_e event_type, ez_void_t *data, ez_int32_t len);
static int dev_event_waitfor(int event_id, int time_ms);
static ez_int32_t tsl_notice(ez_tsl_event_e event_type, ez_void_t *data, ez_int32_t len);
static ez_int32_t tsl_action2dev(const ez_char_t *sn, const ez_tsl_rsc_t *rsc_info, const ez_tsl_key_t *key_info, const ez_tsl_value_t *value_in, ez_tsl_value_t *value_out);
static ez_int32_t tsl_property2cloud(const ez_char_t *sn, const ez_tsl_rsc_t *rsc_info, const ez_tsl_key_t *key_info, ez_tsl_value_t *value_out);
static ez_int32_t tsl_property2dev(const ez_char_t *sn, const ez_tsl_rsc_t *rsc_info, const ez_tsl_key_t *key_info, const ez_tsl_value_t *value);

static int m_event_id = -1;
static ez_server_info_t m_lbs_addr = {CONFIG_EZIOT_UNIT_TEST_CLOUD_HOST, CONFIG_EZIOT_UNIT_TEST_CLOUD_PORT};
static ez_dev_info_t m_dev_info = {0};
static ez_char_t *g_profile = "{\"version\":\"V1.2.0 build 201201\",\"capacities\":{\"support_check_auth\":1},\"resources\":[{\"identifier\":\"PetDryerRes\",\"resourceCategory\":\"PetDryerRes\",\"localIndex\":[\"0\"],\"dynamic\":false,\"global\":true,\"domains\":[{\"identifier\":\"attr_test\",\"props\":[{\"identifier\":\"attr_r_bool\",\"access\":\"r\",\"schema\":{\"type\":\"bool\"}},{\"identifier\":\"attr_rw_bool\",\"access\":\"rw\",\"schema\":{\"type\":\"bool\"}},{\"identifier\":\"attr_rw_int\",\"access\":\"rw\",\"schema\":{\"mn\":10,\"mx\":50,\"type\":\"int\"}},{\"identifier\":\"attr_r_int\",\"access\":\"r\",\"schema\":{\"emx\":20,\"emn\":1,\"type\":\"int\"}},{\"identifier\":\"attr_rw_str\",\"access\":\"rw\",\"schema\":{\"mxl\":30,\"mnl\":5,\"type\":\"str\"}},{\"identifier\":\"attr_r_str\",\"access\":\"r\",\"schema\":{\"type\":\"str\"}},{\"identifier\":\"attr_rw_num\",\"access\":\"rw\",\"schema\":{\"type\":\"num\"}},{\"identifier\":\"attr_r_num\",\"access\":\"r\",\"schema\":{\"type\":\"num\"}},{\"identifier\":\"attr_rw_obj\",\"access\":\"rw\",\"schema\":{\"prop\":[{\"type\":\"int\",\"key\":\"obj_int\"},{\"type\":\"str\",\"key\":\"obj_str\"},{\"type\":\"int\",\"key\":\"obj_int_enum\",\"enum\":[0,1,2]},{\"mn\":1,\"mx\":10,\"type\":\"int\",\"key\":\"obj_int_lim\"}],\"type\":\"obj\",\"req\":[]}},{\"identifier\":\"attr_r_obj\",\"access\":\"r\",\"schema\":{\"prop\":[{\"type\":\"int\",\"key\":\"test_int\"}],\"type\":\"obj\",\"req\":[]}},{\"identifier\":\"attr_r_array\",\"access\":\"r\",\"schema\":{\"itm\":{\"type\":\"int\"},\"type\":\"arr\"}},{\"identifier\":\"attr_rw_array\",\"access\":\"rw\",\"schema\":{\"itm\":{\"prop\":[{\"mn\":10,\"mx\":50,\"type\":\"int\",\"key\":\"p_int\",\"multi\":10},{\"type\":\"int\",\"key\":\"p_enum\",\"enum\":[10,20,30]}],\"type\":\"obj\",\"req\":[\"p_int\"]},\"type\":\"arr\"}},{\"identifier\":\"attr_rw_int_multi\",\"access\":\"rw\",\"schema\":{\"mn\":20,\"mx\":50,\"type\":\"int\",\"multi\":10}},{\"identifier\":\"attr_rw_int_enum\",\"access\":\"rw\",\"schema\":{\"type\":\"int\",\"enum\":[0,1,2]}},{\"identifier\":\"attr_rw_str_enum\",\"access\":\"rw\",\"schema\":{\"type\":\"str\",\"enum\":[\"enum1\",\"enum2\"]}}],\"actions\":[],\"events\":[]},{\"identifier\":\"event_test\",\"props\":[],\"actions\":[],\"events\":[{\"identifier\":\"event_notify\",\"eventType\":[\"notification\",\"general\"],\"input\":{\"schema\":{\"prop\":[{\"prop\":[{\"type\":\"str\",\"key\":\"ipV4Address\"},{\"type\":\"str\",\"key\":\"ipV6Address\"},{\"type\":\"str\",\"key\":\"macAddress\"},{\"mxl\":32,\"mnl\":19,\"type\":\"str\",\"key\":\"dateTime\"},{\"mxl\":64,\"mnl\":0,\"type\":\"str\",\"key\":\"UUID\"},{\"type\":\"num\",\"key\":\"timestamp\"}],\"type\":\"obj\",\"key\":\"basic\",\"req\":[\"dateTime\",\"UUID\"]},{\"prop\":[{\"mxl\":0,\"mnl\":64,\"type\":\"str\",\"key\":\"relationId\"},{\"type\":\"num\",\"key\":\"status\",\"enum\":[1,2]},{\"type\":\"num\",\"key\":\"action\"},{\"type\":\"str\",\"key\":\"location\"}],\"type\":\"obj\",\"key\":\"notification\"}],\"type\":\"obj\",\"req\":[]}}},{\"identifier\":\"event_ext\",\"eventType\":[\"general\"],\"input\":{\"schema\":{\"prop\":[{\"prop\":[{\"type\":\"str\",\"key\":\"ipV4Address\"},{\"type\":\"str\",\"key\":\"ipV6Address\"},{\"type\":\"str\",\"key\":\"macAddress\"},{\"mxl\":32,\"mnl\":19,\"type\":\"str\",\"key\":\"dateTime\"},{\"mxl\":64,\"mnl\":0,\"type\":\"str\",\"key\":\"UUID\"},{\"type\":\"num\",\"key\":\"timestamp\"}],\"type\":\"obj\",\"key\":\"basic\",\"req\":[\"dateTime\",\"UUID\"]},{\"prop\":[{\"type\":\"str\",\"key\":\"psd\"}],\"type\":\"obj\",\"key\":\"ext\",\"req\":[]}],\"type\":\"obj\",\"req\":[]}}},{\"identifier\":\"event_null\",\"eventType\":[\"general\"],\"input\":{\"schema\":{\"prop\":[{\"prop\":[{\"type\":\"str\",\"key\":\"ipV4Address\"},{\"type\":\"str\",\"key\":\"ipV6Address\"},{\"type\":\"str\",\"key\":\"macAddress\"},{\"mxl\":32,\"mnl\":19,\"type\":\"str\",\"key\":\"dateTime\"},{\"mxl\":64,\"mnl\":0,\"type\":\"str\",\"key\":\"UUID\"},{\"type\":\"num\",\"key\":\"timestamp\"}],\"type\":\"obj\",\"key\":\"basic\",\"req\":[\"dateTime\",\"UUID\"]}],\"type\":\"obj\"}}}]},{\"identifier\":\"action_test\",\"props\":[],\"actions\":[{\"identifier\":\"action_null\",\"direction\":\"Plt2Dev\"},{\"identifier\":\"action_int\",\"direction\":\"Plt2Dev\",\"input\":{\"schema\":{\"type\":\"int\"}}},{\"identifier\":\"action_str\",\"direction\":\"Plt2Dev\",\"input\":{\"schema\":{\"type\":\"str\"}}},{\"identifier\":\"action_num\",\"direction\":\"Plt2Dev\",\"input\":{\"schema\":{\"type\":\"num\"}}},{\"identifier\":\"action_bool\",\"direction\":\"Plt2Dev\",\"input\":{\"schema\":{\"type\":\"bool\"}}},{\"identifier\":\"action_obj\",\"direction\":\"Plt2Dev\",\"input\":{\"schema\":{\"prop\":[],\"type\":\"obj\"}}},{\"identifier\":\"action_array\",\"direction\":\"Plt2Dev\",\"input\":{\"schema\":{\"itm\":{\"prop\":[],\"type\":\"obj\"},\"type\":\"arr\"}}}],\"events\":[]},{\"identifier\":\"PetHouseMode\",\"props\":[{\"identifier\":\"WinterMode\",\"access\":\"rw\",\"schema\":{\"prop\":[{\"type\":\"int\",\"key\":\"runningTime\"},{\"mn\":10,\"mx\":50,\"type\":\"int\",\"key\":\"temperature\"},{\"mn\":0,\"mx\":100,\"type\":\"int\",\"key\":\"windSpeed\",\"multi\":1}],\"type\":\"obj\",\"req\":[\"runningTime\",\"temperature\",\"windSpeed\"]}},{\"identifier\":\"SummerMode\",\"access\":\"rw\",\"schema\":{\"prop\":[{\"type\":\"int\",\"key\":\"runningTime\"},{\"mn\":10,\"mx\":50,\"type\":\"int\",\"key\":\"temperature\"},{\"mn\":0,\"mx\":100,\"type\":\"int\",\"key\":\"windSpeed\",\"multi\":1}],\"type\":\"obj\",\"req\":[\"runningTime\",\"temperature\",\"windSpeed\"]}},{\"identifier\":\"TourMode\",\"access\":\"rw\",\"schema\":{\"prop\":[{\"type\":\"int\",\"key\":\"countDown\"},{\"mn\":10,\"mx\":50,\"type\":\"int\",\"key\":\"temperature\"},{\"mn\":0,\"mx\":100,\"type\":\"int\",\"key\":\"windSpeed\",\"multi\":1}],\"type\":\"obj\",\"req\":[\"countDown\",\"temperature\",\"windSpeed\"]}},{\"identifier\":\"FastMode\",\"access\":\"rw\",\"schema\":{\"prop\":[{\"type\":\"int\",\"key\":\"countdown\"},{\"mn\":10,\"mx\":50,\"type\":\"int\",\"key\":\"temperature\"},{\"mn\":0,\"mx\":100,\"type\":\"int\",\"key\":\"windSpeed\",\"multi\":1}],\"type\":\"obj\",\"req\":[\"countdown\",\"temperature\",\"windSpeed\"]}},{\"identifier\":\"O3DisinfectionMode\",\"access\":\"rw\",\"schema\":{\"prop\":[{\"type\":\"int\",\"key\":\"countdown\"}],\"type\":\"obj\",\"req\":[\"countdown\"]}},{\"identifier\":\"RunningMode\",\"access\":\"rw\",\"schema\":{\"type\":\"str\",\"enum\":[\"WinterMode\",\"SummerMode\",\"TourMode\",\"FastMode\",\"O3DisinfectionMode\"]}},{\"identifier\":\"RunningStatus\",\"access\":\"rw\",\"schema\":{\"type\":\"int\",\"enum\":[0,1,2,3,4]}},{\"identifier\":\"CountDownIng\",\"access\":\"rw\",\"schema\":{\"type\":\"int\"}}],\"actions\":[],\"events\":[{\"identifier\":\"RunModeComplate\",\"eventType\":[\"notification\"],\"input\":{\"schema\":{\"prop\":[{\"prop\":[{\"type\":\"str\",\"key\":\"ipV4Address\"},{\"type\":\"str\",\"key\":\"ipV6Address\"},{\"type\":\"str\",\"key\":\"macAddress\"},{\"mxl\":32,\"mnl\":19,\"type\":\"str\",\"key\":\"dateTime\"},{\"mxl\":64,\"mnl\":0,\"type\":\"str\",\"key\":\"UUID\"}],\"type\":\"obj\",\"key\":\"basic\",\"req\":[\"ipV4Address\",\"dateTime\",\"UUID\"]},{\"prop\":[{\"mxl\":0,\"mnl\":64,\"type\":\"str\",\"key\":\"relationId\"},{\"type\":\"num\",\"key\":\"action\"}],\"type\":\"obj\",\"key\":\"notification\"}],\"type\":\"obj\"}}}]}]}]}";

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
    UTEST_UNIT_RUN(ut_tsl_errcode);
    UTEST_UNIT_RUN(ut_tsl_prop_report_bool);
    UTEST_UNIT_RUN(ut_tsl_prop_report_int);
    UTEST_UNIT_RUN(ut_tsl_prop_report_float);
    UTEST_UNIT_RUN(ut_tsl_prop_report_str);
    UTEST_UNIT_RUN(ut_tsl_prop_report_jobj);
    UTEST_UNIT_RUN(ut_tsl_prop_report_jarr);
    UTEST_UNIT_RUN(ut_tsl_event_report_null);
    UTEST_UNIT_RUN(ut_tsl_event_report_obj);

    UTEST_UNIT_RUN(ut_tsl_profile_prop_report_bool);
    UTEST_UNIT_RUN(ut_tsl_profile_prop_report_int);
    UTEST_UNIT_RUN(ut_tsl_profile_prop_report_float);
    UTEST_UNIT_RUN(ut_tsl_profile_prop_report_str);
    UTEST_UNIT_RUN(ut_tsl_profile_prop_report_jobj);
    UTEST_UNIT_RUN(ut_tsl_profile_prop_report_jarr);
    UTEST_UNIT_RUN(ut_tsl_profile_event_report_null);
    UTEST_UNIT_RUN(ut_tsl_profile_event_report_obj);
}
UTEST_TC_EXPORT(testcase, "eziot.ut_tsl", global_init, NULL, CONFIG_EZIOT_UNIT_TEST_CASE_TIEMOUT_SECONDS);

void ut_tsl_errcode()
{
    ez_byte_t devid[32] = {0};
    ez_tsl_callbacks_t tsl_things_cbs = {tsl_notice, tsl_action2dev, tsl_property2cloud, tsl_property2dev};
    ez_tsl_callbacks_t tsl_things_cbs_null = {0};
    ez_tsl_key_t tsl_keyinfo = {.domain = "attr_test", .key = "attr_r_bool"};
    ez_tsl_key_t tsl_keyinfo_event = {.domain = "event_test", .key = "event_null"};
    ez_tsl_value_t tsl_value = {.size = 1, .type = EZ_TSL_DATA_TYPE_BOOL, .value_bool = false};
    ez_tsl_rsc_t rsc_info = {.res_type = "PetDryerRes", .local_index = "0"};

    //sdk core not init
    uassert_int_equal(EZ_TSL_ERR_NOT_READY, ez_iot_tsl_init(&tsl_things_cbs));

    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_ctrl(EZ_CMD_DEVID_SET, (ez_void_t *)devid));
    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_ctrl(EZ_CMD_KVIMPL_SET, (ez_void_t *)&g_kv_func));
    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_init(&m_lbs_addr, &m_dev_info, ez_event_notice_func));

    //tsl not init
    uassert_int_equal(EZ_TSL_ERR_NOT_INIT, ez_iot_tsl_property_report(NULL, NULL, NULL, NULL));
    uassert_int_equal(EZ_TSL_ERR_NOT_INIT, ez_iot_tsl_event_report(NULL, NULL, NULL, NULL));
    uassert_int_equal(EZ_TSL_ERR_NOT_INIT, ez_iot_tsl_deinit());

    //tsl init with invalid params
    uassert_int_equal(EZ_TSL_ERR_PARAM_INVALID, ez_iot_tsl_init(NULL));
    uassert_int_equal(EZ_TSL_ERR_PARAM_INVALID, ez_iot_tsl_init(&tsl_things_cbs_null));

    uassert_int_equal(EZ_TSL_ERR_SUCC, ez_iot_tsl_init(&tsl_things_cbs));
    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_start());

    //interface called with invaild params
    uassert_int_equal(EZ_TSL_ERR_PARAM_INVALID, ez_iot_tsl_property_report(NULL, &rsc_info, &tsl_keyinfo, &tsl_value));
    uassert_int_equal(EZ_TSL_ERR_PARAM_INVALID, ez_iot_tsl_property_report(m_dev_info.dev_subserial, NULL, &tsl_keyinfo, &tsl_value));
    uassert_int_equal(EZ_TSL_ERR_PARAM_INVALID, ez_iot_tsl_property_report(m_dev_info.dev_subserial, &rsc_info, NULL, &tsl_value));
    uassert_int_equal(EZ_TSL_ERR_PARAM_INVALID, ez_iot_tsl_event_report(NULL, &rsc_info, &tsl_keyinfo_event, NULL));
    uassert_int_equal(EZ_TSL_ERR_PARAM_INVALID, ez_iot_tsl_event_report(m_dev_info.dev_subserial, NULL, &tsl_keyinfo_event, NULL));
    uassert_int_equal(EZ_TSL_ERR_PARAM_INVALID, ez_iot_tsl_event_report(m_dev_info.dev_subserial, &rsc_info, NULL, NULL));

    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_stop());
    uassert_int_equal(EZ_TSL_ERR_SUCC, ez_iot_tsl_unreg(NULL));
    uassert_int_equal(EZ_TSL_ERR_SUCC, ez_iot_tsl_deinit());
    ez_iot_core_deinit();
}

void ut_tsl_prop_report_bool()
{
    ez_err_t rv;
    ez_byte_t devid[32] = {0};
    ez_tsl_callbacks_t tsl_things_cbs = {tsl_notice, tsl_action2dev, tsl_property2cloud, tsl_property2dev};
    ez_tsl_key_t tsl_keyinfo = {.domain = "attr_test", .key = "attr_r_bool"};
    ez_tsl_value_t tsl_value = {.size = 1, .type = EZ_TSL_DATA_TYPE_BOOL, .value_bool = false};
    ez_tsl_rsc_t rsc_info = {.res_type = "PetDryerRes", .local_index = "0"};

    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_ctrl(EZ_CMD_DEVID_SET, (ez_void_t *)devid));
    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_ctrl(EZ_CMD_KVIMPL_SET, (ez_void_t *)&g_kv_func));
    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_init(&m_lbs_addr, &m_dev_info, ez_event_notice_func));
    uassert_int_equal(EZ_BASE_ERR_SUCC, ez_iot_base_init(ez_base_notice_func));

    uassert_int_equal(EZ_TSL_ERR_SUCC, ez_iot_tsl_init(&tsl_things_cbs));
    uassert_int_equal(EZ_TSL_ERR_SUCC, ez_iot_tsl_reg(NULL, NULL));
    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_start());

    uassert_int_equal(0, dev_event_waitfor(EZ_EVENT_ONLINE, CONFIG_EZIOT_UNIT_TEST_CASE_TIEMOUT_SECONDS * 1000));
    rv = ez_iot_tsl_property_report(m_dev_info.dev_subserial, &rsc_info, &tsl_keyinfo, &tsl_value);
    if (EZ_TSL_ERR_PROFILE_LOADING == rv)
    {
        ezos_delay_ms(3000);
        rv = ez_iot_tsl_property_report(m_dev_info.dev_subserial, &rsc_info, &tsl_keyinfo, &tsl_value);
    }

    uassert_int_equal(EZ_TSL_ERR_SUCC, rv);
    ezos_delay_ms(3000);
    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_stop());
    uassert_int_equal(EZ_TSL_ERR_SUCC, ez_iot_tsl_unreg(NULL));
    uassert_int_equal(EZ_TSL_ERR_SUCC, ez_iot_tsl_deinit());
    ez_iot_base_deinit();
    ez_iot_core_deinit();
}

void ut_tsl_prop_report_int()
{
    ez_err_t rv;
    ez_byte_t devid[32] = {0};
    ez_tsl_callbacks_t tsl_things_cbs = {tsl_notice, tsl_action2dev, tsl_property2cloud, tsl_property2dev};
    ez_tsl_key_t tsl_keyinfo = {.domain = "attr_test", .key = "attr_r_int"};
    ez_tsl_value_t tsl_value = {.size = 4, .type = EZ_TSL_DATA_TYPE_INT, .value_int = 2};
    ez_tsl_rsc_t rsc_info = {.res_type = "PetDryerRes", .local_index = "0"};

    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_ctrl(EZ_CMD_DEVID_SET, (ez_void_t *)devid));
    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_ctrl(EZ_CMD_KVIMPL_SET, (ez_void_t *)&g_kv_func));
    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_init(&m_lbs_addr, &m_dev_info, ez_event_notice_func));
    uassert_int_equal(EZ_BASE_ERR_SUCC, ez_iot_base_init(ez_base_notice_func));

    uassert_int_equal(EZ_TSL_ERR_SUCC, ez_iot_tsl_init(&tsl_things_cbs));
    uassert_int_equal(EZ_TSL_ERR_SUCC, ez_iot_tsl_reg(NULL, NULL));
    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_start());

    uassert_int_equal(0, dev_event_waitfor(EZ_EVENT_ONLINE, CONFIG_EZIOT_UNIT_TEST_CASE_TIEMOUT_SECONDS * 1000));
    rv = ez_iot_tsl_property_report(m_dev_info.dev_subserial, &rsc_info, &tsl_keyinfo, &tsl_value);
    if (EZ_TSL_ERR_PROFILE_LOADING == rv)
    {
        ezos_delay_ms(3000);
        rv = ez_iot_tsl_property_report(m_dev_info.dev_subserial, &rsc_info, &tsl_keyinfo, &tsl_value);
    }

    uassert_int_equal(EZ_TSL_ERR_SUCC, rv);
    ezos_delay_ms(3000);
    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_stop());
    uassert_int_equal(EZ_TSL_ERR_SUCC, ez_iot_tsl_unreg(NULL));
    uassert_int_equal(EZ_TSL_ERR_SUCC, ez_iot_tsl_deinit());
    ez_iot_base_deinit();

    ez_iot_core_deinit();
}

void ut_tsl_prop_report_float()
{
    ez_err_t rv;
    ez_byte_t devid[32] = {0};
    ez_tsl_callbacks_t tsl_things_cbs = {tsl_notice, tsl_action2dev, tsl_property2cloud, tsl_property2dev};
    ez_tsl_key_t tsl_keyinfo = {.domain = "attr_test", .key = "attr_r_num"};
    ez_tsl_value_t tsl_value = {.size = sizeof(double), .type = EZ_TSL_DATA_TYPE_DOUBLE, .value_double = 10086.11112222};
    ez_tsl_rsc_t rsc_info = {.res_type = "PetDryerRes", .local_index = "0"};

    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_ctrl(EZ_CMD_DEVID_SET, (ez_void_t *)devid));
    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_ctrl(EZ_CMD_KVIMPL_SET, (ez_void_t *)&g_kv_func));
    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_init(&m_lbs_addr, &m_dev_info, ez_event_notice_func));
    uassert_int_equal(EZ_BASE_ERR_SUCC, ez_iot_base_init(ez_base_notice_func));

    uassert_int_equal(EZ_TSL_ERR_SUCC, ez_iot_tsl_init(&tsl_things_cbs));
    uassert_int_equal(EZ_TSL_ERR_SUCC, ez_iot_tsl_reg(NULL, NULL));
    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_start());

    uassert_int_equal(0, dev_event_waitfor(EZ_EVENT_ONLINE, CONFIG_EZIOT_UNIT_TEST_CASE_TIEMOUT_SECONDS * 1000));
    rv = ez_iot_tsl_property_report(m_dev_info.dev_subserial, &rsc_info, &tsl_keyinfo, &tsl_value);
    if (EZ_TSL_ERR_PROFILE_LOADING == rv)
    {
        ezos_delay_ms(3000);
        rv = ez_iot_tsl_property_report(m_dev_info.dev_subserial, &rsc_info, &tsl_keyinfo, &tsl_value);
    }

    uassert_int_equal(EZ_TSL_ERR_SUCC, rv);
    ezos_delay_ms(3000);
    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_stop());
    uassert_int_equal(EZ_TSL_ERR_SUCC, ez_iot_tsl_unreg(NULL));
    uassert_int_equal(EZ_TSL_ERR_SUCC, ez_iot_tsl_deinit());
    ez_iot_base_deinit();

    ez_iot_core_deinit();
}

void ut_tsl_prop_report_str()
{
    ez_err_t rv;
    ez_byte_t devid[32] = {0};
    ez_tsl_callbacks_t tsl_things_cbs = {tsl_notice, tsl_action2dev, tsl_property2cloud, tsl_property2dev};
    ez_tsl_key_t tsl_keyinfo = {.domain = "attr_test", .key = "attr_r_str"};
    ez_tsl_value_t tsl_value = {.size = ezos_strlen("test_str_111222333"), .type = EZ_TSL_DATA_TYPE_STRING, .value = "test_str_111222333"};
    ez_tsl_rsc_t rsc_info = {.res_type = "PetDryerRes", .local_index = "0"};

    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_ctrl(EZ_CMD_DEVID_SET, (ez_void_t *)devid));
    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_ctrl(EZ_CMD_KVIMPL_SET, (ez_void_t *)&g_kv_func));
    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_init(&m_lbs_addr, &m_dev_info, ez_event_notice_func));
    uassert_int_equal(EZ_BASE_ERR_SUCC, ez_iot_base_init(ez_base_notice_func));

    uassert_int_equal(EZ_TSL_ERR_SUCC, ez_iot_tsl_init(&tsl_things_cbs));
    uassert_int_equal(EZ_TSL_ERR_SUCC, ez_iot_tsl_reg(NULL, NULL));
    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_start());

    uassert_int_equal(0, dev_event_waitfor(EZ_EVENT_ONLINE, CONFIG_EZIOT_UNIT_TEST_CASE_TIEMOUT_SECONDS * 1000));
    rv = ez_iot_tsl_property_report(m_dev_info.dev_subserial, &rsc_info, &tsl_keyinfo, &tsl_value);
    if (EZ_TSL_ERR_PROFILE_LOADING == rv)
    {
        ezos_delay_ms(3000);
        rv = ez_iot_tsl_property_report(m_dev_info.dev_subserial, &rsc_info, &tsl_keyinfo, &tsl_value);
    }

    uassert_int_equal(EZ_TSL_ERR_SUCC, rv);
    ezos_delay_ms(3000);
    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_stop());
    uassert_int_equal(EZ_TSL_ERR_SUCC, ez_iot_tsl_unreg(NULL));
    uassert_int_equal(EZ_TSL_ERR_SUCC, ez_iot_tsl_deinit());
    ez_iot_base_deinit();

    ez_iot_core_deinit();
}

void ut_tsl_prop_report_jobj()
{
    ez_err_t rv;
    ez_byte_t devid[32] = {0};
    ez_tsl_callbacks_t tsl_things_cbs = {tsl_notice, tsl_action2dev, tsl_property2cloud, tsl_property2dev};
    ez_tsl_key_t tsl_keyinfo = {.domain = "attr_test", .key = "attr_r_obj"};
    ez_tsl_value_t tsl_value = {.size = ezos_strlen("{\"test_int\":89622462}"), .type = EZ_TSL_DATA_TYPE_OBJECT, .value = "{\"test_int\":89622462}"};
    ez_tsl_rsc_t rsc_info = {.res_type = "PetDryerRes", .local_index = "0"};

    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_ctrl(EZ_CMD_DEVID_SET, (ez_void_t *)devid));
    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_ctrl(EZ_CMD_KVIMPL_SET, (ez_void_t *)&g_kv_func));
    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_init(&m_lbs_addr, &m_dev_info, ez_event_notice_func));
    uassert_int_equal(EZ_BASE_ERR_SUCC, ez_iot_base_init(ez_base_notice_func));

    uassert_int_equal(EZ_TSL_ERR_SUCC, ez_iot_tsl_init(&tsl_things_cbs));
    uassert_int_equal(EZ_TSL_ERR_SUCC, ez_iot_tsl_reg(NULL, NULL));
    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_start());

    uassert_int_equal(0, dev_event_waitfor(EZ_EVENT_ONLINE, CONFIG_EZIOT_UNIT_TEST_CASE_TIEMOUT_SECONDS * 1000));
    rv = ez_iot_tsl_property_report(m_dev_info.dev_subserial, &rsc_info, &tsl_keyinfo, &tsl_value);
    if (EZ_TSL_ERR_PROFILE_LOADING == rv)
    {
        ezos_delay_ms(3000);
        rv = ez_iot_tsl_property_report(m_dev_info.dev_subserial, &rsc_info, &tsl_keyinfo, &tsl_value);
    }

    uassert_int_equal(EZ_TSL_ERR_SUCC, rv);
    ezos_delay_ms(3000);
    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_stop());
    uassert_int_equal(EZ_TSL_ERR_SUCC, ez_iot_tsl_unreg(NULL));
    uassert_int_equal(EZ_TSL_ERR_SUCC, ez_iot_tsl_deinit());
    ez_iot_core_deinit();
}

void ut_tsl_prop_report_jarr()
{
    ez_err_t rv;
    ez_byte_t devid[32] = {0};
    ez_tsl_callbacks_t tsl_things_cbs = {tsl_notice, tsl_action2dev, tsl_property2cloud, tsl_property2dev};
    ez_tsl_key_t tsl_keyinfo = {.domain = "attr_test", .key = "attr_r_array"};
    ez_tsl_value_t tsl_value = {.size = ezos_strlen("[-42570399,56004448]"), .type = EZ_TSL_DATA_TYPE_ARRAY, .value = "[-42570399,56004448]"};
    ez_tsl_rsc_t rsc_info = {.res_type = "PetDryerRes", .local_index = "0"};

    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_ctrl(EZ_CMD_DEVID_SET, (ez_void_t *)devid));
    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_ctrl(EZ_CMD_KVIMPL_SET, (ez_void_t *)&g_kv_func));
    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_init(&m_lbs_addr, &m_dev_info, ez_event_notice_func));
    uassert_int_equal(EZ_BASE_ERR_SUCC, ez_iot_base_init(ez_base_notice_func));

    uassert_int_equal(EZ_TSL_ERR_SUCC, ez_iot_tsl_init(&tsl_things_cbs));
    uassert_int_equal(EZ_TSL_ERR_SUCC, ez_iot_tsl_reg(NULL, NULL));
    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_start());

    uassert_int_equal(0, dev_event_waitfor(EZ_EVENT_ONLINE, CONFIG_EZIOT_UNIT_TEST_CASE_TIEMOUT_SECONDS * 1000));
    rv = ez_iot_tsl_property_report(m_dev_info.dev_subserial, &rsc_info, &tsl_keyinfo, &tsl_value);
    if (EZ_TSL_ERR_PROFILE_LOADING == rv)
    {
        ezos_delay_ms(3000);
        rv = ez_iot_tsl_property_report(m_dev_info.dev_subserial, &rsc_info, &tsl_keyinfo, &tsl_value);
    }

    uassert_int_equal(EZ_TSL_ERR_SUCC, rv);
    ezos_delay_ms(3000);
    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_stop());
    uassert_int_equal(EZ_TSL_ERR_SUCC, ez_iot_tsl_unreg(NULL));
    uassert_int_equal(EZ_TSL_ERR_SUCC, ez_iot_tsl_deinit());
    ez_iot_base_deinit();
    ez_iot_core_deinit();
}

void ut_tsl_event_report_null()
{
    ez_err_t rv;
    ez_byte_t devid[32] = {0};
    ez_tsl_callbacks_t tsl_things_cbs = {tsl_notice, tsl_action2dev, tsl_property2cloud, tsl_property2dev};
    ez_tsl_key_t tsl_keyinfo = {.domain = "event_test", .key = "event_null"};
    ez_tsl_rsc_t rsc_info = {.res_type = "PetDryerRes", .local_index = "0"};

    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_ctrl(EZ_CMD_DEVID_SET, (ez_void_t *)devid));
    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_ctrl(EZ_CMD_KVIMPL_SET, (ez_void_t *)&g_kv_func));
    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_init(&m_lbs_addr, &m_dev_info, ez_event_notice_func));
    uassert_int_equal(EZ_BASE_ERR_SUCC, ez_iot_base_init(ez_base_notice_func));

    uassert_int_equal(EZ_TSL_ERR_SUCC, ez_iot_tsl_init(&tsl_things_cbs));
    uassert_int_equal(EZ_TSL_ERR_SUCC, ez_iot_tsl_reg(NULL, NULL));
    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_start());

    uassert_int_equal(0, dev_event_waitfor(EZ_EVENT_ONLINE, CONFIG_EZIOT_UNIT_TEST_CASE_TIEMOUT_SECONDS * 1000));
    rv = ez_iot_tsl_event_report(m_dev_info.dev_subserial, &rsc_info, &tsl_keyinfo, NULL);
    if (EZ_TSL_ERR_PROFILE_LOADING == rv)
    {
        ezos_delay_ms(3000);
        rv = ez_iot_tsl_event_report(m_dev_info.dev_subserial, &rsc_info, &tsl_keyinfo, NULL);
    }

    uassert_int_equal(EZ_TSL_ERR_SUCC, rv);
    ezos_delay_ms(3000);
    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_stop());
    uassert_int_equal(EZ_TSL_ERR_SUCC, ez_iot_tsl_unreg(NULL));
    uassert_int_equal(EZ_TSL_ERR_SUCC, ez_iot_tsl_deinit());
    ez_iot_base_deinit();

    ez_iot_core_deinit();
}

char *js_rep = "{\"ext\":{\"psd\":\"consequat sit in\"},\"basic\":{\"dateTime\":\"2021-06-30aaaaaaaaaaaaaaa\",\"UUID\":\"1111111111111111111\"}}";

void ut_tsl_event_report_obj()
{
    ez_err_t rv;
    ez_byte_t devid[32] = {0};
    ez_tsl_callbacks_t tsl_things_cbs = {tsl_notice, tsl_action2dev, tsl_property2cloud, tsl_property2dev};
    ez_tsl_key_t tsl_keyinfo = {.domain = "event_test", .key = "event_ext"};
    ez_tsl_rsc_t rsc_info = {.res_type = "PetDryerRes", .local_index = "0"};
    ez_tsl_value_t tsl_value = {.size = ezos_strlen(js_rep), .type = EZ_TSL_DATA_TYPE_OBJECT, .value = js_rep};

    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_ctrl(EZ_CMD_DEVID_SET, (ez_void_t *)devid));
    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_ctrl(EZ_CMD_KVIMPL_SET, (ez_void_t *)&g_kv_func));
    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_init(&m_lbs_addr, &m_dev_info, ez_event_notice_func));
    uassert_int_equal(EZ_BASE_ERR_SUCC, ez_iot_base_init(ez_base_notice_func));

    uassert_int_equal(EZ_TSL_ERR_SUCC, ez_iot_tsl_init(&tsl_things_cbs));
    uassert_int_equal(EZ_TSL_ERR_SUCC, ez_iot_tsl_reg(NULL, NULL));
    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_start());

    uassert_int_equal(0, dev_event_waitfor(EZ_EVENT_ONLINE, CONFIG_EZIOT_UNIT_TEST_CASE_TIEMOUT_SECONDS * 1000));
    rv = ez_iot_tsl_event_report(m_dev_info.dev_subserial, &rsc_info, &tsl_keyinfo, &tsl_value);
    if (EZ_TSL_ERR_PROFILE_LOADING == rv)
    {
        ezos_delay_ms(3000);
        rv = ez_iot_tsl_event_report(m_dev_info.dev_subserial, &rsc_info, &tsl_keyinfo, &tsl_value);
    }

    uassert_int_equal(EZ_TSL_ERR_SUCC, rv);
    ezos_delay_ms(3000);
    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_stop());
    uassert_int_equal(EZ_TSL_ERR_SUCC, ez_iot_tsl_unreg(NULL));
    uassert_int_equal(EZ_TSL_ERR_SUCC, ez_iot_tsl_deinit());
    ez_iot_base_deinit();
    ez_iot_core_deinit();
}

void ut_tsl_profile_prop_report_bool()
{
    ez_err_t rv;
    ez_byte_t devid[32] = {0};
    ez_tsl_callbacks_t tsl_things_cbs = {tsl_notice, tsl_action2dev, tsl_property2cloud, tsl_property2dev};
    ez_tsl_key_t tsl_keyinfo = {.domain = "attr_test", .key = "attr_r_bool"};
    ez_tsl_value_t tsl_value = {.size = 1, .type = EZ_TSL_DATA_TYPE_BOOL, .value_bool = false};
    ez_tsl_rsc_t rsc_info = {.res_type = "PetDryerRes", .local_index = "0"};

    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_ctrl(EZ_CMD_DEVID_SET, (ez_void_t *)devid));
    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_ctrl(EZ_CMD_KVIMPL_SET, (ez_void_t *)&g_kv_func));
    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_init(&m_lbs_addr, &m_dev_info, ez_event_notice_func));

    uassert_int_equal(EZ_TSL_ERR_SUCC, ez_iot_tsl_init(&tsl_things_cbs));
    uassert_int_equal(EZ_TSL_ERR_SUCC, ez_iot_tsl_reg(NULL, g_profile));
    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_start());

    uassert_int_equal(0, dev_event_waitfor(EZ_EVENT_ONLINE, CONFIG_EZIOT_UNIT_TEST_CASE_TIEMOUT_SECONDS * 1000));
    rv = ez_iot_tsl_property_report(m_dev_info.dev_subserial, &rsc_info, &tsl_keyinfo, &tsl_value);
    uassert_int_equal(EZ_TSL_ERR_SUCC, rv);

    ezos_delay_ms(3000);
    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_stop());
    uassert_int_equal(EZ_TSL_ERR_SUCC, ez_iot_tsl_unreg(NULL));
    uassert_int_equal(EZ_TSL_ERR_SUCC, ez_iot_tsl_deinit());
    ez_iot_core_deinit();
}

void ut_tsl_profile_prop_report_int()
{
    ez_err_t rv;
    ez_byte_t devid[32] = {0};
    ez_tsl_callbacks_t tsl_things_cbs = {tsl_notice, tsl_action2dev, tsl_property2cloud, tsl_property2dev};
    ez_tsl_key_t tsl_keyinfo = {.domain = "attr_test", .key = "attr_r_int"};
    ez_tsl_value_t tsl_value = {.size = 4, .type = EZ_TSL_DATA_TYPE_INT, .value_int = 2};
    ez_tsl_rsc_t rsc_info = {.res_type = "PetDryerRes", .local_index = "0"};

    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_ctrl(EZ_CMD_DEVID_SET, (ez_void_t *)devid));
    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_ctrl(EZ_CMD_KVIMPL_SET, (ez_void_t *)&g_kv_func));
    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_init(&m_lbs_addr, &m_dev_info, ez_event_notice_func));

    uassert_int_equal(EZ_TSL_ERR_SUCC, ez_iot_tsl_init(&tsl_things_cbs));
    uassert_int_equal(EZ_TSL_ERR_SUCC, ez_iot_tsl_reg(NULL, g_profile));
    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_start());

    uassert_int_equal(0, dev_event_waitfor(EZ_EVENT_ONLINE, CONFIG_EZIOT_UNIT_TEST_CASE_TIEMOUT_SECONDS * 1000));
    rv = ez_iot_tsl_property_report(m_dev_info.dev_subserial, &rsc_info, &tsl_keyinfo, &tsl_value);
    uassert_int_equal(EZ_TSL_ERR_SUCC, rv);

    ezos_delay_ms(3000);
    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_stop());
    uassert_int_equal(EZ_TSL_ERR_SUCC, ez_iot_tsl_unreg(NULL));
    uassert_int_equal(EZ_TSL_ERR_SUCC, ez_iot_tsl_deinit());
    ez_iot_core_deinit();
}

void ut_tsl_profile_prop_report_float()
{
    ez_err_t rv;
    ez_byte_t devid[32] = {0};
    ez_tsl_callbacks_t tsl_things_cbs = {tsl_notice, tsl_action2dev, tsl_property2cloud, tsl_property2dev};
    ez_tsl_key_t tsl_keyinfo = {.domain = "attr_test", .key = "attr_r_num"};
    ez_tsl_value_t tsl_value = {.size = sizeof(double), .type = EZ_TSL_DATA_TYPE_DOUBLE, .value_double = 10086.11112222};
    ez_tsl_rsc_t rsc_info = {.res_type = "PetDryerRes", .local_index = "0"};

    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_ctrl(EZ_CMD_DEVID_SET, (ez_void_t *)devid));
    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_ctrl(EZ_CMD_KVIMPL_SET, (ez_void_t *)&g_kv_func));
    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_init(&m_lbs_addr, &m_dev_info, ez_event_notice_func));

    uassert_int_equal(EZ_TSL_ERR_SUCC, ez_iot_tsl_init(&tsl_things_cbs));
    uassert_int_equal(EZ_TSL_ERR_SUCC, ez_iot_tsl_reg(NULL, g_profile));
    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_start());

    uassert_int_equal(0, dev_event_waitfor(EZ_EVENT_ONLINE, CONFIG_EZIOT_UNIT_TEST_CASE_TIEMOUT_SECONDS * 1000));
    rv = ez_iot_tsl_property_report(m_dev_info.dev_subserial, &rsc_info, &tsl_keyinfo, &tsl_value);
    uassert_int_equal(EZ_TSL_ERR_SUCC, rv);

    ezos_delay_ms(3000);
    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_stop());
    uassert_int_equal(EZ_TSL_ERR_SUCC, ez_iot_tsl_unreg(NULL));
    uassert_int_equal(EZ_TSL_ERR_SUCC, ez_iot_tsl_deinit());
    ez_iot_core_deinit();
}

void ut_tsl_profile_prop_report_str()
{
    ez_err_t rv;
    ez_byte_t devid[32] = {0};
    ez_tsl_callbacks_t tsl_things_cbs = {tsl_notice, tsl_action2dev, tsl_property2cloud, tsl_property2dev};
    ez_tsl_key_t tsl_keyinfo = {.domain = "attr_test", .key = "attr_r_str"};
    ez_tsl_value_t tsl_value = {.size = ezos_strlen("test_str_111222333"), .type = EZ_TSL_DATA_TYPE_STRING, .value = "test_str_111222333"};
    ez_tsl_rsc_t rsc_info = {.res_type = "PetDryerRes", .local_index = "0"};

    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_ctrl(EZ_CMD_DEVID_SET, (ez_void_t *)devid));
    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_ctrl(EZ_CMD_KVIMPL_SET, (ez_void_t *)&g_kv_func));
    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_init(&m_lbs_addr, &m_dev_info, ez_event_notice_func));

    uassert_int_equal(EZ_TSL_ERR_SUCC, ez_iot_tsl_init(&tsl_things_cbs));
    uassert_int_equal(EZ_TSL_ERR_SUCC, ez_iot_tsl_reg(NULL, g_profile));
    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_start());

    uassert_int_equal(0, dev_event_waitfor(EZ_EVENT_ONLINE, CONFIG_EZIOT_UNIT_TEST_CASE_TIEMOUT_SECONDS * 1000));
    rv = ez_iot_tsl_property_report(m_dev_info.dev_subserial, &rsc_info, &tsl_keyinfo, &tsl_value);
    uassert_int_equal(EZ_TSL_ERR_SUCC, rv);

    ezos_delay_ms(3000);
    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_stop());
    uassert_int_equal(EZ_TSL_ERR_SUCC, ez_iot_tsl_unreg(NULL));
    uassert_int_equal(EZ_TSL_ERR_SUCC, ez_iot_tsl_deinit());
    ez_iot_core_deinit();
}

void ut_tsl_profile_prop_report_jobj()
{
    ez_err_t rv;
    ez_byte_t devid[32] = {0};
    ez_tsl_callbacks_t tsl_things_cbs = {tsl_notice, tsl_action2dev, tsl_property2cloud, tsl_property2dev};
    ez_tsl_key_t tsl_keyinfo = {.domain = "attr_test", .key = "attr_r_obj"};
    ez_tsl_value_t tsl_value = {.size = ezos_strlen("{\"test_int\":89622462}"), .type = EZ_TSL_DATA_TYPE_OBJECT, .value = "{\"test_int\":89622462}"};
    ez_tsl_rsc_t rsc_info = {.res_type = "PetDryerRes", .local_index = "0"};

    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_ctrl(EZ_CMD_DEVID_SET, (ez_void_t *)devid));
    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_ctrl(EZ_CMD_KVIMPL_SET, (ez_void_t *)&g_kv_func));
    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_init(&m_lbs_addr, &m_dev_info, ez_event_notice_func));

    uassert_int_equal(EZ_TSL_ERR_SUCC, ez_iot_tsl_init(&tsl_things_cbs));
    uassert_int_equal(EZ_TSL_ERR_SUCC, ez_iot_tsl_reg(NULL, g_profile));

    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_start());

    uassert_int_equal(0, dev_event_waitfor(EZ_EVENT_ONLINE, CONFIG_EZIOT_UNIT_TEST_CASE_TIEMOUT_SECONDS * 1000));
    rv = ez_iot_tsl_property_report(m_dev_info.dev_subserial, &rsc_info, &tsl_keyinfo, &tsl_value);
    uassert_int_equal(EZ_TSL_ERR_SUCC, rv);

    ezos_delay_ms(3000);
    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_stop());
    uassert_int_equal(EZ_TSL_ERR_SUCC, ez_iot_tsl_unreg(NULL));
    uassert_int_equal(EZ_TSL_ERR_SUCC, ez_iot_tsl_deinit());
    ez_iot_core_deinit();
}

void ut_tsl_profile_prop_report_jarr()
{
    ez_err_t rv;
    ez_byte_t devid[32] = {0};
    ez_tsl_callbacks_t tsl_things_cbs = {tsl_notice, tsl_action2dev, tsl_property2cloud, tsl_property2dev};
    ez_tsl_key_t tsl_keyinfo = {.domain = "attr_test", .key = "attr_r_array"};
    ez_tsl_value_t tsl_value = {.size = ezos_strlen("[-42570399,56004448]"), .type = EZ_TSL_DATA_TYPE_ARRAY, .value = "[-42570399,56004448]"};
    ez_tsl_rsc_t rsc_info = {.res_type = "PetDryerRes", .local_index = "0"};

    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_ctrl(EZ_CMD_DEVID_SET, (ez_void_t *)devid));
    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_ctrl(EZ_CMD_KVIMPL_SET, (ez_void_t *)&g_kv_func));
    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_init(&m_lbs_addr, &m_dev_info, ez_event_notice_func));

    uassert_int_equal(EZ_TSL_ERR_SUCC, ez_iot_tsl_init(&tsl_things_cbs));
    uassert_int_equal(EZ_TSL_ERR_SUCC, ez_iot_tsl_reg(NULL, g_profile));
    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_start());

    uassert_int_equal(0, dev_event_waitfor(EZ_EVENT_ONLINE, CONFIG_EZIOT_UNIT_TEST_CASE_TIEMOUT_SECONDS * 1000));
    rv = ez_iot_tsl_property_report(m_dev_info.dev_subserial, &rsc_info, &tsl_keyinfo, &tsl_value);
    uassert_int_equal(EZ_TSL_ERR_SUCC, rv);

    ezos_delay_ms(3000);
    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_stop());
    uassert_int_equal(EZ_TSL_ERR_SUCC, ez_iot_tsl_unreg(NULL));
    uassert_int_equal(EZ_TSL_ERR_SUCC, ez_iot_tsl_deinit());
    ez_iot_core_deinit();
}

void ut_tsl_profile_event_report_null()
{
    ez_err_t rv;
    ez_byte_t devid[32] = {0};
    ez_tsl_callbacks_t tsl_things_cbs = {tsl_notice, tsl_action2dev, tsl_property2cloud, tsl_property2dev};
    ez_tsl_key_t tsl_keyinfo = {.domain = "event_test", .key = "event_null"};
    ez_tsl_rsc_t rsc_info = {.res_type = "PetDryerRes", .local_index = "0"};

    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_ctrl(EZ_CMD_DEVID_SET, (ez_void_t *)devid));
    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_ctrl(EZ_CMD_KVIMPL_SET, (ez_void_t *)&g_kv_func));
    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_init(&m_lbs_addr, &m_dev_info, ez_event_notice_func));

    uassert_int_equal(EZ_TSL_ERR_SUCC, ez_iot_tsl_init(&tsl_things_cbs));
    uassert_int_equal(EZ_TSL_ERR_SUCC, ez_iot_tsl_reg(NULL, g_profile));
    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_start());

    uassert_int_equal(0, dev_event_waitfor(EZ_EVENT_ONLINE, CONFIG_EZIOT_UNIT_TEST_CASE_TIEMOUT_SECONDS * 1000));
    rv = ez_iot_tsl_event_report(m_dev_info.dev_subserial, &rsc_info, &tsl_keyinfo, NULL);
    uassert_int_equal(EZ_TSL_ERR_SUCC, rv);

    ezos_delay_ms(3000);
    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_stop());
    uassert_int_equal(EZ_TSL_ERR_SUCC, ez_iot_tsl_unreg(NULL));
    uassert_int_equal(EZ_TSL_ERR_SUCC, ez_iot_tsl_deinit());
    ez_iot_core_deinit();
}

void ut_tsl_profile_event_report_obj()
{
    ez_err_t rv;
    ez_byte_t devid[32] = {0};
    ez_tsl_callbacks_t tsl_things_cbs = {tsl_notice, tsl_action2dev, tsl_property2cloud, tsl_property2dev};
    ez_tsl_key_t tsl_keyinfo = {.domain = "event_test", .key = "event_ext"};
    ez_tsl_rsc_t rsc_info = {.res_type = "PetDryerRes", .local_index = "0"};
    ez_tsl_value_t tsl_value = {.size = ezos_strlen(js_rep), .type = EZ_TSL_DATA_TYPE_OBJECT, .value = js_rep};

    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_ctrl(EZ_CMD_DEVID_SET, (ez_void_t *)devid));
    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_ctrl(EZ_CMD_KVIMPL_SET, (ez_void_t *)&g_kv_func));
    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_init(&m_lbs_addr, &m_dev_info, ez_event_notice_func));

    uassert_int_equal(EZ_TSL_ERR_SUCC, ez_iot_tsl_init(&tsl_things_cbs));
    uassert_int_equal(EZ_TSL_ERR_SUCC, ez_iot_tsl_reg(NULL, g_profile));
    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_start());

    uassert_int_equal(0, dev_event_waitfor(EZ_EVENT_ONLINE, CONFIG_EZIOT_UNIT_TEST_CASE_TIEMOUT_SECONDS * 1000));
    rv = ez_iot_tsl_event_report(m_dev_info.dev_subserial, &rsc_info, &tsl_keyinfo, &tsl_value);
    uassert_int_equal(EZ_TSL_ERR_SUCC, rv);

    ezos_delay_ms(3000);
    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_stop());
    uassert_int_equal(EZ_TSL_ERR_SUCC, ez_iot_tsl_unreg(NULL));
    uassert_int_equal(EZ_TSL_ERR_SUCC, ez_iot_tsl_deinit());
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

static ez_int32_t tsl_notice(ez_tsl_event_e event_type, ez_void_t *data, ez_int32_t len)
{
    return 0;
}

static ez_int32_t tsl_action2dev(const ez_char_t *sn, const ez_tsl_rsc_t *rsc_info, const ez_tsl_key_t *key_info,
                                 const ez_tsl_value_t *value_in, ez_tsl_value_t *value_out)
{
    return 0;
}

static ez_int32_t tsl_property2cloud(const ez_char_t *sn, const ez_tsl_rsc_t *rsc_info, const ez_tsl_key_t *key_info, ez_tsl_value_t *value_out)
{
    return -1;
}

static ez_int32_t tsl_property2dev(const ez_char_t *sn, const ez_tsl_rsc_t *rsc_info, const ez_tsl_key_t *key_info, const ez_tsl_value_t *value)
{
    return 1;
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

static ez_int32_t ez_base_notice_func(ez_base_event_e event_type, ez_void_t *data, ez_int32_t len)
{
    return 0;
}