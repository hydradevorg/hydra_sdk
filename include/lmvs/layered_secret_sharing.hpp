#pragma once

#include "lmvs/layered_vector.hpp"
#include <vector>
#include <unordered_map>
#include <string>
#include <stdexcept>
#include <random>

namespace lmvs {

/**
 * @brief Class implementing Shamir's Secret Sharing for layered vectors.
 * 
 * This class provides methods for splitting a layered vector into shares
 * and reconstructing it from a subset of those shares.
 */
class LayeredSecretSharing {
public:
    /**
     * @brief Construct a new Layered Secret Sharing object
     * 
     * @param threshold Minimum number of shares needed for reconstruction
     * @param num_shares Total number of shares to generate
     */
    LayeredSecretSharing(size_t threshold, size_t num_shares);

    /**
     * @brief Split a layered vector into shares
     * 
     * @param secret The layered vector to split
     * @return std::unordered_map<size_t, LayeredVector> Map of share ID to share
     */
    std::unordered_map<size_t, LayeredVector> split(const LayeredVector& secret);

    /**
     * @brief Reconstruct a layered vector from shares
     * 
     * @param shares Map of share ID to share
     * @return LayeredVector The reconstructed layered vector
     * @throws std::invalid_argument if not enough shares are provided
     */
    LayeredVector reconstruct(const std::unordered_map<size_t, LayeredVector>& shares);

    /**
     * @brief Get the threshold
     * 
     * @return size_t Threshold
     */
    size_t getThreshold() const { return m_threshold; }

    /**
     * @brief Get the number of shares
     * 
     * @return size_t Number of shares
     */
    size_t getNumShares() const { return m_num_shares; }

private:
    size_t m_threshold;  // Minimum number of shares needed for reconstruction
    size_t m_num_shares; // Total number of shares to generate
    std::mt19937 m_rng;  // Random number generator

    /**
     * @brief Generate a random polynomial of degree threshold-1
     * 
     * @param secret The secret value (constant term of the polynomial)
     * @return std::vector<double> Coefficients of the polynomial
     */
    std::vector<double> generatePolynomial(double secret);

    /**
     * @brief Evaluate a polynomial at a given point
     * 
     * @param polynomial Coefficients of the polynomial
     * @param x Point at which to evaluate
     * @return double Value of the polynomial at x
     */
    double evaluatePolynomial(const std::vector<double>& polynomial, double x);

    /**
     * @brief Interpolate a polynomial at x=0 using Lagrange basis polynomials
     * 
     * @param points Vector of (x, y) points
     * @return double Interpolated value at x=0
     */
    double lagrangeInterpolation(const std::vector<std::pair<double, double>>& points);
};

} // namespace lmvs
