#include <uzmtp/uzmtp.h>

#include <setjmp.h>

#include <cmocka.h>

static void
test_zmtp_msg_new(void** context_p)
{
    ((void)context_p);
    uzmtp_msg_s* msg = uzmtp_msg_new(0, 255);
    assert_non_null(msg);
    assert_false(uzmtp_msg_is_more(msg));
    assert_false(uzmtp_msg_is_large(msg));
    assert_int_equal(uzmtp_msg_size(msg), 255);
    assert_null(uzmtp_msg_next(msg));
    uzmtp_msg_destroy(&msg);
    assert_null(msg);
}

static void
test_zmtp_msg_new_large(void** context_p)
{
    ((void)context_p);
    uzmtp_msg_s* msg = uzmtp_msg_new(0, 256);
    assert_non_null(msg);
    assert_false(uzmtp_msg_is_more(msg));
    assert_true(uzmtp_msg_is_large(msg));
    assert_int_equal(uzmtp_msg_size(msg), 256);
    assert_null(uzmtp_msg_next(msg));
    uzmtp_msg_destroy(&msg);
    assert_null(msg);
}

static void
test_zmtp_msg_new_large_more(void** context_p)
{
    ((void)context_p);
    uzmtp_msg_s* msg = uzmtp_msg_new(UZMTP_MSG_MORE, 256);
    assert_non_null(msg);
    assert_true(uzmtp_msg_is_more(msg));
    assert_true(uzmtp_msg_is_large(msg));
    assert_int_equal(uzmtp_msg_size(msg), 256);
    assert_null(uzmtp_msg_next(msg));
    uzmtp_msg_destroy(&msg);
    assert_null(msg);
}

static void
test_zmtp_msg_new_from_data(void** context_p)
{
    ((void)context_p);
    uint8_t expect[255], *data = uzmtp_malloc(255);
    memset(data, 'a', 255);
    memset(expect, 'a', 255);
    uzmtp_msg_s* msg = uzmtp_msg_new_from_data(0, &data, 255);
    assert_non_null(msg);
    assert_null(data);
    assert_false(uzmtp_msg_is_more(msg));
    assert_false(uzmtp_msg_is_large(msg));
    assert_int_equal(uzmtp_msg_size(msg), 255);
    assert_memory_equal(uzmtp_msg_data(msg), expect, 255);
    assert_null(uzmtp_msg_next(msg));
    uzmtp_msg_destroy(&msg);
    assert_null(msg);
}

static void
test_zmtp_msg_new_from_data_large(void** context_p)
{
    ((void)context_p);
    uint8_t expect[256], *data = uzmtp_malloc(256);
    memset(data, 'a', 256);
    memset(expect, 'a', 256);
    uzmtp_msg_s* msg = uzmtp_msg_new_from_data(0, &data, 256);
    assert_non_null(msg);
    assert_null(data);
    assert_false(uzmtp_msg_is_more(msg));
    assert_true(uzmtp_msg_is_large(msg));
    assert_int_equal(uzmtp_msg_size(msg), 256);
    assert_memory_equal(uzmtp_msg_data(msg), expect, 256);
    assert_null(uzmtp_msg_next(msg));
    uzmtp_msg_destroy(&msg);
    assert_null(msg);
}

static void
test_zmtp_msg_new_from_data_large_more(void** context_p)
{
    ((void)context_p);
    uint8_t expect[256], *data = uzmtp_malloc(256);
    memset(data, 'a', 256);
    memset(expect, 'a', 256);
    uzmtp_msg_s* msg = uzmtp_msg_new_from_data(UZMTP_MSG_MORE, &data, 256);
    assert_non_null(msg);
    assert_null(data);
    assert_true(uzmtp_msg_is_more(msg));
    assert_true(uzmtp_msg_is_large(msg));
    assert_int_equal(uzmtp_msg_size(msg), 256);
    assert_memory_equal(uzmtp_msg_data(msg), expect, 256);
    assert_null(uzmtp_msg_next(msg));
    uzmtp_msg_destroy(&msg);
    assert_null(msg);
}

