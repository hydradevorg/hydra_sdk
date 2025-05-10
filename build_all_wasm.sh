#!/bin/bash
# Master script to build all required libraries and modules for WebAssembly

set -e  # Exit on error

# Set variables
HYDRA_ROOT="/volumes/bigcode/hydra_sdk"
WASM_LIB_DIR="${HYDRA_ROOT}/lib/wasm"

echo "=== Building all required libraries and modules for WebAssembly ==="

# Check if Emscripten is available
if ! command -v emcc &> /dev/null; then
    echo "Error: Emscripten compiler (emcc) not found. Please source the emsdk_env.sh script first."
    echo "Example: source /Volumes/BIGCODE/emsdk/emsdk_env.sh"
    exit 1
fi

# Create directories
mkdir -p "${WASM_LIB_DIR}/include"
mkdir -p "${WASM_LIB_DIR}/lib"

# Step 1: Build Botan for WebAssembly
echo "Step 1: Building Botan for WebAssembly..."
if [ -f "${HYDRA_ROOT}/build_botan_wasm_from_sources.sh" ]; then
    bash "${HYDRA_ROOT}/build_botan_wasm_from_sources.sh"
else
    echo "Error: Botan build script not found at ${HYDRA_ROOT}/build_botan_wasm_from_sources.sh"
    exit 1
fi

# Step 2: Build GMP for WebAssembly
echo "Step 2: Building GMP for WebAssembly..."
if [ -f "${HYDRA_ROOT}/build_gmp_wasm.sh" ]; then
    bash "${HYDRA_ROOT}/build_gmp_wasm.sh"
else
    echo "Error: GMP build script not found at ${HYDRA_ROOT}/build_gmp_wasm.sh"
    exit 1
fi

# Step 3: Build all modules
echo "Step 3: Building all modules for WebAssembly..."

# Build crypto module
echo "Building crypto module..."
"${HYDRA_ROOT}/wasmbuild.sh" --module crypto

# Build address module
echo "Building address module..."
"${HYDRA_ROOT}/wasmbuild.sh" --module address

# Build vfs module
echo "Building vfs module..."
"${HYDRA_ROOT}/wasmbuild.sh" --module vfs

# Build lmvs module
echo "Building lmvs module..."
"${HYDRA_ROOT}/wasmbuild.sh" --module lmvs

echo "=== All libraries and modules have been built for WebAssembly ==="
