#include "helpers.h"

#define RECV_LARGE_SZ 20480
#define RECV_LARGE_DATA 'A'

/*
static int
test_cb_on_recv_hello(uzmtp_dealer_s* dealer, uint32_t n)
{
    n_recv++;
    uzmtp_msg_s* msg;
    assert_int_equal(n, 2);
    assert_int_equal(uzmtp_dealer_incoming_count(dealer), 2);

    msg = uzmtp_dealer_pop_incoming(dealer);
    assert_non_null(msg);
    assert_int_equal(uzmtp_dealer_incoming_count(dealer), 1);
    assert_int_equal(uzmtp_msg_size(msg), 5);
    assert_memory_equal(uzmtp_msg_data(msg), "hello", 5);
    uzmtp_msg_destroy(&msg);

    msg = uzmtp_dealer_pop_incoming(dealer);
    assert_non_null(msg);
    assert_int_equal(uzmtp_dealer_incoming_count(dealer), 0);
    assert_int_equal(uzmtp_msg_size(msg), 5);
    assert_memory_equal(uzmtp_msg_data(msg), "world", 5);
    uzmtp_msg_destroy(&msg);
    return 0;
}

static int
test_cb_on_recv_large(uzmtp_dealer_s* dealer, uint32_t n)
{
    uint8_t expect[RECV_LARGE_SZ];
    uzmtp_msg_s* msg;

    memset(expect, RECV_LARGE_DATA, RECV_LARGE_SZ);
    n_recv++;

    msg = uzmtp_dealer_pop_incoming(dealer);
    assert_non_null(msg);
    assert_int_equal(n, 1);
    assert_int_equal(uzmtp_dealer_incoming_count(dealer), 0);
    assert_int_equal(uzmtp_msg_size(msg), RECV_LARGE_SZ);
    assert_memory_equal(uzmtp_msg_data(msg), expect, RECV_LARGE_SZ);

    uzmtp_msg_destroy(&msg);

    return 0;
}

static int
test_cb_on_recv_large_more(uzmtp_dealer_s* dealer, uint32_t n)
{
    uint8_t expect[RECV_LARGE_SZ];
    memset(expect, RECV_LARGE_DATA, RECV_LARGE_SZ);

    n_recv++;
    uzmtp_msg_s* msg;
    assert_int_equal(n, 3);
    assert_int_equal(uzmtp_dealer_incoming_count(dealer), 3);

    msg = uzmtp_dealer_pop_incoming(dealer);
    assert_non_null(msg);
    assert_int_equal(uzmtp_dealer_incoming_count(dealer), 2);
    assert_int_equal(uzmtp_msg_size(msg), 5);
    assert_memory_equal(uzmtp_msg_data(msg), "hello", 5);
    uzmtp_msg_destroy(&msg);

    msg = uzmtp_dealer_pop_incoming(dealer);
    assert_non_null(msg);
    assert_int_equal(uzmtp_dealer_incoming_count(dealer), 1);
    assert_int_equal(uzmtp_msg_size(msg), 5);
    assert_memory_equal(uzmtp_msg_data(msg), "world", 5);
    uzmtp_msg_destroy(&msg);

    msg = uzmtp_dealer_pop_incoming(dealer);
    assert_non_null(msg);
    assert_int_equal(uzmtp_dealer_incoming_count(dealer), 0);
    assert_int_equal(uzmtp_msg_size(msg), RECV_LARGE_SZ);
    assert_memory_equal(uzmtp_msg_data(msg), expect, RECV_LARGE_SZ);
    uzmtp_msg_destroy(&msg);

    return 0;
}

static int
test_cb_on_recv_error(uzmtp_dealer_s* dealer, uint32_t n)
{
    ((void)dealer);
    ((void)n);
    return -1;
}

static int
test_cb_write_err(uzmtp_dealer_s* dealer, const uint8_t* b, uint32_t sz)
{
    ((void)dealer);
    ((void)b);
    ((void)sz);
    n_want_write++;
    return -1;
}

static void
test_cb_on_error(uzmtp_dealer_s* d, EUZMTP_ERROR error)
{
    ((void)d);
    n_error++;
    last_error = error;
}

uzmtp_dealer_settings test_settings_ok = { //
    .want_write = test_cb_write_ok,
    .on_recv = test_cb_on_recv_hello,
    .on_error = test_cb_on_error
};
uzmtp_dealer_settings test_settings_err = { //
    .want_write = test_cb_write_err,
    .on_recv = test_cb_on_recv_hello,
    .on_error = test_cb_on_error
};
uzmtp_dealer_settings test_settings_recv_err = { //
    .want_write = test_cb_write_ok,
    .on_recv = test_cb_on_recv_error,
    .on_error = test_cb_on_error
};
uzmtp_dealer_settings test_settings_recv_large = { //
    .want_write = test_cb_write_ok,
    .on_recv = test_cb_on_recv_large,
    .on_error = test_cb_on_error
};
uzmtp_dealer_settings test_settings_recv_large_more = { //
    .want_write = test_cb_write_ok,
    .on_recv = test_cb_on_recv_large_more,
    .on_error = test_cb_on_error
};
*/

