/*
 * uzmtp_msg.c
 */

#include "uzmtp_msg.h"

void
uzmtp_msg_init(
    uzmtp_msg__s* msg,
    uint8_t flags,
    void* data,
    size_t size,
    int greedy)
{
    msg->next = NULL;
    msg->flags = flags;
    msg->data = data;
    msg->size = size;
    msg->greedy = greedy;
    if (msg->size > 255) msg->flags |= UZMTP_MSG_LARGE;
}

void
uzmtp_msg_deinit(uzmtp_msg__s* msg)
{
    if (msg->greedy) uzmtp_free(msg->data);
}

uzmtp_msg__s*
uzmtp_msg_new(uint8_t flags, size_t size)
{
    uzmtp_msg__s* msg = uzmtp_malloc(sizeof(uzmtp_msg__s) + size);
    if (!msg) return NULL;
    uzmtp_msg_init(msg, flags, msg->pad, size, 0);
    return msg;
}

uzmtp_msg__s*
uzmtp_msg_new_from_data(uint8_t flags, uint8_t** data_p, size_t size)
{
    uzmtp_msg__s* msg = uzmtp_malloc(sizeof(uzmtp_msg__s));
    if (!msg) return NULL;
    uzmtp_msg_init(msg, flags, *data_p, size, 1);
    *data_p = NULL;
    return msg;
}

uzmtp_msg__s*
uzmtp_msg_new_from_const_data(uint8_t flags, void* data, size_t size)
{
    uzmtp_msg__s* msg = uzmtp_malloc(sizeof(uzmtp_msg__s));
    if (!msg) return NULL;
    uzmtp_msg_init(msg, flags, data, size, 0);
    return msg;
}

void
uzmtp_msg_destroy(uzmtp_msg__s** self_p)
{
    if (*self_p) {
        uzmtp_msg__s* self = *self_p;
        uzmtp_msg_deinit(self);
        uzmtp_free(self);
        *self_p = NULL;
    }
}

uint8_t
uzmtp_msg_flags(uzmtp_msg__s* self)
{
    return self->flags;
}

void
uzmtp_msg_set_more(uzmtp_msg__s* self)
{
    self->flags |= UZMTP_MSG_MORE;
}

void
uzmtp_msg_clr_more(uzmtp_msg__s* self)
{
    self->flags &= ~(UZMTP_MSG_MORE);
}

int
uzmtp_msg_is_more(uzmtp_msg__s* self)
{
    return ((self->flags & UZMTP_MSG_MORE) == UZMTP_MSG_MORE);
}

int
uzmtp_msg_is_large(uzmtp_msg__s* self)
{
    return ((self->flags & UZMTP_MSG_LARGE) == UZMTP_MSG_LARGE);
}

uint8_t*
uzmtp_msg_data(uzmtp_msg__s* self)
{
    return self->data;
}

size_t
uzmtp_msg_size(uzmtp_msg__s* self)
{
    return self->size;
}

void*
uzmtp_msg_next(uzmtp_msg__s* self)
{
    return self->next;
}

//
//
//
