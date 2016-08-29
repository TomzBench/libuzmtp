#include "tls.h"

// Prototypes IO callbacks for send and recv
int tls_io_tx(_TlsSocket*, char*, int, void*);
int tls_io_rx(_TlsSocket*, char*, int, void*);

// Provide pointers for user override of TLS send and recv
int (*user_tx)(_TlsSocket*, char*, int, void*) = tls_io_tx;
int (*user_rx)(_TlsSocket*, char*, int, void*) = tls_io_rx;

/**
 * @brief Creats WOLFSSL_CTX context.
 *
 * @return WOLFSSL_CTX or NULL if error.
 */
_TlsCtx* tls_new() {
    _TlsCtx* ctx;
    wolfSSL_Init();
    if ((ctx = wolfSSL_CTX_new(wolfTLSv1_2_client_method())) == NULL) {
	return NULL;
    }
    wolfSSL_SetIORecv(ctx, user_rx);
    wolfSSL_SetIOSend(ctx, user_tx);
    return ctx;
}

/**
 * @brief Cleanup our TLS resources.
 *
 * @param tls_p created with tls_new()
 */
void tls_free(_TlsCtx** tls_p) {
    _TlsCtx* tls = *tls_p;
    *tls_p = 0;
    wolfSSL_CTX_free(tls);
    wolfSSL_Cleanup();
}

_TlsSocket* tls_connect(_TlsCtx** ctx_p, int sockfd) {
    // char buffer[80];
    _TlsSocket* tls = wolfSSL_new(*ctx_p);
    if (tls == NULL) return NULL;
    wolfSSL_set_fd(tls, sockfd);
    if (wolfSSL_connect(tls) != SSL_SUCCESS) {
	// int ret = wolfSSL_get_error(tls, 0);
	// wolfSSL_ERR_error_string(ret, buffer);
	wolfSSL_free(tls);
	return NULL;
    } else {
	return tls;
    }
}

void tls_close(_TlsSocket** tls_p) {
    _TlsSocket* tls = *tls_p;
    *tls_p = 0;
    wolfSSL_free(tls);
}

void tls_override_tx(tls_tx_fn fn) { user_tx = fn; }

void tls_override_rx(tls_rx_fn fn) { user_rx = fn; }

int tls_send(_TlsSocket* tls, const unsigned char* b, size_t len) {
    // char buffer[80];
    int ret = wolfSSL_write(tls, b, len);
    if ((ret < 0) || (ret != (int)len)) {
	/* the message is not able to send, or error trying */
	// ret = wolfSSL_get_error(tls, 0);
	// wolfSSL_ERR_error_string(ret, buffer);
    }
    return ret;
}

int tls_recv(_TlsSocket* tls, unsigned char* b, size_t len) {
    char buffer[80];
    int count = 0;
    while (count < (int)len) {
	int bytes = wolfSSL_recv(tls, &b[count], len - count, MSG_DONTWAIT);
	if (bytes < 0) {
	    int err = wolfSSL_get_error(tls, bytes);
	    if (err == SSL_ERROR_WANT_READ) {
		continue;
	    } else {
		wolfSSL_ERR_error_string(err, buffer);
		break;
	    }
	} else if (bytes == 0) {
	    break;  // client did normal close
	} else {
	    count += bytes;
	}
    }
    return count;
}

int tls_server_cert(_TlsCtx** ctx_p, const unsigned char* pem, int pemlen) {
    int ret =
	wolfSSL_CTX_load_verify_buffer(*ctx_p, pem, pemlen, SSL_FILETYPE_PEM);
    if (ret != SSL_SUCCESS) return -1;
    return 0;
}

int tls_io_tx(_TlsSocket* tls, char* b, int len, void* ctx) {
    int bytes_sent = 0;
    int sd = wolfSSL_get_fd(tls);
    ((void)ctx);
    while (bytes_sent < len) {
	const int32_t rc = send(sd, (char*)b + bytes_sent, len - bytes_sent, 0);
	if (rc == -1 && errno == EINTR) continue;
	if (rc == -1) return -1;
	if (rc == 0) break;
	bytes_sent += rc;
    }
    return bytes_sent;
}
int tls_io_rx(_TlsSocket* tls, char* b, int len, void* ctx) {
    int32_t bytes_read = 0;
    int sd = wolfSSL_get_fd(tls);
    ((void)ctx);
    while (bytes_read < len) {
	const int32_t n = recv(sd, (char*)b + bytes_read, len - bytes_read, 0);
	printf("errcode: %d\n", errno);
	if (n == -1 && errno == EINTR) continue;
	if (n == -1) return -1;
	if (n == 0) return bytes_read;
	bytes_read += n;
    }
    return bytes_read;
}

//
//
//