static int
test_send_ok(uzmtp_dealer_s* dealer, const uint8_t* b, uint32_t sz)
{
    return 0;
}

// Test start
static void
test_zmtp_dealer_init(void** context_p)
{
    ((void)context_p);
    void* data = NULL;

    // Allocator
    uzmtp_dealer_s dealer;
    uzmtp_dealer_init(&dealer, NULL, data);
    assert_ptr_equal(uzmtp_dealer_context(&dealer), data);
}

// Callbacks
static int
test_send_greeting_ok(uzmtp_dealer_s* dealer, const uint8_t* b, uint32_t sz)
{
    int connection;
    connection = *(int*)uzmtp_dealer_connection(dealer);
    test_assert_valid_greeting(b, sz);
    *(int*)uzmtp_dealer_context(dealer) = 1;
    return 0;
}

static void
test_zmtp_dealer_connect_send_greeting_ok(void** context_p)
{
    ((void)context_p);
    int err, pass = 0, connection = 0;
    mock_packet_s pack[8];
    uzmtp_dealer_s d;
    uzmtp_dealer_init(&d, test_send_greeting_ok, &pass);

    // Connect and verify outgoing message is sent by reading back outgoing
    // bytes.  Verify state machine in correct state.
    err = uzmtp_dealer_connect(&d, &connection);
    assert_int_equal(err, 0);
    assert_int_equal(UZMTP_CONNECT_WANT_GREETING, uzmtp_dealer_state(&d));
    assert_int_equal(pass, 1);

    // Cleanup
    uzmtp_dealer_deinit(&d);
    test_state_reset();
}

static int
test_send_greeting_error(uzmtp_dealer_s* dealer, const uint8_t* b, uint32_t sz)
{
    return -1;
}

static void
test_zmtp_dealer_connect_send_greeting_error(void** context_p)
{
    ((void)context_p);
    int err, pass = 0, connection = 0;
    mock_packet_s* pack;

    // Connect and verify outgoing message is sent by reading back outgoing
    // bytes.  Verify state machine in correct state.
    uzmtp_dealer_s d;
    uzmtp_dealer_init(&d, test_send_greeting_error, &pass);
    err = uzmtp_dealer_connect(&d, &connection);
    assert_int_equal(UZMTP_ERROR_SEND, err);
    assert_int_equal(UZMTP_NULL, uzmtp_dealer_state(&d));

    // Cleanup
    uzmtp_dealer_deinit(&d);
    test_state_reset();
}

