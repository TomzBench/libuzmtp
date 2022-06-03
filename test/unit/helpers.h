#ifndef TEST_HELPERS_H_
#define TEST_HELPERS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <uzmtp/uzmtp.h>

#include <setjmp.h>

#include <cmocka.h>

typedef struct mock_packet_s
{
    struct mock_packet_s* next;
    uint32_t sz;
    uint8_t b[];
} mock_packet_s;

extern int n_error;
extern int n_recv;
extern int n_want_write;
extern EUZMTP_ERROR last_error;

void test_state_reset();
void test_state_push_outgoing(int sock, const uint8_t* b, uint32_t l);
mock_packet_s* test_state_pop_last_outgoing_packet();
int test_state_get_outgoing_bytes();
void test_state_reset_outgoing();
void test_state_free_packet(mock_packet_s** packet_p);

void test_zmtp_dealer_state_ready(uzmtp_dealer_s* d);
void test_print_greeting(uint8_t* b);
int test_print_incoming(uint8_t* dst, uint8_t* src, uint64_t s, uint8_t f);
void test_assert_valid_greeting(mock_packet_s* pack);
void test_assert_valid_header(mock_packet_s* pack, uint8_t fl, uint64_t sz);
void test_assert_valid_ready(mock_packet_s* pack);

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif
