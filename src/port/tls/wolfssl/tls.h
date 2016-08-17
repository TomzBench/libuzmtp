/*
 * tls.h
 */

#ifndef TLS_H_
#define TLS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <wolfssl/ssl.h>

typedef WOLFSSL_CTX _TlsCtx;

int tls_send(_TlsCtx *, const unsigned char *, size_t);

int tls_recv(_TlsCtx *, unsigned char *, size_t);

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif
