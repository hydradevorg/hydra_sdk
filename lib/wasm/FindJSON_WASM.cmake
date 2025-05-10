# FindJSON_WASM.cmake
# Find the nlohmann/json library for WebAssembly
#
# This module defines
#  JSON_WASM_FOUND        - True if JSON for WASM was found
#  JSON_WASM_INCLUDE_DIRS - The JSON include directories

set(JSON_WASM_ROOT "/volumes/bigcode/hydra_sdk/lib/wasm")

find_path(JSON_WASM_INCLUDE_DIR NAMES nlohmann/json.hpp
          PATHS ${JSON_WASM_ROOT}/include
          NO_DEFAULT_PATH)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(JSON_WASM DEFAULT_MSG
                                  JSON_WASM_INCLUDE_DIR)

if(JSON_WASM_FOUND)
  set(JSON_WASM_INCLUDE_DIRS ${JSON_WASM_INCLUDE_DIR})
endif()

mark_as_advanced(JSON_WASM_INCLUDE_DIR)
