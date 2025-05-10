// Mock OpenSSL SSL header for WebAssembly build
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ssl_st SSL;
typedef struct ssl_ctx_st SSL_CTX;

SSL_CTX *SSL_CTX_new(void);
void SSL_CTX_free(SSL_CTX *ctx);
SSL *SSL_new(SSL_CTX *ctx);
void SSL_free(SSL *ssl);

#ifdef __cplusplus
}
#endif
