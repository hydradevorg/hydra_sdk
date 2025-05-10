#!/bin/bash
# Script to build GMP for WebAssembly using Emscripten

# Ensure Emscripten is activated
if [ -z "${EMSDK}" ]; then
  echo "Error: Emscripten environment not detected. Please run 'source ./emsdk_env.sh' first."
  exit 1
fi

# Set variables
GMP_VERSION="6.2.1"
BUILD_DIR="/tmp/gmp-wasm-build"
INSTALL_DIR="/volumes/bigcode/hydra_sdk/lib/wasm"
EMSCRIPTEN_ROOT="${EMSDK}/upstream/emscripten"

# Create directories
mkdir -p "${BUILD_DIR}"
mkdir -p "${INSTALL_DIR}/include"
mkdir -p "${INSTALL_DIR}/lib"

# Download GMP if not already downloaded
if [ ! -f "${BUILD_DIR}/gmp-${GMP_VERSION}.tar.bz2" ]; then
  echo "Downloading GMP ${GMP_VERSION}..."
  curl -L https://ftp.gnu.org/gnu/gmp/gmp-${GMP_VERSION}.tar.bz2 -o "${BUILD_DIR}/gmp-${GMP_VERSION}.tar.bz2"
fi

# Extract GMP
cd "${BUILD_DIR}"
if [ ! -d "gmp-${GMP_VERSION}" ]; then
  echo "Extracting GMP..."
  tar -xjf "gmp-${GMP_VERSION}.tar.bz2"
fi

# Enter GMP directory
cd "gmp-${GMP_VERSION}"

# Fix shell scripts for macOS compatibility
find . -name "configure" -o -name "config.guess" -o -name "config.sub" | xargs chmod +x

# Create a custom build directory
mkdir -p "${BUILD_DIR}/build"

# Configure GMP for WebAssembly with minimal features
echo "Configuring GMP for WebAssembly..."
emconfigure ./configure \
  --build=$(./config.guess) \
  --host=wasm32-unknown-emscripten \
  --enable-static \
  --disable-shared \
  --disable-assembly \
  --prefix="${INSTALL_DIR}" \
  --with-pic \
  CFLAGS="-O3 -fPIC" \
  CC=emcc \
  CXX=em++ \
  LD=emcc \
  AR=emar \
  RANLIB=emranlib

# Build GMP
echo "Building GMP..."
emmake make -j4

# Install GMP
echo "Installing GMP..."
emmake make install

# Create a minimal GMP header if it doesn't exist
if [ ! -f "${INSTALL_DIR}/include/gmp.h" ]; then
  echo "Creating minimal GMP header..."
  cat > "${INSTALL_DIR}/include/gmp.h" << EOF_GMP
/*
* Minimal GMP header for WebAssembly
*/
#ifndef __GMP_H__
#define __GMP_H__

#include <stddef.h>
#include <stdint.h>
#include <limits.h>

