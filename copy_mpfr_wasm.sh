#!/bin/bash
# Script to copy MPFR files from gmp-wasm to the WebAssembly build directory

# Set variables
WASM_DIR="/volumes/bigcode/hydra_sdk/lib/wasm"
GMP_WASM_DIR="/volumes/bigcode/hydra_sdk/gmp-wasm"
MPFR_SOURCE_DIR="${GMP_WASM_DIR}/binding/mpfr/dist"

# Create directories
mkdir -p "${WASM_DIR}/include"
mkdir -p "${WASM_DIR}/lib"

# Copy MPFR header files
echo "Copying MPFR header files..."
cp "${MPFR_SOURCE_DIR}/include/mpfr.h" "${WASM_DIR}/include/"
cp "${MPFR_SOURCE_DIR}/include/mpf2mpfr.h" "${WASM_DIR}/include/"

# Copy MPFR library files
echo "Copying MPFR library files..."
cp "${MPFR_SOURCE_DIR}/lib/libmpfr.a" "${WASM_DIR}/lib/"

# Create a CMake find script for MPFR
echo "Creating CMake find script for MPFR..."
cat > "${WASM_DIR}/FindMPFR_WASM.cmake" << 'EOF'
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

find_library(MPFR_WASM_LIBRARY NAMES mpfr
             PATHS ${MPFR_WASM_ROOT}/lib
             NO_DEFAULT_PATH)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(MPFR_WASM DEFAULT_MSG
                                  MPFR_WASM_INCLUDE_DIR
                                  MPFR_WASM_LIBRARY)

if(MPFR_WASM_FOUND)
  set(MPFR_WASM_INCLUDE_DIRS ${MPFR_WASM_INCLUDE_DIR})
  set(MPFR_WASM_LIBRARIES ${MPFR_WASM_LIBRARY})
endif()

mark_as_advanced(MPFR_WASM_INCLUDE_DIR MPFR_WASM_LIBRARY)
EOF

# Update the FindWasmDeps.cmake file to include MPFR
echo "Updating FindWasmDeps.cmake..."
if [ -f "${WASM_DIR}/FindWasmDeps.cmake" ]; then
    # Check if MPFR_WASM is already included
    if grep -q "MPFR_WASM" "${WASM_DIR}/FindWasmDeps.cmake"; then
        echo "MPFR_WASM already included in FindWasmDeps.cmake"
    else
        # Create a backup of the original file
        cp "${WASM_DIR}/FindWasmDeps.cmake" "${WASM_DIR}/FindWasmDeps.cmake.bak"
        
        # Add MPFR_WASM to the includes
        sed -i '' 's/# Find GMP/# Find GMP\n\n# Find MPFR\ninclude(${CMAKE_CURRENT_LIST_DIR}\/FindMPFR_WASM.cmake)/g' "${WASM_DIR}/FindWasmDeps.cmake"
        
        # Update the check for all dependencies
        sed -i '' 's/if(GMP_WASM_FOUND AND/if(GMP_WASM_FOUND AND MPFR_WASM_FOUND AND/g' "${WASM_DIR}/FindWasmDeps.cmake"
        
        # Add MPFR_WASM to the include directories
        sed -i '' 's/${GMP_WASM_INCLUDE_DIRS}/${GMP_WASM_INCLUDE_DIRS}\n    ${MPFR_WASM_INCLUDE_DIRS}/g' "${WASM_DIR}/FindWasmDeps.cmake"
        
        # Add MPFR_WASM to the libraries
        sed -i '' 's/${GMP_WASM_LIBRARIES}/${GMP_WASM_LIBRARIES}\n    ${MPFR_WASM_LIBRARIES}/g' "${WASM_DIR}/FindWasmDeps.cmake"
    fi
else
    echo "FindWasmDeps.cmake not found, creating it..."
    cat > "${WASM_DIR}/FindWasmDeps.cmake" << 'EOF'
# FindWasmDeps.cmake
# Find all WebAssembly-compiled dependencies for Hydra SDK
#
# This module defines
#  WASM_DEPS_FOUND        - True if all dependencies were found
#  WASM_DEPS_INCLUDE_DIRS - The include directories
#  WASM_DEPS_LIBRARIES    - The libraries

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

  # Combine libraries
  set(WASM_DEPS_LIBRARIES
    ${GMP_WASM_LIBRARIES}
    ${MPFR_WASM_LIBRARIES}
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
EOF
fi

echo "MPFR files have been copied to the WebAssembly build directory!"
echo "The header files are in ${WASM_DIR}/include"
echo "The library files are in ${WASM_DIR}/lib"
echo "The CMake find script is in ${WASM_DIR}/FindMPFR_WASM.cmake"
