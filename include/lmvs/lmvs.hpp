#pragma once

#include "lmvs/layered_vector.hpp"
#include "lmvs/layered_matrix.hpp"
#include "lmvs/projection_matrix.hpp"
#include "lmvs/secure_consensus_layer.hpp"
#include "lmvs/layered_secret_sharing.hpp"
#include "lmvs/layer_encryption.hpp"

namespace lmvs {

/**
 * @brief Main class for the Layered Matrix and Vector System.
 * 
 * This class integrates all components of the LMVS system and provides
 * a high-level interface for using the system.
 */
class LMVS {
public:
    /**
     * @brief Construct a new LMVS object
     * 
     * @param num_layers Number of layers in vectors
     * @param dimension Dimension of each layer
     * @param num_nodes Number of nodes in the distributed system
     * @param threshold Minimum number of nodes required for consensus
     */
    LMVS(size_t num_layers, size_t dimension, size_t num_nodes, size_t threshold);

    /**
     * @brief Create a layered vector
     * 
     * @param data Vector of vectors, where each inner vector represents a layer
     * @return LayeredVector The created layered vector
     */
    LayeredVector createVector(const std::vector<std::vector<double>>& data);

    /**
     * @brief Create a layered matrix from two vectors
     * 
     * @param vec_a First layered vector
     * @param vec_b Second layered vector
     * @return LayeredMatrix The created layered matrix
     */
    LayeredMatrix createMatrix(const LayeredVector& vec_a, const LayeredVector& vec_b);

    /**
     * @brief Project a vector to a lower dimension
     * 
     * @param vector Vector to project
     * @param output_dim Output dimension
     * @return LayeredVector Projected vector
     */
    LayeredVector projectVector(const LayeredVector& vector, size_t output_dim);

    /**
     * @brief Split a vector into shares using secret sharing
     * 
     * @param vector Vector to split
     * @param num_shares Number of shares to generate
     * @param threshold Minimum number of shares needed for reconstruction
     * @return std::unordered_map<size_t, LayeredVector> Map of share ID to share
     */
    std::unordered_map<size_t, LayeredVector> splitVector(
        const LayeredVector& vector, size_t num_shares, size_t threshold);

    /**
     * @brief Reconstruct a vector from shares
     * 
     * @param shares Map of share ID to share
     * @param threshold Minimum number of shares needed for reconstruction
     * @return LayeredVector Reconstructed vector
     */
    LayeredVector reconstructVector(
        const std::unordered_map<size_t, LayeredVector>& shares, size_t threshold);

    /**
     * @brief Encrypt a vector
     * 
     * @param vector Vector to encrypt
     * @param keys Vector of encryption keys, one for each layer
     * @return LayeredVector Encrypted vector
     */
    LayeredVector encryptVector(const LayeredVector& vector, const std::vector<std::string>& keys);

    /**
     * @brief Decrypt a vector
     * 
     * @param encrypted_vector Encrypted vector
     * @param keys Vector of decryption keys, one for each layer
     * @return LayeredVector Decrypted vector
     */
    LayeredVector decryptVector(const LayeredVector& encrypted_vector, const std::vector<std::string>& keys);

    /**
     * @brief Add a node contribution to the consensus
     * 
     * @param node_id Identifier of the contributing node
     * @param contribution The node's contribution vector
     * @return bool True if the contribution was accepted
     */
    bool addConsensusContribution(const std::string& node_id, const LayeredVector& contribution);

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
     */
    LayeredVector getConsensusVector() const;

    /**
     * @brief Validate a vector against the consensus
     * 
     * @param vector Vector to validate
     * @return bool True if the vector is valid according to the consensus
     */
    bool validateVector(const LayeredVector& vector) const;

private:
    size_t m_num_layers;
    size_t m_dimension;
    std::shared_ptr<SecureConsensusLayer> m_consensus_layer;
    std::shared_ptr<ILayerEncryptionProvider> m_encryption_provider;
    std::shared_ptr<LayerEncryption> m_layer_encryption;
};

} // namespace lmvs
