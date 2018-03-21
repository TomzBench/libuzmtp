/*
 * uzmtp_dealer.c
 */

#include "uzmtp_dealer.h"

#define UERROR(dealer, x)                                                      \
    do {                                                                       \
        dealer->settings->on_error((void*)dealer, x);                          \
        dealer->state = UZMTP_NULL;                                            \
        dealer->ready = 0;                                                     \
        free_incoming(dealer);                                                 \
        d->b = 0;                                                              \
        if (dealer->curr_msg) uzmtp_msg_destroy(&dealer->curr_msg);            \
    } while (0)

typedef struct
{
    uint8_t signature[10];
    uint8_t version[2];
    uint8_t mechanism[20];
    uint8_t as_server[1];
    uint8_t filler[31];
} uzmtp_greeting;

static const uzmtp_greeting greeting = {
    .signature = { 0xff, 0, 0, 0, 0, 0, 0, 0, 1, 0x7f },
    .version = { 3, 0 },
    .mechanism = { 'N', 'U', 'L', 'L', '\0' },
    .filler = { '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
                '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
                '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
                '\0', '\0', '\0', '\0', '\0', '\0', '\0' }
};

// Private prototypes
static int process_command(uzmtp_dealer__s* d, uzmtp_msg_s* msg);
static void free_incoming(uzmtp_dealer__s* d);
static void push_incoming(uzmtp_dealer__s* d, uzmtp_msg_s** msg_p);
static uzmtp_msg_s* pop_incoming(uzmtp_dealer__s* d);

uzmtp_dealer__s*
uzmtp_dealer_new(uzmtp_dealer_settings* settings)
{
    uzmtp_dealer__s* dealer = uzmtp_malloc(sizeof(uzmtp_dealer__s));
    if (dealer) {
        memset(dealer, 0, sizeof(uzmtp_dealer__s));
        dealer->settings = settings;
        dealer->state = UZMTP_NULL;
    }
    return dealer;
}

void
uzmtp_dealer_destroy(uzmtp_dealer__s** self_p)
{
    uzmtp_dealer__s* self = *self_p;
    *self_p = 0;
    free_incoming(self);
    if (self->curr_msg) uzmtp_msg_destroy(&self->curr_msg);
    uzmtp_free(self);
}
void
uzmtp_dealer_context_set(uzmtp_dealer__s* dealer, void* context)
{
    dealer->context = context;
}

void*
uzmtp_dealer_context_get(uzmtp_dealer__s* dealer)
{
    return dealer->context;
}

uzmtp_connection*
uzmtp_dealer_connection_get(uzmtp_dealer__s* dealer)
{
    return dealer->connection;
}

EUZMTP_STATE
uzmtp_dealer_state_get(uzmtp_dealer__s* dealer)
{
    return dealer->state;
}

uint8_t
uzmtp_dealer_ready(uzmtp_dealer__s* d)
{
    return d->ready;
}

uzmtp_msg_s*
uzmtp_dealer_pop_incoming(uzmtp_dealer__s* d)
{
    return pop_incoming(d);
}

uint32_t
uzmtp_dealer_incoming_count(uzmtp_dealer__s* d)
{
    return d->n_incoming;
}

int
uzmtp_dealer_connect(uzmtp_dealer__s* dealer, uzmtp_connection* c)
{
    int err;
    if (!(dealer->state == UZMTP_NULL)) return -1;
    dealer->connection = c;
    err = dealer->settings->want_write(
        (void*)dealer, (const uint8_t*)&greeting, sizeof(greeting));
    if (!err) dealer->state++;
    return err;
}

