#!/bin/bash
# Script to build Eigen for WebAssembly using Emscripten

# Ensure Emscripten is activated
if [ -z "${EMSDK}" ]; then
  echo "Error: Emscripten environment not detected. Please run 'source ./emsdk_env.sh' first."
  exit 1
fi

# Set variables
EIGEN_VERSION="3.4.0"
BUILD_DIR="/tmp/eigen-wasm-build"
INSTALL_DIR="/volumes/bigcode/hydra_sdk/lib/wasm"
EMSCRIPTEN_ROOT="${EMSDK}/upstream/emscripten"

# Create directories
mkdir -p "${BUILD_DIR}"
mkdir -p "${INSTALL_DIR}/include"
mkdir -p "${INSTALL_DIR}/lib"

# Download Eigen if not already downloaded
if [ ! -f "${BUILD_DIR}/eigen-${EIGEN_VERSION}.tar.gz" ]; then
  echo "Downloading Eigen ${EIGEN_VERSION}..."
  curl -L https://gitlab.com/libeigen/eigen/-/archive/${EIGEN_VERSION}/eigen-${EIGEN_VERSION}.tar.gz -o "${BUILD_DIR}/eigen-${EIGEN_VERSION}.tar.gz"
fi

# Extract Eigen
cd "${BUILD_DIR}"
if [ ! -d "eigen-${EIGEN_VERSION}" ]; then
  echo "Extracting Eigen..."
  tar -xzf "eigen-${EIGEN_VERSION}.tar.gz"
fi

# Enter Eigen directory
cd "eigen-${EIGEN_VERSION}"

# Create a build directory
mkdir -p build
cd build

# Configure Eigen for WebAssembly
echo "Configuring Eigen for WebAssembly..."
emcmake cmake .. \
  -DCMAKE_INSTALL_PREFIX="${INSTALL_DIR}" \
  -DCMAKE_BUILD_TYPE=Release \
  -DEIGEN_TEST_CXX11=ON \
  -DBUILD_TESTING=OFF

# Install Eigen (header-only library, no build step needed)
echo "Installing Eigen..."
emmake make install

echo "Eigen for WebAssembly has been installed successfully!"
echo "Headers: ${INSTALL_DIR}/include/eigen3"

# Create CMake find script for the WebAssembly-compiled Eigen
cat > "${INSTALL_DIR}/FindEigen3_WASM.cmake" << EOF
# FindEigen3_WASM.cmake
# Find the Eigen library compiled for WebAssembly
#
# This module defines
#  EIGEN3_WASM_FOUND        - True if Eigen3 for WASM was found
#  EIGEN3_WASM_INCLUDE_DIRS - The Eigen3 include directories

set(EIGEN3_WASM_ROOT "${INSTALL_DIR}")

find_path(EIGEN3_WASM_INCLUDE_DIR NAMES Eigen/Core
          PATHS \${EIGEN3_WASM_ROOT}/include/eigen3
          NO_DEFAULT_PATH)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(EIGEN3_WASM DEFAULT_MSG
                                  EIGEN3_WASM_INCLUDE_DIR)

if(EIGEN3_WASM_FOUND)
  set(EIGEN3_WASM_INCLUDE_DIRS \${EIGEN3_WASM_INCLUDE_DIR})
endif()

mark_as_advanced(EIGEN3_WASM_INCLUDE_DIR)
EOF

echo "Created CMake find script at ${INSTALL_DIR}/FindEigen3_WASM.cmake"
