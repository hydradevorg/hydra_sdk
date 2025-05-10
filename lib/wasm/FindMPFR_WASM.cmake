# FindMPFR_WASM.cmake
# Find the MPFR library compiled for WebAssembly
#
# This module defines
#  MPFR_WASM_FOUND        - True if MPFR for WASM was found
#  MPFR_WASM_INCLUDE_DIRS - The MPFR include directories
#  MPFR_WASM_LIBRARIES    - The MPFR libraries

set(MPFR_WASM_ROOT "${CMAKE_CURRENT_LIST_DIR}")

find_path(MPFR_WASM_INCLUDE_DIR NAMES mpfr.h
          PATHS ${MPFR_WASM_ROOT}/include
          NO_DEFAULT_PATH)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(MPFR_WASM DEFAULT_MSG
                                  MPFR_WASM_INCLUDE_DIR)

if(MPFR_WASM_FOUND)
  set(MPFR_WASM_INCLUDE_DIRS ${MPFR_WASM_INCLUDE_DIR})
endif()

mark_as_advanced(MPFR_WASM_INCLUDE_DIR)
