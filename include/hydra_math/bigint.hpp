#pragma once
#include <gmp.h>
#include <string>
#include <stdexcept>
#include <cstdint>
#include <ostream>
#include <istream>
#include <functional>

namespace hydra { namespace math {

/**
 * @brief Arbitrary-precision integer class based on the GNU Multiple Precision (GMP) library
 *
 * This class provides a C++ wrapper around the GMP library's mpz_t type, offering
 * arbitrary-precision integer arithmetic with a convenient, object-oriented interface.
 * It supports all standard arithmetic operations, comparisons, and conversions to and
 * from standard types.
 */
class BigInt {
public:
    /**
     * @brief Default constructor
     *
     * Initializes the BigInt to zero.
     */
    BigInt() { mpz_init(value_); }

    /**
     * @brief Constructs a BigInt from a 64-bit signed integer
     *
     * @param val The integer value to initialize with
     */
    explicit BigInt(int64_t val) {
        mpz_init_set_si(value_, val);
    }

    /**
     * @brief Constructs a BigInt from a string representation
     *
     * @param str The string representation of the integer
     * @param base The base of the number representation (default: 10)
     * @throws std::invalid_argument if the string is not a valid number in the specified base
     */
    explicit BigInt(const std::string& str, int base = 10) {
        mpz_init(value_);
        if (mpz_set_str(value_, str.c_str(), base) != 0)
            throw std::invalid_argument("Invalid string for BigInt");
    }

    /**
     * @brief Copy constructor
     *
     * @param other The BigInt to copy from
     */
    BigInt(const BigInt& other) {
        mpz_init_set(value_, other.value_);
    }

    /**
     * @brief Copy assignment operator
     *
     * @param other The BigInt to copy from
     * @return Reference to this object after assignment
     */
    BigInt& operator=(const BigInt& other) {
        if (this != &other)
            mpz_set(value_, other.value_);
        return *this;
    }

    /**
     * @brief Move constructor
     *
     * @param other The BigInt to move from
     */
    BigInt(BigInt&& other) noexcept {
        mpz_init(value_);
        mpz_set(value_, other.value_);
        mpz_set_si(other.value_, 0);
    }

    /**
     * @brief Move assignment operator
     *
     * @param other The BigInt to move from
     * @return Reference to this object after assignment
     */
    BigInt& operator=(BigInt&& other) noexcept {
        if (this != &other) {
            mpz_set(value_, other.value_);
            mpz_set_si(other.value_, 0);
        }
        return *this;
    }

    /**
     * @brief Destructor
     *
     * Frees the memory used by the GMP integer.
     */
    ~BigInt() {
        mpz_clear(value_);
    }

    /**
     * @brief Converts the BigInt to a string representation
     *
     * @param base The base for the string representation (default: 10)
     * @return String representation of the BigInt in the specified base
     */
    std::string to_string(int base = 10) const {
        char* cstr = mpz_get_str(nullptr, base, value_);
        std::string result(cstr);
        void (*freefunc)(void*, size_t);
        mp_get_memory_functions(nullptr, nullptr, &freefunc);
        freefunc(cstr, std::strlen(cstr) + 1);
        return result;
    }

    /**
     * @brief Converts the BigInt to a 64-bit signed integer
     *
     * @return The value as an int64_t (truncated if it exceeds the range)
     */
    int64_t to_int() const {
        return mpz_get_si(value_);
    }

    /**
     * @brief Addition operator
     *
     * @param rhs The right-hand side operand
     * @return The sum of this BigInt and rhs
     */
    BigInt operator+(const BigInt& rhs) const {
        BigInt result;
        mpz_add(result.value_, value_, rhs.value_);
        return result;
    }

    /**
     * @brief Subtraction operator
     *
     * @param rhs The right-hand side operand
     * @return The difference between this BigInt and rhs
     */
    BigInt operator-(const BigInt& rhs) const {
        BigInt result;
        mpz_sub(result.value_, value_, rhs.value_);
        return result;
    }

    /**
     * @brief Multiplication operator
     *
     * @param rhs The right-hand side operand
     * @return The product of this BigInt and rhs
     */
    BigInt operator*(const BigInt& rhs) const {
        BigInt result;
        mpz_mul(result.value_, value_, rhs.value_);
        return result;
    }

    /**
     * @brief Division operator
     *
     * Performs integer division, truncating toward negative infinity.
     *
     * @param rhs The right-hand side operand
     * @return The quotient of this BigInt divided by rhs
     * @throws std::domain_error if rhs is zero
     */
    BigInt operator/(const BigInt& rhs) const {
        BigInt result;
        mpz_fdiv_q(result.value_, value_, rhs.value_);
        return result;
    }

    /**
     * @brief Addition assignment operator
     *
     * @param rhs The right-hand side operand
     * @return Reference to this object after addition
     */
    BigInt& operator+=(const BigInt& rhs) {
        mpz_add(value_, value_, rhs.value_);
        return *this;
    }

