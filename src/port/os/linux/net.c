// net.c

#include "net.h"

int uzmtp_net_connect(_UzmtpSocket *sock, const char *host, int port) {
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    if ((sock->sock = (int)socket(AF_INET, SOCK_STREAM, 0)) < 0) {
	return -1;
    }
    inet_pton(AF_INET, host, &addr.sin_addr);
    int rc = connect(sock->sock, (struct sockaddr *)&addr, sizeof(addr));
    if (rc < 0) {
	close(sock->sock);
	return -1;
    }
    return 0;
}

int uzmtp_tls_connect(_TlsCtx **ctx, _UzmtpSocket *sock) {
    sock->tls = tls_connect(ctx, sock->sock);
    if (!sock->tls) {
	return -1;
    } else {
	return 0;
    }
}

int uzmtp_net_socket(_UzmtpSocket *s) { return s->sock; }

int uzmtp_net_recv(_UzmtpSocket *s, unsigned char *b, size_t len) {
    int32_t bytes_read = 0;
    while (bytes_read < len) {
	const int32_t n =
	    recv(s->sock, (char *)b + bytes_read, len - bytes_read, 0);
	if (n == -1 && errno == EINTR) continue;
	if (n == -1) return -1;
	if (n == 0) return bytes_read;
	bytes_read += n;
    }
    return bytes_read;
}

int uzmtp_net_send(_UzmtpSocket *s, const unsigned char *b, size_t len) {
    size_t bytes_sent = 0;
    while (bytes_sent < len) {
	const int32_t rc =
	    send(s->sock, (char *)b + bytes_sent, len - bytes_sent, 0);
	if (rc == -1 && errno == EINTR) continue;
	if (rc == -1) return -1;
	if (rc == 0) break;
	bytes_sent += rc;
    }
    return bytes_sent;
}

int uzmtp_net_select(int *sock, int nsock, int time) {
    if (!nsock) return -1;
    fd_set readfds;
    FD_ZERO(&readfds);

    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = (time * 1000);

    int *sock_p = sock;
    int max_fd = *sock_p;
    for (int i = 0; i < nsock; i++) {
	if (*sock_p >= 0) FD_SET(*sock_p, &readfds);
	if (*sock_p > max_fd) max_fd = *sock_p;
	sock_p++;
    }
    int result = select(max_fd + 1, &readfds, NULL, NULL, &tv);
    if (result < 0) return result;
    sock_p = sock;
    for (int i = 0; i < nsock; i++) {
	if (FD_ISSET(*sock_p, &readfds)) {
	    if ((!*sock_p) && (nsock == 1)) return true;
	    return *sock_p;
	}
	sock_p++;
    }
    return 0;
}

void uzmtp_net_close(_UzmtpSocket *s) {
    close(s->sock);
    if(s->tls) tls_close(&s->tls);
    s->sock = 0;
}

//
//
//
