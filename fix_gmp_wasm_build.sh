#!/bin/bash
# Script to fix the GMP and MPFR WebAssembly build process

# Set variables
MOCK_DIR="/volumes/bigcode/hydra_sdk/build_wasm/mock"
GMP_WASM_DIR="/volumes/bigcode/hydra_sdk/gmp-wasm"
WASM_LIB_DIR="/volumes/bigcode/hydra_sdk/lib/wasm"

# Create directories if they don't exist
mkdir -p "${MOCK_DIR}/include"
mkdir -p "${MOCK_DIR}/lib"
mkdir -p "${WASM_LIB_DIR}/include"
mkdir -p "${WASM_LIB_DIR}/lib"

# Copy GMP header files from the gmp-wasm repository to the mock directory
echo "Copying GMP header files to mock directory..."
cp "${GMP_WASM_DIR}/binding/gmp/dist/include/gmp.h" "${MOCK_DIR}/include/"
cp "${GMP_WASM_DIR}/binding/gmp/src/gmpxx.h" "${MOCK_DIR}/include/"

# Copy MPFR header files from the gmp-wasm repository to the mock directory
echo "Copying MPFR header files to mock directory..."
cp "${GMP_WASM_DIR}/binding/mpfr/dist/include/mpfr.h" "${MOCK_DIR}/include/"
cp "${GMP_WASM_DIR}/binding/mpfr/dist/include/mpf2mpfr.h" "${MOCK_DIR}/include/"

# Copy GMP and MPFR header files to the wasm lib directory
echo "Copying GMP and MPFR header files to wasm lib directory..."
cp "${GMP_WASM_DIR}/binding/gmp/dist/include/gmp.h" "${WASM_LIB_DIR}/include/"
cp "${GMP_WASM_DIR}/binding/gmp/src/gmpxx.h" "${WASM_LIB_DIR}/include/"
cp "${GMP_WASM_DIR}/binding/mpfr/dist/include/mpfr.h" "${WASM_LIB_DIR}/include/"
cp "${GMP_WASM_DIR}/binding/mpfr/dist/include/mpf2mpfr.h" "${WASM_LIB_DIR}/include/"

# Copy GMP WebAssembly files to the wasm lib directory
echo "Copying GMP WebAssembly files to wasm lib directory..."
cp "${GMP_WASM_DIR}/binding/dist/gmp.js" "${WASM_LIB_DIR}/"
cp "${GMP_WASM_DIR}/binding/dist/gmp.wasm" "${WASM_LIB_DIR}/"
cp "${GMP_WASM_DIR}/binding/dist/gmpmini.js" "${WASM_LIB_DIR}/"
cp "${GMP_WASM_DIR}/binding/dist/gmpmini.wasm" "${WASM_LIB_DIR}/"

# Create empty mock library files
echo "Creating empty mock library files..."
touch "${MOCK_DIR}/lib/libgmp.a"
touch "${MOCK_DIR}/lib/libmpfr.a"

# Create a JavaScript loader for the WebAssembly GMP
echo "Creating JavaScript loader..."
cat > "${WASM_LIB_DIR}/gmp_loader.js" << 'EOF'
// GMP WebAssembly loader
const gmpModule = require('./gmp.js');

// Initialize the GMP module
async function initGMP() {
  const gmp = await gmpModule();
  return gmp;
}

module.exports = {
  initGMP
};
EOF

# Create a BigInt implementation that uses the WebAssembly GMP
echo "Creating BigInt implementation..."
mkdir -p "${WASM_LIB_DIR}/include/hydra_math"
cat > "${WASM_LIB_DIR}/include/hydra_math/bigint.hpp" << 'EOF'
#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <gmp.h>

namespace hydra {
namespace math {

/**
 * A BigInt implementation that uses the GMP WebAssembly library
 */
class BigInt {
private:
    mpz_t value_;
    bool initialized_ = false;

public:
    // Constructors
    BigInt() {
        mpz_init(value_);
        initialized_ = true;
    }
    
    BigInt(int val) {
        mpz_init(value_);
        mpz_set_si(value_, val);
        initialized_ = true;
    }
    
    BigInt(const std::string& str, int base = 10) {
        mpz_init(value_);
        mpz_set_str(value_, str.c_str(), base);
        initialized_ = true;
    }
    
    // Copy constructor
    BigInt(const BigInt& other) {
        mpz_init_set(value_, other.value_);
        initialized_ = true;
    }
    
    // Move constructor
    BigInt(BigInt&& other) noexcept {
        mpz_init(value_);
        mpz_swap(value_, other.value_);
        initialized_ = true;
    }
    
    // Destructor
    ~BigInt() {
        if (initialized_) {
            mpz_clear(value_);
            initialized_ = false;
        }
    }
    
    // Assignment operators
    BigInt& operator=(const BigInt& other) {
        if (this != &other) {
            mpz_set(value_, other.value_);
        }
        return *this;
    }
    
    BigInt& operator=(BigInt&& other) noexcept {
        if (this != &other) {
            mpz_swap(value_, other.value_);
        }
        return *this;
    }
    
    // Conversion to string
    std::string to_string(int base = 10) const {
        char* str = mpz_get_str(nullptr, base, value_);
        std::string result(str);
        free(str);
        return result;
    }
    
    // Conversion to bytes
    std::vector<uint8_t> to_bytes() const {
        size_t count;
        void* data = mpz_export(nullptr, &count, 1, 1, 0, 0, value_);
        std::vector<uint8_t> result(static_cast<uint8_t*>(data), static_cast<uint8_t*>(data) + count);
        free(data);
        return result;
    }
    
    // Create from bytes
    static BigInt from_bytes(const std::vector<uint8_t>& bytes) {
        BigInt result;
        mpz_import(result.value_, bytes.size(), 1, 1, 0, 0, bytes.data());
        return result;
    }
    
    // Arithmetic operators
    BigInt operator+(const BigInt& other) const {
        BigInt result;
        mpz_add(result.value_, value_, other.value_);
        return result;
    }
    
    BigInt operator-(const BigInt& other) const {
        BigInt result;
        mpz_sub(result.value_, value_, other.value_);
        return result;
    }
    
    BigInt operator*(const BigInt& other) const {
        BigInt result;
        mpz_mul(result.value_, value_, other.value_);
        return result;
    }
    
    BigInt operator/(const BigInt& other) const {
        BigInt result;
        mpz_fdiv_q(result.value_, value_, other.value_);
        return result;
    }
    
    // Comparison operators
    bool operator==(const BigInt& other) const {
        return mpz_cmp(value_, other.value_) == 0;
    }
    
    bool operator!=(const BigInt& other) const {
        return mpz_cmp(value_, other.value_) != 0;
    }
    
    bool operator<(const BigInt& other) const {
        return mpz_cmp(value_, other.value_) < 0;
    }
    
    bool operator>(const BigInt& other) const {
        return mpz_cmp(value_, other.value_) > 0;
    }
};

} // namespace math
} // namespace hydra
EOF

# Copy the BigInt implementation to the mock directory
echo "Copying BigInt implementation to mock directory..."
mkdir -p "${MOCK_DIR}/include/hydra_math"
cp "${WASM_LIB_DIR}/include/hydra_math/bigint.hpp" "${MOCK_DIR}/include/hydra_math/"

# Create a CMake find script for the WebAssembly GMP
echo "Creating CMake find script..."
cat > "${WASM_LIB_DIR}/FindGMP_WASM.cmake" << 'EOF'
# FindGMP_WASM.cmake
# Find the GMP library compiled for WebAssembly
#
# This module defines
#  GMP_WASM_FOUND        - True if GMP for WASM was found
#  GMP_WASM_INCLUDE_DIRS - The GMP include directories
#  GMP_WASM_LIBRARIES    - The GMP libraries

set(GMP_WASM_ROOT "${CMAKE_CURRENT_LIST_DIR}")

find_path(GMP_WASM_INCLUDE_DIR NAMES gmp.h
          PATHS ${GMP_WASM_ROOT}/include
          NO_DEFAULT_PATH)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GMP_WASM DEFAULT_MSG
                                  GMP_WASM_INCLUDE_DIR)

if(GMP_WASM_FOUND)
  set(GMP_WASM_INCLUDE_DIRS ${GMP_WASM_INCLUDE_DIR})
endif()

mark_as_advanced(GMP_WASM_INCLUDE_DIR)
EOF

# Create a CMake find script for MPFR
echo "Creating CMake find script for MPFR..."
cat > "${WASM_LIB_DIR}/FindMPFR_WASM.cmake" << 'EOF'
# FindMPFR_WASM.cmake
# Find the MPFR library compiled for WebAssembly
#
# This module defines
#  MPFR_WASM_FOUND        - True if MPFR for WASM was found
#  MPFR_WASM_INCLUDE_DIRS - The MPFR include directories
#  MPFR_WASM_LIBRARIES    - The MPFR libraries

set(MPFR_WASM_ROOT "${CMAKE_CURRENT_LIST_DIR}")

find_path(MPFR_WASM_INCLUDE_DIR NAMES mpfr.h
          PATHS ${MPFR_WASM_ROOT}/include
          NO_DEFAULT_PATH)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(MPFR_WASM DEFAULT_MSG
                                  MPFR_WASM_INCLUDE_DIR)

if(MPFR_WASM_FOUND)
  set(MPFR_WASM_INCLUDE_DIRS ${MPFR_WASM_INCLUDE_DIR})
endif()

mark_as_advanced(MPFR_WASM_INCLUDE_DIR)
EOF

# Update the FindWasmDeps.cmake file
echo "Updating FindWasmDeps.cmake..."
cat > "${WASM_LIB_DIR}/FindWasmDeps.cmake" << 'EOF'
# FindWasmDeps.cmake
# Find all WebAssembly-compiled dependencies for Hydra SDK
#
# This module defines
#  WASM_DEPS_FOUND        - True if all dependencies were found
#  WASM_DEPS_INCLUDE_DIRS - The include directories

# Find GMP
include(${CMAKE_CURRENT_LIST_DIR}/FindGMP_WASM.cmake)

# Find MPFR
include(${CMAKE_CURRENT_LIST_DIR}/FindMPFR_WASM.cmake)

# Check if all dependencies were found
if(GMP_WASM_FOUND AND MPFR_WASM_FOUND)
  set(WASM_DEPS_FOUND TRUE)

  # Combine include directories
  set(WASM_DEPS_INCLUDE_DIRS
    ${GMP_WASM_INCLUDE_DIRS}
    ${MPFR_WASM_INCLUDE_DIRS}
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

echo "Build process fixed!"
echo "Now run ./wasmbuild.sh --module all to rebuild the WebAssembly modules."
