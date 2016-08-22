/*
 * tls.h
 */

#ifndef TLS_H_
#define TLS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <uzmtp_settings.h>

#include <wolfssl/ssl.h>

// TODO - remove this include to uzmtp_settings.h?
#ifdef UZMTP_USE_LINUX
#include <sys/socket.h>
#endif

typedef WOLFSSL_CTX _TlsCtx;
typedef WOLFSSL _TlsSocket;

typedef int (*tls_tx_fn)(_TlsSocket*, char*, int, void*);
typedef int (*tls_rx_fn)(_TlsSocket*, char*, int, void*);

_TlsCtx *tls_new();

void tls_free(_TlsCtx **);

int tls_send(_TlsSocket *, const unsigned char *, size_t);

int tls_recv(_TlsSocket *, unsigned char *, size_t);

_TlsSocket *tls_connect(_TlsCtx **ctx_p, int sockfd);

void tls_close(_TlsSocket** tls_p);

int tls_server_cert(_TlsCtx**, const unsigned char*, int);

void tls_override_tx(tls_tx_fn);
void tls_override_rx(tls_rx_fn);

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif
