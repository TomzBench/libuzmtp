/*
 * uzmtp_dealer.c
 */

#include "uzmtp_dealer.h"

int recv_signature_fn(_UzmtpDealer *, _UzmtpMsg **msgs, int msgcount);

struct uzmtp_greeting {
    uint8_t signature[10];
    uint8_t version[2];
    uint8_t mechanism[20];
    uint8_t as_server[1];
    uint8_t filler[31];
};

static const struct uzmtp_greeting greeting = {
    .signature = {0xff, 0, 0, 0, 0, 0, 0, 0, 1, 0x7f},
    .version = {3, 0},
    .mechanism = {'N', 'U', 'L', 'L', '\0'}};

_UzmtpDealer *uzmtp_dealer_new() {
    _UzmtpDealer *d = uzmtp_malloc(sizeof(_UzmtpDealer));
    if (!d) return NULL;
    memset(d, 0, sizeof(_UzmtpDealer));
    d->tx = uzmtp_net_send;
    d->rx = uzmtp_net_recv;
    return d;
}

void uzmtp_dealer_free(_UzmtpDealer **self_p) {
    assert(self_p);
    assert(*self_p);
    _UzmtpDealer *self = *self_p;
    *self_p = 0;
    if (uzmtp_net_socket(&self->conn)) {
	uzmtp_net_close(&self->conn);
    }
    if (self->ctx) {
	uzmtp_tls_free(&self->ctx);
    }
    uzmtp_free(self);
}

int uzmtp_dealer_use_tls(_UzmtpDealer *d, _TlsCtx **ctx_ref) {
    if (ctx_ref) {
	d->ctx_ref = ctx_ref;
    } else {
	d->ctx = uzmtp_tls_new();
	d->ctx_ref = &d->ctx;
	if (!d->ctx) return -1;
    }
    d->tx = uzmtp_tls_send;
    d->rx = uzmtp_tls_recv;
    return 0;
}

int uzmtp_dealer_use_server_pem(_UzmtpDealer *d, const uchar *pem, size_t len) {
    return tls_server_cert(d->ctx_ref, pem, len);
}

int uzmtp_dealer_use_client_pem(_UzmtpDealer *d, const uchar *pem, size_t len) {
    return -1;
}

int uzmtp_dealer_connect_endpoint(_UzmtpDealer *dealer, const char *ep) {
    if ((memcmp(ep, "tcp://", 6))) return -1;
    char *colon = strchr(&ep[6], ':');
    if (colon == NULL) return -1;
    int addr_len = colon - ep - 6;
    char addr[addr_len + 1];
    memcpy(addr, &ep[6], addr_len);
    addr[addr_len] = 0;
    char *portstr = colon + 1;
    int port = atoi(portstr);
    return uzmtp_dealer_connect(dealer, addr, port);
}

int uzmtp_dealer_connect(_UzmtpDealer *self, const char *host, int port) {
    struct uzmtp_greeting incoming;
    assert(self);
    if (uzmtp_net_socket(&self->conn)) return -1;
    int ret = uzmtp_net_connect(&self->conn, host, port);
    if (ret != 0) return -1;
    if (self->ctx_ref) {
	ret = uzmtp_tls_connect(self->ctx_ref, &self->conn);
    }
    if (ret != 0) return -1;
    ret = self->tx(&self->conn, (const unsigned char *)&greeting,
		   sizeof(greeting));
    if (ret != 64) return -1;
    ret = self->rx(&self->conn, (unsigned char *)&incoming, sizeof(incoming));

    // Send Ready
    _UzmtpMsg *ready = uzmtp_msg_from_const_data(0x04, "\5READY", 6);
    ret = uzmtp_dealer_send(self, ready);
    if (ret != 0) {
	uzmtp_msg_destroy(&ready);
	return ret;
    }
    uzmtp_msg_destroy(&ready);

    // Recv Ready
    ready = uzmtp_dealer_recv(self);
    if (!ready) return -1;
    uzmtp_msg_destroy(&ready);
    return ret;
}

int uzmtp_dealer_send(_UzmtpDealer *self, _UzmtpMsg *msg) {
    assert(msg);
    assert(&self->conn);
    int ret = self->tx(&self->conn, &msg->flags, sizeof(msg->flags));
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
	ret = self->tx(&self->conn, buffer, sizeof(buffer));
	if (ret != sizeof(buffer)) return -1;
	uint64_t c = self->tx(&self->conn, uzmtp_msg_data(msg), msg_size);
	if (c != msg_size) return -1;
    } else {
	const uint8_t msg_size = uzmtp_msg_size(msg);
	ret = self->tx(&self->conn, &msg_size, sizeof(msg_size));
	if (ret != sizeof(msg_size)) return -1;
	ret = self->tx(&self->conn, uzmtp_msg_data(msg), msg_size);
	if (ret != msg_size) return -1;
    }

    return 0;
}

_UzmtpMsg *uzmtp_dealer_recv(_UzmtpDealer *self) {
    assert(msg);
    uint8_t flags;
    size_t size;
    int ret = self->rx(&self->conn, &flags, 1);

    if (ret != 1) return NULL;
    //  Check large flag
    if ((flags & UZMTP_MSG_LARGE) == 0) {
	uint8_t buffer[1];
	ret = self->rx(&self->conn, buffer, 1);
	if (ret != 1) return NULL;
	size = (size_t)buffer[0];
    } else {
	uint8_t buffer[8];
	ret = self->rx(&self->conn, buffer, sizeof(buffer));
	if (ret != sizeof(buffer)) return NULL;
	size = (uint64_t)buffer[0] << 56 | (uint64_t)buffer[1] << 48 |
	       (uint64_t)buffer[2] << 40 | (uint64_t)buffer[3] << 32 |
	       (uint64_t)buffer[4] << 24 | (uint64_t)buffer[5] << 16 |
	       (uint64_t)buffer[6] << 8 | (uint64_t)buffer[7];
    }

    _UzmtpMsg *msg = uzmtp_msg_new(flags, size);
    if (!msg) return NULL;
    ret = self->rx(&self->conn, msg->data, size);
    if (ret <= 0) uzmtp_msg_destroy(&msg);
    return msg;
}

int uzmtp_dealer_poll(_UzmtpDealer *self, int time) {
    if (!uzmtp_net_socket(&self->conn)) return -1;
    return uzmtp_net_select(&self->conn.sock, 1, time);
}

int uzmtp_dealer_socket(_UzmtpDealer *self) {
    return uzmtp_net_socket(&self->conn);
}

_TlsCtx *uzmtp_dealer_tls_new() { return uzmtp_tls_new(); }
void uzmtp_dealer_tls_free(_TlsCtx **ctx_p) { return uzmtp_tls_free(ctx_p); }

//
//
//
