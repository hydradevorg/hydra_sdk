#!/bin/bash
# Script to fix the OpenSSL WebAssembly build process (final version)

# Set variables
MOCK_DIR="/volumes/bigcode/hydra_sdk/build_wasm/mock"
WASM_LIB_DIR="/volumes/bigcode/hydra_sdk/lib/wasm"

# Create directories if they don't exist
mkdir -p "${MOCK_DIR}/include/openssl"
mkdir -p "${WASM_LIB_DIR}/include/openssl"

# Create a mock OpenSSL sha.h header file
echo "Creating mock OpenSSL sha.h header file..."
cat > "${MOCK_DIR}/include/openssl/sha.h" << 'EOF'
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
EOF

# Create a mock OpenSSL evp.h header file with EVP_sha256 function
echo "Creating mock OpenSSL evp.h header file with EVP_sha256 function..."
cat > "${MOCK_DIR}/include/openssl/evp.h" << 'EOF'
// Mock OpenSSL evp.h header for WebAssembly build
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

typedef struct evp_cipher_ctx_st EVP_CIPHER_CTX;
typedef struct evp_cipher_st EVP_CIPHER;
typedef struct evp_md_ctx_st EVP_MD_CTX;
typedef struct evp_md_st EVP_MD;
typedef struct evp_pkey_st EVP_PKEY;
typedef struct evp_pkey_ctx_st EVP_PKEY_CTX;
typedef struct engine_st ENGINE;

#define OSSL_DEPRECATEDIN_3_0
#define OSSL_DEPRECATEDIN_1_1_0
#define __owur

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

EVP_CIPHER_CTX *EVP_CIPHER_CTX_new(void);
void EVP_CIPHER_CTX_free(EVP_CIPHER_CTX *ctx);

const EVP_CIPHER *EVP_aes_256_cbc(void);
const EVP_CIPHER *EVP_aes_256_gcm(void);

// Message digest functions
const EVP_MD *EVP_sha1(void);
const EVP_MD *EVP_sha224(void);
const EVP_MD *EVP_sha256(void);
const EVP_MD *EVP_sha384(void);
const EVP_MD *EVP_sha512(void);

#ifdef __cplusplus
}
#endif
EOF

# Create a mock OpenSSL macros.h header file
echo "Creating mock OpenSSL macros.h header file..."
cat > "${MOCK_DIR}/include/openssl/macros.h" << 'EOF'
// Mock OpenSSL macros.h header for WebAssembly build
#pragma once

#define OSSL_DEPRECATED(since)
#define OSSL_DEPRECATEDIN_3_0
#define OSSL_DEPRECATEDIN_1_1_0
EOF

echo "Build process fixed!"
echo "Now run ./wasmbuild.sh --module all to rebuild the WebAssembly modules."
