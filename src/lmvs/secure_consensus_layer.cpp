#include "lmvs/secure_consensus_layer.hpp"
#include <stdexcept>
#include <algorithm>
#include <iostream>

namespace lmvs {

SecureConsensusLayer::SecureConsensusLayer(size_t num_nodes, size_t threshold)
    : m_num_nodes(num_nodes), m_threshold(threshold) {
    
    if (threshold > num_nodes) {
        throw std::invalid_argument("Threshold cannot be greater than the number of nodes");
    }
    
    if (threshold == 0) {
        throw std::invalid_argument("Threshold must be at least 1");
    }
}

bool SecureConsensusLayer::addContribution(const std::string& node_id, const LayeredVector& contribution) {
    // Check if this node has already contributed
    if (m_contributions.find(node_id) != m_contributions.end()) {
        return false;
    }
    
    // Check if we already have enough contributions
    if (m_contributions.size() >= m_num_nodes) {
        return false;
    }
    
    // Add the contribution
    m_contributions[node_id] = contribution;
    
    // Reset consensus state since we have a new contribution
    m_consensus_computed = false;
    
    return true;
}

bool SecureConsensusLayer::hasConsensus() const {
    return m_contributions.size() >= m_threshold;
}

LayeredVector SecureConsensusLayer::getConsensusVector() const {
    if (!hasConsensus()) {
        throw std::runtime_error("Consensus has not been reached");
    }
    
    if (!m_consensus_computed) {
        computeConsensusVector();
    }
    
    return m_consensus_vector;
}

bool SecureConsensusLayer::validateVector(const LayeredVector& vector) const {
    if (!hasConsensus()) {
        return false;
    }
    
    if (!m_consensus_computed) {
        computeConsensusVector();
    }
    
    // Check if the vector has the same structure as the consensus vector
    if (vector.getNumLayers() != m_consensus_vector.getNumLayers() ||
        vector.getDimension() != m_consensus_vector.getDimension()) {
        return false;
    }
    
    // Calculate squared distance between the vector and the consensus vector
    double distance = vector.squaredDistance(m_consensus_vector);
    
    // Define a threshold for validation (this could be a parameter)
    const double validation_threshold = 1e-6;
    
    return distance < validation_threshold;
}

void SecureConsensusLayer::reset() {
    m_contributions.clear();
    m_consensus_computed = false;
}

void SecureConsensusLayer::computeConsensusVector() const {
    if (m_contributions.empty()) {
        throw std::runtime_error("No contributions available");
    }
    
    // Get the first contribution to determine dimensions
    auto it = m_contributions.begin();
    const LayeredVector& first = it->second;
    const size_t num_layers = first.getNumLayers();
    const size_t dimension = first.getDimension();
    
    // Initialize consensus vector with zeros
    std::vector<std::vector<double>> consensus_data(num_layers, std::vector<double>(dimension, 0.0));
    
    // Sum all contributions
    for (const auto& [node_id, contribution] : m_contributions) {
        for (size_t i = 0; i < num_layers; ++i) {
            const auto& layer = contribution.getLayer(i);
            for (size_t j = 0; j < dimension; ++j) {
                consensus_data[i][j] += layer[j];
            }
        }
    }
    
    // Average the contributions
    const double scale = 1.0 / m_contributions.size();
    for (auto& layer : consensus_data) {
        for (auto& value : layer) {
            value *= scale;
        }
    }
    
    m_consensus_vector = LayeredVector(consensus_data);
    m_consensus_computed = true;
}

} // namespace lmvs
