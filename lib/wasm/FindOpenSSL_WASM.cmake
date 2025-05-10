# FindOpenSSL_WASM.cmake
# Find the OpenSSL library compiled for WebAssembly
#
# This module defines
#  OPENSSL_WASM_FOUND        - True if OpenSSL for WASM was found
#  OPENSSL_WASM_INCLUDE_DIRS - The OpenSSL include directories
#  OPENSSL_WASM_LIBRARIES    - The OpenSSL libraries

set(OPENSSL_WASM_ROOT "/volumes/bigcode/hydra_sdk/lib/wasm")

find_path(OPENSSL_WASM_INCLUDE_DIR NAMES openssl/crypto.h
          PATHS ${OPENSSL_WASM_ROOT}/include
          NO_DEFAULT_PATH)

find_library(OPENSSL_WASM_CRYPTO_LIBRARY NAMES libcrypto.a
             PATHS ${OPENSSL_WASM_ROOT}/lib
             NO_DEFAULT_PATH)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(OPENSSL_WASM DEFAULT_MSG
                                  OPENSSL_WASM_CRYPTO_LIBRARY OPENSSL_WASM_INCLUDE_DIR)

if(OPENSSL_WASM_FOUND)
  set(OPENSSL_WASM_LIBRARIES ${OPENSSL_WASM_CRYPTO_LIBRARY})
  set(OPENSSL_WASM_INCLUDE_DIRS ${OPENSSL_WASM_INCLUDE_DIR})
endif()

mark_as_advanced(OPENSSL_WASM_INCLUDE_DIR OPENSSL_WASM_CRYPTO_LIBRARY)