static int
test_send_recv_greeting_ok(uzmtp_dealer_s* d, const uint8_t* b, uint32_t sz)
{
    static int state = 0;
    switch (state) {
        case 0: state++; break;
        case 1:
            test_assert_valid_header(b, sz, UZMTP_MSG_COMMAND, 6);
            state++;
            break;
        case 2:
            test_assert_valid_ready(b, sz);
            state++;
            *(int*)uzmtp_dealer_context(d) = 1;
            break;
    }
    return 0;
}

static void
test_zmtp_dealer_connect_recv_greeting_ok(void** context_p)
{
    ((void)context_p);
    int err, pass = 0, connection = 0;
    uint8_t greeting[64];
    uzmtp_msg_s msgs[1];

    uzmtp_dealer_s d;
    uzmtp_dealer_init(&d, test_send_recv_greeting_ok, &pass);
    err = uzmtp_dealer_connect(&d, &connection);

    // Parse incoming greeting and verify outgoing ready
    test_print_greeting(greeting);
    err = uzmtp_dealer_parse(&d, greeting, sizeof(greeting), msgs, 1);
    assert_int_equal(err, UZMTP_WANT_MORE);
    assert_int_equal(UZMTP_CONNECT_WANT_READY, uzmtp_dealer_state(&d));
    assert_int_equal(pass, 1);

    // Cleanup
    uzmtp_dealer_deinit(&d);
    test_state_reset();
}

static void
test_zmtp_dealer_connect_recv_greeting_error(void** context_p)
{
    ((void)context_p);
    int err, connection = 0;
    uint8_t greeting[64];
    uzmtp_msg_s pack[1];

    // Connect to remote endpoint and pop outgoing greeting
    uzmtp_dealer_s d;
    uzmtp_dealer_init(&d, test_send_ok, NULL);
    err = uzmtp_dealer_connect(&d, &connection);

    // Parse incoming malformed greeting and verify error
    test_print_greeting(greeting);
    greeting[9] = 0x02; // junk
    err = uzmtp_dealer_parse(&d, greeting, sizeof(greeting), pack, 1);
    assert_int_equal(UZMTP_NULL, uzmtp_dealer_state(&d));
    assert_int_equal(err, UZMTP_ERROR_VERSION);

    // Cleanup
    uzmtp_dealer_deinit(&d);
    test_state_reset();
}

static void
test_zmtp_dealer_connect_recv_ready_ok(void** context_p)
{
    ((void)context_p);
    int err, sz, connection = 0;
    uint8_t buffer[64];
    uint8_t ready[6] = "\5READY";
    uzmtp_msg_s pack[1];

    // connect to remote and pop outgoing greeting
    uzmtp_dealer_s d;
    uzmtp_dealer_init(&d, test_send_ok, NULL);
    err = uzmtp_dealer_connect(&d, &connection);

    // Parse incoming greeting and verify outgoing ready
    test_print_greeting(buffer);
    err = uzmtp_dealer_parse(&d, buffer, 64, pack, 1);
    sz = test_print_incoming(buffer, ready, sizeof(ready), UZMTP_MSG_COMMAND);
    err = uzmtp_dealer_parse(&d, buffer, sz, pack, 1);
    assert_int_equal(err, UZMTP_WANT_MORE);
    assert_int_equal(uzmtp_dealer_ready(&d), 1);
    assert_int_equal(uzmtp_dealer_state(&d), UZMTP_RECV_FLAGS);

    // Cleanup
    uzmtp_dealer_deinit(&d);
    test_state_reset();
}

