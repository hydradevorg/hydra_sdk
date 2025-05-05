#ifndef HYDRA_CRYPTO_ROOT_KEY_MANAGER_HPP
#define HYDRA_CRYPTO_ROOT_KEY_MANAGER_HPP

#include <string>
#include <vector>
#include <memory>
#include <chrono>
#include <botan/auto_rng.h>
#include <botan/secmem.h>
#include <hydra_crypto/dilithium_signature.hpp>
#include <hydra_crypto/kyber_kem.hpp>

namespace hydra {
namespace crypto {

/**
 * @class RootKeyManager
 * @brief Manages root keys for the Hydra Edge KMS
 * 
 * This class is responsible for generating, storing, and rotating root keys.
 * Root keys are used to sign and encrypt public keys to ensure their authenticity
 * and confidentiality.
 */
class RootKeyManager {
public:
    /**
     * @struct RootKeyPair
     * @brief Represents a root key pair with metadata
     */
    struct RootKeyPair {
        std::string id;                     // Unique identifier for the key pair
        std::string dilithium_private_key;  // Base64 encoded Dilithium private key
        std::string dilithium_public_key;   // Base64 encoded Dilithium public key
        std::string kyber_private_key;      // Base64 encoded Kyber private key
        std::string kyber_public_key;       // Base64 encoded Kyber public key
        std::string strength;               // Strength of the Dilithium key
        std::chrono::system_clock::time_point created_at;  // Creation timestamp
        std::chrono::system_clock::time_point expires_at;  // Expiration timestamp
        bool is_active;                     // Whether the key is currently active
    };

    /**
     * @brief Constructor
     * @param rotation_interval_days Number of days before keys are rotated
     */
    explicit RootKeyManager(int rotation_interval_days = 90);

    /**
     * @brief Generate a new root key pair
     * @param strength Strength of the Dilithium key
     * @return Unique identifier for the generated key pair
     */
    std::string generateRootKeyPair(const std::string& strength = "ML-DSA-65");

    /**
     * @brief Get the active root key pair
     * @return The active root key pair
     */
    RootKeyPair getActiveRootKeyPair() const;

    /**
     * @brief Get a root key pair by ID
     * @param id Unique identifier for the key pair
     * @return The root key pair with the given ID
     */
    RootKeyPair getRootKeyPairById(const std::string& id) const;

    /**
     * @brief List all root key pairs
     * @return Vector of all root key pairs
     */
    std::vector<RootKeyPair> listRootKeyPairs() const;

    /**
     * @brief Rotate the active root key pair
     * @param force Force rotation even if the current key hasn't expired
     * @return ID of the new active key pair
     */
    std::string rotateRootKeyPair(bool force = false);

    /**
     * @brief Sign a public key using the active root key
     * @param public_key Base64 encoded public key to sign
     * @return Base64 encoded signature
     */
    std::string signPublicKey(const std::string& public_key) const;

    /**
     * @brief Verify a signed public key
     * @param public_key Base64 encoded public key
     * @param signature Base64 encoded signature
     * @param root_key_id ID of the root key used for signing (if empty, uses active key)
     * @return True if the signature is valid, false otherwise
     */
    bool verifyPublicKey(const std::string& public_key, const std::string& signature, 
                         const std::string& root_key_id = "") const;

    /**
     * @brief Encrypt a public key using the active root key
     * @param public_key Base64 encoded public key to encrypt
     * @return Base64 encoded encrypted public key
     */
    std::string encryptPublicKey(const std::string& public_key) const;

    /**
     * @brief Decrypt an encrypted public key
     * @param encrypted_public_key Base64 encoded encrypted public key
     * @param root_key_id ID of the root key used for encryption (if empty, uses active key)
     * @return Base64 encoded decrypted public key
     */
    std::string decryptPublicKey(const std::string& encrypted_public_key, 
                                const std::string& root_key_id = "") const;

    /**
     * @brief Check if any keys need rotation
     * @return True if keys need rotation, false otherwise
     */
    bool needsRotation() const;

private:
    std::vector<RootKeyPair> root_key_pairs_;
    int rotation_interval_days_;
    std::unique_ptr<Botan::AutoSeeded_RNG> rng_;

    /**
     * @brief Generate a unique ID for a key pair
     * @return Unique ID string
     */
    std::string generateUniqueId() const;

    /**
     * @brief Find a root key pair by ID
     * @param id Unique identifier for the key pair
     * @return Iterator to the root key pair, or end if not found
     */
    std::vector<RootKeyPair>::const_iterator findRootKeyPairById(const std::string& id) const;

    /**
     * @brief Find the active root key pair
     * @return Iterator to the active root key pair, or end if not found
     */
    std::vector<RootKeyPair>::const_iterator findActiveRootKeyPair() const;
};

} // namespace crypto
} // namespace hydra

#endif // HYDRA_CRYPTO_ROOT_KEY_MANAGER_HPP
