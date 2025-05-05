#pragma once

#include "hydra_math/bigint.hpp"
#include <vector>

namespace hydra { namespace math {

/**
 * @brief Represents a share in Shamir's Secret Sharing scheme
 *
 * A share consists of an x-coordinate and a y-coordinate (a point on the polynomial).
 */
struct Share {
    BigInt x;  ///< The x-coordinate of the share
    BigInt y;  ///< The y-coordinate of the share (the actual share value)
};

/**
 * @brief Splits a secret into multiple shares using Shamir's Secret Sharing scheme
 *
 * This function implements Shamir's (t,n) threshold secret sharing scheme, which allows
 * a secret to be divided into n shares such that any t or more shares can reconstruct
 * the secret, but fewer than t shares reveal no information about the secret.
 *
 * @param secret The secret value to be shared
 * @param threshold The minimum number of shares required to reconstruct the secret (t)
 * @param shares The total number of shares to generate (n)
 * @param prime A prime number larger than the secret and the number of shares
 * @return A vector containing the generated shares
 * @throws std::invalid_argument if threshold > shares or if prime is too small
 */
std::vector<Share> shamirSplit(const BigInt& secret, int threshold, int shares, const BigInt& prime);

/**
 * @brief Reconstructs a secret from shares using Shamir's Secret Sharing scheme
 *
 * This function uses Lagrange interpolation to reconstruct the original secret from
 * a set of shares. The number of shares must be at least equal to the threshold
 * value used during the splitting process.
 *
 * @param shares The shares to use for reconstruction (must be at least threshold shares)
 * @param prime The same prime number used during the splitting process
 * @return The reconstructed secret
 * @throws std::invalid_argument if not enough shares are provided
 */
BigInt shamirReconstruct(const std::vector<Share>& shares, const BigInt& prime);

}} // namespace hydra::math
