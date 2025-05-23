#!/bin/bash
# Script to create a mock GMP implementation for WebAssembly

# Set variables
MOCK_DIR="/volumes/bigcode/hydra_sdk/build_wasm/mock/include"
GMP_MOCK_FILE="${MOCK_DIR}/gmp.h"

# Create directory if it doesn't exist
mkdir -p "${MOCK_DIR}"

# Create mock GMP header
cat > "${GMP_MOCK_FILE}" << 'EOF'
#ifndef __GMP_H__
#define __GMP_H__

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int _mp_alloc;
    int _mp_size;
    void* _mp_d;
} __mpz_struct;

typedef __mpz_struct mpz_t[1];
typedef __mpz_struct* mpz_ptr;
typedef const __mpz_struct* mpz_srcptr;

typedef void (*gmp_free_function) (void *ptr, size_t size);

// Basic initialization and assignment
void mpz_init(mpz_ptr);
void mpz_init_set_si(mpz_ptr, long);
void mpz_init_set(mpz_ptr, mpz_srcptr);
void mpz_set(mpz_ptr, mpz_srcptr);
void mpz_set_si(mpz_ptr, long);
void mpz_set_ui(mpz_ptr, unsigned long);
int mpz_set_str(mpz_ptr, const char*, int);
void mpz_clear(mpz_ptr);

// Conversion functions
char* mpz_get_str(char*, int, mpz_srcptr);
long mpz_get_si(mpz_srcptr);
unsigned long mpz_get_ui(mpz_srcptr);

// Arithmetic operations
void mpz_add(mpz_ptr, mpz_srcptr, mpz_srcptr);
void mpz_add_ui(mpz_ptr, mpz_srcptr, unsigned long);
void mpz_sub(mpz_ptr, mpz_srcptr, mpz_srcptr);
void mpz_sub_ui(mpz_ptr, mpz_srcptr, unsigned long);
void mpz_mul(mpz_ptr, mpz_srcptr, mpz_srcptr);
void mpz_mul_si(mpz_ptr, mpz_srcptr, long);
void mpz_mul_ui(mpz_ptr, mpz_srcptr, unsigned long);
void mpz_fdiv_q(mpz_ptr, mpz_srcptr, mpz_srcptr);
void mpz_fdiv_r(mpz_ptr, mpz_srcptr, mpz_srcptr);
void mpz_mod(mpz_ptr, mpz_srcptr, mpz_srcptr);
void mpz_neg(mpz_ptr, mpz_srcptr);
void mpz_abs(mpz_ptr, mpz_srcptr);
void mpz_pow_ui(mpz_ptr, mpz_srcptr, unsigned long);

// Comparison functions
int mpz_cmp(mpz_srcptr, mpz_srcptr);
int mpz_cmp_si(mpz_srcptr, long);
int mpz_cmp_ui(mpz_srcptr, unsigned long);
int mpz_sgn(mpz_srcptr);

// Logical operations
void mpz_and(mpz_ptr, mpz_srcptr, mpz_srcptr);
void mpz_ior(mpz_ptr, mpz_srcptr, mpz_srcptr);
void mpz_xor(mpz_ptr, mpz_srcptr, mpz_srcptr);
void mpz_com(mpz_ptr, mpz_srcptr);

// Memory management
void mp_get_memory_functions(void *(**alloc_func_ptr) (size_t),
                            void *(**realloc_func_ptr) (void *, size_t, size_t),
                            void (**free_func_ptr) (void *, size_t));

#ifdef __cplusplus
}
#endif

#endif /* __GMP_H__ */
EOF

# Create mock GMP implementation
cat > "${MOCK_DIR}/gmp_mock.cpp" << 'EOF'
#include "gmp.h"
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <string>

// Basic initialization and assignment
void mpz_init(mpz_ptr x) {
    x->_mp_alloc = 1;
    x->_mp_size = 0;
    x->_mp_d = malloc(sizeof(long));
    *((long*)x->_mp_d) = 0;
}

void mpz_init_set_si(mpz_ptr x, long val) {
    mpz_init(x);
    mpz_set_si(x, val);
}

void mpz_init_set(mpz_ptr x, mpz_srcptr y) {
    mpz_init(x);
    mpz_set(x, y);
}

void mpz_set(mpz_ptr x, mpz_srcptr y) {
    if (x->_mp_d) free(x->_mp_d);
    x->_mp_alloc = y->_mp_alloc;
    x->_mp_size = y->_mp_size;
    x->_mp_d = malloc(sizeof(long));
    *((long*)x->_mp_d) = *((long*)y->_mp_d);
}

void mpz_set_si(mpz_ptr x, long val) {
    x->_mp_size = (val != 0) ? 1 : 0;
    *((long*)x->_mp_d) = val;
}

void mpz_set_ui(mpz_ptr x, unsigned long val) {
    x->_mp_size = (val != 0) ? 1 : 0;
    *((long*)x->_mp_d) = val;
}

int mpz_set_str(mpz_ptr x, const char* str, int base) {
    try {
        long val = std::stol(str, nullptr, base);
        mpz_set_si(x, val);
        return 0;
    } catch (...) {
        return -1;
    }
}

void mpz_clear(mpz_ptr x) {
    if (x->_mp_d) {
        free(x->_mp_d);
        x->_mp_d = nullptr;
    }
    x->_mp_alloc = 0;
    x->_mp_size = 0;
}

// Conversion functions
char* mpz_get_str(char* str, int base, mpz_srcptr x) {
    long val = *((long*)x->_mp_d);
    char buffer[64];
    sprintf(buffer, "%ld", val);
    
    size_t len = strlen(buffer) + 1;
    char* result = str ? str : (char*)malloc(len);
    strcpy(result, buffer);
    return result;
}

