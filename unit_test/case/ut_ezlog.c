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
 * XuRongjun (xurongjun@ezvizlife.com) ezlog component unit-test case
 *******************************************************************************/

#define TAG_FILTER "T_APP_FILTER"
#define KEYWORD_FILTER "ezapp"
#define SHOW_DELAY_TIME_MS 3000

#include <ezos.h>
#include <utest.h>
#include <ezlog.h>

static void ut_log_init();
UTEST_TC_EXPORT(ut_log_init, NULL, NULL, CONFIG_EZIOT_UNIT_TEST_CASE_TIEMOUT_SECONDS);

static void ut_log_lvl_a();
UTEST_TC_EXPORT(ut_log_lvl_a, NULL, NULL, CONFIG_EZIOT_UNIT_TEST_CASE_TIEMOUT_SECONDS);

static void ut_log_lvl_e();
UTEST_TC_EXPORT(ut_log_lvl_e, NULL, NULL, CONFIG_EZIOT_UNIT_TEST_CASE_TIEMOUT_SECONDS);

static void ut_log_lvl_w();
UTEST_TC_EXPORT(ut_log_lvl_w, NULL, NULL, CONFIG_EZIOT_UNIT_TEST_CASE_TIEMOUT_SECONDS);

static void ut_log_lvl_i();
UTEST_TC_EXPORT(ut_log_lvl_i, NULL, NULL, CONFIG_EZIOT_UNIT_TEST_CASE_TIEMOUT_SECONDS);

static void ut_log_lvl_d();
UTEST_TC_EXPORT(ut_log_lvl_d, NULL, NULL, CONFIG_EZIOT_UNIT_TEST_CASE_TIEMOUT_SECONDS);

static void ut_log_lvl_v();
UTEST_TC_EXPORT(ut_log_lvl_v, NULL, NULL, CONFIG_EZIOT_UNIT_TEST_CASE_TIEMOUT_SECONDS);

static void ut_log_tag();
UTEST_TC_EXPORT(ut_log_tag, NULL, NULL, CONFIG_EZIOT_UNIT_TEST_CASE_TIEMOUT_SECONDS);

static void ut_log_kw();
UTEST_TC_EXPORT(ut_log_kw, NULL, NULL, CONFIG_EZIOT_UNIT_TEST_CASE_TIEMOUT_SECONDS);

static void ut_log_dump16();
UTEST_TC_EXPORT(ut_log_dump16, NULL, NULL, CONFIG_EZIOT_UNIT_TEST_CASE_TIEMOUT_SECONDS);

static void ut_log_dump32();
UTEST_TC_EXPORT(ut_log_dump32, NULL, NULL, CONFIG_EZIOT_UNIT_TEST_CASE_TIEMOUT_SECONDS);

static void test_log_lvl(void);
static void test_log_tag(void);
static void test_log_kw(void);

static void ut_log_init()
{
    uassert_int_equal(0, ezlog_init());
    ezlog_start();
    ezlog_stop();
}

static void ut_log_lvl_a()
{
    uassert_int_equal(0, ezlog_init());
    ezlog_start();

    ezlog_filter_lvl(EZ_ELOG_LVL_ASSERT);
    test_log_lvl();
    ezlog_stop();
}

static void ut_log_lvl_e()
{
    uassert_int_equal(0, ezlog_init());
    ezlog_start();

    ezlog_filter_lvl(EZ_ELOG_LVL_ERROR);
    test_log_lvl();
    ezlog_stop();
}

static void ut_log_lvl_w()
{
    uassert_int_equal(0, ezlog_init());
    ezlog_start();

    ezlog_filter_lvl(EZ_ELOG_LVL_WARN);
    test_log_lvl();
    ezlog_stop();
}

static void ut_log_lvl_i()
{
    uassert_int_equal(0, ezlog_init());
    ezlog_start();

    ezlog_filter_lvl(EZ_ELOG_LVL_INFO);
    test_log_lvl();
    ezlog_stop();
}

static void ut_log_lvl_d()
{
    uassert_int_equal(0, ezlog_init());
    ezlog_start();

    ezlog_filter_lvl(EZ_ELOG_LVL_DEBUG);
    test_log_lvl();
    ezlog_stop();
}

static void ut_log_lvl_v()
{
    uassert_int_equal(0, ezlog_init());
    ezlog_start();

    ezlog_filter_lvl(EZ_ELOG_LVL_VERBOSE);
    test_log_lvl();
    ezlog_stop();
}

static void ut_log_tag()
{
    uassert_int_equal(0, ezlog_init());
    ezlog_start();

    ezlog_filter_lvl(EZ_ELOG_LVL_VERBOSE);
    ezlog_filter_tag(TAG_FILTER);
    test_log_lvl();
    test_log_tag();
    ezlog_filter_tag("");
    ezlog_stop();
}

static void ut_log_kw()
{
    uassert_int_equal(0, ezlog_init());
    ezlog_start();

    ezlog_filter_lvl(EZ_ELOG_LVL_VERBOSE);
    ezlog_filter_kw(KEYWORD_FILTER);
    test_log_lvl();
    test_log_tag();
    test_log_kw();
    ezlog_filter_kw("");
    ezlog_stop();
}

