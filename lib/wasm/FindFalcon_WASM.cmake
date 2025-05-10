# FindFalcon_WASM.cmake
# Find the Falcon library for WebAssembly
#
# This module defines
#  FALCON_WASM_FOUND        - True if Falcon for WASM was found
#  FALCON_WASM_INCLUDE_DIRS - The Falcon include directories

set(FALCON_WASM_ROOT "/volumes/bigcode/hydra_sdk/lib/wasm")

find_path(FALCON_WASM_INCLUDE_DIR NAMES falcon/falcon.hpp
          PATHS ${FALCON_WASM_ROOT}/include
          NO_DEFAULT_PATH)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(FALCON_WASM DEFAULT_MSG
                                  FALCON_WASM_INCLUDE_DIR)

if(FALCON_WASM_FOUND)
  set(FALCON_WASM_INCLUDE_DIRS ${FALCON_WASM_INCLUDE_DIR})
endif()

mark_as_advanced(FALCON_WASM_INCLUDE_DIR)
