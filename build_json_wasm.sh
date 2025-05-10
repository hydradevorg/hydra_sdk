#!/bin/bash
# Script to install nlohmann/json for WebAssembly

# Set variables
INSTALL_DIR="/volumes/bigcode/hydra_sdk/lib/wasm"
JSON_VERSION="3.11.2"

# Create directories
mkdir -p "${INSTALL_DIR}/include/nlohmann"

# Download JSON if not already downloaded
if [ ! -f "${INSTALL_DIR}/include/nlohmann/json.hpp" ]; then
  echo "Downloading nlohmann/json ${JSON_VERSION}..."
  curl -L "https://github.com/nlohmann/json/releases/download/v${JSON_VERSION}/json.hpp" -o "${INSTALL_DIR}/include/nlohmann/json.hpp"
fi

echo "nlohmann/json has been installed successfully!"
echo "Header: ${INSTALL_DIR}/include/nlohmann/json.hpp"

# Create CMake find script for JSON
cat > "${INSTALL_DIR}/FindJSON_WASM.cmake" << EOF
# FindJSON_WASM.cmake
# Find the nlohmann/json library for WebAssembly
#
# This module defines
#  JSON_WASM_FOUND        - True if JSON for WASM was found
#  JSON_WASM_INCLUDE_DIRS - The JSON include directories

set(JSON_WASM_ROOT "${INSTALL_DIR}")

find_path(JSON_WASM_INCLUDE_DIR NAMES nlohmann/json.hpp
          PATHS \${JSON_WASM_ROOT}/include
          NO_DEFAULT_PATH)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(JSON_WASM DEFAULT_MSG
                                  JSON_WASM_INCLUDE_DIR)

if(JSON_WASM_FOUND)
  set(JSON_WASM_INCLUDE_DIRS \${JSON_WASM_INCLUDE_DIR})
endif()

mark_as_advanced(JSON_WASM_INCLUDE_DIR)
EOF

echo "Created CMake find script at ${INSTALL_DIR}/FindJSON_WASM.cmake"
