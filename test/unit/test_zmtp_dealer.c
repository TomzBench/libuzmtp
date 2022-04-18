#include "test_helpers.h"

#define RECV_LARGE_SZ 20480
#define RECV_LARGE_DATA 'A'

// Callbacks
static int
test_cb_write_ok(uzmtp_dealer_s* dealer, const uint8_t* b, uint32_t sz)
{
    int connection;
    n_want_write++;
    connection = *(int*)uzmtp_dealer_connection_get(dealer);
    test_state_push_outgoing(connection, b, sz);
    return 0;
}

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

// Test start
static void
test_zmtp_dealer_new(void** context_p)
{
    ((void)context_p);
    void* data = NULL;

    // Allocator
    uzmtp_dealer_s* dealer = uzmtp_dealer_new(&test_settings_ok);
    assert_non_null(dealer);

    // Set / Get context
    uzmtp_dealer_context_set(dealer, data);
    assert_ptr_equal(uzmtp_dealer_context_get(dealer), data);

    // Free
    uzmtp_dealer_destroy(&dealer);
    assert_null(dealer);
}

static void
test_zmtp_dealer_connect_send_greeting_ok(void** context_p)
{
    ((void)context_p);
    int err, connection = 0;
    mock_packet_s* pack;
    uzmtp_dealer_s* d = uzmtp_dealer_new(&test_settings_ok);
    assert_non_null(d);

    // Connect and verify outgoing message is sent by reading back outgoing
    // bytes.  Verify state machine in correct state.
    err = uzmtp_dealer_connect(d, &connection);
    assert_int_equal(err, 0);
    assert_int_equal(n_want_write, 1);
    pack = test_state_pop_last_outgoing_packet();
    test_assert_valid_greeting(pack);
    assert_int_equal(0, test_state_get_outgoing_bytes());
    assert_int_equal(UZMTP_CONNECT_WANT_GREETING, uzmtp_dealer_state_get(d));

    // Cleanup
    test_state_free_packet(&pack);
    uzmtp_dealer_destroy(&d);
    test_state_reset();
}

static void
test_zmtp_dealer_connect_send_greeting_error(void** context_p)
{
    ((void)context_p);
    int err, connection = 0;
    mock_packet_s* pack;

    // Connect and verify outgoing message is sent by reading back outgoing
    // bytes.  Verify state machine in correct state.
    uzmtp_dealer_s* d = uzmtp_dealer_new(&test_settings_err);
    assert_non_null(d);
    err = uzmtp_dealer_connect(d, &connection);
    assert_int_equal(err, -1);
    assert_int_equal(n_want_write, 1);
    pack = test_state_pop_last_outgoing_packet();
    assert_null(pack);
    assert_int_equal(0, test_state_get_outgoing_bytes());
    assert_int_equal(UZMTP_NULL, uzmtp_dealer_state_get(d));

    // Cleanup
    uzmtp_dealer_destroy(&d);
    test_state_reset();
}

static void
test_zmtp_dealer_connect_recv_greeting_ok(void** context_p)
{
    ((void)context_p);
    int err, connection = 0;
    uint8_t greeting[64];
    mock_packet_s* pack;

    // connect to remote and pop outgoing greeting
    uzmtp_dealer_s* d = uzmtp_dealer_new(&test_settings_ok);
    err = uzmtp_dealer_connect(d, &connection);
    pack = test_state_pop_last_outgoing_packet();
    test_state_free_packet(&pack);

    // Parse incoming greeting and verify outgoing ready
    test_print_greeting(greeting);
    err = uzmtp_dealer_parse(d, greeting, sizeof(greeting));
    assert_int_equal(err, 0);
    assert_int_equal(UZMTP_CONNECT_WANT_READY, uzmtp_dealer_state_get(d));
    pack = test_state_pop_last_outgoing_packet();
    test_assert_valid_header(pack, UZMTP_MSG_COMMAND, 6);
    test_state_free_packet(&pack);
    pack = test_state_pop_last_outgoing_packet();
    test_assert_valid_ready(pack);
    test_state_free_packet(&pack);

    // Cleanup
    assert_int_equal(test_state_get_outgoing_bytes(), 0);
    uzmtp_dealer_destroy(&d);
    test_state_reset();
}

