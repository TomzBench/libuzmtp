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
        uzmtp_max_align __phantom;
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
uzmtp_msg_flags_set(uzmtp_msg__s* self, uint8_t flags)
{
    self->flags |= flags;
}

void
uzmtp_msg_flags_clr(uzmtp_msg__s* self, uint8_t flags)
{
    self->flags &= ~flags;
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

void
uzmtp_msg_data_set(uzmtp_msg__s* self, uint8_t* data)
{
    self->data = data;
}

size_t
uzmtp_msg_size(uzmtp_msg__s* self)
{
    return self->size;
}

void
uzmtp_msg_size_set(uzmtp_msg__s* self, size_t size)
{
    self->size = size;
}

size_t
uzmtp_msg_print_head(uint8_t* dst, size_t blen, uint8_t flags, int64_t msg_len)
{
    if (msg_len > 255) {
        flags |= UZMTP_MSG_LARGE;
        if (!(blen < 9)) {
            dst[0] = flags;
            dst[1] = msg_len >> 56;
            dst[2] = msg_len >> 48;
            dst[3] = msg_len >> 40;
            dst[4] = msg_len >> 32;
            dst[5] = msg_len >> 24;
            dst[6] = msg_len >> 16;
            dst[7] = msg_len >> 8;
            dst[8] = msg_len;
        }
        return 9;
    }
    else {
        if (!(blen < 2)) {
            dst[0] = flags;
            dst[1] = msg_len;
        }
        return 2;
    }
}
