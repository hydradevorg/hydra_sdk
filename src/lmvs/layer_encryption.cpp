#include "lmvs/layer_encryption.hpp"
#include <numeric>
#include <algorithm>
#include <cmath>

namespace lmvs {

std::vector<double> SimpleXOREncryptionProvider::encrypt(const std::vector<double>& layer, const std::string& key) {
    std::vector<double> keystream = generateKeystream(key, layer.size());
    std::vector<double> encrypted(layer.size());
    
    for (size_t i = 0; i < layer.size(); ++i) {
        // Simple XOR-like operation for floating-point values
        encrypted[i] = layer[i] + keystream[i];
    }
    
    return encrypted;
}

std::vector<double> SimpleXOREncryptionProvider::decrypt(const std::vector<double>& encrypted_layer, const std::string& key) {
    std::vector<double> keystream = generateKeystream(key, encrypted_layer.size());
    std::vector<double> decrypted(encrypted_layer.size());
    
    for (size_t i = 0; i < encrypted_layer.size(); ++i) {
        // Reverse the XOR-like operation
        decrypted[i] = encrypted_layer[i] - keystream[i];
    }
    
    return decrypted;
}

std::vector<double> SimpleXOREncryptionProvider::generateKeystream(const std::string& key, size_t length) {
    std::vector<double> keystream(length);
    
    // Simple keystream generation based on the key
    // This is NOT cryptographically secure, just for demonstration
    
    // Use a simple hash of the key as a seed
    size_t seed = 0;
    for (char c : key) {
        seed = seed * 31 + c;
    }
    
    // Generate pseudorandom values
    for (size_t i = 0; i < length; ++i) {
        // Simple linear congruential generator
        seed = (seed * 1103515245 + 12345) & 0x7FFFFFFF;
        keystream[i] = static_cast<double>(seed) / 0x7FFFFFFF;
    }
    
    return keystream;
}

LayerEncryption::LayerEncryption(std::shared_ptr<ILayerEncryptionProvider> provider)
    : m_provider(provider) {
    
    if (!provider) {
        throw std::invalid_argument("Encryption provider cannot be null");
    }
}

LayeredVector LayerEncryption::encrypt(const LayeredVector& vector, const std::vector<std::string>& keys) {
    if (keys.size() != vector.getNumLayers()) {
        throw std::invalid_argument("Number of keys must match number of layers");
    }
    
    std::vector<std::vector<double>> encrypted_layers;
    encrypted_layers.reserve(vector.getNumLayers());
    
    for (size_t i = 0; i < vector.getNumLayers(); ++i) {
        encrypted_layers.push_back(m_provider->encrypt(vector.getLayer(i), keys[i]));
    }
    
    return LayeredVector(encrypted_layers);
}

LayeredVector LayerEncryption::decrypt(const LayeredVector& encrypted_vector, const std::vector<std::string>& keys) {
    if (keys.size() != encrypted_vector.getNumLayers()) {
        throw std::invalid_argument("Number of keys must match number of layers");
    }
    
    std::vector<std::vector<double>> decrypted_layers;
    decrypted_layers.reserve(encrypted_vector.getNumLayers());
    
    for (size_t i = 0; i < encrypted_vector.getNumLayers(); ++i) {
        decrypted_layers.push_back(m_provider->decrypt(encrypted_vector.getLayer(i), keys[i]));
    }
    
    return LayeredVector(decrypted_layers);
}

} // namespace lmvs