static void
test_zmtp_dealer_connect_recv_greeting_error(void** context_p)
{
    ((void)context_p);
    int err, connection = 0;
    uint8_t greeting[64];
    mock_packet_s* pack;

    // Connect to remote endpoint and pop outgoing greeting
    uzmtp_dealer_s* d = uzmtp_dealer_new(&test_settings_ok);
    err = uzmtp_dealer_connect(d, &connection);
    pack = test_state_pop_last_outgoing_packet();
    test_state_free_packet(&pack);

    // Parse incoming malformed greeting and verify error
    test_print_greeting(greeting);
    greeting[9] = 0x02; // junk
    err = uzmtp_dealer_parse(d, greeting, sizeof(greeting));
    assert_int_equal(err, -1);
    assert_int_equal(UZMTP_NULL, uzmtp_dealer_state_get(d));
    assert_int_equal(n_error, 1);
    assert_int_equal(UZMTP_ERROR_VERSION, last_error);

    // Cleanup
    assert_int_equal(test_state_get_outgoing_bytes(), 0);
    uzmtp_dealer_destroy(&d);
    test_state_reset();
}

static void
test_zmtp_dealer_connect_recv_ready_ok(void** context_p)
{
    ((void)context_p);
    int err, sz, connection = 0;
    uint8_t buffer[64];
    uint8_t ready[6] = "\5READY";
    mock_packet_s* pack;

    // connect to remote and pop outgoing greeting
    uzmtp_dealer_s* d = uzmtp_dealer_new(&test_settings_ok);
    err = uzmtp_dealer_connect(d, &connection);

    // Parse incoming greeting and verify outgoing ready
    test_print_greeting(buffer);
    err = uzmtp_dealer_parse(d, buffer, 64);
    sz = test_print_incoming(buffer, ready, sizeof(ready), UZMTP_MSG_COMMAND);
    err = uzmtp_dealer_parse(d, buffer, sz);
    assert_int_equal(err, 0);
    assert_int_equal(uzmtp_dealer_ready(d), 1);
    assert_int_equal(uzmtp_dealer_state_get(d), UZMTP_RECV_FLAGS);
    for (int i = 0; i < 3; i++) {
        // Outgoing greeting, ready prefix and ready body from outgoing
        pack = test_state_pop_last_outgoing_packet();
        test_state_free_packet(&pack);
    }

    // Cleanup
    assert_int_equal(test_state_get_outgoing_bytes(), 0);
    uzmtp_dealer_destroy(&d);
    test_state_reset();
}

static void
test_zmtp_dealer_connect_recv_ready_error(void** context_p)
{
    ((void)context_p);
    int err, sz, connection = 0;
    uint8_t buffer[64];
    uint8_t ready[6] = "\5RXXDY"; // bad ready
    mock_packet_s* pack;

    // connect to remote and pop outgoing greeting
    uzmtp_dealer_s* d = uzmtp_dealer_new(&test_settings_ok);
    err = uzmtp_dealer_connect(d, &connection);

    // Parse incoming greeting and verify outgoing ready
    test_print_greeting(buffer);
    err = uzmtp_dealer_parse(d, buffer, 64);
    sz = test_print_incoming(buffer, ready, sizeof(ready), UZMTP_MSG_COMMAND);
    err = uzmtp_dealer_parse(d, buffer, sz);
    assert_int_equal(err, -1);
    assert_int_equal(uzmtp_dealer_ready(d), 0);
    assert_int_equal(uzmtp_dealer_state_get(d), UZMTP_NULL);
    assert_int_equal(n_error, 1);
    assert_int_equal(UZMTP_ERROR_PROTOCOL, last_error);
    for (int i = 0; i < 3; i++) {
        // Outgoing greeting, ready prefix and ready body from outgoing
        pack = test_state_pop_last_outgoing_packet();
        test_state_free_packet(&pack);
    }

    // Cleanup
    assert_int_equal(test_state_get_outgoing_bytes(), 0);
    uzmtp_dealer_destroy(&d);
    test_state_reset();
}

