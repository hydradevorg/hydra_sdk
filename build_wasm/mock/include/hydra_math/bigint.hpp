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

    // For compatibility with the original BigInt implementation
    mpz_t& get_mpz_t() {
        return value_.get_mpz_t();
    }

    const mpz_t& get_mpz_t() const {
        return value_.get_mpz_t();
    }

    mpz_t* raw() {
        return &value_.get_mpz_t();
    }

    const mpz_t* raw() const {
        return &value_.get_mpz_t();
    }
};

} // namespace math
} // namespace hydra
