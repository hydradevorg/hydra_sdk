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
