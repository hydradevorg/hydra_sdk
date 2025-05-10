// Mock OpenSSL EVP header for WebAssembly build
#pragma once

#include "crypto.h"

#ifdef __cplusplus
extern "C" {
#endif

int EVP_EncryptInit(EVP_CIPHER_CTX *ctx, const EVP_CIPHER *cipher,
                   const unsigned char *key, const unsigned char *iv);
int EVP_EncryptUpdate(EVP_CIPHER_CTX *ctx, unsigned char *out, int *outl,
                     const unsigned char *in, int inl);
int EVP_EncryptFinal(EVP_CIPHER_CTX *ctx, unsigned char *out, int *outl);

int EVP_DecryptInit(EVP_CIPHER_CTX *ctx, const EVP_CIPHER *cipher,
                   const unsigned char *key, const unsigned char *iv);
int EVP_DecryptUpdate(EVP_CIPHER_CTX *ctx, unsigned char *out, int *outl,
                     const unsigned char *in, int inl);
int EVP_DecryptFinal(EVP_CIPHER_CTX *ctx, unsigned char *out, int *outl);

#ifdef __cplusplus
}
#endif