long mpz_get_si(mpz_srcptr x) {
    return *((long*)x->_mp_d);
}

unsigned long mpz_get_ui(mpz_srcptr x) {
    return (unsigned long)(*((long*)x->_mp_d));
}

// Arithmetic operations
void mpz_add(mpz_ptr r, mpz_srcptr x, mpz_srcptr y) {
    long val_x = *((long*)x->_mp_d);
    long val_y = *((long*)y->_mp_d);
    mpz_set_si(r, val_x + val_y);
}

void mpz_add_ui(mpz_ptr r, mpz_srcptr x, unsigned long y) {
    long val_x = *((long*)x->_mp_d);
    mpz_set_si(r, val_x + y);
}

void mpz_sub(mpz_ptr r, mpz_srcptr x, mpz_srcptr y) {
    long val_x = *((long*)x->_mp_d);
    long val_y = *((long*)y->_mp_d);
    mpz_set_si(r, val_x - val_y);
}

void mpz_sub_ui(mpz_ptr r, mpz_srcptr x, unsigned long y) {
    long val_x = *((long*)x->_mp_d);
    mpz_set_si(r, val_x - y);
}

void mpz_mul(mpz_ptr r, mpz_srcptr x, mpz_srcptr y) {
    long val_x = *((long*)x->_mp_d);
    long val_y = *((long*)y->_mp_d);
    mpz_set_si(r, val_x * val_y);
}

void mpz_mul_si(mpz_ptr r, mpz_srcptr x, long y) {
    long val_x = *((long*)x->_mp_d);
    mpz_set_si(r, val_x * y);
}

void mpz_mul_ui(mpz_ptr r, mpz_srcptr x, unsigned long y) {
    long val_x = *((long*)x->_mp_d);
    mpz_set_si(r, val_x * y);
}

void mpz_fdiv_q(mpz_ptr r, mpz_srcptr x, mpz_srcptr y) {
    long val_x = *((long*)x->_mp_d);
    long val_y = *((long*)y->_mp_d);
    if (val_y == 0) {
        mpz_set_si(r, 0);
    } else {
        mpz_set_si(r, val_x / val_y);
    }
}

void mpz_fdiv_r(mpz_ptr r, mpz_srcptr x, mpz_srcptr y) {
    long val_x = *((long*)x->_mp_d);
    long val_y = *((long*)y->_mp_d);
    if (val_y == 0) {
        mpz_set_si(r, 0);
    } else {
        mpz_set_si(r, val_x % val_y);
    }
}

void mpz_mod(mpz_ptr r, mpz_srcptr x, mpz_srcptr y) {
    mpz_fdiv_r(r, x, y);
}

void mpz_neg(mpz_ptr r, mpz_srcptr x) {
    long val_x = *((long*)x->_mp_d);
    mpz_set_si(r, -val_x);
}

void mpz_abs(mpz_ptr r, mpz_srcptr x) {
    long val_x = *((long*)x->_mp_d);
    mpz_set_si(r, val_x >= 0 ? val_x : -val_x);
}

void mpz_pow_ui(mpz_ptr r, mpz_srcptr x, unsigned long y) {
    long val_x = *((long*)x->_mp_d);
    long result = 1;
    for (unsigned long i = 0; i < y; i++) {
        result *= val_x;
    }
    mpz_set_si(r, result);
}

// Comparison functions
int mpz_cmp(mpz_srcptr x, mpz_srcptr y) {
    long val_x = *((long*)x->_mp_d);
    long val_y = *((long*)y->_mp_d);
    return (val_x > val_y) ? 1 : ((val_x < val_y) ? -1 : 0);
}

int mpz_cmp_si(mpz_srcptr x, long y) {
    long val_x = *((long*)x->_mp_d);
    return (val_x > y) ? 1 : ((val_x < y) ? -1 : 0);
}

int mpz_cmp_ui(mpz_srcptr x, unsigned long y) {
    long val_x = *((long*)x->_mp_d);
    return (val_x > (long)y) ? 1 : ((val_x < (long)y) ? -1 : 0);
}

int mpz_sgn(mpz_srcptr x) {
    long val_x = *((long*)x->_mp_d);
    return (val_x > 0) ? 1 : ((val_x < 0) ? -1 : 0);
}

// Logical operations
void mpz_and(mpz_ptr r, mpz_srcptr x, mpz_srcptr y) {
    long val_x = *((long*)x->_mp_d);
    long val_y = *((long*)y->_mp_d);
    mpz_set_si(r, val_x & val_y);
}

void mpz_ior(mpz_ptr r, mpz_srcptr x, mpz_srcptr y) {
    long val_x = *((long*)x->_mp_d);
    long val_y = *((long*)y->_mp_d);
    mpz_set_si(r, val_x | val_y);
}

void mpz_xor(mpz_ptr r, mpz_srcptr x, mpz_srcptr y) {
    long val_x = *((long*)x->_mp_d);
    long val_y = *((long*)y->_mp_d);
    mpz_set_si(r, val_x ^ val_y);
}

void mpz_com(mpz_ptr r, mpz_srcptr x) {
    long val_x = *((long*)x->_mp_d);
    mpz_set_si(r, ~val_x);
}

// Memory management
void mp_get_memory_functions(void *(**alloc_func_ptr) (size_t),
                            void *(**realloc_func_ptr) (void *, size_t, size_t),
                            void (**free_func_ptr) (void *, size_t)) {
    if (free_func_ptr) *free_func_ptr = [](void* ptr, size_t size) { free(ptr); };
}
EOF

echo "Mock GMP implementation created successfully!"