static void ut_log_dump16()
{
    ez_uint8_t buf[32] = {0};
    int loglvl = (CONFIG_EZIOT_UNIT_TEST_SDK_LOGLVL < 0) ? 0 : CONFIG_EZIOT_UNIT_TEST_SDK_LOGLVL;

    uassert_int_equal(0, ezlog_init());
    ezlog_start();

    ezlog_filter_lvl(loglvl);
    ezlog_hexdump(TAG_APP, 16, buf, sizeof(buf));
}

static void ut_log_dump32()
{
    ez_uint8_t buf[32] = {0};
    int loglvl = (CONFIG_EZIOT_UNIT_TEST_SDK_LOGLVL < 0) ? 0 : CONFIG_EZIOT_UNIT_TEST_SDK_LOGLVL;
    uassert_int_equal(0, ezlog_init());
    ezlog_start();

    ezlog_filter_lvl(loglvl);
    ezlog_hexdump(TAG_APP, 32, buf, sizeof(buf));
}

static void test_log_lvl(void)
{
    if (CONFIG_EZIOT_UNIT_TEST_SDK_LOGLVL >= EZ_ELOG_LVL_ASSERT)
    {
        ezlog_a(TAG_APP, "hello word!");
    }
    if (CONFIG_EZIOT_UNIT_TEST_SDK_LOGLVL >= EZ_ELOG_LVL_ERROR)
    {
        ezlog_e(TAG_APP, "hello word!");
    }
    if (CONFIG_EZIOT_UNIT_TEST_SDK_LOGLVL >= EZ_ELOG_LVL_WARN)
    {
        ezlog_w(TAG_APP, "hello word!");
    }
    if (CONFIG_EZIOT_UNIT_TEST_SDK_LOGLVL >= EZ_ELOG_LVL_INFO)
    {
        ezlog_i(TAG_APP, "hello word!");
    }
    if (CONFIG_EZIOT_UNIT_TEST_SDK_LOGLVL >= EZ_ELOG_LVL_DEBUG)
    {
        ezlog_d(TAG_APP, "hello word!");
    }
    if (CONFIG_EZIOT_UNIT_TEST_SDK_LOGLVL >= EZ_ELOG_LVL_VERBOSE)
    {
        ezlog_v(TAG_APP, "hello word!");
    }
}

static void test_log_tag(void)
{
    if (CONFIG_EZIOT_UNIT_TEST_SDK_LOGLVL >= EZ_ELOG_LVL_ASSERT)
    {
        ezlog_a(TAG_FILTER, "hello word!");
    }
    if (CONFIG_EZIOT_UNIT_TEST_SDK_LOGLVL >= EZ_ELOG_LVL_ERROR)
    {
        ezlog_e(TAG_FILTER, "hello word!");
    }
    if (CONFIG_EZIOT_UNIT_TEST_SDK_LOGLVL >= EZ_ELOG_LVL_WARN)
    {
        ezlog_w(TAG_FILTER, "hello word!");
    }
    if (CONFIG_EZIOT_UNIT_TEST_SDK_LOGLVL >= EZ_ELOG_LVL_INFO)
    {
        ezlog_i(TAG_FILTER, "hello word!");
    }
    if (CONFIG_EZIOT_UNIT_TEST_SDK_LOGLVL >= EZ_ELOG_LVL_DEBUG)
    {
        ezlog_d(TAG_FILTER, "hello word!");
    }
    if (CONFIG_EZIOT_UNIT_TEST_SDK_LOGLVL >= EZ_ELOG_LVL_VERBOSE)
    {
        ezlog_v(TAG_FILTER, "hello word!");
    }
}

static void test_log_kw(void)
{
    if (CONFIG_EZIOT_UNIT_TEST_SDK_LOGLVL >= EZ_ELOG_LVL_ASSERT)
    {
        ezlog_a(TAG_APP, "ezapp, easy your life!");
    }
    if (CONFIG_EZIOT_UNIT_TEST_SDK_LOGLVL >= EZ_ELOG_LVL_ERROR)
    {
        ezlog_e(TAG_APP, "ezapp, easy your life!");
    }
    if (CONFIG_EZIOT_UNIT_TEST_SDK_LOGLVL >= EZ_ELOG_LVL_WARN)
    {
        ezlog_w(TAG_APP, "ezapp, easy your life!");
    }
    if (CONFIG_EZIOT_UNIT_TEST_SDK_LOGLVL >= EZ_ELOG_LVL_INFO)
    {
        ezlog_i(TAG_APP, "ezapp, easy your life!");
    }
    if (CONFIG_EZIOT_UNIT_TEST_SDK_LOGLVL >= EZ_ELOG_LVL_DEBUG)
    {
        ezlog_d(TAG_APP, "ezapp, easy your life!");
    }
    if (CONFIG_EZIOT_UNIT_TEST_SDK_LOGLVL >= EZ_ELOG_LVL_VERBOSE)
    {
        ezlog_v(TAG_APP, "ezapp, easy your life!");
    }
}