    /**
     * @brief Subtraction assignment operator
     *
     * @param rhs The right-hand side operand
     * @return Reference to this object after subtraction
     */
    BigInt& operator-=(const BigInt& rhs) {
        mpz_sub(value_, value_, rhs.value_);
        return *this;
    }

    /**
     * @brief Multiplication assignment operator
     *
     * @param rhs The right-hand side operand
     * @return Reference to this object after multiplication
     */
    BigInt& operator*=(const BigInt& rhs) {
        mpz_mul(value_, value_, rhs.value_);
        return *this;
    }

    /**
     * @brief Division assignment operator
     *
     * @param rhs The right-hand side operand
     * @return Reference to this object after division
     * @throws std::domain_error if rhs is zero
     */
    BigInt& operator/=(const BigInt& rhs) {
        mpz_fdiv_q(value_, value_, rhs.value_);
        return *this;
    }

    /**
     * @brief Equality operator
     *
     * @param rhs The right-hand side operand
     * @return true if this BigInt equals rhs, false otherwise
     */
    bool operator==(const BigInt& rhs) const {
        return mpz_cmp(value_, rhs.value_) == 0;
    }

    /**
     * @brief Inequality operator
     *
     * @param rhs The right-hand side operand
     * @return true if this BigInt is not equal to rhs, false otherwise
     */
    bool operator!=(const BigInt& rhs) const {
        return !(*this == rhs);
    }

    /**
     * @brief Less-than operator
     *
     * @param rhs The right-hand side operand
     * @return true if this BigInt is less than rhs, false otherwise
     */
    bool operator<(const BigInt& rhs) const {
        return mpz_cmp(value_, rhs.value_) < 0;
    }

    /**
     * @brief Greater-than operator
     *
     * @param rhs The right-hand side operand
     * @return true if this BigInt is greater than rhs, false otherwise
     */
    bool operator>(const BigInt& rhs) const {
        return mpz_cmp(value_, rhs.value_) > 0;
    }

    /**
     * @brief Less-than-or-equal operator
     *
     * @param rhs The right-hand side operand
     * @return true if this BigInt is less than or equal to rhs, false otherwise
     */
    bool operator<=(const BigInt& rhs) const {
        return mpz_cmp(value_, rhs.value_) <= 0;
    }

    /**
     * @brief Greater-than-or-equal operator
     *
     * @param rhs The right-hand side operand
     * @return true if this BigInt is greater than or equal to rhs, false otherwise
     */
    bool operator>=(const BigInt& rhs) const {
        return mpz_cmp(value_, rhs.value_) >= 0;
    }

    /**
     * @brief Stream insertion operator
     *
     * Allows writing a BigInt to an output stream.
     *
     * @param os The output stream
     * @param bi The BigInt to write
     * @return Reference to the output stream
     */
    friend std::ostream& operator<<(std::ostream& os, const BigInt& bi) {
        return os << bi.to_string();
    }

    /**
     * @brief Stream extraction operator
     *
     * Allows reading a BigInt from an input stream.
     *
     * @param is The input stream
     * @param bi The BigInt to read into
     * @return Reference to the input stream
     */
    friend std::istream& operator>>(std::istream& is, BigInt& bi) {
        std::string str;
        is >> str;
        mpz_set_str(bi.value_, str.c_str(), 10);
        return is;
    }

    /**
     * @brief Gets a reference to the underlying GMP integer
     *
     * @return Reference to the underlying mpz_t
     */
    mpz_t& get_mpz_t() { return value_; }

    /**
     * @brief Gets a const reference to the underlying GMP integer
     *
     * @return Const reference to the underlying mpz_t
     */
    const mpz_t& get_mpz_t() const { return value_; }

    /**
     * @brief Gets a pointer to the underlying GMP integer
     *
     * @return Pointer to the underlying mpz_t
     */
    mpz_t* raw() { return &value_; }

    /**
     * @brief Gets a const pointer to the underlying GMP integer
     *
     * @return Const pointer to the underlying mpz_t
     */
    const mpz_t* raw() const { return &value_; }

private:
    mpz_t value_;  ///< The underlying GMP integer
};

}} // namespace hydra::math

// --- Hash Support ---
namespace std {
/**
 * @brief Hash function specialization for hydra::math::BigInt
 *
 * Allows using BigInt as a key in unordered containers like std::unordered_map and std::unordered_set.
 */
template<>
struct hash<hydra::math::BigInt> {
    /**
     * @brief Computes a hash value for a BigInt
     *
     * @param k The BigInt to hash
     * @return The hash value
     */
    std::size_t operator()(const hydra::math::BigInt& k) const {
        auto str = k.to_string();
        return std::hash<std::string>{}(str);
    }
};

} // namespace std
