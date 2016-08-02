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

typedef struct {
    _UzmtpSocket conn;
} _UzmtpDealer;

// Public
_UzmtpDealer *uzmtp_dealer_new();
void uzmtp_dealer_free(_UzmtpDealer **);
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
