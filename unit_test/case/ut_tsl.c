/**
 * @file ut_tsl.cc
 * @author xurongjun (xurongjun@ezvizlife.com)
 * @brief 物模型模块单元测试
 * @version 0.1
 * @date 2020-11-06
 * 
 * @copyright Copyright (c) 2020
 * 
 */

#include <stdlib.h>
#include <flashdb.h>
#include <string.h>
#include "ut_config.h"
#include "ez_iot_core.h"
#include "ez_iot_log.h"
#include "ez_iot_tsl.h"
#include "ez_hal/hal_thread.h"
#include "kv_imp.h"
#include "utest.h"

/* 测试接口错误码 */
void ut_tsl_errcode();
UTEST_TC_EXPORT(ut_tsl_errcode, NULL, NULL, DEFAULT_TIMEOUT_S);

/* 上报属性-布尔 */
void ut_tsl_prop_report_bool();
UTEST_TC_EXPORT(ut_tsl_prop_report_bool, NULL, NULL, DEFAULT_TIMEOUT_S);

/* 上报属性-整形 */
void ut_tsl_prop_report_int();
UTEST_TC_EXPORT(ut_tsl_prop_report_int, NULL, NULL, DEFAULT_TIMEOUT_S);

/* 上报属性-浮点 */
void ut_tsl_prop_report_float();
UTEST_TC_EXPORT(ut_tsl_prop_report_float, NULL, NULL, DEFAULT_TIMEOUT_S);

/* 上报属性-字符串 */
void ut_tsl_prop_report_str();
UTEST_TC_EXPORT(ut_tsl_prop_report_str, NULL, NULL, DEFAULT_TIMEOUT_S);

/* 上报属性-json对象 */
void ut_tsl_prop_report_jobj();
UTEST_TC_EXPORT(ut_tsl_prop_report_jobj, NULL, NULL, DEFAULT_TIMEOUT_S);

/* 上报属性-json数组 */
void ut_tsl_prop_report_jarr();
UTEST_TC_EXPORT(ut_tsl_prop_report_jarr, NULL, NULL, DEFAULT_TIMEOUT_S);

/* 上报事件-无数据 */
void ut_tsl_event_report_null();
UTEST_TC_EXPORT(ut_tsl_event_report_null, NULL, NULL, DEFAULT_TIMEOUT_S);

/* 上报事件-json对象 */
void ut_tsl_event_report_obj();
UTEST_TC_EXPORT(ut_tsl_event_report_obj, NULL, NULL, DEFAULT_TIMEOUT_S);

/* 上报事件-json对象(带notification) */
void ut_tsl_event_report_obj2();
UTEST_TC_EXPORT(ut_tsl_event_report_obj2, NULL, NULL, DEFAULT_TIMEOUT_S);

static int32_t ez_recv_msg_cb(ez_iot_cloud2dev_msg_type_t msg_type, void *data, int len);
static int32_t ez_recv_event_cb(ez_iot_event_t event_type, void *data, int len);
static int dev_event_waitfor(int event_id, int time_ms);

static int m_event_id = -1;
static int m_last_err = 0;
static ez_iot_srv_info_t m_lbs_addr = {(int8_t *)EZ_IOT_CLOUD_ENTRY_HOST, EZ_IOT_CLOUD_ENTRY_PORT};
static ez_iot_callbacks_t m_cbs = {ez_recv_msg_cb, ez_recv_event_cb};
static ez_iot_dev_info_t m_dev_info = {
    .auth_mode = EZ_IOT_DEV_AUTH_MODE,
    .dev_subserial = EZ_IOT_DEV_UUID,
    .dev_verification_code = EZ_IOT_DEV_LICENSE,
    .dev_firmwareversion = EZ_IOT_DEV_FWVER,
    .dev_type = EZ_IOT_DEV_PRODUCT_KEY,
    .dev_typedisplay = EZ_IOT_DEV_DISPLAY_NAME,
    .dev_id = EZ_IOT_DEV_ID,
};

static ez_iot_kv_callbacks_t m_kv_cbs = {
    .ez_kv_init = kv_init,
    .ez_kv_raw_set = kv_raw_set,
    .ez_kv_raw_get = kv_raw_get,
    .ez_kv_del = kv_del,
    .ez_kv_del_by_prefix = kv_del_by_prefix,
    .ez_kv_print = kv_print,
    .ez_kv_deinit = kv_deinit,
};

static int32_t tsl_things_action2dev(const int8_t *sn, const ez_tsl_rsc_t *rsc_info, const ez_tsl_key_t *key_info,
                                     const ez_tsl_value_t *value_in, ez_tsl_value_t *value_out)
{
  return 0;
}

static int32_t tsl_things_property2cloud(const int8_t *sn, const ez_tsl_rsc_t *rsc_info, const ez_tsl_key_t *key_info, ez_tsl_value_t *value_out)
{
  return -1;
}

int32_t tsl_things_property2dev(const int8_t *sn, const ez_tsl_rsc_t *rsc_info, const ez_tsl_key_t *key_info, const ez_tsl_value_t *value)
{
  return 1;
}

void ut_tsl_errcode()
{
  tsl_things_callbacks_t tsl_things_cbs = {tsl_things_action2dev, tsl_things_property2cloud, tsl_things_property2dev};
  tsl_things_callbacks_t tsl_things_cbs_null = {0};
  tsl_devinfo_t tsl_devinfo = {(int8_t *)EZ_IOT_DEV_UUID, (int8_t *)EZ_IOT_DEV_PRODUCT_KEY, (int8_t *)EZ_IOT_DEV_FWVER};
  ez_tsl_key_t tsl_keyinfo = {.domain = (int8_t *)"attr_test", .key = (int8_t *)"attr_r_bool"};
  ez_tsl_key_t tsl_keyinfo_event = {.domain = (int8_t *)"event_test", .key = (int8_t *)"event_null"};
  ez_tsl_value_t tsl_value = {.size = 1, .type = tsl_data_type_bool, .value_bool = false};
  ez_tsl_rsc_t rsc_info = {.res_type = (int8_t *)"PetDryerRes", .local_index = (int8_t *)"0"};

  //sdk core not init
  uassert_int_equal(ez_errno_tsl_not_init, ez_iot_tsl_init(NULL));
  uassert_int_equal(ez_errno_tsl_not_init, ez_iot_tsl_property_report(NULL, NULL, NULL, NULL));
  uassert_int_equal(ez_errno_tsl_not_init, ez_iot_tsl_event_report(NULL, NULL, NULL, NULL));
  uassert_int_equal(ez_errno_tsl_not_init, ez_iot_tsl_deinit());

  uassert_int_equal(ez_errno_succ, ez_iot_init(&m_lbs_addr, &m_dev_info, &m_cbs, &m_kv_cbs));

  //tsl not init
  uassert_int_equal(ez_errno_tsl_not_init, ez_iot_tsl_property_report(NULL, NULL, NULL, NULL));
  uassert_int_equal(ez_errno_tsl_not_init, ez_iot_tsl_event_report(NULL, NULL, NULL, NULL));
  uassert_int_equal(ez_errno_tsl_not_init, ez_iot_tsl_deinit());

  //tsl init with invalid params
  uassert_int_equal(EZ_TSL_ERR_PARAM_INVALID, ez_iot_tsl_init(NULL));
  uassert_int_equal(EZ_TSL_ERR_PARAM_INVALID, ez_iot_tsl_init(&tsl_things_cbs_null));

  uassert_int_equal(ez_errno_succ, ez_iot_tsl_init(&tsl_things_cbs));

  //sdk core module is not started
  uassert_int_equal(EZ_TSL_ERR_NOT_READY, ez_iot_tsl_property_report(NULL, NULL, NULL, NULL));
  uassert_int_equal(EZ_TSL_ERR_NOT_READY, ez_iot_tsl_event_report(NULL, NULL, NULL, NULL));

  uassert_int_equal(ez_errno_succ, ez_iot_start());

  //interface called with invaild params
  uassert_int_equal(EZ_TSL_ERR_PARAM_INVALID, ez_iot_tsl_property_report(NULL, &rsc_info, &tsl_keyinfo, &tsl_value));
  uassert_int_equal(EZ_TSL_ERR_PARAM_INVALID, ez_iot_tsl_property_report(tsl_devinfo.dev_subserial, NULL, &tsl_keyinfo, &tsl_value));
  uassert_int_equal(EZ_TSL_ERR_PARAM_INVALID, ez_iot_tsl_property_report(tsl_devinfo.dev_subserial, &rsc_info, NULL, &tsl_value));
  uassert_int_equal(EZ_TSL_ERR_PARAM_INVALID, ez_iot_tsl_event_report(NULL, &rsc_info, &tsl_keyinfo_event, NULL));
  uassert_int_equal(EZ_TSL_ERR_PARAM_INVALID, ez_iot_tsl_event_report(tsl_devinfo.dev_subserial, NULL, &tsl_keyinfo_event, NULL));
  uassert_int_equal(EZ_TSL_ERR_PARAM_INVALID, ez_iot_tsl_event_report(tsl_devinfo.dev_subserial, &rsc_info, NULL, NULL));

  uassert_int_equal(ez_errno_succ, ez_iot_stop());
  uassert_int_equal(ez_errno_succ, ez_iot_tsl_deinit());
  ez_iot_fini();
}

void ut_tsl_prop_report_bool()
{
  ez_core_err_e rv = ez_errno_succ;
  tsl_things_callbacks_t tsl_things_cbs = {tsl_things_action2dev, tsl_things_property2cloud, tsl_things_property2dev};
  tsl_devinfo_t tsl_devinfo = {(int8_t *)EZ_IOT_DEV_UUID, (int8_t *)EZ_IOT_DEV_PRODUCT_KEY, (int8_t *)EZ_IOT_DEV_FWVER};
  ez_tsl_key_t tsl_keyinfo = {.domain = (int8_t *)"attr_test", .key = (int8_t *)"attr_r_bool"};
  ez_tsl_value_t tsl_value = {.size = 1, .type = tsl_data_type_bool, .value_bool = false};
  ez_tsl_rsc_t rsc_info = {.res_type = (int8_t *)"PetDryerRes", .local_index = (int8_t *)"0"};

  uassert_int_equal(ez_errno_succ, ez_iot_init(&m_lbs_addr, &m_dev_info, &m_cbs, &m_kv_cbs));
  uassert_int_equal(ez_errno_succ, ez_iot_tsl_init(&tsl_things_cbs));
  uassert_int_equal(ez_errno_succ, ez_iot_start());

  uassert_int_equal(ez_errno_succ, dev_event_waitfor(ez_iot_event_online, DEFAULT_TIMEOUT_S * 1000));
  rv = ez_iot_tsl_property_report(tsl_devinfo.dev_subserial, &rsc_info, &tsl_keyinfo, &tsl_value);
  if(EZ_TSL_ERR_PROFILE_LOADING == rv)
  {
    hal_thread_sleep(3000);
    rv = ez_iot_tsl_property_report(tsl_devinfo.dev_subserial, &rsc_info, &tsl_keyinfo, &tsl_value);
  }

  // uassert_int_equal(ez_errno_succ, rv);
  hal_thread_sleep(30000);
  uassert_int_equal(ez_errno_succ, ez_iot_stop());
  uassert_int_equal(ez_errno_succ, ez_iot_tsl_deinit());
  ez_iot_fini();
}

void ut_tsl_prop_report_int()
{
  ez_core_err_e rv = ez_errno_succ;
  tsl_things_callbacks_t tsl_things_cbs = {tsl_things_action2dev, tsl_things_property2cloud, tsl_things_property2dev};
  tsl_devinfo_t tsl_devinfo = {(int8_t *)EZ_IOT_DEV_UUID, (int8_t *)EZ_IOT_DEV_PRODUCT_KEY, (int8_t *)EZ_IOT_DEV_FWVER};
  ez_tsl_key_t tsl_keyinfo = {.domain = (int8_t *)"attr_test", .key = (int8_t *)"attr_r_int"};
  ez_tsl_value_t tsl_value = {.size = 4, .type = tsl_data_type_int, .value_int = 2};
  ez_tsl_rsc_t rsc_info = {.res_type = (int8_t *)"PetDryerRes", .local_index = (int8_t *)"0"};

  uassert_int_equal(ez_errno_succ, ez_iot_init(&m_lbs_addr, &m_dev_info, &m_cbs, &m_kv_cbs));
  uassert_int_equal(ez_errno_succ, ez_iot_tsl_init(&tsl_things_cbs));
  uassert_int_equal(ez_errno_succ, ez_iot_start());

  uassert_int_equal(ez_errno_succ, dev_event_waitfor(ez_iot_event_online, DEFAULT_TIMEOUT_S * 1000));
  rv = ez_iot_tsl_property_report(tsl_devinfo.dev_subserial, &rsc_info, &tsl_keyinfo, &tsl_value);
  if(EZ_TSL_ERR_PROFILE_LOADING == rv)
  {
    hal_thread_sleep(3000);
    rv = ez_iot_tsl_property_report(tsl_devinfo.dev_subserial, &rsc_info, &tsl_keyinfo, &tsl_value);
  }

  uassert_int_equal(ez_errno_succ, rv);
  hal_thread_sleep(3000);
  uassert_int_equal(ez_errno_succ, ez_iot_stop());
  uassert_int_equal(ez_errno_succ, ez_iot_tsl_deinit());
  ez_iot_fini();
}

void ut_tsl_prop_report_float()
{
  ez_core_err_e rv = ez_errno_succ;
  tsl_things_callbacks_t tsl_things_cbs = {tsl_things_action2dev, tsl_things_property2cloud, tsl_things_property2dev};
  tsl_devinfo_t tsl_devinfo = {(int8_t *)EZ_IOT_DEV_UUID, (int8_t *)EZ_IOT_DEV_PRODUCT_KEY, (int8_t *)EZ_IOT_DEV_FWVER};
  ez_tsl_key_t tsl_keyinfo = {.domain = (int8_t *)"attr_test", .key = (int8_t *)"attr_r_num"};
  ez_tsl_value_t tsl_value = {.size = sizeof(double), .type = tsl_data_type_double, .value_double = 10086.11112222};
  ez_tsl_rsc_t rsc_info = {.res_type = (int8_t *)"PetDryerRes", .local_index = (int8_t *)"0"};

  uassert_int_equal(ez_errno_succ, ez_iot_init(&m_lbs_addr, &m_dev_info, &m_cbs, &m_kv_cbs));
  uassert_int_equal(ez_errno_succ, ez_iot_tsl_init(&tsl_things_cbs));
  uassert_int_equal(ez_errno_succ, ez_iot_start());

  uassert_int_equal(ez_errno_succ, dev_event_waitfor(ez_iot_event_online, DEFAULT_TIMEOUT_S * 1000));
  rv = ez_iot_tsl_property_report(tsl_devinfo.dev_subserial, &rsc_info, &tsl_keyinfo, &tsl_value);
  if(EZ_TSL_ERR_PROFILE_LOADING == rv)
  {
    hal_thread_sleep(3000);
    rv = ez_iot_tsl_property_report(tsl_devinfo.dev_subserial, &rsc_info, &tsl_keyinfo, &tsl_value);
  }

  uassert_int_equal(ez_errno_succ, rv);
  hal_thread_sleep(3000);
  uassert_int_equal(ez_errno_succ, ez_iot_stop());
  uassert_int_equal(ez_errno_succ, ez_iot_tsl_deinit());
  ez_iot_fini();
}

void ut_tsl_prop_report_str()
{
  ez_core_err_e rv = ez_errno_succ;
  tsl_things_callbacks_t tsl_things_cbs = {tsl_things_action2dev, tsl_things_property2cloud, tsl_things_property2dev};
  tsl_devinfo_t tsl_devinfo = {(int8_t *)EZ_IOT_DEV_UUID, (int8_t *)EZ_IOT_DEV_PRODUCT_KEY, (int8_t *)EZ_IOT_DEV_FWVER};
  ez_tsl_key_t tsl_keyinfo = {.domain = (int8_t *)"attr_test", .key = (int8_t *)"attr_r_str"};
  ez_tsl_value_t tsl_value = {.size = strlen("test_str_111222333"), .type = tsl_data_type_string, .value = "test_str_111222333"};
  ez_tsl_rsc_t rsc_info = {.res_type = (int8_t *)"PetDryerRes", .local_index = (int8_t *)"0"};

  uassert_int_equal(ez_errno_succ, ez_iot_init(&m_lbs_addr, &m_dev_info, &m_cbs, &m_kv_cbs));
  uassert_int_equal(ez_errno_succ, ez_iot_tsl_init(&tsl_things_cbs));
  uassert_int_equal(ez_errno_succ, ez_iot_start());

  uassert_int_equal(ez_errno_succ, dev_event_waitfor(ez_iot_event_online, DEFAULT_TIMEOUT_S * 1000));
  rv = ez_iot_tsl_property_report(tsl_devinfo.dev_subserial, &rsc_info, &tsl_keyinfo, &tsl_value);
  if(EZ_TSL_ERR_PROFILE_LOADING == rv)
  {
    hal_thread_sleep(3000);
    rv = ez_iot_tsl_property_report(tsl_devinfo.dev_subserial, &rsc_info, &tsl_keyinfo, &tsl_value);
  }

  uassert_int_equal(ez_errno_succ, rv);
    hal_thread_sleep(3000);
  uassert_int_equal(ez_errno_succ, ez_iot_stop());
  uassert_int_equal(ez_errno_succ, ez_iot_tsl_deinit());
  ez_iot_fini();
}

void ut_tsl_prop_report_jobj()
{
  ez_core_err_e rv = ez_errno_succ;
  tsl_things_callbacks_t tsl_things_cbs = {tsl_things_action2dev, tsl_things_property2cloud, tsl_things_property2dev};
  tsl_devinfo_t tsl_devinfo = {(int8_t *)EZ_IOT_DEV_UUID, (int8_t *)EZ_IOT_DEV_PRODUCT_KEY, (int8_t *)EZ_IOT_DEV_FWVER};
  ez_tsl_key_t tsl_keyinfo = {.domain = (int8_t *)"attr_test", .key = (int8_t *)"attr_r_obj"};
  ez_tsl_value_t tsl_value = {.size = strlen("{\"test_int\":89622462}"), .type = tsl_data_type_object, .value = "{\"test_int\":89622462}"};
  ez_tsl_rsc_t rsc_info = {.res_type = (int8_t *)"PetDryerRes", .local_index = (int8_t *)"0"};

  uassert_int_equal(ez_errno_succ, ez_iot_init(&m_lbs_addr, &m_dev_info, &m_cbs, &m_kv_cbs));
  uassert_int_equal(ez_errno_succ, ez_iot_tsl_init(&tsl_things_cbs));
  uassert_int_equal(ez_errno_succ, ez_iot_start());

  uassert_int_equal(ez_errno_succ, dev_event_waitfor(ez_iot_event_online, DEFAULT_TIMEOUT_S * 1000));
  rv = ez_iot_tsl_property_report(tsl_devinfo.dev_subserial, &rsc_info, &tsl_keyinfo, &tsl_value);
  if(EZ_TSL_ERR_PROFILE_LOADING == rv)
  {
    hal_thread_sleep(3000);
    rv = ez_iot_tsl_property_report(tsl_devinfo.dev_subserial, &rsc_info, &tsl_keyinfo, &tsl_value);
  }

  uassert_int_equal(ez_errno_succ, rv);
  hal_thread_sleep(3000);
  uassert_int_equal(ez_errno_succ, ez_iot_stop());
  uassert_int_equal(ez_errno_succ, ez_iot_tsl_deinit());
  ez_iot_fini();
}

