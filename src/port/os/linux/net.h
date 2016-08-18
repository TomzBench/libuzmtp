/*
 * net.h
 */

#ifndef NET_H_
#define NET_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>

#ifdef UZMTP_USE_WOLFSSL
#include "tls/wolfssl/tls.h"
#endif

typedef struct _TlsCtx TlsCtx;
typedef struct _TlsSocket TlsSocket;
typedef unsigned char uchar;

typedef struct {
    _TlsSocket *tls;
    int sock;
} _UzmtpSocket;

int uzmtp_net_connect(_UzmtpSocket *, const char *, int port);

int uzmtp_tls_connect(_TlsCtx **, _UzmtpSocket *);

int uzmtp_net_socket(_UzmtpSocket *);

int uzmtp_net_recv(_UzmtpSocket *, unsigned char *buff, size_t len);

int uzmtp_net_send(_UzmtpSocket *, const unsigned char *, size_t len);

int uzmtp_net_select(int *sock, int nsock, int time);

void uzmtp_net_close(_UzmtpSocket *);

static inline _TlsCtx *uzmtp_tls_new() { return tls_new(); }

static inline void uzmtp_tls_free(_TlsCtx **self_p) { return tls_free(self_p); }

static inline int uzmtp_tls_recv(_UzmtpSocket *s, unsigned char *b, size_t l) {
    return tls_recv(s->tls, b, l);
}

static inline int uzmtp_tls_send(_UzmtpSocket *s, const uchar *b, size_t l) {
    return tls_send(s->tls, b, l);
}

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif
