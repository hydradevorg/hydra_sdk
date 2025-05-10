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
