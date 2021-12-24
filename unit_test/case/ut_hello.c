
#include "utest.h"

static void ut_hello(void)
{
    char a[10] = {0};
    char b[10] = {0};
    char c[10] = {2};

    uassert_not_null(!NULL);
    uassert_int_equal(1, 1);
    uassert_int_not_equal(1, 2);
    uassert_str_equal("1", "1");
    uassert_str_not_equal("1", "2");

    uassert_buf_equal(a, b, sizeof(a));
    uassert_buf_not_equal(a, c, sizeof(a));

    uassert_in_range(1, 0, 2);
    uassert_not_in_range(3, 0, 2);
}

static void testcase(void)
{
    UTEST_UNIT_RUN(ut_hello);
}
UTEST_TC_EXPORT(testcase, "eziot.ut_hello", NULL, NULL, CONFIG_EZIOT_UNIT_TEST_CASE_TIEMOUT_SECONDS);

// utest_log_lv_set(1)
// [==========] [ utest    ] loop 1/1
// [==========] [ utest    ] started
// [----------] [ testcase ] (ut_hello) started
// [    OK    ] [ unit     ] (ut_hello:13) is passed
// [    OK    ] [ unit     ] (ut_hello:14) is passed
// [    OK    ] [ unit     ] (ut_hello:15) is passed
// [    OK    ] [ unit     ] (ut_hello:16) is passed
// [    OK    ] [ unit     ] (ut_hello:17) is passed
// [    OK    ] [ unit     ] (ut_hello:18) is passed
// [    OK    ] [ unit     ] (ut_hello:19) is passed
// [    OK    ] [ unit     ] (ut_hello:20) is passed
// [    OK    ] [ unit     ] (ut_hello:22) is passed
// [    OK    ] [ unit     ] (ut_hello:23) is passed
// [    OK    ] [ unit     ] (ut_hello:25) is passed
// [    OK    ] [ unit     ] (ut_hello:26) is passed
// [  PASSED  ] [ result   ] testcase (ut_hello)
// [----------] [ testcase ] (ut_hello) finished
// [==========] [ utest    ] finished

// utest_log_lv_set(2)
// [==========] [ utest    ] loop 1/1
// [==========] [ utest    ] started
// [----------] [ testcase ] (ut_hello) started
// [  PASSED  ] [ result   ] testcase (ut_hello)
// [----------] [ testcase ] (ut_hello) finished
// [==========] [ utest    ] finished