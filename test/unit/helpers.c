#include <uzmtp/uzmtp.h>

#include "helpers.h"

static int mock_bytes_sent = 0;
static mock_packet_s* outgoing = NULL;
int n_error = 0;
int n_recv = 0;
int n_want_write = 0;
EUZMTP_ERROR last_error = 0;

void
test_state_reset()
{
    n_want_write = 0;
    n_error = 0;
    n_recv = 0;
    last_error = 0;
    test_state_reset_outgoing();
}

void
test_state_push_outgoing(int sock, const uint8_t* b, uint32_t l)
{
    ((void)sock);
    mock_packet_s* next = outgoing;
    mock_bytes_sent += l;
    if (!outgoing) {
        outgoing = uzmtp_malloc(sizeof(mock_packet_s) + l);
        next = outgoing;
    } else {
        while (next->next) next = next->next;
        next->next = uzmtp_malloc(sizeof(mock_packet_s) + l);
        next = next->next;
    }
    if (b) memcpy((next)->b, b, l);
    (next)->sz = l;
    (next)->next = NULL;
}

mock_packet_s*
test_state_pop_last_outgoing_packet()
{
    mock_packet_s* next = outgoing;
    if (outgoing) {
        outgoing = outgoing->next;
        mock_bytes_sent -= next->sz;
    }
    return next;
}

int
test_state_get_outgoing_bytes()
{
    return mock_bytes_sent;
}

void
test_state_reset_outgoing()
{
    mock_packet_s* packet;
    while (outgoing) {
        packet = test_state_pop_last_outgoing_packet();
        test_state_free_packet(&packet);
    }
}

void
test_state_free_packet(mock_packet_s** packet_p)
{
    mock_packet_s* packet = *packet_p;
    *packet_p = NULL;
    uzmtp_free(packet);
}

void
test_zmtp_dealer_state_ready(uzmtp_dealer_s* d)
{
    int sz;
    uint8_t buffer[64];
    uint8_t ready[6] = "\5READY";
    mock_packet_s* pack;

    test_print_greeting(buffer);
    uzmtp_dealer_parse(d, buffer, 64);
    sz = test_print_incoming(buffer, ready, sizeof(ready), UZMTP_MSG_COMMAND);
    uzmtp_dealer_parse(d, buffer, sz);
    for (int i = 0; i < 3; i++) {
        // Outgoing greeting, ready prefix and ready body from outgoing
        pack = test_state_pop_last_outgoing_packet();
        test_state_free_packet(&pack);
    }
}

void
test_print_greeting(uint8_t* b)
{
    const uint8_t greeting[] = { 0xff, 0, 0, 0,   0,   0,   0,   0,   1,
                                 0x7f, 3, 0, 'N', 'U', 'L', 'L', '\0' };
    uint8_t padding[64 - sizeof(greeting)];
    memset(padding, 0, sizeof(padding));
    memcpy(b, greeting, sizeof(greeting));
    memcpy(b + 17, padding, sizeof(padding));
}

int
test_print_incoming(uint8_t* dst, uint8_t* src, uint64_t sz, uint8_t flags)
{
    if (sz > 255) flags |= UZMTP_MSG_LARGE;
    if (flags & UZMTP_MSG_LARGE) {
        dst[0] = flags;
        dst[1] = sz >> 56;
        dst[2] = sz >> 48;
        dst[3] = sz >> 40;
        dst[4] = sz >> 32;
        dst[5] = sz >> 24;
        dst[6] = sz >> 16;
        dst[7] = sz >> 8;
        dst[8] = sz;
        memcpy(&dst[9], src, sz);
        return sz + 9;
    } else {
        dst[0] = flags;
        dst[1] = sz;
        memcpy(&dst[2], src, sz);
        return sz + 2;
    }
}

void
test_assert_valid_greeting(mock_packet_s* pack)
{
    uint8_t expect[64];
    test_print_greeting(expect);
    assert_non_null(pack);
    assert_int_equal(pack->sz, 64);
    assert_memory_equal(pack->b, expect, 64);
}

void
test_assert_valid_header(mock_packet_s* pack, uint8_t flags, uint64_t size)
{
    uint8_t prefix[9] = { flags };

    assert_non_null(pack);
    if (flags & UZMTP_MSG_LARGE) {
        prefix[1] = size >> 56;
        prefix[2] = size >> 48;
        prefix[3] = size >> 40;
        prefix[4] = size >> 32;
        prefix[5] = size >> 24;
        prefix[6] = size >> 16;
        prefix[7] = size >> 8;
        prefix[8] = size;
        assert_int_equal(pack->sz, sizeof(prefix));
        assert_memory_equal(prefix, pack->b, pack->sz);
    } else {
        prefix[1] = size;
        assert_int_equal(pack->sz, 2);
        assert_memory_equal(prefix, pack->b, pack->sz);
    }
}

void
test_assert_valid_ready(mock_packet_s* pack)
{
    uint8_t expect[] = { 5, 'R', 'E', 'A', 'D', 'Y' };
    assert_non_null(pack);
    assert_int_equal(pack->sz, sizeof(expect));
    assert_memory_equal(expect, pack->b, pack->sz);
}
