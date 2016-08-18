/*
 * uzmtp_dealer.h
 */

#ifndef UZMTP_DEALER_H_
#define UZMTP_DEALER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "os/net.h"
#include "uzmtp_msg.h"

typedef int (*tx_fn)(_UzmtpSocket *, const unsigned char *, size_t);
typedef int (*rx_fn)(_UzmtpSocket *, unsigned char *, size_t);

typedef struct {
    _UzmtpSocket conn;
    _TlsCtx *ctx;      /* Simplify API if user is only using single TLS ctx */
    _TlsCtx **ctx_ref; /* share ctx when need more than 1 tls dealer socket */
    tx_fn tx;	  /*!< tls/net tx method */
    rx_fn rx;	  /*!< tls/net rx method */
} _UzmtpDealer;

// Public
_UzmtpDealer *uzmtp_dealer_new();
void uzmtp_dealer_free(_UzmtpDealer **);
int uzmtp_dealer_use_tls(_UzmtpDealer *, _TlsCtx **);
int uzmtp_dealer_use_server_pem(_UzmtpDealer *, const uchar *, size_t);
int uzmtp_dealer_use_client_pem(_UzmtpDealer *, const uchar *, size_t);
int uzmtp_dealer_connect_endpoint(_UzmtpDealer *, const char *);
int uzmtp_dealer_connect(_UzmtpDealer *self, const char *host, int port);
int uzmtp_dealer_send(_UzmtpDealer *, _UzmtpMsg *);
_UzmtpMsg *uzmtp_dealer_recv(_UzmtpDealer *);
int uzmtp_dealer_poll(_UzmtpDealer *self, int time);
int uzmtp_dealer_socket(_UzmtpDealer *);

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif
