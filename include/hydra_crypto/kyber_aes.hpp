#ifndef HYDRA_EDGE_KMS_KYBER_AES_HPP
#define HYDRA_EDGE_KMS_KYBER_AES_HPP

#include <string>
#include <vector>
#include <memory>
#include <botan/auto_rng.h>
#include <botan/secmem.h>
#include <hydra_crypto/kyber_kem.hpp>

namespace hydra {
namespace crypto {

/**
 * @brief KyberAES hybrid encryption mechanism
 * 
 * This class implements a hybrid encryption scheme using Kyber for key exchange
 * and AES-GCM for authenticated encryption of data.
 * 
 * Example usage:
 * ```
 * // Generate a keypair
 * KyberAES cryptor("Kyber768");
 * auto [public_key, private_key] = cryptor.generate_keypair();
 * 
 * // Encrypt data
 * std::string message = "Secret message";
 * auto ciphertext = cryptor.encrypt(message, public_key);
 * 
 * // Decrypt data
 * auto decrypted = cryptor.decrypt(ciphertext, private_key);
 * // decrypted == message
 * ```
 */
class KyberAES {
public:
    /**
     * @brief Constructs a KyberAES object
     * @param kyber_mode Security mode to use (Kyber512, Kyber768, or Kyber1024)
     */
    explicit KyberAES(const std::string& kyber_mode = "Kyber768");
    ~KyberAES();

    // Delete copy/move
    KyberAES(const KyberAES&) = delete;
    KyberAES& operator=(const KyberAES&) = delete;
    KyberAES(KyberAES&&) = delete;
    KyberAES& operator=(KyberAES&&) = delete;

    /**
     * Generate a new key pair
     * @return A pair containing {public_key, private_key} as raw byte vectors
     */
    std::pair<std::vector<uint8_t>, std::vector<uint8_t>> generate_keypair();
    
    /**
     * Encrypt data using a public key
     * 
     * @param data The data to encrypt
     * @param public_key The public key to use for encryption
     * @return The encrypted data (encapsulated key + AES ciphertext)
     */
    std::vector<uint8_t> encrypt(const std::vector<uint8_t>& data, 
                                 const std::vector<uint8_t>& public_key);
    
    /**
     * Encrypt string data using a public key
     * 
     * @param data The string data to encrypt
     * @param public_key The public key to use for encryption
     * @return The encrypted data (encapsulated key + AES ciphertext)
     */
    std::vector<uint8_t> encrypt(const std::string& data, 
                               const std::vector<uint8_t>& public_key);
    
    /**
     * Decrypt data using a private key
     * 
     * @param ciphertext The encrypted data
     * @param private_key The private key to use for decryption
     * @return The decrypted data
     */
    std::vector<uint8_t> decrypt(const std::vector<uint8_t>& ciphertext,
                               const std::vector<uint8_t>& private_key);
    
    /**
     * Decrypt data to string using a private key
     * 
     * @param ciphertext The encrypted data
     * @param private_key The private key to use for decryption
     * @return The decrypted data as a string
     */
    std::string decrypt_to_string(const std::vector<uint8_t>& ciphertext,
                                const std::vector<uint8_t>& private_key);

    /**
     * @return Size in bytes of a public key
     */
    size_t get_public_key_size() const;
    
    /**
     * @return Size in bytes of a private key
     */
    size_t get_private_key_size() const;
    
    /**
     * @return Minimum overhead size in bytes for encryption
     */
    size_t get_encryption_overhead() const;
    
    /**
     * @return Name of the algorithm (e.g., "Kyber768-AES256-GCM")
     */
    std::string get_name() const;
    
    /**
     * @return Security level in bits
     */
    int get_security_level() const;

    /**
     * Run a demonstration of KyberAES operations
     */
    static void demo();

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace crypto
} // namespace hydra

#endif // HYDRA_EDGE_KMS_KYBER_AES_HPP