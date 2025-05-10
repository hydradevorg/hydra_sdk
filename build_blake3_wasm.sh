#!/bin/bash
# Script to build Blake3 for WebAssembly using Emscripten

# Ensure Emscripten is activated
if [ -z "${EMSDK}" ]; then
  echo "Error: Emscripten environment not detected. Please run 'source ./emsdk_env.sh' first."
  exit 1
fi

# Set variables
BLAKE3_VERSION="1.5.0"
BUILD_DIR="/tmp/blake3-wasm-build"
INSTALL_DIR="/volumes/bigcode/hydra_sdk/lib/wasm"
EMSCRIPTEN_ROOT="${EMSDK}/upstream/emscripten"

# Create directories
mkdir -p "${BUILD_DIR}"
mkdir -p "${INSTALL_DIR}/include"
mkdir -p "${INSTALL_DIR}/lib"

# Download Blake3 if not already downloaded
if [ ! -f "${BUILD_DIR}/blake3-${BLAKE3_VERSION}.tar.gz" ]; then
  echo "Downloading Blake3 ${BLAKE3_VERSION}..."
  curl -L https://github.com/BLAKE3-team/BLAKE3/archive/refs/tags/${BLAKE3_VERSION}.tar.gz -o "${BUILD_DIR}/blake3-${BLAKE3_VERSION}.tar.gz"
fi

# Extract Blake3
cd "${BUILD_DIR}"
if [ ! -d "BLAKE3-${BLAKE3_VERSION}" ]; then
  echo "Extracting Blake3..."
  tar -xzf "blake3-${BLAKE3_VERSION}.tar.gz"
fi

# Enter Blake3 directory
cd "BLAKE3-${BLAKE3_VERSION}"

# Create a build directory
mkdir -p build

# Create a simple CMakeLists.txt file for Blake3
cat > CMakeLists.txt << EOF
cmake_minimum_required(VERSION 3.10)
project(blake3 C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Blake3 source files (C implementation)
set(BLAKE3_SOURCES
    c/blake3.c
    c/blake3_dispatch.c
    c/blake3_portable.c
)

# Create static library
add_library(blake3 STATIC \${BLAKE3_SOURCES})

# Include directories
target_include_directories(blake3 PUBLIC
    \${CMAKE_CURRENT_SOURCE_DIR}/c
)

# Install
install(TARGETS blake3 DESTINATION lib)
install(FILES c/blake3.h DESTINATION include)
EOF

# Configure and build Blake3
cd build
echo "Configuring Blake3 for WebAssembly..."
emcmake cmake .. \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX="${INSTALL_DIR}" \
  -DCMAKE_C_FLAGS="-O3 -fPIC"

# Build Blake3
echo "Building Blake3..."
emmake make -j4

# Install Blake3
echo "Installing Blake3..."
emmake make install

echo "Blake3 for WebAssembly has been built successfully!"
echo "Headers: ${INSTALL_DIR}/include/blake3.h"
echo "Library: ${INSTALL_DIR}/lib/libblake3.a"

# Create CMake find script for the WebAssembly-compiled Blake3
cat > "${INSTALL_DIR}/FindBlake3_WASM.cmake" << EOF
# FindBlake3_WASM.cmake
# Find the Blake3 library compiled for WebAssembly
#
# This module defines
#  BLAKE3_WASM_FOUND        - True if Blake3 for WASM was found
#  BLAKE3_WASM_INCLUDE_DIRS - The Blake3 include directories
#  BLAKE3_WASM_LIBRARIES    - The Blake3 libraries

set(BLAKE3_WASM_ROOT "${INSTALL_DIR}")

find_path(BLAKE3_WASM_INCLUDE_DIR NAMES blake3.h
          PATHS \${BLAKE3_WASM_ROOT}/include
          NO_DEFAULT_PATH)

find_library(BLAKE3_WASM_LIBRARY NAMES libblake3.a
             PATHS \${BLAKE3_WASM_ROOT}/lib
             NO_DEFAULT_PATH)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(BLAKE3_WASM DEFAULT_MSG
                                  BLAKE3_WASM_LIBRARY BLAKE3_WASM_INCLUDE_DIR)

if(BLAKE3_WASM_FOUND)
  set(BLAKE3_WASM_LIBRARIES \${BLAKE3_WASM_LIBRARY})
  set(BLAKE3_WASM_INCLUDE_DIRS \${BLAKE3_WASM_INCLUDE_DIR})
endif()

mark_as_advanced(BLAKE3_WASM_INCLUDE_DIR BLAKE3_WASM_LIBRARY)
EOF

echo "Created CMake find script at ${INSTALL_DIR}/FindBlake3_WASM.cmake"
