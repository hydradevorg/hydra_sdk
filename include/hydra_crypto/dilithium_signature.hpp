#pragma once

#include <string>
#include <vector>
#include <memory>

namespace hydra {
namespace crypto {

/**
 * @brief DilithiumSignature (ML-DSA) digital signature algorithm
 * 
 * This class implements the Dilithium digital signature algorithm (now standardized as ML-DSA),
 * providing secure post-quantum signatures.
 * 
 * Example usage:
 * ```
 * // Generate a key pair
 * DilithiumSignature signer;
 * signer.generate_key_pair();
 * 
 * // Sign a message
 * std::string message = "Hello, post-quantum world!";
 * auto signature = signer.sign_message(message);
 * 
 * // Verify the signature
 * bool is_valid = signer.verify_signature(message, signature);
 * ```
 */
class DilithiumSignature {
public:
    /**
     * @brief Constructs a DilithiumSignature object
     * @param strength Security strength to use (ML-DSA-44, ML-DSA-65, or ML-DSA-87)
     */
    explicit DilithiumSignature(const std::string& strength = "ML-DSA-65");
    ~DilithiumSignature();

    // Delete copy/move
    DilithiumSignature(const DilithiumSignature&) = delete;
    DilithiumSignature& operator=(const DilithiumSignature&) = delete;
    DilithiumSignature(DilithiumSignature&&) = delete;
    DilithiumSignature& operator=(DilithiumSignature&&) = delete;

    /**
     * Generate a new ML-DSA key pair
     * @return A pair containing {public_key, private_key} as raw byte vectors
     */
    std::pair<std::vector<uint8_t>, std::vector<uint8_t>> generate_key_pair();
    
    /**
     * @return The current public key
     */
    std::vector<uint8_t> get_public_key() const;
    
    /**
     * @return The current private key
     * @note Take care to protect this data
     */
    std::vector<uint8_t> get_private_key() const;

    /**
     * Sign a message using the private key
     * 
     * @param message The message to sign
     * @return The signature as a byte vector
     */
    std::vector<uint8_t> sign_message(const std::vector<uint8_t>& message);
    
    /**
     * Overload to sign a string message
     */
    std::vector<uint8_t> sign_message(const std::string& message);

    /**
     * Verify a signature using the public key
     * 
     * @param message The original message
     * @param signature The signature to verify
     * @return True if the signature is valid, false otherwise
     */
    bool verify_signature(const std::vector<uint8_t>& message, const std::vector<uint8_t>& signature);
    
    /**
     * Overload to verify a string message
     */
    bool verify_signature(const std::string& message, const std::vector<uint8_t>& signature);

    /**
     * Set the public key from external data
     * 
     * @param key The public key as a byte vector
     */
    void set_public_key(const std::vector<uint8_t>& key);
    
    /**
     * Set the private key from external data
     * 
     * @param key The private key as a byte vector
     */
    void set_private_key(const std::vector<uint8_t>& key);

    // Parameters and info
    /**
     * @return Size in bytes of a public key
     */
    size_t get_public_key_size() const;
    
    /**
     * @return Size in bytes of a private key
     */
    size_t get_private_key_size() const;
    
    /**
     * @return Size in bytes of a signature
     */
    size_t get_signature_size() const;
    
    /**
     * @return Name of the algorithm (e.g., "ML-DSA-65")
     */
    std::string get_name() const;
    
    /**
     * @return Security level in bits
     */
    int get_security_level() const;

    /**
     * Run a demonstration of ML-DSA operations
     * Generates keys, signs, and verifies a message
     */
    static void demo();

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace crypto
} // namespace hydra