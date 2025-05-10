#!/bin/bash
# Script to fix the GMP and MPFR WebAssembly build process (final version)

# Set variables
MOCK_DIR="/volumes/bigcode/hydra_sdk/build_wasm/mock"
GMP_WASM_DIR="/volumes/bigcode/hydra_sdk/gmp-wasm"
WASM_LIB_DIR="/volumes/bigcode/hydra_sdk/lib/wasm"

# Create directories if they don't exist
mkdir -p "${MOCK_DIR}/include"
mkdir -p "${MOCK_DIR}/lib"
mkdir -p "${WASM_LIB_DIR}/include"
mkdir -p "${WASM_LIB_DIR}/lib"

# Copy the real GMP library from the gmp-wasm repository
echo "Copying GMP library files..."
cp -f "${GMP_WASM_DIR}/binding/gmp/dist/lib/libgmp.a" "${WASM_LIB_DIR}/lib/"
cp -f "${GMP_WASM_DIR}/binding/mpfr/dist/lib/libmpfr.a" "${WASM_LIB_DIR}/lib/"

# Create symlinks to the real GMP library in the mock directory
echo "Creating symlinks to GMP library files..."
ln -sf "${WASM_LIB_DIR}/lib/libgmp.a" "${MOCK_DIR}/lib/libgmp.a"
ln -sf "${WASM_LIB_DIR}/lib/libmpfr.a" "${MOCK_DIR}/lib/libmpfr.a"

# Create a custom implementation of the BigInt class that uses the GMP WebAssembly library
echo "Creating custom BigInt implementation..."
mkdir -p "${MOCK_DIR}/include/hydra_math"

cat > "${MOCK_DIR}/include/hydra_math/bigint.hpp" << 'EOF'
#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <stdexcept>
#include <gmp.h>

namespace hydra {
namespace math {

/**
 * A BigInt implementation that uses the GMP library
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
        if (mpz_set_str(value_, str.c_str(), base) != 0) {
            throw std::invalid_argument("Invalid string format for BigInt");
        }
        initialized_ = true;
    }
    
    // Copy constructor
    BigInt(const BigInt& other) {
        mpz_init(value_);
        mpz_set(value_, other.value_);
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
    
    // Conversion to int
    int to_int() const {
        return static_cast<int>(mpz_get_si(value_));
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
        if (mpz_sgn(other.value_) == 0) {
            throw std::invalid_argument("Division by zero");
        }
        BigInt result;
        mpz_fdiv_q(result.value_, value_, other.value_);
        return result;
    }
    
    BigInt& operator+=(const BigInt& other) {
        mpz_add(value_, value_, other.value_);
        return *this;
    }
    
    BigInt& operator-=(const BigInt& other) {
        mpz_sub(value_, value_, other.value_);
        return *this;
    }
    
    BigInt& operator*=(const BigInt& other) {
        mpz_mul(value_, value_, other.value_);
        return *this;
    }
    
    BigInt& operator/=(const BigInt& other) {
        if (mpz_sgn(other.value_) == 0) {
            throw std::invalid_argument("Division by zero");
        }
        mpz_fdiv_q(value_, value_, other.value_);
        return *this;
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
    
    bool operator<=(const BigInt& other) const {
        return mpz_cmp(value_, other.value_) <= 0;
    }
    
    bool operator>(const BigInt& other) const {
        return mpz_cmp(value_, other.value_) > 0;
    }
    
    bool operator>=(const BigInt& other) const {
        return mpz_cmp(value_, other.value_) >= 0;
    }
    
    // Static methods
    static BigInt from_string(const std::string& str, int base = 10) {
        return BigInt(str, base);
    }
};

} // namespace math
} // namespace hydra
EOF

echo "Build process fixed!"
echo "Now run ./wasmbuild.sh --module all to rebuild the WebAssembly modules."
