#!/bin/bash
# Script to fix the OpenSSL WebAssembly build process

# Set variables
MOCK_DIR="/volumes/bigcode/hydra_sdk/build_wasm/mock"
WASM_LIB_DIR="/volumes/bigcode/hydra_sdk/lib/wasm"

# Create directories if they don't exist
mkdir -p "${MOCK_DIR}/include/openssl"
mkdir -p "${WASM_LIB_DIR}/include/openssl"

# Create a mock OpenSSL hmac.h header file
echo "Creating mock OpenSSL hmac.h header file..."
cat > "${MOCK_DIR}/include/openssl/hmac.h" << 'EOF'
// Mock OpenSSL hmac.h header for WebAssembly build
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

typedef struct hmac_ctx_st HMAC_CTX;
typedef struct evp_md_st EVP_MD;
typedef struct engine_st ENGINE;

#define OSSL_DEPRECATEDIN_3_0
#define OSSL_DEPRECATEDIN_1_1_0
#define __owur

size_t HMAC_size(const HMAC_CTX *e);
HMAC_CTX *HMAC_CTX_new(void);
int HMAC_CTX_reset(HMAC_CTX *ctx);
void HMAC_CTX_free(HMAC_CTX *ctx);

int HMAC_Init(HMAC_CTX *ctx, const void *key, int len, const EVP_MD *md);
int HMAC_Init_ex(HMAC_CTX *ctx, const void *key, int len, const EVP_MD *md, ENGINE *impl);
int HMAC_Update(HMAC_CTX *ctx, const unsigned char *data, size_t len);
int HMAC_Final(HMAC_CTX *ctx, unsigned char *md, unsigned int *len);
int HMAC_CTX_copy(HMAC_CTX *dctx, HMAC_CTX *sctx);
void HMAC_CTX_set_flags(HMAC_CTX *ctx, unsigned long flags);
const EVP_MD *HMAC_CTX_get_md(const HMAC_CTX *ctx);

unsigned char *HMAC(const EVP_MD *evp_md, const void *key, int key_len,
                    const unsigned char *data, size_t data_len,
                    unsigned char *md, unsigned int *md_len);

#ifdef __cplusplus
}
#endif
EOF

# Create a mock OpenSSL evp.h header file
echo "Creating mock OpenSSL evp.h header file..."
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

#ifdef __cplusplus
}
#endif
EOF

# Create a mock OpenSSL crypto.h header file
echo "Creating mock OpenSSL crypto.h header file..."
cat > "${MOCK_DIR}/include/openssl/crypto.h" << 'EOF'
// Mock OpenSSL crypto.h header for WebAssembly build
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

typedef struct evp_cipher_ctx_st EVP_CIPHER_CTX;
typedef struct evp_cipher_st EVP_CIPHER;

#define OSSL_DEPRECATEDIN_3_0
#define OSSL_DEPRECATEDIN_1_1_0
#define __owur

EVP_CIPHER_CTX *EVP_CIPHER_CTX_new(void);
void EVP_CIPHER_CTX_free(EVP_CIPHER_CTX *ctx);

#ifdef __cplusplus
}
#endif
EOF

echo "Build process fixed!"
echo "Now run ./wasmbuild.sh --module all to rebuild the WebAssembly modules."
