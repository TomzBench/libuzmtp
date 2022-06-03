#include <uzmtp/uzmtp.h>

#include <setjmp.h>

#include <cmocka.h>

static void
test_zmtp_msg_init(void** context_p)
{
    ((void)context_p);
    uzmtp_msg_s msg;
    uzmtp_msg_init(&msg, 0, NULL, 255);
    assert_false(uzmtp_msg_is_more(&msg));
    assert_false(uzmtp_msg_is_large(&msg));
    assert_int_equal(uzmtp_msg_size(&msg), 255);
    uzmtp_msg_deinit(&msg);
}

static void
test_zmtp_msg_init_large(void** context_p)
{
    ((void)context_p);
    uzmtp_msg_s msg;
    uzmtp_msg_init(&msg, 0, NULL, 256);
    assert_false(uzmtp_msg_is_more(&msg));
    assert_true(uzmtp_msg_is_large(&msg));
    assert_int_equal(uzmtp_msg_size(&msg), 256);
    uzmtp_msg_deinit(&msg);
}

static void
test_zmtp_msg_init_large_more(void** context_p)
{
    ((void)context_p);
    uzmtp_msg_s msg;
    uzmtp_msg_init(&msg, UZMTP_MSG_MORE, NULL, 256);
    assert_true(uzmtp_msg_is_more(&msg));
    assert_true(uzmtp_msg_is_large(&msg));
    assert_int_equal(uzmtp_msg_size(&msg), 256);
    uzmtp_msg_deinit(&msg);
}

static void
test_zmtp_msg_init_str(void** context_p)
{

    ((void)context_p);
    uzmtp_msg_s msg;
    uzmtp_msg_init_str(&msg, UZMTP_MSG_MORE, "hello");
    assert_true(uzmtp_msg_is_more(&msg));
    assert_false(uzmtp_msg_is_large(&msg));
    assert_int_equal(uzmtp_msg_size(&msg), 5);
    uzmtp_msg_deinit(&msg);
}

static void
test_zmtp_msg_init_mem(void** context_p)
{
    ((void)context_p);
    uint8_t expect[255], *data = uzmtp_malloc(255);
    memset(data, 'a', 255);
    memset(expect, 'a', 255);
    uzmtp_msg_s msg;
    uzmtp_msg_init_mem(&msg, 0, &data, 255);
    assert_null(data);
    assert_false(uzmtp_msg_is_more(&msg));
    assert_false(uzmtp_msg_is_large(&msg));
    assert_int_equal(uzmtp_msg_size(&msg), 255);
    assert_memory_equal(uzmtp_msg_data(&msg), expect, 255);
    uzmtp_msg_deinit(&msg);
}

static void
test_zmtp_msg_init_mem_large(void** context_p)
{
    ((void)context_p);
    uint8_t expect[256], *data = uzmtp_malloc(256);
    memset(data, 'a', 256);
    memset(expect, 'a', 256);
    uzmtp_msg_s msg;
    uzmtp_msg_init_mem(&msg, 0, &data, 256);
    assert_null(data);
    assert_false(uzmtp_msg_is_more(&msg));
    assert_true(uzmtp_msg_is_large(&msg));
    assert_int_equal(uzmtp_msg_size(&msg), 256);
    assert_memory_equal(uzmtp_msg_data(&msg), expect, 256);
    uzmtp_msg_deinit(&msg);
}

static void
test_zmtp_msg_init_mem_large_more(void** context_p)
{
    ((void)context_p);
    uint8_t expect[256], *data = uzmtp_malloc(256);
    memset(data, 'a', 256);
    memset(expect, 'a', 256);
    uzmtp_msg_s msg;
    uzmtp_msg_init_mem(&msg, UZMTP_MSG_MORE, &data, 256);
    assert_null(data);
    assert_true(uzmtp_msg_is_more(&msg));
    assert_true(uzmtp_msg_is_large(&msg));
    assert_int_equal(uzmtp_msg_size(&msg), 256);
    assert_memory_equal(uzmtp_msg_data(&msg), expect, 256);
    uzmtp_msg_deinit(&msg);
}

static void
test_zmtp_msg_set_get_clr_more(void** context_p)
{
    ((void)context_p);
    uzmtp_msg_s msg;
    uzmtp_msg_init(&msg, 0, "", 1);
    assert_false(uzmtp_msg_is_more(&msg));
    assert_int_equal(uzmtp_msg_flags(&msg), 0);
    uzmtp_msg_flags_set(&msg, UZMTP_MSG_MORE);
    assert_true(uzmtp_msg_is_more(&msg));
    assert_int_equal(uzmtp_msg_flags(&msg), UZMTP_MSG_MORE);
    uzmtp_msg_flags_clr(&msg, UZMTP_MSG_MORE);
    assert_false(uzmtp_msg_is_more(&msg));
    assert_int_equal(uzmtp_msg_flags(&msg), 0);
    uzmtp_msg_flags_clr(&msg, UZMTP_MSG_MORE);
    uzmtp_msg_deinit(&msg);
}

int
main(void)
{
    int err;

    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_zmtp_msg_init),
        cmocka_unit_test(test_zmtp_msg_init_large),
        cmocka_unit_test(test_zmtp_msg_init_large_more),
        cmocka_unit_test(test_zmtp_msg_init_str),
        cmocka_unit_test(test_zmtp_msg_init_mem),
        cmocka_unit_test(test_zmtp_msg_init_mem_large),
        cmocka_unit_test(test_zmtp_msg_init_mem_large_more),
        cmocka_unit_test(test_zmtp_msg_set_get_clr_more),
    };

    err = cmocka_run_group_tests(tests, NULL, NULL);

    return err;
}
