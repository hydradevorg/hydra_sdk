#!/bin/bash
# Script to properly build Botan for WebAssembly using existing sources

set -e  # Exit on error

# Set variables
INSTALL_DIR="/volumes/bigcode/hydra_sdk/lib/wasm"
BOTAN_SRC_DIR="/volumes/bigcode/hydra_sdk/lib/botan"

echo "=== Building Botan for WebAssembly from existing sources ==="

# Create directories
mkdir -p "${INSTALL_DIR}/include"
mkdir -p "${INSTALL_DIR}/lib"

# Check if Emscripten is available
if ! command -v emcc &> /dev/null; then
    echo "Error: Emscripten compiler (emcc) not found. Please source the emsdk_env.sh script first."
    echo "Example: source /Volumes/BIGCODE/emsdk/emsdk_env.sh"
    exit 1
fi

# Check if source directory exists
if [ ! -d "${BOTAN_SRC_DIR}" ]; then
    echo "Error: Botan source directory not found at ${BOTAN_SRC_DIR}"
    exit 1
fi

# Configure and build Botan for WebAssembly
echo "Configuring Botan for WebAssembly..."
cd "${BOTAN_SRC_DIR}"

# Run the configure script with WebAssembly/Emscripten settings
python3 ./configure.py --cpu=wasm --os=emscripten \
    --cc=emcc --cc-bin=emcc \
    --disable-shared \
    --without-documentation \
    --minimized-build \
    --enable-modules=aes,sha2_32,sha2_64,hmac,pbkdf2,auto_rng,system_rng,hash,block,stream,modes,kdf,pubkey,rsa,ecdsa,ecdh \
    --prefix="${INSTALL_DIR}"

# Build and install
echo "Building Botan for WebAssembly..."
make -j$(sysctl -n hw.ncpu)

echo "Installing Botan for WebAssembly..."
# Copy the built library manually to avoid permission issues
cp -f ./libbotan-3.a "${INSTALL_DIR}/lib/"
mkdir -p "${INSTALL_DIR}/include/botan-3"
cp -rf ./build/include/* "${INSTALL_DIR}/include/"

echo "Botan for WebAssembly has been built and installed successfully!"
echo "Headers: ${INSTALL_DIR}/include/botan-3"
echo "Library: ${INSTALL_DIR}/lib/libbotan-3.a"