int
uzmtp_dealer_parse(uzmtp_dealer__s* d, const uint8_t* bytes, uint32_t sz)
{
    int ret = 0, flags;
    const uint8_t* ptr = (const uint8_t*)&greeting;
    uint32_t c, remaining;
    uzmtp_msg_s* msg;

start:
    switch (d->state) {
        case UZMTP_NULL: return -1;
        case UZMTP_CONNECT_WANT_GREETING:
            remaining = 64 - d->b;
            c = sz > remaining ? remaining : sz;
            sz -= c;
            while ((c) && ((d->b == 10 && (!(*bytes >= 3))) ||
                           (d->b == 11 && (!(*bytes >= 0))) ||
                           (*bytes == ptr[d->b]))) {
                bytes++;
                d->b++;
                c--;
            }
            if (c) {
                UERROR(d, UZMTP_ERROR_VERSION);
                ret = -1;
                break;
            } else if (d->b == 64) {
                msg = uzmtp_msg_new_from_const_data(
                    UZMTP_MSG_COMMAND, "\5READY", 6);
                if (msg) {
                    ret = uzmtp_dealer_send(d, &msg);
                    if (ret == 0) d->state++;
                } else {
                    UERROR(d, UZMTP_ERROR_MEMORY);
                    ret = -1;
                    break;
                }
                d->b = 0;
            }
            if (!sz) break;
        case UZMTP_CONNECT_WANT_READY: d->state++;
        case UZMTP_RECV_FLAGS:
            d->curr_flags = *bytes;
            d->state++;
            bytes++;
            sz--;
            if (!sz) break;
        case UZMTP_RECV_LENGTH:
            if (d->curr_flags & UZMTP_MSG_LARGE) {
                remaining = 8 - d->b;
                c = sz > remaining ? remaining : sz;
                sz -= c;
                while (c) {
                    ((uint8_t*)&d->curr_size)[7 - d->b] = *bytes;
                    bytes++;
                    d->b++;
                    c--;
                }
                if (d->b == 8) {
                    d->b = 0;
                    d->state++;
                    if (d->curr_msg) uzmtp_msg_destroy(&d->curr_msg);
                    d->curr_msg = uzmtp_msg_new(d->curr_flags, d->curr_size);
                    if (!d->curr_msg) {
                        UERROR(d, UZMTP_ERROR_MEMORY);
                        ret = -1;
                        break;
                    }
                }
            } else {
                d->curr_size = *bytes;
                bytes++;
                sz--;
                d->state++;
                if (d->curr_msg) uzmtp_msg_destroy(&d->curr_msg);
                d->curr_msg = uzmtp_msg_new(d->curr_flags, d->curr_size);
                if (!d->curr_msg) {
                    UERROR(d, UZMTP_ERROR_MEMORY);
                    ret = -1;
                    break;
                }
            }
            if (!sz) break;
        case UZMTP_RECV_BODY:
            remaining = d->curr_size - d->b;
            c = sz > remaining ? remaining : sz;
            memcpy(uzmtp_msg_data(d->curr_msg) + d->b, bytes, c);
            d->b += c;
            bytes += c;
            sz -= c;
            if (d->b == d->curr_size) {
                d->b = 0;
                d->curr_size = d->curr_flags = 0;
                d->state = UZMTP_RECV_FLAGS;
                flags = uzmtp_msg_flags(d->curr_msg);
                if (flags & UZMTP_MSG_COMMAND) {
                    ret = process_command(d, d->curr_msg);
                    if (ret == 0) {
                        uzmtp_msg_destroy(&d->curr_msg);
                    } else {
                        UERROR(d, UZMTP_ERROR_PROTOCOL);
                        ret = -1;
                    }
                } else if (!(flags & UZMTP_MSG_MORE)) {
                    push_incoming(d, &d->curr_msg);
                    ret = d->settings->on_recv((void*)d, d->n_incoming);
                    if (ret < 0) {
                        UERROR(d, UZMTP_ERROR_RECV);
                        ret = -1;
                    }
                    free_incoming(d); // free whatever app didn't pop
                } else if (flags & UZMTP_MSG_MORE) {
                    push_incoming(d, &d->curr_msg);
                }
            }
            break;
    }

    // if more to parse and not in an error state, parse some more
    if (sz && !ret) goto start;
    return ret;
}

int
uzmtp_dealer_send(uzmtp_dealer__s* d, uzmtp_msg__s** msg_p)
{
    int err;
    uzmtp_msg_s* msg = *msg_p;
    uint8_t prefix[9] = { uzmtp_msg_flags(msg) };
    const uint64_t msg_size = (uint64_t)uzmtp_msg_size(msg);

    // TODO - if this is not a command and remote not ready, return -1

    // Send header
    if (uzmtp_msg_flags(msg) & UZMTP_MSG_LARGE) {
        prefix[1] = msg_size >> 56;
        prefix[2] = msg_size >> 48;
        prefix[3] = msg_size >> 40;
        prefix[4] = msg_size >> 32;
        prefix[5] = msg_size >> 24;
        prefix[6] = msg_size >> 16;
        prefix[7] = msg_size >> 8;
        prefix[8] = msg_size;
        err = d->settings->want_write((void*)d, prefix, sizeof(prefix));
    } else {
        prefix[1] = uzmtp_msg_size(msg);
        err = d->settings->want_write((void*)d, prefix, 2);
    }

    // Send body
    if (!err) {
        err = d->settings->want_write(
            (void*)d, uzmtp_msg_data(msg), uzmtp_msg_size(msg));
    }

    // Only free outgoing if no error
    if (!err) {
        *msg_p = NULL;
        uzmtp_msg_destroy(&msg);
    } else {
        UERROR(d, UZMTP_ERROR_SEND);
    }
    return err;
}

static int
process_command(uzmtp_dealer__s* d, uzmtp_msg_s* msg)
{
    if ((uzmtp_msg_size(msg) >= 6) &&
        !(memcmp("\5READY", uzmtp_msg_data(msg), 6))) {
        d->ready = 1;
        return 0;
    }
    return -1;
}

static void
push_incoming(uzmtp_dealer__s* d, uzmtp_msg_s** msg_p)
{
    if (d->tail) {
        d->tail->next = *msg_p;
        d->tail = d->tail->next;
        d->n_incoming++;
        *msg_p = NULL;
    } else {
        d->incoming = d->tail = *msg_p;
        d->n_incoming++;
        *msg_p = NULL;
    }
}
static uzmtp_msg_s*
pop_incoming(uzmtp_dealer__s* d)
{
    uzmtp_msg_s* next = NULL;
    if (d->incoming) {
        next = d->incoming;
        d->incoming = d->incoming->next;
        d->n_incoming--;
        if (!d->incoming) d->tail = NULL;
    }
    return next;
}

static void
free_incoming(uzmtp_dealer__s* d)
{
    uzmtp_msg_s* deleteme;
    while (d->incoming) {
        deleteme = pop_incoming(d);
        uzmtp_msg_destroy(&deleteme);
    }
}

//
//
//
