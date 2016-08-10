/*
 * net.h
 */

#ifndef NET_H_
#define NET_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <assert.h>

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
