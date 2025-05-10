#!/bin/bash
# Script to replace the GMP mock implementation with the WebAssembly implementation

# Set variables
GMP_WASM_DIR="gmp-wasm"
MOCK_DIR="build_wasm/mock"
WASM_DIR="lib/wasm"
INCLUDE_DIR="include"

# Create directories if they don't exist
mkdir -p "${WASM_DIR}/include/hydra_math"
mkdir -p "${WASM_DIR}/lib"

# Copy the WebAssembly files
echo "Copying WebAssembly files..."
cp "${GMP_WASM_DIR}/binding/dist/gmp.js" "${WASM_DIR}/gmp.js"
cp "${GMP_WASM_DIR}/binding/dist/gmp.wasm" "${WASM_DIR}/gmp.wasm"
cp "${GMP_WASM_DIR}/binding/dist/gmpmini.js" "${WASM_DIR}/gmpmini.js"
cp "${GMP_WASM_DIR}/binding/dist/gmpmini.wasm" "${WASM_DIR}/gmpmini.wasm"

# Copy the TypeScript definitions
echo "Copying TypeScript definitions..."
cp -r "${GMP_WASM_DIR}/dist/types" "${WASM_DIR}/types"

# Create a C++ wrapper for the WebAssembly implementation
echo "Creating C++ wrapper for WebAssembly implementation..."
cat > "${WASM_DIR}/include/gmp.h" << 'EOF'
/* GMP header for WebAssembly build */
#ifndef __GMP_H__
#define __GMP_H__

#ifdef __cplusplus
extern "C" {
#endif

/* For size_t */
#include <stddef.h>

/* For intmax_t */
#include <stdint.h>

/* Basic types */
typedef unsigned long mp_limb_t;
typedef long mp_size_t;
typedef long mp_exp_t;

/* Integer (i.e. Z) */
typedef struct {
  int _mp_alloc;
  int _mp_size;
  mp_limb_t *_mp_d;
} __mpz_struct;

/* Rational (i.e. Q) */
typedef struct {
  __mpz_struct _mp_num;
  __mpz_struct _mp_den;
} __mpq_struct;

/* Floating-point (i.e. R) */
typedef struct {
  int _mp_prec;
  int _mp_size;
  mp_exp_t _mp_exp;
  mp_limb_t *_mp_d;
} __mpf_struct;

typedef __mpz_struct mpz_t[1];
typedef __mpq_struct mpq_t[1];
typedef __mpf_struct mpf_t[1];

typedef __mpz_struct *mpz_ptr;
typedef __mpq_struct *mpq_ptr;
typedef __mpf_struct *mpf_ptr;

typedef const __mpz_struct *mpz_srcptr;
typedef const __mpq_struct *mpq_srcptr;
typedef const __mpf_struct *mpf_srcptr;

/* For compatibility with GMP */
typedef unsigned long mp_bitcnt_t;

/* Memory management */
void *_mpz_realloc (mpz_ptr, mp_size_t);

/* Initialization */
void mpz_init (mpz_ptr);
void mpz_init2 (mpz_ptr, mp_size_t);
void mpz_clear (mpz_ptr);

void mpz_init_set (mpz_ptr, mpz_srcptr);
void mpz_init_set_ui (mpz_ptr, unsigned long);
void mpz_init_set_si (mpz_ptr, long);
int mpz_init_set_str (mpz_ptr, const char *, int);

/* Assignment */
void mpz_set (mpz_ptr, mpz_srcptr);
void mpz_set_ui (mpz_ptr, unsigned long);
void mpz_set_si (mpz_ptr, long);
int mpz_set_str (mpz_ptr, const char *, int);

/* Conversion */
unsigned long mpz_get_ui (mpz_srcptr);
long mpz_get_si (mpz_srcptr);
char *mpz_get_str (char *, int, mpz_srcptr);
size_t mpz_sizeinbase (mpz_srcptr, int);

/* Arithmetic */
void mpz_add (mpz_ptr, mpz_srcptr, mpz_srcptr);
void mpz_add_ui (mpz_ptr, mpz_srcptr, unsigned long);
void mpz_sub (mpz_ptr, mpz_srcptr, mpz_srcptr);
void mpz_sub_ui (mpz_ptr, mpz_srcptr, unsigned long);
void mpz_mul (mpz_ptr, mpz_srcptr, mpz_srcptr);
void mpz_mul_si (mpz_ptr, mpz_srcptr, long);
void mpz_mul_ui (mpz_ptr, mpz_srcptr, unsigned long);
void mpz_neg (mpz_ptr, mpz_srcptr);
void mpz_abs (mpz_ptr, mpz_srcptr);

/* Division */
void mpz_fdiv_q (mpz_ptr, mpz_srcptr, mpz_srcptr);
void mpz_fdiv_r (mpz_ptr, mpz_srcptr, mpz_srcptr);
void mpz_fdiv_qr (mpz_ptr, mpz_ptr, mpz_srcptr, mpz_srcptr);
void mpz_fdiv_q_ui (mpz_ptr, mpz_srcptr, unsigned long);
unsigned long mpz_fdiv_r_ui (mpz_ptr, mpz_srcptr, unsigned long);
unsigned long mpz_fdiv_qr_ui (mpz_ptr, mpz_ptr, mpz_srcptr, unsigned long);
void mpz_fdiv_q_2exp (mpz_ptr, mpz_srcptr, mp_bitcnt_t);
void mpz_fdiv_r_2exp (mpz_ptr, mpz_srcptr, mp_bitcnt_t);

void mpz_tdiv_q (mpz_ptr, mpz_srcptr, mpz_srcptr);
void mpz_tdiv_r (mpz_ptr, mpz_srcptr, mpz_srcptr);
void mpz_tdiv_qr (mpz_ptr, mpz_ptr, mpz_srcptr, mpz_srcptr);
void mpz_tdiv_q_ui (mpz_ptr, mpz_srcptr, unsigned long);
unsigned long mpz_tdiv_r_ui (mpz_ptr, mpz_srcptr, unsigned long);
unsigned long mpz_tdiv_qr_ui (mpz_ptr, mpz_ptr, mpz_srcptr, unsigned long);
void mpz_tdiv_q_2exp (mpz_ptr, mpz_srcptr, mp_bitcnt_t);
void mpz_tdiv_r_2exp (mpz_ptr, mpz_srcptr, mp_bitcnt_t);

void mpz_mod (mpz_ptr, mpz_srcptr, mpz_srcptr);
void mpz_mod_ui (mpz_ptr, mpz_srcptr, unsigned long);

/* Comparison */
int mpz_cmp (mpz_srcptr, mpz_srcptr);
int mpz_cmp_ui (mpz_srcptr, unsigned long);
int mpz_cmp_si (mpz_srcptr, long);
int mpz_sgn (mpz_srcptr);

/* Logical operations */
void mpz_and (mpz_ptr, mpz_srcptr, mpz_srcptr);
void mpz_ior (mpz_ptr, mpz_srcptr, mpz_srcptr);
void mpz_xor (mpz_ptr, mpz_srcptr, mpz_srcptr);
void mpz_com (mpz_ptr, mpz_srcptr);

/* Memory management functions */
typedef void (*gmp_free_func) (void *, size_t);
void mp_get_memory_functions (void *(**) (size_t),
                             void *(**) (void *, size_t, size_t),
                             void (**) (void *, size_t));

/* Power functions */
void mpz_pow_ui (mpz_ptr, mpz_srcptr, unsigned long);
void mpz_ui_pow_ui (mpz_ptr, unsigned long, unsigned long);

#ifdef __cplusplus
}
#endif

#endif /* __GMP_H__ */
EOF

# Create a BigInt implementation that uses the WebAssembly GMP
echo "Creating BigInt implementation..."
cat > "${WASM_DIR}/include/hydra_math/bigint.hpp" << 'EOF'
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
        mpz_init_set_si(value_, val);
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

# Create a CMake find script for the WebAssembly GMP
echo "Creating CMake find script..."
cat > "${WASM_DIR}/FindGMP_WASM.cmake" << 'EOF'
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

# Update the FindWasmDeps.cmake file to include GMP
echo "Updating FindWasmDeps.cmake..."
if [ -f "${WASM_DIR}/FindWasmDeps.cmake" ]; then
    # Check if GMP_WASM is already included
    if grep -q "GMP_WASM" "${WASM_DIR}/FindWasmDeps.cmake"; then
        echo "GMP_WASM already included in FindWasmDeps.cmake"
    else
        # Create a backup of the original file
        cp "${WASM_DIR}/FindWasmDeps.cmake" "${WASM_DIR}/FindWasmDeps.cmake.bak"
        
        # Add GMP_WASM to the includes
        sed -i '' 's/# Find GMP/# Find GMP\ninclude(${CMAKE_CURRENT_LIST_DIR}\/FindGMP_WASM.cmake)/g' "${WASM_DIR}/FindWasmDeps.cmake"
        
        # Update the check for all dependencies
        sed -i '' 's/if(GMP_WASM_FOUND AND/if(GMP_WASM_FOUND AND/g' "${WASM_DIR}/FindWasmDeps.cmake"
        
        # Add GMP_WASM to the include directories
        sed -i '' 's/${GMP_WASM_INCLUDE_DIRS}/${GMP_WASM_INCLUDE_DIRS}/g' "${WASM_DIR}/FindWasmDeps.cmake"
    fi
else
    echo "FindWasmDeps.cmake not found, creating it..."
    cat > "${WASM_DIR}/FindWasmDeps.cmake" << 'EOF'
# FindWasmDeps.cmake
# Find all WebAssembly-compiled dependencies for Hydra SDK
#
# This module defines
#  WASM_DEPS_FOUND        - True if all dependencies were found
#  WASM_DEPS_INCLUDE_DIRS - The include directories

# Find GMP
include(${CMAKE_CURRENT_LIST_DIR}/FindGMP_WASM.cmake)

# Check if all dependencies were found
if(GMP_WASM_FOUND)
  set(WASM_DEPS_FOUND TRUE)

  # Combine include directories
  set(WASM_DEPS_INCLUDE_DIRS
    ${GMP_WASM_INCLUDE_DIRS}
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
fi

# Create a JavaScript loader for the WebAssembly GMP
echo "Creating JavaScript loader..."
cat > "${WASM_DIR}/gmp_loader.js" << 'EOF'
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

echo "GMP mock has been replaced with the WebAssembly implementation!"
echo "The WebAssembly files are in ${WASM_DIR}"
echo "The header files are in ${WASM_DIR}/include"
echo "The JavaScript loader is in ${WASM_DIR}/gmp_loader.js"
