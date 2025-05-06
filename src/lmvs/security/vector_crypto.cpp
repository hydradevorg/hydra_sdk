#include "lmvs/security/vector_crypto.hpp"
#include <sstream>
#include <iomanip>
#include <cstring>
#include <openssl/hmac.h>
#include <openssl/sha.h>

namespace lmvs {
namespace security {

VectorCrypto::VectorCrypto(const std::string& kyber_mode, int falcon_degree)
    : m_kyber_kem(std::make_unique<hydra::crypto::KyberKEM>(kyber_mode)),
      m_kyber_aes(std::make_unique<hydra::crypto::KyberAES>(kyber_mode)),
      m_falcon(std::make_unique<hydra::crypto::FalconSignature>(falcon_degree)) {
}

std::pair<std::vector<uint8_t>, std::vector<uint8_t>> VectorCrypto::generate_encryption_keypair() {
    return m_kyber_kem->generate_keypair();
}

std::pair<std::vector<uint8_t>, std::vector<uint8_t>> VectorCrypto::generate_signing_keypair() {
    return m_falcon->generate_key_pair();
}

std::vector<uint8_t> VectorCrypto::encrypt_vector(const LayeredBigIntVector& vector, 
                                                const std::vector<uint8_t>& public_key) {
    // Convert vector to bytes
    std::vector<uint8_t> vector_bytes = vector_to_bytes(vector);
    
    // Encrypt using KyberAES
    return m_kyber_aes->encrypt(vector_bytes, public_key);
}

LayeredBigIntVector VectorCrypto::decrypt_vector(const std::vector<uint8_t>& encrypted_data,
                                               const std::vector<uint8_t>& private_key) {
    // Set up KyberAES with the private key
    m_falcon->set_private_key(private_key);
    
    // Decrypt the data
    std::vector<uint8_t> decrypted_bytes = m_kyber_aes->decrypt(encrypted_data, private_key);
    
    // Convert bytes back to vector
    return bytes_to_vector(decrypted_bytes);
}

std::vector<uint8_t> VectorCrypto::sign_vector(const LayeredBigIntVector& vector,
                                             const std::vector<uint8_t>& private_key) {
    // Set the private key for signing
    m_falcon->set_private_key(private_key);
    
    // Convert vector to bytes
    std::vector<uint8_t> vector_bytes = vector_to_bytes(vector);
    
    // Sign the vector bytes
    return m_falcon->sign_message(vector_bytes);
}

bool VectorCrypto::verify_vector(const LayeredBigIntVector& vector,
                               const std::vector<uint8_t>& signature,
                               const std::vector<uint8_t>& public_key) {
    // Set the public key for verification
    m_falcon->set_public_key(public_key);
    
    // Convert vector to bytes
    std::vector<uint8_t> vector_bytes = vector_to_bytes(vector);
    
    // Verify the signature
    return m_falcon->verify_signature(vector_bytes, signature);
}

std::pair<std::vector<uint8_t>, std::vector<uint8_t>> VectorCrypto::encrypt_and_sign(
    const LayeredBigIntVector& vector,
    const std::vector<uint8_t>& encryption_public_key,
    const std::vector<uint8_t>& signing_private_key) {
    
    // Encrypt the vector
    std::vector<uint8_t> encrypted_data = encrypt_vector(vector, encryption_public_key);
    
    // Set the private key for signing
    m_falcon->set_private_key(signing_private_key);
    
    // Sign the encrypted data
    std::vector<uint8_t> signature = m_falcon->sign_message(encrypted_data);
    
    return {encrypted_data, signature};
}

std::pair<LayeredBigIntVector, bool> VectorCrypto::decrypt_and_verify(
    const std::vector<uint8_t>& encrypted_data,
    const std::vector<uint8_t>& signature,
    const std::vector<uint8_t>& encryption_private_key,
    const std::vector<uint8_t>& signing_public_key) {
    
    // Set the public key for verification
    m_falcon->set_public_key(signing_public_key);
    
    // Verify the signature of the encrypted data
    bool is_valid = m_falcon->verify_signature(encrypted_data, signature);
    
    // Decrypt the data
    LayeredBigIntVector decrypted_vector;
    if (is_valid) {
        decrypted_vector = decrypt_vector(encrypted_data, encryption_private_key);
    }
    
    return {decrypted_vector, is_valid};
}

std::vector<uint8_t> VectorCrypto::authenticate_vector(const LayeredBigIntVector& vector,
                                                    const std::vector<uint8_t>& shared_key) {
    // Convert vector to bytes
    std::vector<uint8_t> vector_bytes = vector_to_bytes(vector);
    
    // Create HMAC-SHA256 authentication tag
    std::vector<uint8_t> auth_tag(SHA256_DIGEST_LENGTH);
    
    HMAC_CTX* ctx = HMAC_CTX_new();
    HMAC_Init_ex(ctx, shared_key.data(), shared_key.size(), EVP_sha256(), nullptr);
    HMAC_Update(ctx, vector_bytes.data(), vector_bytes.size());
    unsigned int len = 0;
    HMAC_Final(ctx, auth_tag.data(), &len);
    HMAC_CTX_free(ctx);
    
    return auth_tag;
}

bool VectorCrypto::verify_authentication(const LayeredBigIntVector& vector,
                                       const std::vector<uint8_t>& auth_tag,
                                       const std::vector<uint8_t>& shared_key) {
    // Generate a new authentication tag
    std::vector<uint8_t> computed_tag = authenticate_vector(vector, shared_key);
    
    // Compare with the provided tag
    if (computed_tag.size() != auth_tag.size()) {
        return false;
    }
    
    // Constant-time comparison to prevent timing attacks
    unsigned char result = 0;
    for (size_t i = 0; i < computed_tag.size(); i++) {
        result |= computed_tag[i] ^ auth_tag[i];
    }
    
    return result == 0;
}

std::vector<uint8_t> VectorCrypto::vector_to_bytes(const LayeredBigIntVector& vector) {
    // Use the vector's serialization method
    return vector.serialize();
}

LayeredBigIntVector VectorCrypto::bytes_to_vector(const std::vector<uint8_t>& data) {
    // Use the vector's deserialization method
    return LayeredBigIntVector::deserialize(data);
}

} // namespace security
} // namespace lmvs
