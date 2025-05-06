#pragma once

#include "lmvs/layered_vector.hpp"
#include "lmvs/layered_matrix.hpp"
#include <vector>
#include <unordered_map>
#include <string>
#include <memory>

namespace lmvs {

/**
 * @brief Class implementing the Secure Consensus Layer (SCL) as described in the LMVS paper.
 * 
 * This class provides mechanisms for validating data integrity across all layers
 * in a distributed system.
 */
class SecureConsensusLayer {
public:
    /**
     * @brief Construct a new Secure Consensus Layer
     * 
     * @param num_nodes Number of nodes in the distributed system
     * @param threshold Minimum number of nodes required for consensus
     */
    SecureConsensusLayer(size_t num_nodes, size_t threshold);

    /**
     * @brief Add a node contribution to the consensus
     * 
     * @param node_id Identifier of the contributing node
     * @param contribution The node's contribution vector
     * @return bool True if the contribution was accepted
     */
    bool addContribution(const std::string& node_id, const LayeredVector& contribution);

    /**
     * @brief Check if consensus has been reached
     * 
     * @return bool True if consensus has been reached
     */
    bool hasConsensus() const;

    /**
     * @brief Get the consensus vector if consensus has been reached
     * 
     * @return LayeredVector The consensus vector
     * @throws std::runtime_error if consensus has not been reached
     */
    LayeredVector getConsensusVector() const;

    /**
     * @brief Validate a vector against the consensus
     * 
     * @param vector Vector to validate
     * @return bool True if the vector is valid according to the consensus
     */
    bool validateVector(const LayeredVector& vector) const;

    /**
     * @brief Reset the consensus process
     */
    void reset();

    /**
     * @brief Get the number of contributions received so far
     * 
     * @return size_t Number of contributions
     */
    size_t getNumContributions() const { return m_contributions.size(); }

    /**
     * @brief Get the threshold for consensus
     * 
     * @return size_t Threshold
     */
    size_t getThreshold() const { return m_threshold; }

    /**
     * @brief Get the total number of nodes
     * 
     * @return size_t Number of nodes
     */
    size_t getNumNodes() const { return m_num_nodes; }

private:
    size_t m_num_nodes;                                      // Total number of nodes
    size_t m_threshold;                                      // Minimum nodes for consensus
    std::unordered_map<std::string, LayeredVector> m_contributions; // Node contributions
    mutable bool m_consensus_computed = false;               // Flag if consensus is computed
    mutable LayeredVector m_consensus_vector;                // Cached consensus vector
    
    /**
     * @brief Compute the consensus vector from contributions
     */
    void computeConsensusVector() const;
};

} // namespace lmvs
