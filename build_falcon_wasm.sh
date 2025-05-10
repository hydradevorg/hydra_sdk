#!/bin/bash
# Script to prepare Falcon library for WebAssembly

# Ensure Emscripten is activated
if [ -z "${EMSDK}" ]; then
  echo "Error: Emscripten environment not detected. Please run 'source ./emsdk_env.sh' first."
  exit 1
fi

# Set variables
INSTALL_DIR="/volumes/bigcode/hydra_sdk/lib/wasm"
FALCON_SOURCE_DIR="/volumes/bigcode/hydra_sdk/include/falcon"
FALCON_DEST_DIR="${INSTALL_DIR}/include/falcon"

# Create directories
mkdir -p "${FALCON_DEST_DIR}"

# Copy Falcon headers
echo "Copying Falcon headers to ${FALCON_DEST_DIR}..."
cp -r "${FALCON_SOURCE_DIR}"/* "${FALCON_DEST_DIR}/"

# Since Falcon is a header-only library, we don't need to compile anything
# But we'll create a CMake find script for it

# Create CMake find script for the WebAssembly-compatible Falcon
cat > "${INSTALL_DIR}/FindFalcon_WASM.cmake" << EOF
# FindFalcon_WASM.cmake
# Find the Falcon library for WebAssembly
#
# This module defines
#  FALCON_WASM_FOUND        - True if Falcon for WASM was found
#  FALCON_WASM_INCLUDE_DIRS - The Falcon include directories

set(FALCON_WASM_ROOT "${INSTALL_DIR}")

find_path(FALCON_WASM_INCLUDE_DIR NAMES falcon/falcon.hpp
          PATHS \${FALCON_WASM_ROOT}/include
          NO_DEFAULT_PATH)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(FALCON_WASM DEFAULT_MSG
                                  FALCON_WASM_INCLUDE_DIR)

if(FALCON_WASM_FOUND)
  set(FALCON_WASM_INCLUDE_DIRS \${FALCON_WASM_INCLUDE_DIR})
endif()

mark_as_advanced(FALCON_WASM_INCLUDE_DIR)
EOF

echo "Created CMake find script at ${INSTALL_DIR}/FindFalcon_WASM.cmake"

# Update the FindWasmDeps.cmake file to include Falcon
if grep -q "FALCON_WASM" "${INSTALL_DIR}/FindWasmDeps.cmake"; then
  echo "Falcon already included in FindWasmDeps.cmake"
else
  echo "Updating FindWasmDeps.cmake to include Falcon..."
  # Create a backup of the original file
  cp "${INSTALL_DIR}/FindWasmDeps.cmake" "${INSTALL_DIR}/FindWasmDeps.cmake.bak"

  # Add Falcon to the includes
  sed -i '' 's/# Find JSON/# Find JSON\n\n# Find Falcon\ninclude(${CMAKE_CURRENT_LIST_DIR}\/FindFalcon_WASM.cmake)/g' "${INSTALL_DIR}/FindWasmDeps.cmake"

  # Update the check for all dependencies
  sed -i '' 's/if(GMP_WASM_FOUND AND OPENSSL_WASM_FOUND AND BOTAN_WASM_FOUND AND BLAKE3_WASM_FOUND AND EIGEN3_WASM_FOUND AND JSON_WASM_FOUND)/if(GMP_WASM_FOUND AND OPENSSL_WASM_FOUND AND BOTAN_WASM_FOUND AND BLAKE3_WASM_FOUND AND EIGEN3_WASM_FOUND AND JSON_WASM_FOUND AND FALCON_WASM_FOUND)/g' "${INSTALL_DIR}/FindWasmDeps.cmake"

  # Add Falcon to the include directories
  sed -i '' 's/${JSON_WASM_INCLUDE_DIRS}/${JSON_WASM_INCLUDE_DIRS}\n    ${FALCON_WASM_INCLUDE_DIRS}/g' "${INSTALL_DIR}/FindWasmDeps.cmake"
fi

echo "Falcon library has been prepared for WebAssembly successfully!"
echo "Headers: ${FALCON_DEST_DIR}"
echo "CMake Find Script: ${INSTALL_DIR}/FindFalcon_WASM.cmake"

# Make the script executable
chmod +x build_falcon_wasm.sh