static void
test_zmtp_dealer_recv_message_ok(void** context_p)
{
    ((void)context_p);
    int connection = 0, sz, err;
    uint8_t buffer[1024];
    uzmtp_dealer_s* d = uzmtp_dealer_new(&test_settings_ok);
    uzmtp_dealer_connect(d, &connection);

    sz = test_print_incoming(buffer, (uint8_t*)"hello", 5, UZMTP_MSG_MORE);
    sz += test_print_incoming(&buffer[sz], (uint8_t*)"world", 5, 0);

    test_zmtp_dealer_state_ready(d);
    err = uzmtp_dealer_parse(d, buffer, sz);
    assert_int_equal(err, 0);
    assert_int_equal(n_recv, 1);

    // Cleanup
    assert_int_equal(test_state_get_outgoing_bytes(), 0);
    uzmtp_dealer_destroy(&d);
    test_state_reset();
}

static void
test_zmtp_dealer_recv_message_error(void** context_p)
{
    ((void)context_p);
    int connection = 0, sz, err;
    uint8_t buffer[1024];
    uzmtp_dealer_s* d = uzmtp_dealer_new(&test_settings_recv_err);
    uzmtp_dealer_connect(d, &connection);

    sz = test_print_incoming(buffer, (uint8_t*)"hello", 5, UZMTP_MSG_MORE);
    sz += test_print_incoming(&buffer[sz], (uint8_t*)"world", 5, 0);

    test_zmtp_dealer_state_ready(d);
    err = uzmtp_dealer_parse(d, buffer, sz);
    assert_int_equal(err, -1);
    assert_int_equal(uzmtp_dealer_state_get(d), UZMTP_NULL);

    // Cleanup
    assert_int_equal(test_state_get_outgoing_bytes(), 0);
    uzmtp_dealer_destroy(&d);
    test_state_reset();
}

static void
test_zmtp_dealer_recv_message_large(void** context_p)
{
    ((void)context_p);
    int connection = 0, sz, err;
    uint8_t buffer[RECV_LARGE_SZ + 1024], payload[RECV_LARGE_SZ];
    uzmtp_dealer_s* d = uzmtp_dealer_new(&test_settings_recv_large);
    uzmtp_dealer_connect(d, &connection);

    // Setup
    memset(payload, RECV_LARGE_DATA, RECV_LARGE_SZ);
    sz = test_print_incoming(buffer, payload, sizeof(payload), 0);

    // Recv a single large message and verify callback is called
    test_zmtp_dealer_state_ready(d);
    err = uzmtp_dealer_parse(d, buffer, sz);
    assert_int_equal(err, 0);
    assert_int_equal(n_recv, 1);

    // Cleanup
    assert_int_equal(test_state_get_outgoing_bytes(), 0);
    uzmtp_dealer_destroy(&d);
    test_state_reset();
}

static void
test_zmtp_dealer_recv_message_large_split(void** context_p)
{
    ((void)context_p);
    uint8_t buffer[RECV_LARGE_SZ + 1024], payload[RECV_LARGE_SZ];
    uint8_t ready[6] = "\5READY", hello[5] = "hello", world[5] = "world";
    uint8_t *ptr = buffer, *end = ptr;
    int i = 0, sz = 64, connection = 0, err;
    int chunks[] = { 1, 2, 7, 23, RECV_LARGE_SZ, RECV_LARGE_SZ + 1024, 0 },
        chunk = 0, remaining = 0;
    uzmtp_dealer_s* dealer;

    memset(payload, RECV_LARGE_DATA, RECV_LARGE_SZ);

    test_print_greeting(buffer);
    sz += test_print_incoming(&buffer[sz], ready, 6, UZMTP_MSG_COMMAND);
    sz += test_print_incoming(&buffer[sz], hello, 5, UZMTP_MSG_MORE);
    sz += test_print_incoming(&buffer[sz], world, 5, UZMTP_MSG_MORE);
    sz += test_print_incoming(&buffer[sz], payload, RECV_LARGE_SZ, 0);

    while (chunks[i]) {

        dealer = uzmtp_dealer_new(&test_settings_recv_large_more);
        uzmtp_dealer_connect(dealer, &connection);
        ptr = buffer;
        end = ptr + sz;
        remaining = sz;
        while (ptr < end) {
            chunk = remaining < chunks[i] ? remaining : chunks[i];
            err = uzmtp_dealer_parse(dealer, ptr, chunk);
            assert_int_equal(err, 0);
            ptr += chunk;
            remaining -= chunk;
        }
        assert_int_equal(n_recv, 1);

        uzmtp_dealer_destroy(&dealer);
        test_state_reset();
        i++;
    }

    // Cleanup
}

