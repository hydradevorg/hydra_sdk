#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <mpfr.h>

namespace hydra {
namespace math {

/**
 * A wrapper for MPFR (Multiple Precision Floating-Point Reliable) library
 */
class MPFRFloat {
private:
    mpfr_t value_;
    bool initialized_ = false;

public:
    // Default precision (in bits)
    static constexpr int DEFAULT_PRECISION = 128;

    // Constructors
    MPFRFloat(int precision = DEFAULT_PRECISION) {
        mpfr_init2(value_, precision);
        mpfr_set_d(value_, 0.0, MPFR_RNDN);
        initialized_ = true;
    }
    
    MPFRFloat(double val, int precision = DEFAULT_PRECISION) {
        mpfr_init2(value_, precision);
        mpfr_set_d(value_, val, MPFR_RNDN);
        initialized_ = true;
    }
    
    MPFRFloat(const std::string& str, int base = 10, int precision = DEFAULT_PRECISION) {
        mpfr_init2(value_, precision);
        mpfr_set_str(value_, str.c_str(), base, MPFR_RNDN);
        initialized_ = true;
    }
    
    // Copy constructor
    MPFRFloat(const MPFRFloat& other) {
        mpfr_init2(value_, mpfr_get_prec(other.value_));
        mpfr_set(value_, other.value_, MPFR_RNDN);
        initialized_ = true;
    }
    
    // Move constructor
    MPFRFloat(MPFRFloat&& other) noexcept {
        mpfr_init2(value_, mpfr_get_prec(other.value_));
        mpfr_swap(value_, other.value_);
        initialized_ = true;
    }
    
    // Destructor
    ~MPFRFloat() {
        if (initialized_) {
            mpfr_clear(value_);
            initialized_ = false;
        }
    }
    
    // Assignment operators
    MPFRFloat& operator=(const MPFRFloat& other) {
        if (this != &other) {
            mpfr_set_prec(value_, mpfr_get_prec(other.value_));
            mpfr_set(value_, other.value_, MPFR_RNDN);
        }
        return *this;
    }
    
    MPFRFloat& operator=(MPFRFloat&& other) noexcept {
        if (this != &other) {
            mpfr_swap(value_, other.value_);
        }
        return *this;
    }
    
    // Conversion to string
    std::string to_string(int base = 10, int precision = 0) const {
        if (precision <= 0) {
            precision = mpfr_get_prec(value_);
        }
        
        // Calculate the required buffer size
        size_t size = mpfr_get_str_ndigits(base, precision);
        size += 10; // Extra space for sign, decimal point, exponent, etc.
        
        // Allocate buffer
        char* buffer = new char[size];
        
        // Convert to string
        mpfr_snprintf(buffer, size, "%.*Rg", precision, value_);
        
        // Create string and free buffer
        std::string result(buffer);
        delete[] buffer;
        
        return result;
    }
    
    // Conversion to double
    double to_double() const {
        return mpfr_get_d(value_, MPFR_RNDN);
    }
    
    // Arithmetic operators
    MPFRFloat operator+(const MPFRFloat& other) const {
        MPFRFloat result(std::max(mpfr_get_prec(value_), mpfr_get_prec(other.value_)));
        mpfr_add(result.value_, value_, other.value_, MPFR_RNDN);
        return result;
    }
    
    MPFRFloat operator-(const MPFRFloat& other) const {
        MPFRFloat result(std::max(mpfr_get_prec(value_), mpfr_get_prec(other.value_)));
        mpfr_sub(result.value_, value_, other.value_, MPFR_RNDN);
        return result;
    }
    
    MPFRFloat operator*(const MPFRFloat& other) const {
        MPFRFloat result(std::max(mpfr_get_prec(value_), mpfr_get_prec(other.value_)));
        mpfr_mul(result.value_, value_, other.value_, MPFR_RNDN);
        return result;
    }
    
    MPFRFloat operator/(const MPFRFloat& other) const {
        MPFRFloat result(std::max(mpfr_get_prec(value_), mpfr_get_prec(other.value_)));
        mpfr_div(result.value_, value_, other.value_, MPFR_RNDN);
        return result;
    }
    
    // Comparison operators
    bool operator==(const MPFRFloat& other) const {
        return mpfr_cmp(value_, other.value_) == 0;
    }
    
    bool operator!=(const MPFRFloat& other) const {
        return mpfr_cmp(value_, other.value_) != 0;
    }
    
    bool operator<(const MPFRFloat& other) const {
        return mpfr_cmp(value_, other.value_) < 0;
    }
    
    bool operator>(const MPFRFloat& other) const {
        return mpfr_cmp(value_, other.value_) > 0;
    }
    
    // Mathematical functions
    MPFRFloat sqrt() const {
        MPFRFloat result(mpfr_get_prec(value_));
        mpfr_sqrt(result.value_, value_, MPFR_RNDN);
        return result;
    }
    
    MPFRFloat pow(const MPFRFloat& exponent) const {
        MPFRFloat result(std::max(mpfr_get_prec(value_), mpfr_get_prec(exponent.value_)));
        mpfr_pow(result.value_, value_, exponent.value_, MPFR_RNDN);
        return result;
    }
    
    MPFRFloat exp() const {
        MPFRFloat result(mpfr_get_prec(value_));
        mpfr_exp(result.value_, value_, MPFR_RNDN);
        return result;
    }
    
    MPFRFloat log() const {
        MPFRFloat result(mpfr_get_prec(value_));
        mpfr_log(result.value_, value_, MPFR_RNDN);
        return result;
    }
    
    MPFRFloat sin() const {
        MPFRFloat result(mpfr_get_prec(value_));
        mpfr_sin(result.value_, value_, MPFR_RNDN);
        return result;
    }
    
    MPFRFloat cos() const {
        MPFRFloat result(mpfr_get_prec(value_));
        mpfr_cos(result.value_, value_, MPFR_RNDN);
        return result;
    }
    
    MPFRFloat tan() const {
        MPFRFloat result(mpfr_get_prec(value_));
        mpfr_tan(result.value_, value_, MPFR_RNDN);
        return result;
    }
    
    // Get precision
    int get_precision() const {
        return mpfr_get_prec(value_);
    }
    
    // Set precision
    void set_precision(int precision) {
        mpfr_prec_round(value_, precision, MPFR_RNDN);
    }
};

} // namespace math
} // namespace hydra
