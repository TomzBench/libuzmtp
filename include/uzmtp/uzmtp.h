/**
 * @file uzmtp.h
 *
 * @brief uzmtp library public api prototypes.
 */

#ifndef UZMTP_H_
#define UZMTP_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "uzmtp/uzmtp_types.h"

#define UZMTP_MSG_HEADER_SIZE(__l) (__l > 255 ? 9 : 2)
#define UZMTP_DEALER_MSG_SIZE(__l) (__l + UZMTP_MSG_HEADER_SIZE(__l))

// uzmtp_msg_...()
void uzmtp_msg_init(uzmtp_msg_s*, uint8_t, void*, size_t);
void uzmtp_msg_init_str(uzmtp_msg_s*, uint8_t, char*);
void uzmtp_msg_init_mem(uzmtp_msg_s*, uint8_t, uint8_t**, size_t);
void uzmtp_msg_init_const(uzmtp_msg_s*, uint8_t, uint8_t*, size_t);
void uzmtp_msg_deinit(uzmtp_msg_s* msg);
uint8_t uzmtp_msg_flags(uzmtp_msg_s* self);
void uzmtp_msg_flags_set(uzmtp_msg_s* self, uint8_t);
void uzmtp_msg_flags_clr(uzmtp_msg_s* self, uint8_t);
int uzmtp_msg_is_more(uzmtp_msg_s* self);
int uzmtp_msg_is_large(uzmtp_msg_s* self);
uint8_t* uzmtp_msg_data(uzmtp_msg_s* self);
void uzmtp_msg_data_set(uzmtp_msg_s* self, uint8_t*);
size_t uzmtp_msg_size(uzmtp_msg_s* self);
void uzmtp_msg_size_set(uzmtp_msg_s* self, size_t);
size_t uzmtp_msg_print_head(uint8_t*, size_t, uint8_t, int64_t);

// uzmtp_dealer_...()
void uzmtp_dealer_init(uzmtp_dealer_s*, uzmtp_send_fn fn, void* ctx);
void uzmtp_dealer_deinit(uzmtp_dealer_s* dealer);
void* uzmtp_dealer_context(uzmtp_dealer_s* dealer);
void uzmtp_dealer_context_set(uzmtp_dealer_s*, void*);
EUZMTP_STATE uzmtp_dealer_state(uzmtp_dealer_s* dealer);
uzmtp_connection* uzmtp_dealer_connection(uzmtp_dealer_s*);
uint8_t uzmtp_dealer_ready(uzmtp_dealer_s*);
uint32_t uzmtp_dealer_count(uzmtp_dealer_s* d);
uzmtp_msg_s* uzmtp_dealer_pop_incoming(uzmtp_dealer_s*);
uint32_t uzmtp_dealer_incoming_count(uzmtp_dealer_s*);
int uzmtp_dealer_connect(uzmtp_dealer_s* dealer, uzmtp_connection*);
int uzmtp_dealer_parse(
    uzmtp_dealer_s* dealer,
    const uint8_t* packet,
    uint32_t packet_len,
    uzmtp_msg_s* msg,
    uint32_t);
int uzmtp_dealer_send(uzmtp_dealer_s* d, uzmtp_msg_s* msg);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* UZMTP_H_ */
