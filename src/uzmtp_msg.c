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

int uzmtp_msg_send(_UzmtpMsg *msg, _UzmtpSocket *sock) {
    assert(msg);
    assert(sock);
    int ret = uzmtp_net_send(sock, &msg->flags, sizeof(msg->flags));
    if (ret != sizeof(msg->flags)) return -1;

    if (uzmtp_msg_flags(msg) & UZMTP_MSG_LARGE) {
	uint8_t buffer[8];
	const uint64_t msg_size = (uint64_t)uzmtp_msg_size(msg);
	buffer[0] = msg_size >> 56;
	buffer[1] = msg_size >> 48;
	buffer[2] = msg_size >> 40;
	buffer[3] = msg_size >> 32;
	buffer[4] = msg_size >> 24;
	buffer[5] = msg_size >> 16;
	buffer[6] = msg_size >> 8;
	buffer[7] = msg_size;
	ret = uzmtp_net_send(sock, buffer, sizeof(buffer));
	if (ret != sizeof(buffer)) return -1;
	uint64_t c = uzmtp_net_send(sock, uzmtp_msg_data(msg), msg_size);
	if (c != msg_size) return -1;
    } else {
	const uint8_t msg_size = uzmtp_msg_size(msg);
	ret = uzmtp_net_send(sock, &msg_size, sizeof(msg_size));
	if (ret != sizeof(msg_size)) return -1;
	ret = uzmtp_net_send(sock, uzmtp_msg_data(msg), msg_size);
	if (ret != msg_size) return -1;
    }

    return 0;
}

_UzmtpMsg *uzmtp_msg_recv(_UzmtpSocket *sock) {
    assert(msg);
    uint8_t flags;
    size_t size;
    int ret = uzmtp_net_recv(sock, &flags, 1);
    if (ret != 1) return NULL;
    //  Check large flag
    if ((flags & UZMTP_MSG_LARGE) == 0) {
	uint8_t buffer[1];
	ret = uzmtp_net_recv(sock, buffer, 1);
	if (ret != 1) return NULL;
	size = (size_t)buffer[0];
    } else {
	uint8_t buffer[8];
	ret = uzmtp_net_recv(sock, buffer, sizeof(buffer));
	if (ret != sizeof(buffer)) return NULL;
	size = (uint64_t)buffer[0] << 56 | (uint64_t)buffer[1] << 48 |
	       (uint64_t)buffer[2] << 40 | (uint64_t)buffer[3] << 32 |
	       (uint64_t)buffer[4] << 24 | (uint64_t)buffer[5] << 16 |
	       (uint64_t)buffer[6] << 8 | (uint64_t)buffer[7];
    }
    _UzmtpMsg *msg = uzmtp_msg_new(flags, size);
    if (!msg) return NULL;
    ret = uzmtp_net_recv(sock, msg->data, size);
    if (ret <= 0) uzmtp_msg_destroy(&msg);
    return msg;
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
