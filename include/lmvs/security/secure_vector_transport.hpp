#pragma once

#include "lmvs/layered_bigint_vector.hpp"
#include "lmvs/security/vector_crypto.hpp"
#include <vector>
#include <string>
#include <memory>
#include <unordered_map>

namespace lmvs {
namespace security {

/**
 * @brief Structure representing a secure vector package for transport
 */
struct SecureVectorPackage {
    std::vector<uint8_t> encrypted_data;  // Encrypted vector data
    std::vector<uint8_t> signature;       // Signature of the encrypted data
    std::vector<uint8_t> auth_tag;        // Authentication tag
    std::vector<uint8_t> metadata;        // Additional metadata (e.g., sender ID, timestamp)
    
    /**
     * @brief Serialize the package to a byte array
     * 
     * @return std::vector<uint8_t> Serialized package
     */
    std::vector<uint8_t> serialize() const;
    
    /**
     * @brief Deserialize a byte array to a package
     * 
     * @param data Serialized package data
     * @return SecureVectorPackage Deserialized package
     */
    static SecureVectorPackage deserialize(const std::vector<uint8_t>& data);
};

/**
 * @brief Class for secure transport of layered vectors
 * 
 * This class provides methods for securely packaging and transporting
 * layered vectors using encryption, signatures, and authentication.
 */
class SecureVectorTransport {
public:
    /**
     * @brief Construct a new Secure Vector Transport object
     * 
     * @param kyber_mode Kyber security mode (Kyber512, Kyber768, or Kyber1024)
     * @param falcon_degree Falcon degree parameter (512 or 1024)
     */
    SecureVectorTransport(const std::string& kyber_mode = "Kyber768", int falcon_degree = 512);
    
    /**
     * @brief Generate key pairs for a node
     * 
     * @param node_id Unique identifier for the node
     * @return std::pair<std::vector<uint8_t>, std::vector<uint8_t>> {public_key_bundle, private_key_bundle}
     */
    std::pair<std::vector<uint8_t>, std::vector<uint8_t>> generate_node_keys(const std::string& node_id);
    
    /**
     * @brief Add a known node with its public keys
     * 
     * @param node_id Unique identifier for the node
     * @param public_key_bundle The node's public key bundle
     */
    void add_known_node(const std::string& node_id, const std::vector<uint8_t>& public_key_bundle);
    
    /**
     * @brief Package a vector for secure transport
     * 
     * @param vector The vector to package
     * @param recipient_id The recipient's node ID
     * @param sender_private_key_bundle The sender's private key bundle
     * @return SecureVectorPackage The secure package
     */
    SecureVectorPackage package_vector(
        const LayeredBigIntVector& vector,
        const std::string& recipient_id,
        const std::vector<uint8_t>& sender_private_key_bundle);
    
    /**
     * @brief Unpackage a secure vector package
     * 
     * @param package The secure package
     * @param sender_id The sender's node ID
     * @param recipient_private_key_bundle The recipient's private key bundle
     * @return std::pair<LayeredBigIntVector, bool> {vector, is_valid}
     */
    std::pair<LayeredBigIntVector, bool> unpackage_vector(
        const SecureVectorPackage& package,
        const std::string& sender_id,
        const std::vector<uint8_t>& recipient_private_key_bundle);
    
    /**
     * @brief Establish a shared key with another node
     * 
     * @param node_id The other node's ID
     * @param private_key_bundle The local node's private key bundle
     * @return std::vector<uint8_t> The shared key
     */
    std::vector<uint8_t> establish_shared_key(
        const std::string& node_id,
        const std::vector<uint8_t>& private_key_bundle);
    
private:
    std::unique_ptr<VectorCrypto> m_crypto;
    std::unordered_map<std::string, std::vector<uint8_t>> m_known_nodes; // node_id -> public_key_bundle
    
    // Helper methods
    std::pair<std::vector<uint8_t>, std::vector<uint8_t>> extract_keys_from_bundle(
        const std::vector<uint8_t>& key_bundle, bool is_public);
    std::vector<uint8_t> create_key_bundle(
        const std::vector<uint8_t>& encryption_key,
        const std::vector<uint8_t>& signing_key);
};

} // namespace security
} // namespace lmvs
