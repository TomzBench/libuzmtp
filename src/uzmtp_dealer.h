/*
 * uzmtp_dealer.h
 */

#ifndef UZMTP_DEALER_H_
#define UZMTP_DEALER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "uzmtp_msg.h"
#include "uzmtp_types.h"

// Main context
typedef struct
{
    uzmtp_connection* connection;
    uzmtp_dealer_settings* settings;
    EUZMTP_STATE state;
    void* context;
    int32_t n_incoming;
    uint64_t b;
    uint8_t ready;
    uint8_t curr_flags;
    uint64_t curr_size;
    uzmtp_msg_s* curr_msg;
    uzmtp_msg_s* incoming;
    uzmtp_msg_s* tail;
} uzmtp_dealer__s;

// Constructors
uzmtp_dealer__s* uzmtp_dealer_new(uzmtp_dealer_settings* settings);
void uzmtp_dealer_destroy(uzmtp_dealer__s** dealer_p);
// Setters / Getters
void uzmtp_dealer_context_set(uzmtp_dealer__s* dealer, void* context);
void* uzmtp_dealer_context_get(uzmtp_dealer__s* dealer);
uzmtp_connection* uzmtp_dealer_connection_get(uzmtp_dealer__s* dealer);
EUZMTP_STATE uzmtp_dealer_state_get(uzmtp_dealer__s* dealer);
uint8_t uzmtp_dealer_ready(uzmtp_dealer__s*);
uzmtp_msg_s* uzmtp_dealer_pop_incoming(uzmtp_dealer__s* d);
uint32_t uzmtp_dealer_incoming_count(uzmtp_dealer__s* d);
// Other methods
int uzmtp_dealer_connect(uzmtp_dealer__s* dealer, uzmtp_connection*);
int uzmtp_dealer_parse(uzmtp_dealer__s* d, const uint8_t* bytes, uint32_t sz);
int uzmtp_dealer_send(uzmtp_dealer__s* d, uzmtp_msg__s** msg_p);

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif
