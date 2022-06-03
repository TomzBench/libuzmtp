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

// uzmtp_msg_...()
void uzmtp_msg_init(uzmtp_msg_s*, uint8_t, void*, size_t);
void uzmtp_msg_init_str(uzmtp_msg_s*, uint8_t, char*);
void uzmtp_msg_init_mem(uzmtp_msg_s*, uint8_t, uint8_t**, size_t);
void uzmtp_msg_init_const(uzmtp_msg_s*, uint8_t, uint8_t*, size_t);
void uzmtp_msg_deinit(uzmtp_msg_s* msg);
uint8_t uzmtp_msg_flags(uzmtp_msg_s* self);
void uzmtp_msg_set_more(uzmtp_msg_s* self);
void uzmtp_msg_clr_more(uzmtp_msg_s* self);
int uzmtp_msg_is_more(uzmtp_msg_s* self);
int uzmtp_msg_is_large(uzmtp_msg_s* self);
uint8_t* uzmtp_msg_data(uzmtp_msg_s* self);
size_t uzmtp_msg_size(uzmtp_msg_s* self);
void* uzmtp_msg_next(uzmtp_msg_s* self);

// uzmtp_dealer_...()
uzmtp_dealer_s* uzmtp_dealer_new(uzmtp_dealer_settings* settings);
void uzmtp_dealer_destroy(uzmtp_dealer_s** dealer_p);
void uzmtp_dealer_context_set(uzmtp_dealer_s* dealer, void* context);
void* uzmtp_dealer_context_get(uzmtp_dealer_s* dealer);
EUZMTP_STATE uzmtp_dealer_state_get(uzmtp_dealer_s* dealer);
uzmtp_connection* uzmtp_dealer_connection_get(uzmtp_dealer_s*);
uint8_t uzmtp_dealer_ready(uzmtp_dealer_s*);
uzmtp_msg_s* uzmtp_dealer_pop_incoming(uzmtp_dealer_s*);
uint32_t uzmtp_dealer_incoming_count(uzmtp_dealer_s*);
int uzmtp_dealer_connect(uzmtp_dealer_s* dealer, uzmtp_connection*);
int uzmtp_dealer_parse(uzmtp_dealer_s* d, const uint8_t* bytes, uint32_t sz);
int uzmtp_dealer_send(uzmtp_dealer_s* d, uzmtp_msg_s** msg);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* UZMTP_H_ */
