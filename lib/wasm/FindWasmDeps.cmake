# FindWasmDeps.cmake
# Find all WebAssembly-compiled dependencies for Hydra SDK
#
# This module defines
#  WASM_DEPS_FOUND        - True if all dependencies were found
#  WASM_DEPS_INCLUDE_DIRS - The include directories

# Find GMP
include(${CMAKE_CURRENT_LIST_DIR}/FindGMP_WASM.cmake)

# Find MPFR
include(${CMAKE_CURRENT_LIST_DIR}/FindMPFR_WASM.cmake)

# Check if all dependencies were found
if(GMP_WASM_FOUND AND MPFR_WASM_FOUND)
  set(WASM_DEPS_FOUND TRUE)

  # Combine include directories
  set(WASM_DEPS_INCLUDE_DIRS
    ${GMP_WASM_INCLUDE_DIRS}
    ${MPFR_WASM_INCLUDE_DIRS}
  )
else()
  set(WASM_DEPS_FOUND FALSE)
endif()

# Report status
if(WASM_DEPS_FOUND)
  message(STATUS "Found all WebAssembly dependencies")
else()
  message(WARNING "Some WebAssembly dependencies were not found")
endif()
