// Mock OpenSSL sha.h header for WebAssembly build
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

typedef struct sha_ctx_st SHA_CTX;
typedef struct sha256_ctx_st SHA256_CTX;
typedef struct sha512_ctx_st SHA512_CTX;

#define SHA_DIGEST_LENGTH 20
#define SHA256_DIGEST_LENGTH 32
#define SHA512_DIGEST_LENGTH 64

int SHA1_Init(SHA_CTX *c);
int SHA1_Update(SHA_CTX *c, const void *data, size_t len);
int SHA1_Final(unsigned char *md, SHA_CTX *c);
unsigned char *SHA1(const unsigned char *d, size_t n, unsigned char *md);

int SHA256_Init(SHA256_CTX *c);
int SHA256_Update(SHA256_CTX *c, const void *data, size_t len);
int SHA256_Final(unsigned char *md, SHA256_CTX *c);
unsigned char *SHA256(const unsigned char *d, size_t n, unsigned char *md);

int SHA512_Init(SHA512_CTX *c);
int SHA512_Update(SHA512_CTX *c, const void *data, size_t len);
int SHA512_Final(unsigned char *md, SHA512_CTX *c);
unsigned char *SHA512(const unsigned char *d, size_t n, unsigned char *md);

#ifdef __cplusplus
}
#endif
