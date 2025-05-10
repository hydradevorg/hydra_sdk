# FindBotan_WASM.cmake
# Find the Botan library compiled for WebAssembly
#
# This module defines
#  BOTAN_WASM_FOUND        - True if Botan for WASM was found
#  BOTAN_WASM_INCLUDE_DIRS - The Botan include directories
#  BOTAN_WASM_LIBRARIES    - The Botan libraries

set(BOTAN_WASM_ROOT "/volumes/bigcode/hydra_sdk/lib/wasm")

find_path(BOTAN_WASM_INCLUDE_DIR NAMES botan/botan.h
          PATHS ${BOTAN_WASM_ROOT}/include
          NO_DEFAULT_PATH)

find_library(BOTAN_WASM_LIBRARY NAMES libbotan-3.a
             PATHS ${BOTAN_WASM_ROOT}/lib
             NO_DEFAULT_PATH)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(BOTAN_WASM DEFAULT_MSG
                                  BOTAN_WASM_LIBRARY BOTAN_WASM_INCLUDE_DIR)

if(BOTAN_WASM_FOUND)
  set(BOTAN_WASM_LIBRARIES ${BOTAN_WASM_LIBRARY})
  set(BOTAN_WASM_INCLUDE_DIRS ${BOTAN_WASM_INCLUDE_DIR})
endif()

mark_as_advanced(BOTAN_WASM_INCLUDE_DIR BOTAN_WASM_LIBRARY)