#if defined (__cplusplus)
extern "C" {
#endif

typedef unsigned long mp_limb_t;
typedef long mp_size_t;
typedef unsigned long mp_bitcnt_t;

typedef struct {
  int _mp_alloc;
  int _mp_size;
  mp_limb_t *_mp_d;
} __mpz_struct;

typedef __mpz_struct mpz_t[1];
typedef __mpz_struct *mpz_ptr;
typedef const __mpz_struct *mpz_srcptr;

void mpz_init(mpz_ptr);
void mpz_init_set_ui(mpz_ptr, unsigned long);
void mpz_init_set_si(mpz_ptr, signed long);
void mpz_init_set_str(mpz_ptr, const char *, int);
void mpz_clear(mpz_ptr);
void mpz_set(mpz_ptr, mpz_srcptr);
void mpz_set_ui(mpz_ptr, unsigned long);
void mpz_set_si(mpz_ptr, signed long);
int mpz_set_str(mpz_ptr, const char *, int);
void mpz_swap(mpz_ptr, mpz_ptr);

int mpz_cmp(mpz_srcptr, mpz_srcptr);
int mpz_cmp_ui(mpz_srcptr, unsigned long);
int mpz_cmp_si(mpz_srcptr, signed long);

void mpz_add(mpz_ptr, mpz_srcptr, mpz_srcptr);
void mpz_add_ui(mpz_ptr, mpz_srcptr, unsigned long);
void mpz_sub(mpz_ptr, mpz_srcptr, mpz_srcptr);
void mpz_sub_ui(mpz_ptr, mpz_srcptr, unsigned long);
void mpz_mul(mpz_ptr, mpz_srcptr, mpz_srcptr);
void mpz_mul_ui(mpz_ptr, mpz_srcptr, unsigned long);
void mpz_mul_si(mpz_ptr, mpz_srcptr, long);
void mpz_mul_2exp(mpz_ptr, mpz_srcptr, mp_bitcnt_t);
void mpz_neg(mpz_ptr, mpz_srcptr);
void mpz_abs(mpz_ptr, mpz_srcptr);

void mpz_tdiv_q(mpz_ptr, mpz_srcptr, mpz_srcptr);
void mpz_tdiv_r(mpz_ptr, mpz_srcptr, mpz_srcptr);
void mpz_tdiv_qr(mpz_ptr, mpz_ptr, mpz_srcptr, mpz_srcptr);
void mpz_tdiv_q_ui(mpz_ptr, mpz_srcptr, unsigned long);
void mpz_tdiv_r_ui(mpz_ptr, mpz_srcptr, unsigned long);
unsigned long mpz_tdiv_qr_ui(mpz_ptr, mpz_ptr, mpz_srcptr, unsigned long);
void mpz_tdiv_q_2exp(mpz_ptr, mpz_srcptr, mp_bitcnt_t);
void mpz_tdiv_r_2exp(mpz_ptr, mpz_srcptr, mp_bitcnt_t);

void mpz_mod(mpz_ptr, mpz_srcptr, mpz_srcptr);
void mpz_mod_ui(mpz_ptr, mpz_srcptr, unsigned long);
void mpz_divexact(mpz_ptr, mpz_srcptr, mpz_srcptr);
void mpz_divexact_ui(mpz_ptr, mpz_srcptr, unsigned long);

void mpz_powm(mpz_ptr, mpz_srcptr, mpz_srcptr, mpz_srcptr);
void mpz_powm_ui(mpz_ptr, mpz_srcptr, unsigned long, mpz_srcptr);
void mpz_pow_ui(mpz_ptr, mpz_srcptr, unsigned long);
void mpz_ui_pow_ui(mpz_ptr, unsigned long, unsigned long);

void mpz_sqrt(mpz_ptr, mpz_srcptr);
void mpz_sqrtrem(mpz_ptr, mpz_ptr, mpz_srcptr);
int mpz_perfect_square_p(mpz_srcptr);

void mpz_and(mpz_ptr, mpz_srcptr, mpz_srcptr);
void mpz_ior(mpz_ptr, mpz_srcptr, mpz_srcptr);
void mpz_xor(mpz_ptr, mpz_srcptr, mpz_srcptr);
void mpz_com(mpz_ptr, mpz_srcptr);

mp_bitcnt_t mpz_popcount(mpz_srcptr);
mp_bitcnt_t mpz_hamdist(mpz_srcptr, mpz_srcptr);
mp_bitcnt_t mpz_scan0(mpz_srcptr, mp_bitcnt_t);
mp_bitcnt_t mpz_scan1(mpz_srcptr, mp_bitcnt_t);
void mpz_setbit(mpz_ptr, mp_bitcnt_t);
void mpz_clrbit(mpz_ptr, mp_bitcnt_t);
void mpz_combit(mpz_ptr, mp_bitcnt_t);
int mpz_tstbit(mpz_srcptr, mp_bitcnt_t);

size_t mpz_size(mpz_srcptr);
mp_limb_t mpz_getlimbn(mpz_srcptr, mp_size_t);
int mpz_fits_ulong_p(mpz_srcptr);
int mpz_fits_slong_p(mpz_srcptr);
int mpz_fits_uint_p(mpz_srcptr);
int mpz_fits_sint_p(mpz_srcptr);
int mpz_fits_ushort_p(mpz_srcptr);
int mpz_fits_sshort_p(mpz_srcptr);
unsigned long mpz_get_ui(mpz_srcptr);
long mpz_get_si(mpz_srcptr);
char *mpz_get_str(char *, int, mpz_srcptr);

void mpz_import(mpz_ptr, size_t, int, size_t, int, size_t, const void *);
void *mpz_export(void *, size_t *, int, size_t, int, size_t, mpz_srcptr);

#if defined (__cplusplus)
}
#endif

#endif /* __GMP_H__ */
EOF_GMP
fi

echo "GMP for WebAssembly has been built successfully!"
echo "Headers: ${INSTALL_DIR}/include"
echo "Library: ${INSTALL_DIR}/lib/libgmp.a"

# Create CMake find script for the WebAssembly-compiled GMP
cat > "${INSTALL_DIR}/FindGMP_WASM.cmake" << EOF
# FindGMP_WASM.cmake
# Find the GMP library compiled for WebAssembly
#
# This module defines
#  GMP_WASM_FOUND        - True if GMP for WASM was found
#  GMP_WASM_INCLUDE_DIRS - The GMP include directories
#  GMP_WASM_LIBRARIES    - The GMP libraries

set(GMP_WASM_ROOT "${INSTALL_DIR}")

find_path(GMP_WASM_INCLUDE_DIR NAMES gmp.h
          PATHS \${GMP_WASM_ROOT}/include
          NO_DEFAULT_PATH)

find_library(GMP_WASM_LIBRARY NAMES libgmp.a
             PATHS \${GMP_WASM_ROOT}/lib
             NO_DEFAULT_PATH)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GMP_WASM DEFAULT_MSG
                                  GMP_WASM_LIBRARY GMP_WASM_INCLUDE_DIR)

if(GMP_WASM_FOUND)
  set(GMP_WASM_LIBRARIES \${GMP_WASM_LIBRARY})
  set(GMP_WASM_INCLUDE_DIRS \${GMP_WASM_INCLUDE_DIR})
endif()

mark_as_advanced(GMP_WASM_INCLUDE_DIR GMP_WASM_LIBRARY)
EOF

echo "Created CMake find script at ${INSTALL_DIR}/FindGMP_WASM.cmake"
