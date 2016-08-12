/*
 * uzmtp_msg.c
 */

#include "uzmtp_msg.h"

void uzmtp_msg_init(_UzmtpMsg *msg, uint8_t flags, void *data, size_t size,
		    bool greedy);

_UzmtpMsg *uzmtp_msg_new(uint8_t flags, size_t size) {
    _UzmtpMsg *msg =
	uzmtp_malloc(sizeof(_UzmtpMsg) + size - UZMTP_ANYSIZE_ARRAY);
    if (!msg) return NULL;
    uzmtp_msg_init(msg, flags, msg->pad, size, false);
    return msg;
}

_UzmtpMsg *uzmtp_msg_from_data(uint8_t flags, uint8_t **data_p, size_t size) {
    assert(data_p);
    _UzmtpMsg *msg = uzmtp_malloc(sizeof(_UzmtpMsg));
    if (!msg) return NULL;
    uzmtp_msg_init(msg, flags, *data_p, size, true);
    *data_p = NULL;
    return msg;
}

_UzmtpMsg *uzmtp_msg_from_const_data(uint8_t flags, void *data, size_t size) {
    _UzmtpMsg *msg = uzmtp_malloc(sizeof(_UzmtpMsg));
    if (!msg) return NULL;
    uzmtp_msg_init(msg, flags, data, size, false);
    return msg;
}

void uzmtp_msg_init(_UzmtpMsg *msg, uint8_t flags, void *data, size_t size,
		    bool greedy) {
    msg->flags = flags;
    msg->data = data;
    msg->size = size;
    msg->greedy = greedy;
    if (msg->size > 255) msg->flags |= UZMTP_MSG_LARGE;
}

void uzmtp_msg_destroy(_UzmtpMsg **self_p) {
    assert(self_p);
    if (*self_p) {
	_UzmtpMsg *self = *self_p;
	if (self->greedy) uzmtp_free(self->data);
	uzmtp_free(self);
	*self_p = NULL;
    }
}

uint8_t uzmtp_msg_flags(_UzmtpMsg *self) { return self->flags; }

void uzmtp_msg_set_more(_UzmtpMsg *self) { self->flags |= UZMTP_MSG_MORE; }

void uzmtp_msg_clr_more(_UzmtpMsg *self) { self->flags &= ~(UZMTP_MSG_MORE); }

bool uzmtp_msg_more(_UzmtpMsg *self) {
    return ((self->flags & UZMTP_MSG_MORE) == UZMTP_MSG_MORE);
}

uint8_t *uzmtp_msg_data(_UzmtpMsg *self) { return self->data; }

size_t uzmtp_msg_size(_UzmtpMsg *self) { return self->size; }

//
//
//
