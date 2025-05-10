// Mock OpenSSL Crypto header for WebAssembly build
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef struct evp_cipher_ctx_st EVP_CIPHER_CTX;
typedef struct evp_cipher_st EVP_CIPHER;

EVP_CIPHER_CTX *EVP_CIPHER_CTX_new(void);
void EVP_CIPHER_CTX_free(EVP_CIPHER_CTX *ctx);

#ifdef __cplusplus
}
#endif