void ut_tsl_prop_report_jarr()
{
  ez_core_err_e rv = ez_errno_succ;
  tsl_things_callbacks_t tsl_things_cbs = {tsl_things_action2dev, tsl_things_property2cloud, tsl_things_property2dev};
  tsl_devinfo_t tsl_devinfo = {(int8_t *)EZ_IOT_DEV_UUID, (int8_t *)EZ_IOT_DEV_PRODUCT_KEY, (int8_t *)EZ_IOT_DEV_FWVER};
  ez_tsl_key_t tsl_keyinfo = {.domain = (int8_t *)"attr_test", .key = (int8_t *)"attr_r_array"};
  ez_tsl_value_t tsl_value = {.size = strlen("[-42570399,56004448]"), .type = tsl_data_type_array, .value = "[-42570399,56004448]"};
  ez_tsl_rsc_t rsc_info = {.res_type = (int8_t *)"PetDryerRes", .local_index = (int8_t *)"0"};

  uassert_int_equal(ez_errno_succ, ez_iot_init(&m_lbs_addr, &m_dev_info, &m_cbs, &m_kv_cbs));
  uassert_int_equal(ez_errno_succ, ez_iot_tsl_init(&tsl_things_cbs));
  uassert_int_equal(ez_errno_succ, ez_iot_start());

  uassert_int_equal(ez_errno_succ, dev_event_waitfor(ez_iot_event_online, DEFAULT_TIMEOUT_S * 1000));
  rv = ez_iot_tsl_property_report(tsl_devinfo.dev_subserial, &rsc_info, &tsl_keyinfo, &tsl_value);
  if(EZ_TSL_ERR_PROFILE_LOADING == rv)
  {
    hal_thread_sleep(3000);
    rv = ez_iot_tsl_property_report(tsl_devinfo.dev_subserial, &rsc_info, &tsl_keyinfo, &tsl_value);
  }

  uassert_int_equal(ez_errno_succ, rv);
  hal_thread_sleep(3000);
  uassert_int_equal(ez_errno_succ, ez_iot_stop());
  uassert_int_equal(ez_errno_succ, ez_iot_tsl_deinit());
  ez_iot_fini();
}

void ut_tsl_event_report_null()
{
  ez_core_err_e rv = ez_errno_succ;
  tsl_things_callbacks_t tsl_things_cbs = {tsl_things_action2dev, tsl_things_property2cloud, tsl_things_property2dev};
  tsl_devinfo_t tsl_devinfo = {(int8_t *)EZ_IOT_DEV_UUID, (int8_t *)EZ_IOT_DEV_PRODUCT_KEY, (int8_t *)EZ_IOT_DEV_FWVER};
  ez_tsl_key_t tsl_keyinfo = {.domain = (int8_t *)"event_test", .key = (int8_t *)"event_null"};
  ez_tsl_rsc_t rsc_info = {.res_type = (int8_t *)"PetDryerRes", .local_index = (int8_t *)"0"};

  uassert_int_equal(ez_errno_succ, ez_iot_init(&m_lbs_addr, &m_dev_info, &m_cbs, &m_kv_cbs));
  uassert_int_equal(ez_errno_succ, ez_iot_tsl_init(&tsl_things_cbs));
  uassert_int_equal(ez_errno_succ, ez_iot_start());

  uassert_int_equal(ez_errno_succ, dev_event_waitfor(ez_iot_event_online, DEFAULT_TIMEOUT_S * 1000));
  rv = ez_iot_tsl_event_report(tsl_devinfo.dev_subserial, &rsc_info, &tsl_keyinfo, NULL);
  if(EZ_TSL_ERR_PROFILE_LOADING == rv)
  {
    hal_thread_sleep(3000);
    rv = ez_iot_tsl_event_report(tsl_devinfo.dev_subserial, &rsc_info, &tsl_keyinfo, NULL);
  }

  uassert_int_equal(ez_errno_succ, rv);
  hal_thread_sleep(3000);
  uassert_int_equal(ez_errno_succ, ez_iot_stop());
  uassert_int_equal(ez_errno_succ, ez_iot_tsl_deinit());
  ez_iot_fini();
}

const char *js_rep = "{\"ext\":{\"psd\":\"consequat sit in\"},\"basic\":{\"dateTime\":\"2021-06-30aaaaaaaaaaaaaaa\",\"UUID\":\"1111111111111111111\"}}";

