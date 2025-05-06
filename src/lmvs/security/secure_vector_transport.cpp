#include "lmvs/security/secure_vector_transport.hpp"
#include <sstream>
#include <iomanip>
#include <cstring>

namespace lmvs {
namespace security {

std::vector<uint8_t> SecureVectorPackage::serialize() const {
    std::vector<uint8_t> result;

    // Store sizes of each component
    size_t encrypted_size = encrypted_data.size();
    size_t signature_size = signature.size();
    size_t auth_tag_size = auth_tag.size();
    size_t metadata_size = metadata.size();

    // Allocate space for sizes and data
    result.resize(4 * sizeof(size_t) + encrypted_size + signature_size + auth_tag_size + metadata_size);

    // Write sizes
    size_t offset = 0;
    std::memcpy(result.data() + offset, &encrypted_size, sizeof(size_t));
    offset += sizeof(size_t);

    std::memcpy(result.data() + offset, &signature_size, sizeof(size_t));
    offset += sizeof(size_t);

    std::memcpy(result.data() + offset, &auth_tag_size, sizeof(size_t));
    offset += sizeof(size_t);

    std::memcpy(result.data() + offset, &metadata_size, sizeof(size_t));
    offset += sizeof(size_t);

    // Write data
    if (encrypted_size > 0) {
        std::memcpy(result.data() + offset, encrypted_data.data(), encrypted_size);
        offset += encrypted_size;
    }

    if (signature_size > 0) {
        std::memcpy(result.data() + offset, signature.data(), signature_size);
        offset += signature_size;
    }

    if (auth_tag_size > 0) {
        std::memcpy(result.data() + offset, auth_tag.data(), auth_tag_size);
        offset += auth_tag_size;
    }

    if (metadata_size > 0) {
        std::memcpy(result.data() + offset, metadata.data(), metadata_size);
    }

    return result;
}

SecureVectorPackage SecureVectorPackage::deserialize(const std::vector<uint8_t>& data) {
    SecureVectorPackage package;

    if (data.size() < 4 * sizeof(size_t)) {
        throw std::invalid_argument("Invalid serialized package data");
    }

    // Read sizes
    size_t offset = 0;
    size_t encrypted_size, signature_size, auth_tag_size, metadata_size;

    std::memcpy(&encrypted_size, data.data() + offset, sizeof(size_t));
    offset += sizeof(size_t);

    std::memcpy(&signature_size, data.data() + offset, sizeof(size_t));
    offset += sizeof(size_t);

    std::memcpy(&auth_tag_size, data.data() + offset, sizeof(size_t));
    offset += sizeof(size_t);

    std::memcpy(&metadata_size, data.data() + offset, sizeof(size_t));
    offset += sizeof(size_t);

    // Validate sizes
    if (offset + encrypted_size + signature_size + auth_tag_size + metadata_size > data.size()) {
        throw std::invalid_argument("Invalid serialized package data");
    }

    // Read data
    if (encrypted_size > 0) {
        package.encrypted_data.resize(encrypted_size);
        std::memcpy(package.encrypted_data.data(), data.data() + offset, encrypted_size);
        offset += encrypted_size;
    }

    if (signature_size > 0) {
        package.signature.resize(signature_size);
        std::memcpy(package.signature.data(), data.data() + offset, signature_size);
        offset += signature_size;
    }

    if (auth_tag_size > 0) {
        package.auth_tag.resize(auth_tag_size);
        std::memcpy(package.auth_tag.data(), data.data() + offset, auth_tag_size);
        offset += auth_tag_size;
    }

    if (metadata_size > 0) {
        package.metadata.resize(metadata_size);
        std::memcpy(package.metadata.data(), data.data() + offset, metadata_size);
    }

    return package;
}

SecureVectorTransport::SecureVectorTransport(const std::string& kyber_mode, int falcon_degree)
    : m_crypto(std::make_unique<VectorCrypto>(kyber_mode, falcon_degree)) {
}

std::pair<std::vector<uint8_t>, std::vector<uint8_t>> SecureVectorTransport::generate_node_keys(const std::string& node_id) {
    // Generate encryption key pair
    auto [encryption_public_key, encryption_private_key] = m_crypto->generate_encryption_keypair();

    // Generate signing key pair
    auto [signing_public_key, signing_private_key] = m_crypto->generate_signing_keypair();

    // Create key bundles
    std::vector<uint8_t> public_key_bundle = create_key_bundle(encryption_public_key, signing_public_key);
    std::vector<uint8_t> private_key_bundle = create_key_bundle(encryption_private_key, signing_private_key);

    // Add this node to known nodes
    m_known_nodes[node_id] = public_key_bundle;

    return {public_key_bundle, private_key_bundle};
}

void SecureVectorTransport::add_known_node(const std::string& node_id, const std::vector<uint8_t>& public_key_bundle) {
    m_known_nodes[node_id] = public_key_bundle;
}

SecureVectorPackage SecureVectorTransport::package_vector(
    const LayeredBigIntVector& vector,
    const std::string& recipient_id,
    const std::vector<uint8_t>& sender_private_key_bundle) {

    // Check if recipient is known
    if (m_known_nodes.find(recipient_id) == m_known_nodes.end()) {
        throw std::invalid_argument("Unknown recipient");
    }

    // Extract keys
    auto [recipient_encryption_public_key, recipient_signing_public_key] =
        extract_keys_from_bundle(m_known_nodes[recipient_id], true);

    auto [sender_encryption_private_key, sender_signing_private_key] =
        extract_keys_from_bundle(sender_private_key_bundle, false);

    // Encrypt and sign the vector
    auto [encrypted_data, signature] = m_crypto->encrypt_and_sign(
        vector, recipient_encryption_public_key, sender_signing_private_key);

    // Establish a shared key for authentication
    std::vector<uint8_t> shared_key = establish_shared_key(recipient_id, sender_private_key_bundle);

    // Create authentication tag
    std::vector<uint8_t> auth_tag = m_crypto->authenticate_vector(vector, shared_key);

    // Create metadata (e.g., sender ID, timestamp)
    std::vector<uint8_t> metadata(recipient_id.begin(), recipient_id.end());

    // Create the package
    SecureVectorPackage package;
    package.encrypted_data = encrypted_data;
    package.signature = signature;
    package.auth_tag = auth_tag;
    package.metadata = metadata;

    return package;
}

std::pair<LayeredBigIntVector, bool> SecureVectorTransport::unpackage_vector(
    const SecureVectorPackage& package,
    const std::string& sender_id,
    const std::vector<uint8_t>& recipient_private_key_bundle) {

    // Check if sender is known
    if (m_known_nodes.find(sender_id) == m_known_nodes.end()) {
        throw std::invalid_argument("Unknown sender");
    }

    // Extract keys
    auto [sender_encryption_public_key, sender_signing_public_key] =
        extract_keys_from_bundle(m_known_nodes[sender_id], true);

    auto [recipient_encryption_private_key, recipient_signing_private_key] =
        extract_keys_from_bundle(recipient_private_key_bundle, false);

    // Decrypt and verify the vector
    auto [vector, is_valid] = m_crypto->decrypt_and_verify(
        package.encrypted_data, package.signature,
        recipient_encryption_private_key, sender_signing_public_key);

    if (!is_valid) {
        return {vector, false};
    }

    // Establish a shared key for authentication
    std::vector<uint8_t> shared_key = establish_shared_key(sender_id, recipient_private_key_bundle);

    // Verify authentication tag
    bool auth_valid = m_crypto->verify_authentication(vector, package.auth_tag, shared_key);

    return {vector, auth_valid};
}

std::vector<uint8_t> SecureVectorTransport::establish_shared_key(
    const std::string& node_id,
    const std::vector<uint8_t>& private_key_bundle) {

    // Check if node is known
    if (m_known_nodes.find(node_id) == m_known_nodes.end()) {
        throw std::invalid_argument("Unknown node");
    }

    // Extract keys
    auto [node_encryption_public_key, _] = extract_keys_from_bundle(m_known_nodes[node_id], true);
    auto [local_encryption_private_key, __] = extract_keys_from_bundle(private_key_bundle, false);

    // Use KyberKEM to establish a shared key
    auto [_, shared_key] = m_crypto->m_kyber_kem->encapsulate(node_encryption_public_key);

    return shared_key;
}

std::pair<std::vector<uint8_t>, std::vector<uint8_t>> SecureVectorTransport::extract_keys_from_bundle(
    const std::vector<uint8_t>& key_bundle, bool /* is_public */) {

    if (key_bundle.size() < 2 * sizeof(size_t)) {
        throw std::invalid_argument("Invalid key bundle");
    }

    // Read key sizes
    size_t offset = 0;
    size_t encryption_key_size, signing_key_size;

    std::memcpy(&encryption_key_size, key_bundle.data() + offset, sizeof(size_t));
    offset += sizeof(size_t);

    std::memcpy(&signing_key_size, key_bundle.data() + offset, sizeof(size_t));
    offset += sizeof(size_t);

    // Validate sizes
    if (offset + encryption_key_size + signing_key_size > key_bundle.size()) {
        throw std::invalid_argument("Invalid key bundle");
    }

    // Extract keys
    std::vector<uint8_t> encryption_key(encryption_key_size);
    std::vector<uint8_t> signing_key(signing_key_size);

    std::memcpy(encryption_key.data(), key_bundle.data() + offset, encryption_key_size);
    offset += encryption_key_size;

    std::memcpy(signing_key.data(), key_bundle.data() + offset, signing_key_size);

    return {encryption_key, signing_key};
}

std::vector<uint8_t> SecureVectorTransport::create_key_bundle(
    const std::vector<uint8_t>& encryption_key,
    const std::vector<uint8_t>& signing_key) {

    std::vector<uint8_t> bundle;

    // Store key sizes
    size_t encryption_key_size = encryption_key.size();
    size_t signing_key_size = signing_key.size();

    // Allocate space for sizes and keys
    bundle.resize(2 * sizeof(size_t) + encryption_key_size + signing_key_size);

    // Write sizes
    size_t offset = 0;
    std::memcpy(bundle.data() + offset, &encryption_key_size, sizeof(size_t));
    offset += sizeof(size_t);

    std::memcpy(bundle.data() + offset, &signing_key_size, sizeof(size_t));
    offset += sizeof(size_t);

    // Write keys
    std::memcpy(bundle.data() + offset, encryption_key.data(), encryption_key_size);
    offset += encryption_key_size;

    std::memcpy(bundle.data() + offset, signing_key.data(), signing_key_size);

    return bundle;
}

} // namespace security
} // namespace lmvs
