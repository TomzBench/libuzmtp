/*
 * net.h
 */

#ifndef NET_H_
#define NET_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <bsp.h>
#include <ipcfg.h>
#include <mqx.h>
#include <mutex.h>
#include <rtcs.h>
#define uzmtp_malloc _mem_alloc
#define uzmtp_free _mem_free
#define assert(x)

typedef struct _TlsCtx TlsCtx;
typedef struct _TlsSocket TlsSocket;

typedef struct {
    TlsSocket *ctx;
    int sock;
} _UzmtpSocket;

int uzmtp_net_connect(_UzmtpSocket *, const char *, int port);

int uzmtp_net_socket(_UzmtpSocket *);

int uzmtp_net_recv(_UzmtpSocket *, unsigned char *buff, size_t len);

int uzmtp_net_send(_UzmtpSocket *, const unsigned char *, size_t len);

int uzmtp_net_select(int *sock, int nsock, int time);

void uzmtp_net_close(_UzmtpSocket *);

static inline _TlsCtx *uzmtp_tls_new() { return tls_new(); }

static inline void uzmtp_tls_free(_TlsCtx **self) { return tls_free(self_p); }

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif
