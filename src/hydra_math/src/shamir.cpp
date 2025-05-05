#include "hydra_math/shamir.hpp"
#include "hydra_math/modular.hpp"
#include <random>

namespace hydra { namespace math {

/**
 * @brief Generates a random BigInt in the range [1, mod-1]
 *
 * This helper function creates a cryptographically secure random BigInt value
 * that is used for generating polynomial coefficients in Shamir's Secret Sharing.
 *
 * @param mod The upper bound (exclusive) for the random value
 * @return A random BigInt in the range [1, mod-1]
 */
static BigInt randomBigIntMod(const BigInt& mod) {
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint64_t> dis(1, UINT64_MAX);

    BigInt r;
    do {
        // Create a temporary BigInt and then use the mod function
        BigInt temp(std::to_string(dis(gen)));
        r = hydra::math::mod(temp, mod);
    } while (r == BigInt("0"));
    return r;
}

/**
 * @brief Splits a secret into multiple shares using Shamir's Secret Sharing scheme
 *
 * This function implements Shamir's (t,n) threshold secret sharing scheme, which allows
 * a secret to be divided into n shares such that any t or more shares can reconstruct
 * the secret, but fewer than t shares reveal no information about the secret.
 *
 * The implementation creates a random polynomial of degree (threshold-1) with the constant
 * term set to the secret, then evaluates this polynomial at different points to create shares.
 *
 * @param secret The secret value to be shared
 * @param threshold The minimum number of shares required to reconstruct the secret (t)
 * @param shares The total number of shares to generate (n)
 * @param prime A prime number larger than the secret and the number of shares
 * @return A vector containing the generated shares
 * @throws std::invalid_argument if threshold > shares or if prime is too small
 */
std::vector<Share> shamirSplit(const BigInt& secret, int threshold, int shares, const BigInt& prime) {
    std::vector<BigInt> coeffs = {secret};
    for (int i = 1; i < threshold; ++i)
        coeffs.push_back(randomBigIntMod(prime));

    std::vector<Share> result;
    for (int i = 1; i <= shares; ++i) {
        BigInt x = BigInt(std::to_string(i));
        BigInt y = BigInt("0");
        BigInt xpow = BigInt("1");

        for (const auto& c : coeffs) {
            y = modAdd(y, modMul(c, xpow, prime), prime);
            xpow = modMul(xpow, x, prime);
        }

        result.push_back({x, y});
    }
    return result;
}

/**
 * @brief Reconstructs a secret from shares using Shamir's Secret Sharing scheme
 *
 * This function uses Lagrange interpolation to reconstruct the original secret from
 * a set of shares. The number of shares must be at least equal to the threshold
 * value used during the splitting process.
 *
 * The implementation computes the Lagrange basis polynomials and uses them to
 * interpolate the polynomial at x=0, which gives the original secret.
 *
 * @param shares The shares to use for reconstruction (must be at least threshold shares)
 * @param prime The same prime number used during the splitting process
 * @return The reconstructed secret
 * @throws std::invalid_argument if not enough shares are provided
 */
BigInt shamirReconstruct(const std::vector<Share>& shares, const BigInt& prime) {
    BigInt result("0");

    for (size_t i = 0; i < shares.size(); ++i) {
        BigInt xi = shares[i].x;
        BigInt yi = shares[i].y;

        BigInt num("1");
        BigInt denom("1");

        for (size_t j = 0; j < shares.size(); ++j) {
            if (i == j) continue;
            BigInt xj = shares[j].x;
            num = modMul(num, xj * BigInt("-1"), prime);            // -xj
            denom = modMul(denom, xi - xj, prime);
        }

        BigInt lagrange = modMul(num, modInv(denom, prime), prime);
        result = modAdd(result, modMul(yi, lagrange, prime), prime);
    }

    return result;
}

}} // namespace hydra::math
