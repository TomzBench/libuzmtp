// net.c

#include "net.h"

int _mqx_io_tx(_TlsSocket *s, char *, int, void *);
int _mqx_io_rx(_TlsSocket *s, char *, int, void *);

int uzmtp_net_connect(_UzmtpSocket *s, const char *ip, int port) {
    struct in_addr binip;
    struct sockaddr_in addr;

    s->sock = socket(AF_INET, SOCK_STREAM, 0);
    if (s->sock == RTCS_SOCKET_ERROR) return -1;
    inet_pton(AF_INET, ip, &binip, sizeof(binip));
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = port;
    addr.sin_addr = binip;
    uint32_t is_err = 0;
    uint32_t option = 25000;
    is_err |= setsockopt(s->sock, SOL_TCP, OPT_SEND_TIMEOUT, &option,  //
			 sizeof(option));
    option = 2750;
    is_err |= setsockopt(s->sock, SOL_TCP, OPT_CONNECT_TIMEOUT, &option,
			 sizeof(option));
    option = 25000;
    is_err |= setsockopt(s->sock, SOL_TCP, OPT_RECEIVE_TIMEOUT, &option,
			 sizeof(option));
    option = FALSE;
    is_err |= setsockopt(s->sock, SOL_TCP, OPT_RECEIVE_NOWAIT, &option,  //
			 sizeof(option));
    option = FALSE;
    is_err |= setsockopt(s->sock, SOL_TCP, OPT_SEND_NOWAIT, &option,  //
			 sizeof(option));
    option = TRUE;
    is_err |= setsockopt(s->sock, SOL_TCP, OPT_RECEIVE_PUSH, &option,  //
			 sizeof(option));
    option = 2048;
    is_err |= setsockopt(s->sock, SOL_TCP, OPT_TBSIZE, &option,  //
			 sizeof(option));
    option = 2048;
    is_err |= setsockopt(s->sock, SOL_TCP, OPT_RBSIZE, &option,  //
			 sizeof(option));

    const int rc = connect(s->sock, &addr, sizeof(addr));
    if (rc != RTCS_OK) {
	// freeaddrinfo(addrinfo);
	uint32_t err = RTCS_geterror(s->sock);
	((void)err);
	shutdown(s->sock, FLAG_ABORT_CONNECTION);
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
    return uzmtp_net_recv_fd(s->sock, b, len);
}
int uzmtp_net_recv_fd(int sockfd, unsigned char *b, size_t len) {
    int bytes_read = 0;
    while (bytes_read < len) {
	int active = uzmtp_net_select(&sockfd, 1, 200);
	if (active == sockfd) {
	    const int n = recv(sockfd, (char *)b + bytes_read,
			       (len - bytes_read), MSG_DONTWAIT);
	    if (n == -1) {
		uint32_t error = RTCS_geterror(sockfd);
		if (error == RTCSERR_TCP_TIMED_OUT) continue;
		else if(error == RTCSERR_TCP_CONN_CLOSING) return -1;
		break;
	    } else if (n == 0) {
		break;
	    } else {
		bytes_read += n;
	    }
	} else {
	    return bytes_read > 0 ? bytes_read : -1;
	}
    }
    return bytes_read;
}

int uzmtp_net_send(_UzmtpSocket *s, const unsigned char *b, size_t len) {
    return uzmtp_net_send_fd(s->sock, b, len);
}

int uzmtp_net_send_fd(int sockfd, const unsigned char *b, size_t len) {
    int bytes_sent = 0;
    while (bytes_sent < len) {
	const int rc =
	    send(sockfd, (char *)b + bytes_sent, (len - bytes_sent), 0);
	if (rc == 0) {
	    break;
	} /*!<socket closed */
	else if (rc == -1) {
	    if (errno == EINTR) continue;
	    bytes_sent = -1;
	    break; /*!<Socket error */
	} else
	    bytes_sent += rc;
    }
    return bytes_sent;
}

int uzmtp_net_select(int *sock, int nsock, int time) {
    return RTCS_selectset(sock, nsock, time);
}

void uzmtp_net_close(_UzmtpSocket *s) {
    shutdown(s->sock, FLAG_CLOSE_TX);
    if (s->tls) tls_close(&s->tls);
    s->sock = 0;
}

_TlsCtx *uzmtp_tls_new() {
    tls_override_tx(_mqx_io_tx);
    tls_override_rx(_mqx_io_rx);
    _TlsCtx *ctx = tls_new();
    if (!ctx) return NULL;
    return ctx;
}

int _mqx_io_tx(_TlsSocket *s, char *b, int len, void *ctx) {
    ((void)ctx);
    return uzmtp_net_send_fd(wolfSSL_get_fd(s), (uchar *)b, len);
}
int _mqx_io_rx(_TlsSocket *s, char *b, int len, void *ctx) {
    ((void)ctx);
    return uzmtp_net_recv_fd(wolfSSL_get_fd(s), (uchar *)b, len);
}

//
//
//
