#pragma once

#include "lmvs/layered_bigint_vector.hpp"
#include "hydra_crypto/kyber_kem.hpp"
#include "hydra_crypto/kyber_aes.hpp"
#include "hydra_crypto/falcon_signature.hpp"
#include <vector>
#include <string>
#include <memory>
#include <unordered_map>

namespace lmvs {
namespace security {

/**
 * @brief Class for securing layered vectors using post-quantum cryptography
 *
 * This class provides methods for encrypting, authenticating, and signing
 * layered vectors using Kyber KEM, Kyber AES, and Falcon signatures.
 */
class VectorCrypto {
public:
    /**
     * @brief Construct a new Vector Crypto object
     *
     * @param kyber_mode Kyber security mode (Kyber512, Kyber768, or Kyber1024)
     * @param falcon_degree Falcon degree parameter (512 or 1024)
     */
    VectorCrypto(const std::string& kyber_mode = "Kyber768", int falcon_degree = 512);

    /**
     * @brief Generate a new key pair for encryption
     *
     * @return std::pair<std::vector<uint8_t>, std::vector<uint8_t>> {public_key, private_key}
     */
    std::pair<std::vector<uint8_t>, std::vector<uint8_t>> generate_encryption_keypair();

    /**
     * @brief Generate a new key pair for signing
     *
     * @return std::pair<std::vector<uint8_t>, std::vector<uint8_t>> {public_key, private_key}
     */
    std::pair<std::vector<uint8_t>, std::vector<uint8_t>> generate_signing_keypair();

    /**
     * @brief Encrypt a layered vector
     *
     * @param vector The vector to encrypt
     * @param public_key The recipient's public key
     * @return std::vector<uint8_t> Encrypted vector data
     */
    std::vector<uint8_t> encrypt_vector(const LayeredBigIntVector& vector,
                                       const std::vector<uint8_t>& public_key);

    /**
     * @brief Decrypt an encrypted vector
     *
     * @param encrypted_data The encrypted vector data
     * @param private_key The recipient's private key
     * @return LayeredBigIntVector The decrypted vector
     */
    LayeredBigIntVector decrypt_vector(const std::vector<uint8_t>& encrypted_data,
                                      const std::vector<uint8_t>& private_key);

    /**
     * @brief Sign a layered vector
     *
     * @param vector The vector to sign
     * @param private_key The signer's private key
     * @return std::vector<uint8_t> Signature
     */
    std::vector<uint8_t> sign_vector(const LayeredBigIntVector& vector,
                                    const std::vector<uint8_t>& private_key);

    /**
     * @brief Verify a vector signature
     *
     * @param vector The vector to verify
     * @param signature The signature to verify
     * @param public_key The signer's public key
     * @return bool True if the signature is valid
     */
    bool verify_vector(const LayeredBigIntVector& vector,
                      const std::vector<uint8_t>& signature,
                      const std::vector<uint8_t>& public_key);

    /**
     * @brief Encrypt and sign a layered vector
     *
     * @param vector The vector to encrypt and sign
     * @param encryption_public_key The recipient's public key
     * @param signing_private_key The signer's private key
     * @return std::pair<std::vector<uint8_t>, std::vector<uint8_t>> {encrypted_data, signature}
     */
    std::pair<std::vector<uint8_t>, std::vector<uint8_t>> encrypt_and_sign(
        const LayeredBigIntVector& vector,
        const std::vector<uint8_t>& encryption_public_key,
        const std::vector<uint8_t>& signing_private_key);

    /**
     * @brief Decrypt and verify a layered vector
     *
     * @param encrypted_data The encrypted vector data
     * @param signature The signature to verify
     * @param encryption_private_key The recipient's private key
     * @param signing_public_key The signer's public key
     * @return std::pair<LayeredBigIntVector, bool> {decrypted_vector, is_valid}
     */
    std::pair<LayeredBigIntVector, bool> decrypt_and_verify(
        const std::vector<uint8_t>& encrypted_data,
        const std::vector<uint8_t>& signature,
        const std::vector<uint8_t>& encryption_private_key,
        const std::vector<uint8_t>& signing_public_key);

    /**
     * @brief Authenticate a vector using a shared key
     *
     * @param vector The vector to authenticate
     * @param shared_key The shared key for authentication
     * @return std::vector<uint8_t> Authentication tag
     */
    std::vector<uint8_t> authenticate_vector(const LayeredBigIntVector& vector,
                                           const std::vector<uint8_t>& shared_key);

    /**
     * @brief Verify a vector's authentication tag
     *
     * @param vector The vector to verify
     * @param auth_tag The authentication tag
     * @param shared_key The shared key for authentication
     * @return bool True if the authentication tag is valid
     */
    bool verify_authentication(const LayeredBigIntVector& vector,
                              const std::vector<uint8_t>& auth_tag,
                              const std::vector<uint8_t>& shared_key);

    // Allow SecureVectorTransport to access private members
    friend class SecureVectorTransport;

private:
    std::unique_ptr<hydra::crypto::KyberKEM> m_kyber_kem;
    std::unique_ptr<hydra::crypto::KyberAES> m_kyber_aes;
    std::unique_ptr<hydra::crypto::FalconSignature> m_falcon;

    // Helper methods
    std::vector<uint8_t> vector_to_bytes(const LayeredBigIntVector& vector);
    LayeredBigIntVector bytes_to_vector(const std::vector<uint8_t>& data);
};

} // namespace security
} // namespace lmvs
