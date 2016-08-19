#include "tls.h"

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
    // char buffer[80];
    int read = 0;
    while (read < (int)len) {
	int bytes = wolfSSL_read(tls, &b[read], len - read);
	if (bytes < 0) {
	    int err = wolfSSL_get_error(tls, 0);
	    if (err == SSL_ERROR_WANT_READ) {
		continue;
	    } else {
		// wolfSSL_ERR_error_string(err, buffer);
		break;
	    }
	} else if (read == 0) {
	    printf("%s\n", "connection closed");
	    break;  // client did normal close
	} else {
	    read += bytes;
	}
    }
    return read;
}

int tls_server_cert(_TlsCtx** ctx_p, const unsigned char* pem, int pemlen) {
    int ret =
	wolfSSL_CTX_load_verify_buffer(*ctx_p, pem, pemlen, SSL_FILETYPE_PEM);
    if (ret != SSL_SUCCESS) return -1;
    return 0;
}

//
//
//
