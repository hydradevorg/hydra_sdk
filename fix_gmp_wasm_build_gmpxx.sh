#!/bin/bash
# Script to fix the GMP and MPFR WebAssembly build process using gmpxx.h

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

# Create a custom implementation of the BigInt class that uses the gmpxx.h header
echo "Creating custom BigInt implementation..."
mkdir -p "${MOCK_DIR}/include/hydra_math"

cat > "${MOCK_DIR}/include/hydra_math/bigint.hpp" << 'EOF'
#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <stdexcept>
#include <gmpxx.h>

namespace hydra {
namespace math {

/**
 * A BigInt implementation that uses the GMP C++ wrapper
 */
class BigInt {
private:
    mpz_class value_;

public:
    // Constructors
    BigInt() : value_(0) {}
    
    BigInt(int val) : value_(val) {}
    
    BigInt(const std::string& str, int base = 10) : value_(str, base) {}
    
    // Copy constructor
    BigInt(const BigInt& other) : value_(other.value_) {}
    
    // Move constructor
    BigInt(BigInt&& other) noexcept : value_(std::move(other.value_)) {}
    
    // Destructor
    ~BigInt() = default;
    
    // Assignment operators
    BigInt& operator=(const BigInt& other) {
        if (this != &other) {
            value_ = other.value_;
        }
        return *this;
    }
    
    BigInt& operator=(BigInt&& other) noexcept {
        if (this != &other) {
            value_ = std::move(other.value_);
        }
        return *this;
    }
    
    // Conversion to string
    std::string to_string(int base = 10) const {
        return value_.get_str(base);
    }
    
    // Conversion to int
    int to_int() const {
        return value_.get_si();
    }
    
    // Arithmetic operators
    BigInt operator+(const BigInt& other) const {
        BigInt result;
        result.value_ = value_ + other.value_;
        return result;
    }
    
    BigInt operator-(const BigInt& other) const {
        BigInt result;
        result.value_ = value_ - other.value_;
        return result;
    }
    
    BigInt operator*(const BigInt& other) const {
        BigInt result;
        result.value_ = value_ * other.value_;
        return result;
    }
    
    BigInt operator/(const BigInt& other) const {
        if (other.value_ == 0) {
            throw std::invalid_argument("Division by zero");
        }
        BigInt result;
        result.value_ = value_ / other.value_;
        return result;
    }
    
    BigInt& operator+=(const BigInt& other) {
        value_ += other.value_;
        return *this;
    }
    
    BigInt& operator-=(const BigInt& other) {
        value_ -= other.value_;
        return *this;
    }
    
    BigInt& operator*=(const BigInt& other) {
        value_ *= other.value_;
        return *this;
    }
    
    BigInt& operator/=(const BigInt& other) {
        if (other.value_ == 0) {
            throw std::invalid_argument("Division by zero");
        }
        value_ /= other.value_;
        return *this;
    }
    
    // Comparison operators
    bool operator==(const BigInt& other) const {
        return value_ == other.value_;
    }
    
    bool operator!=(const BigInt& other) const {
        return value_ != other.value_;
    }
    
    bool operator<(const BigInt& other) const {
        return value_ < other.value_;
    }
    
    bool operator<=(const BigInt& other) const {
        return value_ <= other.value_;
    }
    
    bool operator>(const BigInt& other) const {
        return value_ > other.value_;
    }
    
    bool operator>=(const BigInt& other) const {
        return value_ >= other.value_;
    }
    
    // Conversion to bytes
    std::vector<uint8_t> to_bytes() const {
        size_t count;
        void* data = mpz_export(nullptr, &count, 1, 1, 0, 0, value_.get_mpz_t());
        if (!data) {
            return std::vector<uint8_t>();
        }
        std::vector<uint8_t> result(static_cast<uint8_t*>(data), static_cast<uint8_t*>(data) + count);
        free(data);
        return result;
    }
    
    // Create from bytes
    static BigInt from_bytes(const std::vector<uint8_t>& bytes) {
        BigInt result;
        if (!bytes.empty()) {
            mpz_import(result.value_.get_mpz_t(), bytes.size(), 1, 1, 0, 0, bytes.data());
        }
        return result;
    }
    
    // Static methods
    static BigInt from_string(const std::string& str, int base = 10) {
        return BigInt(str, base);
    }
    
    // Access to the underlying mpz_class
    mpz_class& get_mpz_class() {
        return value_;
    }
    
    const mpz_class& get_mpz_class() const {
        return value_;
    }
};

} // namespace math
} // namespace hydra
EOF

# Create a mock gmp.h header file that includes the real gmp.h
echo "Creating mock gmp.h header file..."
cat > "${MOCK_DIR}/include/gmp.h" << 'EOF'
// Mock GMP header for WebAssembly build
#pragma once

#include "/volumes/bigcode/hydra_sdk/gmp-wasm/binding/gmp/dist/include/gmp.h"
EOF

# Create a mock gmpxx.h header file that includes the real gmpxx.h
echo "Creating mock gmpxx.h header file..."
cat > "${MOCK_DIR}/include/gmpxx.h" << 'EOF'
// Mock GMPXX header for WebAssembly build
#pragma once

#include "/volumes/bigcode/hydra_sdk/gmp-wasm/binding/gmp/src/gmpxx.h"
EOF

# Create a mock mpfr.h header file that includes the real mpfr.h
echo "Creating mock mpfr.h header file..."
cat > "${MOCK_DIR}/include/mpfr.h" << 'EOF'
// Mock MPFR header for WebAssembly build
#pragma once

#include "/volumes/bigcode/hydra_sdk/gmp-wasm/binding/mpfr/dist/include/mpfr.h"
EOF

# Create a mock mpf2mpfr.h header file that includes the real mpf2mpfr.h
echo "Creating mock mpf2mpfr.h header file..."
cat > "${MOCK_DIR}/include/mpf2mpfr.h" << 'EOF'
// Mock MPF2MPFR header for WebAssembly build
#pragma once

#include "/volumes/bigcode/hydra_sdk/gmp-wasm/binding/mpfr/dist/include/mpf2mpfr.h"
EOF

echo "Build process fixed!"
echo "Now run ./wasmbuild.sh --module all to rebuild the WebAssembly modules."
