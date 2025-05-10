#!/bin/bash
# Script to use Homebrew-installed OpenSSL for WebAssembly builds

# Set variables
INSTALL_DIR="/volumes/bigcode/hydra_sdk/lib/wasm"
OPENSSL_BREW_DIR="$(brew --prefix openssl@3)"

echo "Using OpenSSL from Homebrew at: ${OPENSSL_BREW_DIR}"

# Create directories
mkdir -p "${INSTALL_DIR}/include"
mkdir -p "${INSTALL_DIR}/lib"

# Copy OpenSSL headers from Homebrew
echo "Copying OpenSSL headers from Homebrew..."
mkdir -p "${INSTALL_DIR}/include/openssl"
cp -r "${OPENSSL_BREW_DIR}/include/openssl/"* "${INSTALL_DIR}/include/openssl/"

# Create a symbolic link to the Homebrew OpenSSL library
echo "Creating symbolic link to Homebrew OpenSSL library..."
ln -sf "${OPENSSL_BREW_DIR}/lib/libcrypto.a" "${INSTALL_DIR}/lib/libcrypto.a"
ln -sf "${OPENSSL_BREW_DIR}/lib/libssl.a" "${INSTALL_DIR}/lib/libssl.a"

echo "OpenSSL for WebAssembly has been set up successfully!"
echo "Headers: ${INSTALL_DIR}/include/openssl"
echo "Library: ${INSTALL_DIR}/lib/libcrypto.a"

# Create CMake find script for the WebAssembly-compiled OpenSSL
cat > "${INSTALL_DIR}/FindOpenSSL_WASM.cmake" << EOF
# FindOpenSSL_WASM.cmake
# Find the OpenSSL library compiled for WebAssembly
#
# This module defines
#  OPENSSL_WASM_FOUND        - True if OpenSSL for WASM was found
#  OPENSSL_WASM_INCLUDE_DIRS - The OpenSSL include directories
#  OPENSSL_WASM_LIBRARIES    - The OpenSSL libraries

set(OPENSSL_WASM_ROOT "${INSTALL_DIR}")

find_path(OPENSSL_WASM_INCLUDE_DIR NAMES openssl/crypto.h
          PATHS \${OPENSSL_WASM_ROOT}/include
          NO_DEFAULT_PATH)

find_library(OPENSSL_WASM_CRYPTO_LIBRARY NAMES libcrypto.a
             PATHS \${OPENSSL_WASM_ROOT}/lib
             NO_DEFAULT_PATH)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(OPENSSL_WASM DEFAULT_MSG
                                  OPENSSL_WASM_CRYPTO_LIBRARY OPENSSL_WASM_INCLUDE_DIR)

if(OPENSSL_WASM_FOUND)
  set(OPENSSL_WASM_LIBRARIES \${OPENSSL_WASM_CRYPTO_LIBRARY})
  set(OPENSSL_WASM_INCLUDE_DIRS \${OPENSSL_WASM_INCLUDE_DIR})
endif()

mark_as_advanced(OPENSSL_WASM_INCLUDE_DIR OPENSSL_WASM_CRYPTO_LIBRARY)
EOF

echo "Created CMake find script at ${INSTALL_DIR}/FindOpenSSL_WASM.cmake"
