#!/bin/bash
# Master script to build all dependencies for WebAssembly

# Ensure Emscripten is activated
if [ -z "${EMSDK}" ]; then
  echo "Error: Emscripten environment not detected. Please run 'source ./emsdk_env.sh' first."
  exit 1
fi

# Set variables
INSTALL_DIR="/volumes/bigcode/hydra_sdk/lib/wasm"

# Create installation directory
mkdir -p "${INSTALL_DIR}"

# Function to display section header
section() {
  echo ""
  echo "========================================"
  echo "  $1"
  echo "========================================"
  echo ""
}

# Build GMP
section "Building GMP for WebAssembly"
./build_gmp_wasm.sh
if [ $? -ne 0 ]; then
  echo "Error: Failed to build GMP for WebAssembly"
  exit 1
fi

# Build OpenSSL
section "Building OpenSSL for WebAssembly"
./build_openssl_wasm.sh
if [ $? -ne 0 ]; then
  echo "Error: Failed to build OpenSSL for WebAssembly"
  exit 1
fi

# Build Botan
section "Building Botan for WebAssembly"
./build_botan_wasm.sh
if [ $? -ne 0 ]; then
  echo "Error: Failed to build Botan for WebAssembly"
  exit 1
fi

# Build Blake3
section "Building Blake3 for WebAssembly"
./build_blake3_wasm.sh
if [ $? -ne 0 ]; then
  echo "Error: Failed to build Blake3 for WebAssembly"
  exit 1
fi

# Build Eigen
section "Building Eigen for WebAssembly"
./build_eigen_wasm.sh
if [ $? -ne 0 ]; then
  echo "Error: Failed to build Eigen for WebAssembly"
  exit 1
fi

# Install JSON
section "Installing JSON for WebAssembly"
./build_json_wasm.sh
if [ $? -ne 0 ]; then
  echo "Error: Failed to install JSON for WebAssembly"
  exit 1
fi

# Create a CMake module to find all WebAssembly dependencies
section "Creating CMake module for WebAssembly dependencies"
cat > "${INSTALL_DIR}/FindWasmDeps.cmake" << EOF
# FindWasmDeps.cmake
# Find all WebAssembly-compiled dependencies for Hydra SDK
#
# This module defines
#  WASM_DEPS_FOUND        - True if all dependencies were found
#  WASM_DEPS_INCLUDE_DIRS - The include directories
#  WASM_DEPS_LIBRARIES    - The libraries

# Find GMP
include(\${CMAKE_CURRENT_LIST_DIR}/FindGMP_WASM.cmake)

# Find OpenSSL
include(\${CMAKE_CURRENT_LIST_DIR}/FindOpenSSL_WASM.cmake)

# Find Botan
include(\${CMAKE_CURRENT_LIST_DIR}/FindBotan_WASM.cmake)

# Find Blake3
include(\${CMAKE_CURRENT_LIST_DIR}/FindBlake3_WASM.cmake)

# Find Eigen3
include(\${CMAKE_CURRENT_LIST_DIR}/FindEigen3_WASM.cmake)

# Find JSON
include(\${CMAKE_CURRENT_LIST_DIR}/FindJSON_WASM.cmake)

# Check if all dependencies were found
if(GMP_WASM_FOUND AND OPENSSL_WASM_FOUND AND BOTAN_WASM_FOUND AND BLAKE3_WASM_FOUND AND EIGEN3_WASM_FOUND AND JSON_WASM_FOUND)
  set(WASM_DEPS_FOUND TRUE)

  # Combine include directories
  set(WASM_DEPS_INCLUDE_DIRS
    \${GMP_WASM_INCLUDE_DIRS}
    \${OPENSSL_WASM_INCLUDE_DIRS}
    \${BOTAN_WASM_INCLUDE_DIRS}
    \${BLAKE3_WASM_INCLUDE_DIRS}
    \${EIGEN3_WASM_INCLUDE_DIRS}
    \${JSON_WASM_INCLUDE_DIRS}
  )

  # Combine libraries
  set(WASM_DEPS_LIBRARIES
    \${GMP_WASM_LIBRARIES}
    \${OPENSSL_WASM_LIBRARIES}
    \${BOTAN_WASM_LIBRARIES}
    \${BLAKE3_WASM_LIBRARIES}
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

echo ""
echo "All WebAssembly dependencies have been built successfully!"
echo "Installation directory: ${INSTALL_DIR}"
echo ""
echo "To use these dependencies in your CMake project, add the following:"
echo "  set(CMAKE_MODULE_PATH \${CMAKE_MODULE_PATH} \"${INSTALL_DIR}\")"
echo "  find_package(WasmDeps REQUIRED)"
echo "  target_include_directories(your_target PRIVATE \${WASM_DEPS_INCLUDE_DIRS})"
echo "  target_link_libraries(your_target PRIVATE \${WASM_DEPS_LIBRARIES})"
echo ""
