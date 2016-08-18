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
