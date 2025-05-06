#pragma once

#include "lmvs/layered_vector.hpp"
#include <vector>
#include <string>
#include <memory>
#include <stdexcept>

namespace lmvs {

/**
 * @brief Abstract base class for layer encryption providers.
 * 
 * This class defines the interface for encrypting and decrypting
 * individual layers of a layered vector.
 */
class ILayerEncryptionProvider {
public:
    virtual ~ILayerEncryptionProvider() = default;
    
    /**
     * @brief Encrypt a layer
     * 
     * @param layer Layer to encrypt
     * @param key Encryption key
     * @return std::vector<double> Encrypted layer
     */
    virtual std::vector<double> encrypt(const std::vector<double>& layer, const std::string& key) = 0;
    
    /**
     * @brief Decrypt a layer
     * 
     * @param encrypted_layer Encrypted layer
     * @param key Decryption key
     * @return std::vector<double> Decrypted layer
     */
    virtual std::vector<double> decrypt(const std::vector<double>& encrypted_layer, const std::string& key) = 0;
};

/**
 * @brief Simple XOR-based encryption provider for demonstration purposes.
 * 
 * This is NOT secure for production use, but demonstrates the concept.
 */
class SimpleXOREncryptionProvider : public ILayerEncryptionProvider {
public:
    std::vector<double> encrypt(const std::vector<double>& layer, const std::string& key) override;
    std::vector<double> decrypt(const std::vector<double>& encrypted_layer, const std::string& key) override;

private:
    /**
     * @brief Generate a keystream from the key
     * 
     * @param key Encryption key
     * @param length Length of the keystream
     * @return std::vector<double> Keystream
     */
    std::vector<double> generateKeystream(const std::string& key, size_t length);
};

/**
 * @brief Class for encrypting and decrypting layered vectors.
 * 
 * This class provides methods for encrypting and decrypting
 * individual layers of a layered vector using different keys.
 */
class LayerEncryption {
public:
    /**
     * @brief Construct a new Layer Encryption object
     * 
     * @param provider Encryption provider
     */
    explicit LayerEncryption(std::shared_ptr<ILayerEncryptionProvider> provider);

    /**
     * @brief Encrypt a layered vector
     * 
     * @param vector Layered vector to encrypt
     * @param keys Vector of encryption keys, one for each layer
     * @return LayeredVector Encrypted layered vector
     * @throws std::invalid_argument if keys.size() != vector.getNumLayers()
     */
    LayeredVector encrypt(const LayeredVector& vector, const std::vector<std::string>& keys);

    /**
     * @brief Decrypt a layered vector
     * 
     * @param encrypted_vector Encrypted layered vector
     * @param keys Vector of decryption keys, one for each layer
     * @return LayeredVector Decrypted layered vector
     * @throws std::invalid_argument if keys.size() != encrypted_vector.getNumLayers()
     */
    LayeredVector decrypt(const LayeredVector& encrypted_vector, const std::vector<std::string>& keys);

private:
    std::shared_ptr<ILayerEncryptionProvider> m_provider;
};

} // namespace lmvs
