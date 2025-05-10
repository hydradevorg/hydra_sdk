# FindGMP_WASM.cmake
# Find the GMP library compiled for WebAssembly
#
# This module defines
#  GMP_WASM_FOUND        - True if GMP for WASM was found
#  GMP_WASM_INCLUDE_DIRS - The GMP include directories
#  GMP_WASM_LIBRARIES    - The GMP libraries

set(GMP_WASM_ROOT "${CMAKE_CURRENT_LIST_DIR}")

find_path(GMP_WASM_INCLUDE_DIR NAMES gmp.h
          PATHS ${GMP_WASM_ROOT}/include
          NO_DEFAULT_PATH)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GMP_WASM DEFAULT_MSG
                                  GMP_WASM_INCLUDE_DIR)

if(GMP_WASM_FOUND)
  set(GMP_WASM_INCLUDE_DIRS ${GMP_WASM_INCLUDE_DIR})
endif()

mark_as_advanced(GMP_WASM_INCLUDE_DIR)