static void
test_zmtp_dealer_connect_recv_ready_error(void** context_p)
{
    ((void)context_p);
    int err, sz, connection = 0;
    uint8_t buffer[64];
    uint8_t ready[6] = "\5RXXDY"; // bad ready
    uzmtp_msg_s pack[1];

    // connect to remote and pop outgoing greeting
    uzmtp_dealer_s d;
    uzmtp_dealer_init(&d, test_send_ok, NULL);
    err = uzmtp_dealer_connect(&d, &connection);

    // Parse incoming greeting and verify outgoing ready
    test_print_greeting(buffer);
    err = uzmtp_dealer_parse(&d, buffer, 64, pack, 1);
    sz = test_print_incoming(buffer, ready, sizeof(ready), UZMTP_MSG_COMMAND);
    err = uzmtp_dealer_parse(&d, buffer, sz, pack, 1);
    assert_int_equal(err, UZMTP_ERROR_PROTOCOL);
    assert_int_equal(uzmtp_dealer_ready(&d), 0);
    assert_int_equal(uzmtp_dealer_state(&d), UZMTP_NULL);

    // Cleanup
    uzmtp_dealer_deinit(&d);
    test_state_reset();
}

static void
test_zmtp_dealer_recv_message_ok(void** context_p)
{
    ((void)context_p);
    int connection = 0, sz, ret;
    uint8_t buffer[1024];
    uzmtp_msg_s msgs[2];
    uzmtp_dealer_s d;
    uzmtp_dealer_init(&d, test_send_ok, NULL);
    uzmtp_dealer_connect(&d, &connection);

    sz = test_print_incoming(buffer, (uint8_t*)"hello", 5, UZMTP_MSG_MORE);
    sz += test_print_incoming(&buffer[sz], (uint8_t*)"world", 5, 0);

    test_zmtp_dealer_drive_ready(&d, msgs, 2);
    ret = uzmtp_dealer_parse(&d, buffer, sz, msgs, 2);
    assert_int_equal(ret, sz);
    assert_int_equal(uzmtp_dealer_count(&d), 2);
    assert_int_equal(uzmtp_msg_size(&msgs[0]), 5);
    assert_true(uzmtp_msg_is_more(&msgs[0]));
    assert_memory_equal(uzmtp_msg_data(&msgs[0]), "hello", 5);
    assert_int_equal(uzmtp_msg_size(&msgs[1]), 5);
    assert_false(uzmtp_msg_is_more(&msgs[1]));
    assert_memory_equal(uzmtp_msg_data(&msgs[1]), "world", 5);

    // Cleanup
    uzmtp_dealer_deinit(&d);
    test_state_reset();
}

static void
test_zmtp_dealer_recv_message_large(void** context_p)
{
    ((void)context_p);
    int connection = 0, sz, ret;
    uint8_t buffer[RECV_LARGE_SZ + 1024], payload[RECV_LARGE_SZ];
    uint8_t expect[RECV_LARGE_SZ];
    memset(expect, RECV_LARGE_DATA, RECV_LARGE_SZ);

    uzmtp_msg_s msg;
    uzmtp_dealer_s d;
    uzmtp_dealer_init(&d, test_send_ok, NULL);
    uzmtp_dealer_connect(&d, &connection);

    // Setup
    memset(payload, RECV_LARGE_DATA, RECV_LARGE_SZ);
    sz = test_print_incoming(buffer, payload, sizeof(payload), 0);

    // Recv a single large message and verify callback is called
    test_zmtp_dealer_drive_ready(&d, &msg, 1);
    ret = uzmtp_dealer_parse(&d, buffer, sz, &msg, 1);
    assert_int_equal(ret, sz);
    assert_int_equal(uzmtp_dealer_count(&d), 1);
    assert_int_equal(uzmtp_msg_size(&msg), RECV_LARGE_SZ);
    assert_memory_equal(uzmtp_msg_data(&msg), expect, RECV_LARGE_SZ);

    // Cleanup
    uzmtp_dealer_deinit(&d);
    test_state_reset();
}