static void
test_zmtp_dealer_send_message_large(void** context_p)
{
    ((void)context_p);
    int err, sz = 0, connection = 0;
    uzmtp_dealer_s *a, *b;
    uint8_t payload[1024 + RECV_LARGE_SZ];
    uint8_t large[RECV_LARGE_SZ], hello[5] = "hello", world[5] = "world";
    uzmtp_msg_s* msg;
    mock_packet_s* pack;

    memset(large, RECV_LARGE_DATA, RECV_LARGE_SZ);

    // Send a message, read back message, and parse, verify parse ok
    a = uzmtp_dealer_new(&test_settings_recv_large_more);
    b = uzmtp_dealer_new(&test_settings_recv_large_more);
    uzmtp_dealer_connect(a, &connection);
    uzmtp_dealer_connect(b, &connection);
    test_zmtp_dealer_state_ready(a);
    test_zmtp_dealer_state_ready(b);

    msg = uzmtp_msg_new_from_const_data(UZMTP_MSG_MORE, hello, 5);
    err = uzmtp_dealer_send(a, &msg);
    assert_null(msg);
    assert_int_equal(err, 0);

    msg = uzmtp_msg_new_from_const_data(UZMTP_MSG_MORE, world, 5);
    err = uzmtp_dealer_send(a, &msg);
    assert_null(msg);
    assert_int_equal(err, 0);

    msg = uzmtp_msg_new_from_const_data(0, large, sizeof(large));
    err = uzmtp_dealer_send(a, &msg);
    assert_null(msg);
    assert_int_equal(err, 0);

    // Readback our outgoing message and parse it
    sz = 0;
    while ((pack = test_state_pop_last_outgoing_packet())) {
        memcpy(&payload[sz], pack->b, pack->sz);
        sz += pack->sz;
        test_state_free_packet(&pack);
    }
    err = uzmtp_dealer_parse(b, payload, sz);
    assert_int_equal(err, 0);
    assert_int_equal(n_recv, 1);

    // cleanup
    uzmtp_dealer_destroy(&a);
    uzmtp_dealer_destroy(&b);
}

int
zmtp_dealer_tests()
{
    int err;

    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_zmtp_dealer_new),
        cmocka_unit_test(test_zmtp_dealer_connect_send_greeting_ok),
        cmocka_unit_test(test_zmtp_dealer_connect_send_greeting_error),
        cmocka_unit_test(test_zmtp_dealer_connect_recv_greeting_ok),
        cmocka_unit_test(test_zmtp_dealer_connect_recv_greeting_error),
        cmocka_unit_test(test_zmtp_dealer_connect_recv_ready_ok),
        cmocka_unit_test(test_zmtp_dealer_connect_recv_ready_error),
        cmocka_unit_test(test_zmtp_dealer_recv_message_ok),
        cmocka_unit_test(test_zmtp_dealer_recv_message_error),
        cmocka_unit_test(test_zmtp_dealer_recv_message_large),
        cmocka_unit_test(test_zmtp_dealer_recv_message_large_split),
        cmocka_unit_test(test_zmtp_dealer_send_message_large)
    };

    err = cmocka_run_group_tests(tests, NULL, NULL);

    return err;
}
