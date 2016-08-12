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

typedef struct {
    void *ctx;
    int sock;
}_UzmtpSocket;

int uzmtp_net_connect(_UzmtpSocket *, const char *, int port);

int uzmtp_net_socket(_UzmtpSocket *);

int uzmtp_net_recv(_UzmtpSocket *, unsigned char *buff, size_t len);

int uzmtp_net_send(_UzmtpSocket *, const unsigned char *, size_t len);

int uzmtp_net_select(int *sock, int nsock, int time);

void uzmtp_net_close(_UzmtpSocket *);

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif
