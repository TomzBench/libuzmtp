/*
 * uzmtp_msg.c
 */

#include "uzmtp/uzmtp_types.h"

/// NOTE - order of fields is important to maintain efficient layout and to
///        ensure our size assumption matches the opaque type when placing on
///        the stack
typedef struct uzmtp_msg__s
{
    union
    {
        struct
        {
            uint8_t* data;
            size_t size;
            uint8_t flags;
            uint8_t greedy;
        };
        max_align_t __phantom;
    };
} uzmtp_msg__s;

_Static_assert(
    sizeof(uzmtp_msg_s) == sizeof(uzmtp_msg__s),
    "invalid uzmtp_msg__s size assumption");

void
uzmtp_msg_init(uzmtp_msg__s* msg, uint8_t flags, void* data, size_t size)
{
    msg->flags = flags;
    msg->data = data;
    msg->size = size;
    msg->greedy = 0;
    if (msg->size > 255) msg->flags |= UZMTP_MSG_LARGE;
}

void
uzmtp_msg_init_str(uzmtp_msg__s* msg, uint8_t flags, const char* str)

{
    uzmtp_msg_init(msg, flags, (uint8_t*)str, strlen(str));
}

void
uzmtp_msg_init_mem(
    uzmtp_msg__s* msg,
    uint8_t flags,
    uint8_t** data,
    size_t size)
{
    uzmtp_msg_init(msg, flags, *data, size);
    *data = NULL;
    msg->greedy = 1;
}

void
uzmtp_msg_deinit(uzmtp_msg__s* msg)
{
    if (msg->greedy) uzmtp_free(msg->data);
    memset(msg, 0, sizeof(uzmtp_msg__s));
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
