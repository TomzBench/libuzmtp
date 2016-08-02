// net.c

#include "net.h"

int uzmtp_net_connect(_UzmtpSocket *s, const char *ip, int port) {
    struct in_addr binip;
    struct sockaddr_in addr;

    *s = socket(AF_INET, SOCK_STREAM, 0);
    if (*s == RTCS_SOCKET_ERROR) return -1;
    inet_pton(AF_INET, ip, &binip, sizeof(binip));
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = port;
    addr.sin_addr = binip;
    uint32_t is_err = 0;
    uint32_t option = 5000;
    is_err |= setsockopt(*s, SOL_TCP, OPT_SEND_TIMEOUT, &option,  //
			 sizeof(option));
    option = 750;
    is_err |= setsockopt(*s, SOL_TCP, OPT_CONNECT_TIMEOUT, &option,
			 sizeof(option));
    option = 5000;
    is_err |= setsockopt(*s, SOL_TCP, OPT_RECEIVE_TIMEOUT, &option,
			 sizeof(option));
    option = FALSE;
    is_err |= setsockopt(*s, SOL_TCP, OPT_RECEIVE_NOWAIT, &option,  //
			 sizeof(option));
    option = FALSE;
    is_err |= setsockopt(*s, SOL_TCP, OPT_SEND_NOWAIT, &option,  //
			 sizeof(option));
    option = TRUE;
    is_err |= setsockopt(*s, SOL_TCP, OPT_RECEIVE_PUSH, &option,  //
			 sizeof(option));
    option = 2048;
    is_err |= setsockopt(*s, SOL_TCP, OPT_TBSIZE, &option,  //
			 sizeof(option));
    option = 2048;
    is_err |= setsockopt(*s, SOL_TCP, OPT_RBSIZE, &option,  //
			 sizeof(option));

    const int rc = connect(*s, &addr, sizeof(addr));
    if (rc != RTCS_OK) {
	// freeaddrinfo(addrinfo);
	uint32_t err = RTCS_geterror(*s);
	((void)err);
	shutdown(*s, FLAG_ABORT_CONNECTION);
	return -1;
    }
    return 0;
}

int uzmtp_net_recv(_UzmtpSocket *s, unsigned char *b, size_t len) {
    int bytes_read = 0;
    while (bytes_read < len) {
	const int n =
	    recv(*s, (char *)b + bytes_read, (len - bytes_read), 0);
	if (n == -1)
	    break;
	else if (n == 0)
	    break;
	else {
	    bytes_read += n;
	}
    }
    return bytes_read;
}

int uzmtp_net_send(_UzmtpSocket *s, const unsigned char *b, size_t len) {
    int bytes_sent = 0;
    while (bytes_sent < len) {
	const int rc =
	    send(*s, (char *)b + bytes_sent, (len - bytes_sent), 0);
	if (rc == 0) {
	    break;
	} /*!<socket closed */
	else if (rc == -1) {
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

void uzmtp_net_close(_UzmtpSocket *s) { shutdown(*s, FLAG_CLOSE_TX); }

//
//
//
