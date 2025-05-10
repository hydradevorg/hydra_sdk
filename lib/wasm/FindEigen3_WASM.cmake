# FindEigen3_WASM.cmake
# Find the Eigen library compiled for WebAssembly
#
# This module defines
#  EIGEN3_WASM_FOUND        - True if Eigen3 for WASM was found
#  EIGEN3_WASM_INCLUDE_DIRS - The Eigen3 include directories

set(EIGEN3_WASM_ROOT "/volumes/bigcode/hydra_sdk/lib/wasm")

find_path(EIGEN3_WASM_INCLUDE_DIR NAMES Eigen/Core
          PATHS ${EIGEN3_WASM_ROOT}/include/eigen3
          NO_DEFAULT_PATH)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(EIGEN3_WASM DEFAULT_MSG
                                  EIGEN3_WASM_INCLUDE_DIR)

if(EIGEN3_WASM_FOUND)
  set(EIGEN3_WASM_INCLUDE_DIRS ${EIGEN3_WASM_INCLUDE_DIR})
endif()

mark_as_advanced(EIGEN3_WASM_INCLUDE_DIR)