void ut_tsl_event_report_obj()
{
  ez_core_err_e rv = ez_errno_succ;
  tsl_things_callbacks_t tsl_things_cbs = {tsl_things_action2dev, tsl_things_property2cloud, tsl_things_property2dev};
  tsl_devinfo_t tsl_devinfo = {(int8_t *)EZ_IOT_DEV_UUID, (int8_t *)EZ_IOT_DEV_PRODUCT_KEY, (int8_t *)EZ_IOT_DEV_FWVER};
  ez_tsl_key_t tsl_keyinfo = {.domain = (int8_t *)"event_test", .key = (int8_t *)"event_ext"};
  ez_tsl_rsc_t rsc_info = {.res_type = (int8_t *)"PetDryerRes", .local_index = (int8_t *)"0"};
  ez_tsl_value_t tsl_value = {.size = strlen(js_rep), .type = tsl_data_type_object, .value = (int8_t *)js_rep};

  uassert_int_equal(ez_errno_succ, ez_iot_init(&m_lbs_addr, &m_dev_info, &m_cbs, &m_kv_cbs));
  uassert_int_equal(ez_errno_succ, ez_iot_tsl_init(&tsl_things_cbs));
  uassert_int_equal(ez_errno_succ, ez_iot_start());

  uassert_int_equal(ez_errno_succ, dev_event_waitfor(ez_iot_event_online, DEFAULT_TIMEOUT_S * 1000));
  rv = ez_iot_tsl_event_report(tsl_devinfo.dev_subserial, &rsc_info, &tsl_keyinfo, &tsl_value);
  if(EZ_TSL_ERR_PROFILE_LOADING == rv)
  {
    hal_thread_sleep(3000);
    rv = ez_iot_tsl_event_report(tsl_devinfo.dev_subserial, &rsc_info, &tsl_keyinfo, &tsl_value);
  }

  uassert_int_equal(ez_errno_succ, rv);
  hal_thread_sleep(3000);
  uassert_int_equal(ez_errno_succ, ez_iot_stop());
  uassert_int_equal(ez_errno_succ, ez_iot_tsl_deinit());
  ez_iot_fini();
}

void ut_tsl_event_report_obj2()
{
  ez_core_err_e rv = ez_errno_succ;
  tsl_things_callbacks_t tsl_things_cbs = {tsl_things_action2dev, tsl_things_property2cloud, tsl_things_property2dev};
  tsl_devinfo_t tsl_devinfo = {(int8_t *)EZ_IOT_DEV_UUID, (int8_t *)EZ_IOT_DEV_PRODUCT_KEY, (int8_t *)EZ_IOT_DEV_FWVER};
  ez_tsl_key_t tsl_keyinfo = {.domain = (int8_t *)"event_test", .key = (int8_t *)"event_notify"};
  ez_tsl_rsc_t rsc_info = {.res_type = (int8_t *)"PetDryerRes", .local_index = (int8_t *)"0"};
  ez_tsl_value_t tsl_value = {.size = strlen("{\"action\":8}"), .type = tsl_data_type_object, .value = (int8_t *)"{\"action\":8}"};

  uassert_int_equal(ez_errno_succ, ez_iot_init(&m_lbs_addr, &m_dev_info, &m_cbs, &m_kv_cbs));
  uassert_int_equal(ez_errno_succ, ez_iot_tsl_init(&tsl_things_cbs));
  uassert_int_equal(ez_errno_succ, ez_iot_start());

  uassert_int_equal(ez_errno_succ, dev_event_waitfor(ez_iot_event_online, DEFAULT_TIMEOUT_S * 1000));
  rv = ez_iot_tsl_event_report(tsl_devinfo.dev_subserial, &rsc_info, &tsl_keyinfo, &tsl_value);
  if(EZ_TSL_ERR_PROFILE_LOADING == rv)
  {
    hal_thread_sleep(3000);
    rv = ez_iot_tsl_event_report(tsl_devinfo.dev_subserial, &rsc_info, &tsl_keyinfo, &tsl_value);
  }

  uassert_int_equal(ez_errno_succ, rv);
  hal_thread_sleep(3000);
  uassert_int_equal(ez_errno_succ, ez_iot_stop());
  uassert_int_equal(ez_errno_succ, ez_iot_tsl_deinit());
  ez_iot_fini();
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

    hal_thread_sleep(10);
    index += 10;
  } while (index < time_ms);

  return ret;
}

static int32_t ez_recv_msg_cb(ez_iot_cloud2dev_msg_type_t msg_type, void *data, int len)
{
  return 0;
}

static int32_t ez_recv_event_cb(ez_iot_event_t event_type, void *data, int len)
{
  switch (event_type)
  {
  case ez_iot_event_online:
    m_event_id = ez_iot_event_online;
    break;
  case ez_iot_event_devid_update:
    /* save devid */
    break;

  default:
    break;
  }

  return 0;
}