static void
test_zmtp_msg_new_from_const_data(void** context_p)
{
    ((void)context_p);
    char data[255], expect[255];
    memset(data, 'a', 255);
    memset(expect, 'a', 255);
    uzmtp_msg_s* msg = uzmtp_msg_new_from_const_data(0, data, 255);
    assert_non_null(msg);
    assert_false(uzmtp_msg_is_more(msg));
    assert_false(uzmtp_msg_is_large(msg));
    assert_int_equal(uzmtp_msg_size(msg), 255);
    assert_memory_equal(uzmtp_msg_data(msg), expect, 255);
    assert_null(uzmtp_msg_next(msg));
    uzmtp_msg_destroy(&msg);
    assert_null(msg);
}

static void
test_zmtp_msg_new_from_const_data_large(void** context_p)
{
    ((void)context_p);
    char data[256], expect[256];
    memset(data, 'a', 256);
    memset(expect, 'a', 256);
    uzmtp_msg_s* msg = uzmtp_msg_new_from_const_data(0, data, 256);
    assert_non_null(msg);
    assert_false(uzmtp_msg_is_more(msg));
    assert_true(uzmtp_msg_is_large(msg));
    assert_int_equal(uzmtp_msg_size(msg), 256);
    assert_memory_equal(uzmtp_msg_data(msg), expect, 256);
    assert_null(uzmtp_msg_next(msg));
    uzmtp_msg_destroy(&msg);
    assert_null(msg);
}

static void
test_zmtp_msg_new_from_const_data_large_more(void** context_p)
{
    ((void)context_p);
    char data[256], expect[256];
    memset(data, 'a', 256);
    memset(expect, 'a', 256);
    uzmtp_msg_s* msg = uzmtp_msg_new_from_const_data(UZMTP_MSG_MORE, data, 256);
    assert_non_null(msg);
    assert_true(uzmtp_msg_is_more(msg));
    assert_true(uzmtp_msg_is_large(msg));
    assert_int_equal(uzmtp_msg_size(msg), 256);
    assert_memory_equal(uzmtp_msg_data(msg), expect, 256);
    assert_null(uzmtp_msg_next(msg));
    uzmtp_msg_destroy(&msg);
    assert_null(msg);
}

static void
test_zmtp_msg_set_get_clr_more(void** context_p)
{
    ((void)context_p);
    uzmtp_msg_s* msg = uzmtp_msg_new(0, 256);
    assert_false(uzmtp_msg_is_more(msg));
    assert_int_equal(uzmtp_msg_flags(msg), UZMTP_MSG_LARGE);
    uzmtp_msg_set_more(msg);
    assert_true(uzmtp_msg_is_more(msg));
    assert_int_equal(uzmtp_msg_flags(msg), UZMTP_MSG_MORE | UZMTP_MSG_LARGE);
    uzmtp_msg_clr_more(msg);
    assert_false(uzmtp_msg_is_more(msg));
    assert_int_equal(uzmtp_msg_flags(msg), UZMTP_MSG_LARGE);
    uzmtp_msg_clr_more(msg);
    uzmtp_msg_destroy(&msg);
}

int
zmtp_msg_tests()
{
    int err;

    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_zmtp_msg_new),
        cmocka_unit_test(test_zmtp_msg_new_large),
        cmocka_unit_test(test_zmtp_msg_new_large_more),
        cmocka_unit_test(test_zmtp_msg_new_from_data),
        cmocka_unit_test(test_zmtp_msg_new_from_data_large),
        cmocka_unit_test(test_zmtp_msg_new_from_data_large_more),
        cmocka_unit_test(test_zmtp_msg_new_from_const_data),
        cmocka_unit_test(test_zmtp_msg_new_from_const_data_large),
        cmocka_unit_test(test_zmtp_msg_new_from_const_data_large_more),
        cmocka_unit_test(test_zmtp_msg_set_get_clr_more)
    };

    err = cmocka_run_group_tests(tests, NULL, NULL);

    return err;
}
