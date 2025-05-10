#!/bin/bash
# Script to copy all Botan headers from Homebrew for WebAssembly

# Set variables
INSTALL_DIR="/volumes/bigcode/hydra_sdk/lib/wasm"
BOTAN_BREW_DIR="/usr/local/opt/botan"

# Create directories
mkdir -p "${INSTALL_DIR}/include/botan-3"
mkdir -p "${INSTALL_DIR}/lib"

# Copy all Botan headers from Homebrew
echo "Copying Botan headers from Homebrew..."
cp -r "${BOTAN_BREW_DIR}/include/botan-3/botan" "${INSTALL_DIR}/include/botan-3/"

# Create a symbolic link to the Homebrew Botan library
echo "Creating symbolic link to Homebrew Botan library..."
ln -sf "${BOTAN_BREW_DIR}/lib/libbotan-3.dylib" "${INSTALL_DIR}/lib/libbotan-3.a"

# Add Cipher_Dir enum if not already defined
if ! grep -q "enum class Cipher_Dir" "${INSTALL_DIR}/include/botan-3/botan/botan.h"; then
  echo "Adding Cipher_Dir enum to botan.h..."
  # Append to botan.h
  cat >> "${INSTALL_DIR}/include/botan-3/botan/botan.h" << EOF

// Added for WebAssembly compatibility
namespace Botan {
enum class Cipher_Dir {
    ENCRYPTION,
    DECRYPTION
};
}  // namespace Botan
EOF
fi

echo "Botan headers have been copied successfully!"
echo "Headers: ${INSTALL_DIR}/include/botan-3/botan"
echo "Library: ${INSTALL_DIR}/lib/libbotan-3.a"
