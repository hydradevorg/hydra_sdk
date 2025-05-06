#include "lmvs/layered_secret_sharing.hpp"
#include <random>
#include <algorithm>
#include <iostream>
#include <ctime>

namespace lmvs {

LayeredSecretSharing::LayeredSecretSharing(size_t threshold, size_t num_shares)
    : m_threshold(threshold), m_num_shares(num_shares), m_rng(std::random_device{}()) {

    if (threshold > num_shares) {
        throw std::invalid_argument("Threshold cannot be greater than the number of shares");
    }

    if (threshold < 2) {
        throw std::invalid_argument("Threshold must be at least 2");
    }
}

std::unordered_map<size_t, LayeredVector> LayeredSecretSharing::split(const LayeredVector& secret) {
    const size_t num_layers = secret.getNumLayers();
    const size_t dimension = secret.getDimension();

    std::unordered_map<size_t, LayeredVector> shares;

    // Initialize shares with empty layered vectors
    for (size_t i = 1; i <= m_num_shares; ++i) {
        shares[i] = LayeredVector(num_layers, dimension);
    }

    // For each layer and each element in the layer
    for (size_t layer_idx = 0; layer_idx < num_layers; ++layer_idx) {
        const auto& layer = secret.getLayer(layer_idx);

        for (size_t dim_idx = 0; dim_idx < dimension; ++dim_idx) {
            // Generate a random polynomial with the secret as the constant term
            std::vector<double> polynomial = generatePolynomial(layer.at(dim_idx));

            // Evaluate the polynomial at different points to generate shares
            for (size_t share_idx = 1; share_idx <= m_num_shares; ++share_idx) {
                double x = static_cast<double>(share_idx);
                double y = evaluatePolynomial(polynomial, x);

                // Update the share
                auto& share_layer = shares[share_idx].getLayerMutable(layer_idx);
                share_layer[dim_idx] = y;
            }
        }
    }

    return shares;
}

LayeredVector LayeredSecretSharing::reconstruct(const std::unordered_map<size_t, LayeredVector>& shares) {
    if (shares.size() < m_threshold) {
        throw std::invalid_argument("Not enough shares for reconstruction");
    }

    // Get dimensions from the first share
    auto it = shares.begin();
    const size_t num_layers = it->second.getNumLayers();
    const size_t dimension = it->second.getDimension();

    // Initialize the reconstructed secret
    LayeredVector reconstructed(num_layers, dimension);

    // For each layer and each element in the layer
    for (size_t layer_idx = 0; layer_idx < num_layers; ++layer_idx) {
        for (size_t dim_idx = 0; dim_idx < dimension; ++dim_idx) {
            // Collect points for interpolation
            std::vector<std::pair<double, double>> points;
            for (const auto& [share_idx, share] : shares) {
                double x = static_cast<double>(share_idx);
                double y = share.getLayer(layer_idx).at(dim_idx);
                points.emplace_back(x, y);

                // We only need threshold points for interpolation
                if (points.size() >= m_threshold) {
                    break;
                }
            }

            // Interpolate to get the secret
            double secret = lagrangeInterpolation(points);

            // Update the reconstructed secret
            auto& reconstructed_layer = reconstructed.getLayerMutable(layer_idx);
            reconstructed_layer[dim_idx] = secret;
        }
    }

    return reconstructed;
}

std::vector<double> LayeredSecretSharing::generatePolynomial(double secret) {
    std::vector<double> coefficients(m_threshold);

    // Set the constant term to the secret
    coefficients[0] = secret;

    // Generate random coefficients for higher-degree terms
    std::uniform_real_distribution<double> dist(-1.0, 1.0);
    for (size_t i = 1; i < m_threshold; ++i) {
        coefficients[i] = dist(m_rng);
    }

    return coefficients;
}

double LayeredSecretSharing::evaluatePolynomial(const std::vector<double>& polynomial, double x) {
    double result = 0.0;
    double x_power = 1.0;

    for (double coef : polynomial) {
        result += coef * x_power;
        x_power *= x;
    }

    return result;
}

double LayeredSecretSharing::lagrangeInterpolation(const std::vector<std::pair<double, double>>& points) {
    double result = 0.0;

    for (size_t j = 0; j < points.size(); ++j) {
        double term = points[j].second;

        for (size_t i = 0; i < points.size(); ++i) {
            if (i != j) {
                term *= (-points[i].first) / (points[j].first - points[i].first);
            }
        }

        result += term;
    }

    return result;
}

} // namespace lmvs
