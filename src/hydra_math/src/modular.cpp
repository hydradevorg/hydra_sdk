#include "hydra_math/modular.hpp"
#include <stdexcept>

namespace hydra { namespace math {

/**
 * @brief Computes the modulo of a BigInt value
 *
 * This function ensures that the result is in the range [0, q-1].
 *
 * @param a The BigInt value to compute modulo of
 * @param q The modulus
 * @return The value a mod q in the range [0, q-1]
 */
BigInt mod(const BigInt& a, const BigInt& q) {
    // Create a temporary result
    BigInt r;
    // Use mpz_mod directly since operator% isn't implemented yet
    mpz_mod(r.get_mpz_t(), a.get_mpz_t(), q.get_mpz_t());
    // Check if result is negative
    if (mpz_sgn(r.get_mpz_t()) < 0) r = r + q;
    return r;
}

/**
 * @brief Performs modular addition of two BigInt values
 *
 * Computes (a + b) mod q, ensuring the result is in the range [0, q-1].
 *
 * @param a First operand
 * @param b Second operand
 * @param q The modulus
 * @return The result of (a + b) mod q
 */
BigInt modAdd(const BigInt& a, const BigInt& b, const BigInt& q) {
    return mod(a + b, q);
}

/**
 * @brief Performs modular subtraction of two BigInt values
 *
 * Computes (a - b) mod q, ensuring the result is in the range [0, q-1].
 *
 * @param a First operand
 * @param b Second operand
 * @param q The modulus
 * @return The result of (a - b) mod q
 */
BigInt modSub(const BigInt& a, const BigInt& b, const BigInt& q) {
    return mod(a - b, q);
}

/**
 * @brief Performs modular multiplication of two BigInt values
 *
 * Computes (a * b) mod q, ensuring the result is in the range [0, q-1].
 *
 * @param a First operand
 * @param b Second operand
 * @param q The modulus
 * @return The result of (a * b) mod q
 */
BigInt modMul(const BigInt& a, const BigInt& b, const BigInt& q) {
    return mod(a * b, q);
}

/**
 * @brief Computes the modular multiplicative inverse of a BigInt value
 *
 * Finds x such that (a * x) mod q = 1. Only works when a and q are coprime.
 *
 * @param a The value to find the inverse of
 * @param q The modulus
 * @return The modular multiplicative inverse of a mod q
 * @throws std::invalid_argument if the inverse does not exist (a and q not coprime)
 */
BigInt modInv(const BigInt& a, const BigInt& q) {
    BigInt inv;
    if (mpz_invert(inv.get_mpz_t(), a.get_mpz_t(), q.get_mpz_t()) == 0) {
        throw std::invalid_argument("No modular inverse exists");
    }
    return inv;
}

}} // namespace hydra::math
