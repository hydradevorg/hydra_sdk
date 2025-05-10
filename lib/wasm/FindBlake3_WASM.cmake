# FindBlake3_WASM.cmake
# Find the Blake3 library compiled for WebAssembly
#
# This module defines
#  BLAKE3_WASM_FOUND        - True if Blake3 for WASM was found
#  BLAKE3_WASM_INCLUDE_DIRS - The Blake3 include directories
#  BLAKE3_WASM_LIBRARIES    - The Blake3 libraries

set(BLAKE3_WASM_ROOT "/volumes/bigcode/hydra_sdk/lib/wasm")

find_path(BLAKE3_WASM_INCLUDE_DIR NAMES blake3.h
          PATHS ${BLAKE3_WASM_ROOT}/include
          NO_DEFAULT_PATH)

find_library(BLAKE3_WASM_LIBRARY NAMES libblake3.a
             PATHS ${BLAKE3_WASM_ROOT}/lib
             NO_DEFAULT_PATH)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(BLAKE3_WASM DEFAULT_MSG
                                  BLAKE3_WASM_LIBRARY BLAKE3_WASM_INCLUDE_DIR)

if(BLAKE3_WASM_FOUND)
  set(BLAKE3_WASM_LIBRARIES ${BLAKE3_WASM_LIBRARY})
  set(BLAKE3_WASM_INCLUDE_DIRS ${BLAKE3_WASM_INCLUDE_DIR})
endif()

mark_as_advanced(BLAKE3_WASM_INCLUDE_DIR BLAKE3_WASM_LIBRARY)