static void
test_zmtp_dealer_recv_message_large_split(void** context_p)
{
    ((void)context_p);
    uint8_t buffer[RECV_LARGE_SZ + 1024], payload[RECV_LARGE_SZ];
    uint8_t ready[6] = "\5READY", hello[5] = "hello", world[5] = "world";
    uint8_t *ptr = buffer, *end = ptr;
    uzmtp_msg_s msgs[4];
    int i = 0, sz = 64, connection = 0, ret;
    int chunks[] = { 1, 2, 7, 23, RECV_LARGE_SZ, RECV_LARGE_SZ + 1024, 0 },
        chunk = 0, remaining = 0;
    uzmtp_dealer_s dealer;

    memset(payload, RECV_LARGE_DATA, RECV_LARGE_SZ);

    test_print_greeting(buffer);
    sz += test_print_incoming(&buffer[sz], ready, 6, UZMTP_MSG_COMMAND);
    sz += test_print_incoming(&buffer[sz], hello, 5, UZMTP_MSG_MORE);
    sz += test_print_incoming(&buffer[sz], world, 5, UZMTP_MSG_MORE);
    sz += test_print_incoming(&buffer[sz], payload, RECV_LARGE_SZ, 0);

    while (chunks[i]) {

        uzmtp_dealer_init(&dealer, test_send_ok, NULL);
        uzmtp_dealer_connect(&dealer, &connection);
        ptr = buffer;
        end = ptr + sz;
        remaining = sz;
        while (ptr < end) {
            chunk = remaining < chunks[i] ? remaining : chunks[i];
            ret = uzmtp_dealer_parse(&dealer, ptr, chunk, msgs, 4);
            if (ret > 0) break;
            assert_int_equal(ret, UZMTP_WANT_MORE);
            ptr += chunk;
            remaining -= chunk;
        }
        assert_int_equal(ret, chunk);
        assert_int_equal(uzmtp_dealer_count(&dealer), 3);
        assert_int_equal(uzmtp_msg_is_more(&msgs[0]), 1);
        assert_int_equal(uzmtp_msg_is_more(&msgs[1]), 1);
        assert_int_equal(uzmtp_msg_is_more(&msgs[2]), 0);
        assert_int_equal(uzmtp_msg_size(&msgs[0]), 5);
        assert_int_equal(uzmtp_msg_size(&msgs[1]), 5);
        assert_int_equal(uzmtp_msg_size(&msgs[2]), RECV_LARGE_SZ);
        assert_memory_equal(uzmtp_msg_data(&msgs[0]), "hello", 5);
        assert_memory_equal(uzmtp_msg_data(&msgs[1]), "world", 5);
        assert_memory_equal(uzmtp_msg_data(&msgs[2]), payload, RECV_LARGE_SZ);

        uzmtp_dealer_deinit(&dealer);
        test_state_reset();
        i++;
    }

    // Cleanup
}

void
test_zmtp_dealer_recv_overflow(void** context_p)
{
    ((void)context_p);
    int connection = 0, sz, ret;
    uint8_t buffer[1024];
    uzmtp_msg_s msgs[2];
    uzmtp_dealer_s d;
    uzmtp_dealer_init(&d, test_send_ok, NULL);
    uzmtp_dealer_connect(&d, &connection);

    sz = test_print_incoming(buffer, (uint8_t*)"a", 1, UZMTP_MSG_MORE);
    sz += test_print_incoming(&buffer[sz], (uint8_t*)"b", 1, UZMTP_MSG_MORE);
    sz += test_print_incoming(&buffer[sz], (uint8_t*)"c", 1, 0);

    test_zmtp_dealer_drive_ready(&d, msgs, 2);
    ret = uzmtp_dealer_parse(&d, buffer, sz, msgs, 2);
    assert_int_equal(ret, UZMTP_ERROR_OVERFLOW);

    // Cleanup
    uzmtp_dealer_deinit(&d);
    test_state_reset();
}

static int
test_send_track_outgoing(uzmtp_dealer_s* dealer, const uint8_t* b, uint32_t l)
{
    uint8_t* bytes = uzmtp_dealer_context(dealer);
    if (bytes) {
        memcpy(bytes, b, l);
        uzmtp_dealer_context_set(dealer, (uint8_t*)bytes + l);
    }
    return 0;
}

