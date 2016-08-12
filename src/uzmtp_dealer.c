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
    memset(d, 0, sizeof(_UzmtpDealer));
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
    uzmtp_free(self);
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
    ret = uzmtp_net_send(&self->conn, (const unsigned char *)&greeting,
			 sizeof(greeting));
    if (ret != 64) return -1;
    ret = uzmtp_net_recv(&self->conn, (unsigned char *)&incoming,
			 sizeof(incoming));

    // Send Ready
    _UzmtpMsg *ready = uzmtp_msg_from_const_data(0x04, "\5READY", 6);
    ret = uzmtp_msg_send(ready, &self->conn);
    if (ret != 0) {
	uzmtp_msg_destroy(&ready);
	return ret;
    }
    uzmtp_msg_destroy(&ready);

    // Recv Ready
    ready = uzmtp_msg_recv(&self->conn);
    if (!ready) return -1;
    uzmtp_msg_destroy(&ready);
    return ret;
}

int uzmtp_dealer_send(_UzmtpDealer *self, _UzmtpMsg *msg) {
    return uzmtp_msg_send(msg, &self->conn);
}

_UzmtpMsg *uzmtp_dealer_recv(_UzmtpDealer *self) {
    return uzmtp_msg_recv(&self->conn);
}

int uzmtp_dealer_poll(_UzmtpDealer *self, int time) {
    if (!uzmtp_net_socket(&self->conn)) return -1;
    return uzmtp_net_select(&self->conn.sock, 1, time);
}

int uzmtp_dealer_socket(_UzmtpDealer *self) {
    return uzmtp_net_socket(&self->conn);
}

//
//
//
