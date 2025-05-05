#pragma once
#include <vector>
#include <string>
#include <cstdint>
#include <memory>

namespace hydra {
namespace crypto {

/**
 * @brief KyberKEM key encapsulation mechanism
 * 
 * This class implements the Kyber key encapsulation mechanism for post-quantum
 * secure key exchange.
 * 
 * Example usage:
 * ```
 * // Generate a keypair
 * KyberKEM kem("Kyber768");
 * auto [public_key, private_key] = kem.generate_keypair();
 * 
 * // Encapsulate a shared key
 * auto [ciphertext, shared_key] = kem.encapsulate(public_key);
 * 
 * // Decapsulate the shared key
 * auto decapsulated_key = kem.decapsulate(ciphertext, private_key);
 * // shared_key == decapsulated_key
 * ```
 */
class KyberKEM {
public:
    /**
     * @brief Constructs a KyberKEM object
     * @param mode Security mode to use (Kyber512, Kyber768, Kyber1024, Kyber512-90s, Kyber768-90s, or Kyber1024-90s)
     */
    explicit KyberKEM(const std::string& mode = "Kyber768");
    ~KyberKEM();

    // Delete copy/move
    KyberKEM(const KyberKEM&) = delete;
    KyberKEM& operator=(const KyberKEM&) = delete;
    KyberKEM(KyberKEM&&) = delete;
    KyberKEM& operator=(KyberKEM&&) = delete;

    /**
     * Generate a new Kyber key pair
     * @return A pair containing {public_key, private_key} as raw byte vectors
     */
    std::pair<std::vector<uint8_t>, std::vector<uint8_t>> generate_keypair();
    
    /**
     * Encapsulate a shared key using a public key
     * @param public_key The public key to use for encapsulation
     * @return A pair containing {ciphertext, shared_key}
     */
    std::pair<std::vector<uint8_t>, std::vector<uint8_t>> encapsulate(
        const std::vector<uint8_t>& public_key);
    
    /**
     * Decapsulate a shared key using a private key and ciphertext
     * @param ciphertext The ciphertext from encapsulation
     * @param private_key The private key to use for decapsulation
     * @return The decapsulated shared key
     */
    std::vector<uint8_t> decapsulate(
        const std::vector<uint8_t>& ciphertext,
        const std::vector<uint8_t>& private_key);

    /**
     * @return The Kyber mode being used (e.g., "Kyber768")
     */
    std::string get_mode() const;

    /**
     * @return Size in bytes of a public key
     */
    size_t get_public_key_size() const;
    
    /**
     * @return Size in bytes of a private key
     */
    size_t get_private_key_size() const;
    
    /**
     * @return Size in bytes of a ciphertext
     */
    size_t get_ciphertext_size() const;
    
    /**
     * @return Size in bytes of a shared key
     */
    size_t get_shared_key_size() const;

    /**
     * Run a demonstration of the KyberKEM functionality
     */
    static void demo();

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace crypto
} // namespace hydra
