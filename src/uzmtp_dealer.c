/*
 * uzmtp_dealer.c
 */

#include "uzmtp/uzmtp.h"

// Main context
typedef struct
{
    union
    {
        struct
        {
            uzmtp_connection* connection;
            uzmtp_send_fn send;
            void* context;
            EUZMTP_STATE state;
            size_t curr_size;
            uint32_t b;
            uint32_t m;
            uint8_t ready;
        };
        max_align_t __phanton;
    };
} uzmtp_dealer__s;

_Static_assert(
    sizeof(uzmtp_dealer_s) == sizeof(uzmtp_dealer__s),
    "invalid uzmtp_msg__s size assumption");

#define UERROR(dealer)                                                         \
    do {                                                                       \
        dealer->state = UZMTP_NULL;                                            \
        dealer->ready = 0;                                                     \
        dealer->curr_size = 0;                                                 \
        d->b = 0;                                                              \
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

void
uzmtp_dealer_init(uzmtp_dealer_s* dealer, uzmtp_send_fn fn, void* ctx)
{
    memset(dealer, 0, sizeof(uzmtp_dealer__s));
    ((uzmtp_dealer__s*)dealer)->send = fn;
    ((uzmtp_dealer__s*)dealer)->state = UZMTP_NULL;
    ((uzmtp_dealer__s*)dealer)->context = ctx;
}

void
uzmtp_dealer_deinit(uzmtp_dealer_s* self_p)
{
    uzmtp_dealer__s* self = (uzmtp_dealer__s*)self_p;
    memset(self, 0, sizeof(uzmtp_dealer__s));
}

void*
uzmtp_dealer_context(uzmtp_dealer_s* dealer)
{
    return ((uzmtp_dealer__s*)dealer)->context;
}

void
uzmtp_dealer_context_set(uzmtp_dealer_s* dealer, void* context)
{
    ((uzmtp_dealer__s*)dealer)->context = context;
}

uzmtp_connection*
uzmtp_dealer_connection(uzmtp_dealer_s* dealer)
{
    return ((uzmtp_dealer__s*)dealer)->connection;
}

EUZMTP_STATE
uzmtp_dealer_state(uzmtp_dealer_s* dealer)
{
    return ((uzmtp_dealer__s*)dealer)->state;
}

uint8_t
uzmtp_dealer_ready(uzmtp_dealer_s* d)
{
    return ((uzmtp_dealer__s*)d)->ready;
}

int
uzmtp_dealer_connect(uzmtp_dealer_s* d, uzmtp_connection* c)
{
    int err = UZMTP_ERROR_SEND;
    uzmtp_dealer__s* dealer = (uzmtp_dealer__s*)d;
    if (!(dealer->state == UZMTP_NULL)) return -1;
    dealer->connection = c;
    if (!dealer->send(d, (const uint8_t*)&greeting, sizeof(greeting))) {
        dealer->state++;
        err = 0;
    }
    return err;
}

int
uzmtp_dealer_parse(
    uzmtp_dealer_s* dealer,
    const uint8_t* bytes,
    uint32_t sz,
    uzmtp_msg_s* msgs,
    uint32_t n_msg)
{
    int ret = 0, flags;
    const uint8_t* ptr = (const uint8_t*)&greeting;
    uint32_t c, remaining;
    uzmtp_dealer__s* d = (uzmtp_dealer__s*)dealer;

start:
    switch (d->state) {
        case UZMTP_NULL: return -1;
        case UZMTP_CONNECT_WANT_GREETING:
            remaining = 64 - d->b;
            c = sz > remaining ? remaining : sz;
            sz -= c;
            while ((c) && ((d->b == 10 && ((*bytes >= 3))) ||
                           (d->b == 11 && ((*bytes >= 0))) ||
                           (d->b > 1 && d->b < 9) || (*bytes == ptr[d->b]))) {
                bytes++;
                d->b++;
                c--;
            }
            if (c) {
                UERROR(d);
                ret = UZMTP_ERROR_VERSION;
                break;
            }
            else if (d->b == 64) {
                uzmtp_msg_s msg;
                uzmtp_msg_init_str(&msg, UZMTP_MSG_COMMAND, "\5READY");
                ret = uzmtp_dealer_send(dealer, &msg);
                if (ret == 0) d->state++;
                d->b = 0;
            }
            if (!sz) break;
        case UZMTP_CONNECT_WANT_READY: d->state++;
        case UZMTP_RECV_FLAGS:
            if (d->m < n_msg) {
                uzmtp_msg_init(&msgs[d->m], *bytes, NULL, 0);
                d->state++;
                bytes++;
                sz--;
            }
            else {
                ret = UZMTP_ERROR_OVERFLOW;
                break;
            }
            if (!sz) break;
        case UZMTP_RECV_LENGTH:
            if (uzmtp_msg_is_large(&msgs[d->m])) {
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
                    uzmtp_msg_size_set(&msgs[d->m], d->curr_size);
                    d->curr_size = 0;
                }
            }
            else {
                uzmtp_msg_size_set(&msgs[d->m], *bytes);
                bytes++;
                sz--;
                d->state++;
            }
            if (!sz) break;
        case UZMTP_RECV_BODY:
            remaining = uzmtp_msg_size(&msgs[d->m]) - d->b;
            c = sz > remaining ? remaining : sz;
            if (!uzmtp_msg_data(&msgs[d->m])) {
                // TODO should handle this in a seperate state
                //      is done this way because of refactor
                uzmtp_msg_data_set(&msgs[d->m], (uint8_t*)bytes);
            }
            d->b += c;
            bytes += c;
            sz -= c;
            if (d->b == uzmtp_msg_size(&msgs[d->m])) {
                d->b = 0;
                d->state = UZMTP_RECV_FLAGS;
                flags = uzmtp_msg_flags(&msgs[d->m]);
                if (flags & UZMTP_MSG_COMMAND) {
                    ret = process_command(d, &msgs[d->m]);
                    if (ret == 0) {
                        uzmtp_msg_deinit(&msgs[d->m]);
                    }
                    else {
                        UERROR(d);
                        ret = UZMTP_ERROR_PROTOCOL;
                    }
                }
                else {
                    d->m++;
                    if (!(flags & UZMTP_MSG_MORE)) {
                        ret = d->m;
                    }
                    else if (flags & UZMTP_MSG_MORE) {
                    }
                }
            }
            break;
    }

    // if more to parse and not in an error state, parse some more
    if (sz && !ret) goto start;
    return ret;
}

int
uzmtp_dealer_send(uzmtp_dealer_s* dealer, uzmtp_msg_s* msg)
{
    int err;
    uint8_t prefix[9] = { uzmtp_msg_flags(msg) };
    const uint64_t msg_size = (uint64_t)uzmtp_msg_size(msg);
    uzmtp_dealer__s* d = (uzmtp_dealer__s*)dealer;

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
        err = d->send((void*)d, prefix, sizeof(prefix));
    }
    else {
        prefix[1] = uzmtp_msg_size(msg);
        err = d->send((void*)d, prefix, 2);
    }

    // Send body
    if (!err) err = d->send((void*)d, uzmtp_msg_data(msg), uzmtp_msg_size(msg));
    if (err) {
        UERROR(d);
        err = UZMTP_ERROR_SEND;
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
