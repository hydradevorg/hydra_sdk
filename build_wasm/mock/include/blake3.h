// Mock Blake3 header for WebAssembly build
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint8_t dummy[64];
} blake3_hasher;

void blake3_hasher_init(blake3_hasher *hasher);
void blake3_hasher_update(blake3_hasher *hasher, const void *input, size_t input_len);
void blake3_hasher_finalize(const blake3_hasher *hasher, uint8_t *out, size_t out_len);

#ifdef __cplusplus
}
#endif
