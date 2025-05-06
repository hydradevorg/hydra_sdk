#include "lmvs/lmvs.hpp"

namespace lmvs {

LMVS::LMVS(size_t num_layers, size_t dimension, size_t num_nodes, size_t threshold)
    : m_num_layers(num_layers), m_dimension(dimension) {
    
    m_consensus_layer = std::make_shared<SecureConsensusLayer>(num_nodes, threshold);
    m_encryption_provider = std::make_shared<SimpleXOREncryptionProvider>();
    m_layer_encryption = std::make_shared<LayerEncryption>(m_encryption_provider);
}

LayeredVector LMVS::createVector(const std::vector<std::vector<double>>& data) {
    return LayeredVector(data);
}

LayeredMatrix LMVS::createMatrix(const LayeredVector& vec_a, const LayeredVector& vec_b) {
    return LayeredMatrix(vec_a, vec_b);
}

LayeredVector LMVS::projectVector(const LayeredVector& vector, size_t output_dim) {
    if (output_dim > vector.getDimension()) {
        throw std::invalid_argument("Output dimension cannot be larger than input dimension");
    }
    
    ProjectionMatrix proj = ProjectionMatrix::createOrthogonal(vector.getDimension(), output_dim);
    return proj.project(vector);
}

std::unordered_map<size_t, LayeredVector> LMVS::splitVector(
    const LayeredVector& vector, size_t num_shares, size_t threshold) {
    
    LayeredSecretSharing secret_sharing(threshold, num_shares);
    return secret_sharing.split(vector);
}

LayeredVector LMVS::reconstructVector(
    const std::unordered_map<size_t, LayeredVector>& shares, size_t threshold) {
    
    LayeredSecretSharing secret_sharing(threshold, shares.size());
    return secret_sharing.reconstruct(shares);
}

LayeredVector LMVS::encryptVector(const LayeredVector& vector, const std::vector<std::string>& keys) {
    return m_layer_encryption->encrypt(vector, keys);
}

LayeredVector LMVS::decryptVector(const LayeredVector& encrypted_vector, const std::vector<std::string>& keys) {
    return m_layer_encryption->decrypt(encrypted_vector, keys);
}

bool LMVS::addConsensusContribution(const std::string& node_id, const LayeredVector& contribution) {
    return m_consensus_layer->addContribution(node_id, contribution);
}

bool LMVS::hasConsensus() const {
    return m_consensus_layer->hasConsensus();
}

LayeredVector LMVS::getConsensusVector() const {
    return m_consensus_layer->getConsensusVector();
}

bool LMVS::validateVector(const LayeredVector& vector) const {
    return m_consensus_layer->validateVector(vector);
}

} // namespace lmvs
