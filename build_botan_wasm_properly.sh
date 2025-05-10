#!/bin/bash
# Script to properly build Botan for WebAssembly using Emscripten

# Set variables
INSTALL_DIR="/volumes/bigcode/hydra_sdk/lib/wasm"
BOTAN_VERSION="3.0.0"
BOTAN_SRC_DIR="/tmp/botan-${BOTAN_VERSION}"
BOTAN_TARBALL="/tmp/botan-${BOTAN_VERSION}.tar.xz"

# Create directories
mkdir -p "${INSTALL_DIR}/include"
mkdir -p "${INSTALL_DIR}/lib"

# Check if Emscripten is available
if ! command -v emcc &> /dev/null; then
    echo "Error: Emscripten compiler (emcc) not found. Please source the emsdk_env.sh script first."
    echo "Example: source /Volumes/BIGCODE/emsdk/emsdk_env.sh"
    exit 1
fi

# Download Botan if not already downloaded
if [ ! -f "${BOTAN_TARBALL}" ]; then
    echo "Downloading Botan ${BOTAN_VERSION}..."
    curl -L "https://botan.randombit.net/releases/Botan-${BOTAN_VERSION}.tar.xz" -o "${BOTAN_TARBALL}"
fi

# Extract Botan if not already extracted
if [ ! -d "${BOTAN_SRC_DIR}" ]; then
    echo "Extracting Botan ${BOTAN_VERSION}..."
    tar -xf "${BOTAN_TARBALL}" -C /tmp
    mv "/tmp/Botan-${BOTAN_VERSION}" "${BOTAN_SRC_DIR}"
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
    --enable-modules=aes,sha2_32,sha2_64,hmac,pbkdf2,auto_rng,system_rng,hash,block,stream,modes,kdf,pubkey,rsa,ecdsa,ecdh,curve25519,ed25519,kyber \
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
