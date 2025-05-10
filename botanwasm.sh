#!/bin/bash
# Script to build Botan for WebAssembly using Emscripten

# Ensure Emscripten is activated
if [ -z "${EMSDK}" ]; then
  echo "Error: Emscripten environment not detected. Please run 'source ./emsdk_env.sh' first."
  exit 1
fi

# Set variables
BOTAN_VERSION="3.1.1"
BUILD_DIR="/tmp/botan-wasm-build"
INSTALL_DIR="/volumes/bigcode/hydra_sdk/lib/wasm"
EMSCRIPTEN_ROOT="${EMSDK}/upstream/emscripten"

# Create directories
mkdir -p "${BUILD_DIR}"
mkdir -p "${INSTALL_DIR}/include"
mkdir -p "${INSTALL_DIR}/lib"




# Enter Botan directory
cd "lib/botan"

# Create a custom target for WebAssembly
cat > wasm_config.py << EOF
#!/usr/bin/env python3

def target(target_cc, target_os, target_cpu, target_endian, target_bits):
    if target_os == 'emscripten' and target_cpu == 'wasm':
        return 'wasm32'
    return None

def configure_build(conf, options):
    conf.lib_opts.update({
        'visibility': 'hidden',
        'prefer_static_libs': True,
        'single_amalgamation': True,
    })
    
    # Disable features not needed or not working in WebAssembly
    conf.disable_feature('thread_utils')
    conf.disable_feature('pthread')
    conf.disable_module('tls')
    conf.disable_module('x509')
    conf.disable_module('pkcs11')
    conf.disable_module('tpm')
    
    # Enable only the crypto primitives we need
    conf.enable_module('hash')
    conf.enable_module('block')
    conf.enable_module('stream')
    conf.enable_module('modes')
    conf.enable_module('kdf')
    conf.enable_module('pubkey')
    conf.enable_module('rng')
EOF

# Configure Botan for WebAssembly
echo "Configuring Botan for WebAssembly..."
CC=emcc CXX=em++ python3 ./configure.py \
  --cc=emcc \
  --cpu=wasm \
  --os=emscripten \
  --without-documentation \
  --disable-modules=tls,pkcs11,tpm,x509 \
  --enable-shared-library=no \
  --with-build-dir="${BUILD_DIR}/build" \
  --prefix="${INSTALL_DIR}" \
  --extra-cxxflags="-O3 -fno-rtti -fno-exceptions -DBOTAN_NO_DYNAMIC_LOADER"

# Build Botan
echo "Building Botan..."
emmake make -j4

# Install Botan
echo "Installing Botan..."
emmake make install

echo "Botan for WebAssembly has been built successfully!"
echo "Headers: ${INSTALL_DIR}/include/botan-3"
echo "Library: ${INSTALL_DIR}/lib/libbotan-3.a"

# Create CMake find script for the WebAssembly-compiled Botan
cat > "${INSTALL_DIR}/FindBotan_WASM.cmake" << EOF
# FindBotan_WASM.cmake
# Find the Botan library compiled for WebAssembly
#
# This module defines
#  BOTAN_WASM_FOUND        - True if Botan for WASM was found
#  BOTAN_WASM_INCLUDE_DIRS - The Botan include directories
#  BOTAN_WASM_LIBRARIES    - The Botan libraries

set(BOTAN_WASM_ROOT "${INSTALL_DIR}")

find_path(BOTAN_WASM_INCLUDE_DIR NAMES botan/botan.h
          PATHS ${BOTAN_WASM_ROOT}/include/botan-3
          NO_DEFAULT_PATH)

find_library(BOTAN_WASM_LIBRARY NAMES libbotan-3.a
             PATHS ${BOTAN_WASM_ROOT}/lib
             NO_DEFAULT_PATH)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(BOTAN_WASM DEFAULT_MSG
                                  BOTAN_WASM_LIBRARY BOTAN_WASM_INCLUDE_DIR)

if(BOTAN_WASM_FOUND)
  set(BOTAN_WASM_LIBRARIES ${BOTAN_WASM_LIBRARY})
  set(BOTAN_WASM_INCLUDE_DIRS ${BOTAN_WASM_INCLUDE_DIR})
endif()

mark_as_advanced(BOTAN_WASM_INCLUDE_DIR BOTAN_WASM_LIBRARY)
EOF

echo "Created CMake find script at ${INSTALL_DIR}/FindBotan_WASM.cmake"