static void
test_zmtp_dealer_send_message_large(void** context_p)
{
    ((void)context_p);
    int err, sz = 0, connection = 0;
    uzmtp_dealer_s a, b;
    uint8_t payload[1024 + RECV_LARGE_SZ];
    uint8_t large[RECV_LARGE_SZ], hello[5] = "hello", world[5] = "world";
    uzmtp_msg_s msgs_a;
    uzmtp_msg_s msgs_b[3];
    mock_packet_s* pack;

    memset(large, RECV_LARGE_DATA, RECV_LARGE_SZ);
    memset(payload, 0, sizeof(payload));

    // Send a message, read back message, and parse, verify parse ok
    uzmtp_dealer_init(&a, test_send_track_outgoing, payload);
    uzmtp_dealer_init(&b, test_send_ok, NULL);
    uzmtp_dealer_connect(&a, &connection);
    uzmtp_dealer_connect(&b, &connection);
    test_zmtp_dealer_drive_ready(&a, &msgs_a, 1);

    uzmtp_msg_init_str(&msgs_a, UZMTP_MSG_MORE, "hello");
    err = uzmtp_dealer_send(&a, &msgs_a);
    assert_int_equal(err, 0);

    uzmtp_msg_init_str(&msgs_a, UZMTP_MSG_MORE, "world");
    err = uzmtp_dealer_send(&a, &msgs_a);
    assert_int_equal(err, 0);

    uzmtp_msg_init(&msgs_a, 0, large, sizeof(large));
    err = uzmtp_dealer_send(&a, &msgs_a);
    assert_int_equal(err, 0);

    sz = (uint8_t*)uzmtp_dealer_context(&a) - payload;
    err = uzmtp_dealer_parse(&b, payload, sz, msgs_b, 3);
    assert_int_equal(err, sz);
    assert_int_equal(uzmtp_dealer_count(&b), 3);
    assert_int_equal(uzmtp_msg_is_more(&msgs_b[0]), 1);
    assert_int_equal(uzmtp_msg_is_more(&msgs_b[1]), 1);
    assert_int_equal(uzmtp_msg_is_more(&msgs_b[2]), 0);
    assert_int_equal(uzmtp_msg_size(&msgs_b[0]), 5);
    assert_int_equal(uzmtp_msg_size(&msgs_b[1]), 5);
    assert_int_equal(uzmtp_msg_size(&msgs_b[2]), RECV_LARGE_SZ);
    assert_memory_equal(uzmtp_msg_data(&msgs_b[0]), "hello", 5);
    assert_memory_equal(uzmtp_msg_data(&msgs_b[1]), "world", 5);
    assert_memory_equal(uzmtp_msg_data(&msgs_b[2]), large, RECV_LARGE_SZ);

    // cleanup
    uzmtp_dealer_deinit(&a);
    uzmtp_dealer_deinit(&b);
}

int
main(void)
{
    int err;

    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_zmtp_dealer_init),
        cmocka_unit_test(test_zmtp_dealer_connect_send_greeting_ok),
        cmocka_unit_test(test_zmtp_dealer_connect_send_greeting_error),
        cmocka_unit_test(test_zmtp_dealer_connect_recv_greeting_ok),
        cmocka_unit_test(test_zmtp_dealer_connect_recv_greeting_error),
        cmocka_unit_test(test_zmtp_dealer_connect_recv_ready_ok),
        cmocka_unit_test(test_zmtp_dealer_connect_recv_ready_error),
        cmocka_unit_test(test_zmtp_dealer_recv_message_ok),
        cmocka_unit_test(test_zmtp_dealer_recv_message_large),
        cmocka_unit_test(test_zmtp_dealer_recv_message_large_split),
        cmocka_unit_test(test_zmtp_dealer_recv_overflow),
        cmocka_unit_test(test_zmtp_dealer_send_message_large)
    };

    err = cmocka_run_group_tests(tests, NULL, NULL);

    return err;
}
