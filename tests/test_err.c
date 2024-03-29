#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include "err.h"

static void test_err(void **status)
{
    assert_string_equal(errx_strerr(ERRX_EPERM), "no permission");
    assert_string_equal(errx_strerr(ERRX_EINVAL), "invalid argument");

    assert_string_equal(errx_strerr(10010), "Unknown errno");
    assert_int_equal(errx_register(10010, "test errno 10010"), 0);
    assert_string_equal(errx_strerr(10010), "test errno 10010");
    assert_int_equal(errx_register(10010, "test errno 10010"), -1);

    errx_register(10020, "test errno 10020");
    assert_string_equal(errx_strerr(10020), "test errno 10020");
    errx_register(10030, "test errno 10030");
    assert_string_equal(errx_strerr(10030), "test errno 10030");
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_err),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
