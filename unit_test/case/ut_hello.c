
#include "utest.h"

static void ut_hello();
UTEST_TC_EXPORT(ut_hello, NULL, NULL, 10);

static void ut_hello()
{
    char a[10] = {0};
    char b[10] = {0};
    char c[10] = {2};

    uassert_true(true);
    uassert_false(false);
    uassert_null(NULL);
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