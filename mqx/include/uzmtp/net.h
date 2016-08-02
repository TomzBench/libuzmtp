/*
 * net.h
 */

#ifndef NET_H_
#define NET_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <mqx.h>
#include <bsp.h>
#include <rtcs.h>
#include <ipcfg.h>
#include <mutex.h>
#define uzmtp_malloc _mem_alloc
#define uzmtp_free _mem_free
#define assert(x) 

typedef int _UzmtpSocket;

int uzmtp_net_connect(_UzmtpSocket *, const char *, int port);

int uzmtp_net_recv(_UzmtpSocket *, unsigned char *buff, size_t len);

int uzmtp_net_send(_UzmtpSocket *, const unsigned char *, size_t len);

int uzmtp_net_select(int *sock, int nsock, int time);

void uzmtp_net_close(_UzmtpSocket *);

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif
