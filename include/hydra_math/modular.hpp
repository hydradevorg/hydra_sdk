#pragma once

#include "hydra_math/bigint.hpp"
#include <vector>

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
BigInt mod(const BigInt& a, const BigInt& q);

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
BigInt modAdd(const BigInt& a, const BigInt& b, const BigInt& q);

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
BigInt modSub(const BigInt& a, const BigInt& b, const BigInt& q);

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
BigInt modMul(const BigInt& a, const BigInt& b, const BigInt& q);

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
BigInt modInv(const BigInt& a, const BigInt& q);

}} // namespace hydra::math
