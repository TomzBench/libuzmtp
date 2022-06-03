/*
 * uzmtp_msg.h
 */

#ifndef UZMTP_MSG_H_
#define UZMTP_MSG_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "uzmtp/uzmtp_types.h"

/// NOTE - order of fields is important to maintain efficient layout and to
///        ensure our size assumption matches the opaque type when placing on
///        the stack
typedef struct uzmtp_msg__s
{
    struct uzmtp_msg__s* next;
    uint8_t* data;
    size_t size;
    int greedy;
    uint8_t flags;
    uint8_t pad[0];
} uzmtp_msg__s;

typedef uzmtp_msg_opaque uzmtp_msg__s0;

_Static_assert(
    sizeof(uzmtp_msg__s0) == sizeof(uzmtp_msg__s),
    "invalid uzmtp_msg__s size assumption");

void uzmtp_msg_init(uzmtp_msg__s*, uint8_t, void*, size_t, int);
uzmtp_msg__s* uzmtp_msg_new(uint8_t flags, size_t size);
uzmtp_msg__s* uzmtp_msg_new_from_data(uint8_t, uint8_t** data_p, size_t size);
uzmtp_msg__s* uzmtp_msg_new_from_const_data(uint8_t flags, void*, size_t);
void uzmtp_msg_destroy(uzmtp_msg__s** self_p);
uint8_t uzmtp_msg_flags(uzmtp_msg__s* self);
void uzmtp_msg_set_more(uzmtp_msg__s* self);
void uzmtp_msg_clr_more(uzmtp_msg__s* self);
int uzmtp_msg_is_more(uzmtp_msg__s* self);
int uzmtp_msg_is_large(uzmtp_msg__s* self);
uint8_t* uzmtp_msg_data(uzmtp_msg__s* self);
size_t uzmtp_msg_size(uzmtp_msg__s* self);
void* uzmtp_msg_next(uzmtp_msg__s* self);

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif
