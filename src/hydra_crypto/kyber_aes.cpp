#include <hydra_crypto/kyber_aes.hpp>
#include <hydra_crypto/kyber_kem.hpp>

#include <botan/auto_rng.h>
#include <botan/aead.h>
#include <botan/base64.h>
#include <botan/block_cipher.h>
#include <botan/cipher_mode.h>
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <iomanip>

namespace hydra {
namespace crypto {

class KyberAES::Impl {
public:
    Impl(const std::string& kyber_mode) : 
        m_kyber_mode(kyber_mode),
        m_rng(std::make_unique<Botan::AutoSeeded_RNG>()) {
        // Create Kyber KEM instance
        m_kem = std::make_unique<KyberKEM>(kyber_mode);
    }

    std::pair<std::vector<uint8_t>, std::vector<uint8_t>> generate_keypair() {
        try {
            return m_kem->generate_keypair();
        } catch (const std::exception& e) {
            throw std::runtime_error(std::string("Key generation failed: ") + e.what());
        }
    }

    std::vector<uint8_t> encrypt(const std::vector<uint8_t>& data, 
                                const std::vector<uint8_t>& public_key) {
        try {
            // 1. Encapsulate a shared secret using Kyber
            auto [kyber_ciphertext, shared_secret] = m_kem->encapsulate(public_key);
            
            // 2. Create AES-GCM encryption object
            auto aes_gcm = Botan::Cipher_Mode::create("AES-256/GCM", Botan::Cipher_Dir::Encryption);
            if (!aes_gcm) {
                throw std::runtime_error("Failed to create AES-GCM encryption object");
            }
            
            // 3. Generate a random nonce/IV
            Botan::secure_vector<uint8_t> nonce_secure = m_rng->random_vec(12); // 96 bits for GCM
            std::vector<uint8_t> nonce(nonce_secure.begin(), nonce_secure.end());
            
            // 4. Set key and nonce
            aes_gcm->set_key(shared_secret);
            aes_gcm->start(nonce);
            
            // 5. Encrypt the data
            Botan::secure_vector<uint8_t> encrypted_data(data.begin(), data.end());
            aes_gcm->finish(encrypted_data);
            
            // 6. Construct the final ciphertext format:
            // [kyber_ciphertext_size(4 bytes)][kyber_ciphertext][nonce(12 bytes)][encrypted_data]
            std::vector<uint8_t> result;
            
            // Add Kyber ciphertext size (4 bytes, big-endian)
            uint32_t kyber_size = static_cast<uint32_t>(kyber_ciphertext.size());
            result.push_back(static_cast<uint8_t>((kyber_size >> 24) & 0xFF));
            result.push_back(static_cast<uint8_t>((kyber_size >> 16) & 0xFF));
            result.push_back(static_cast<uint8_t>((kyber_size >> 8) & 0xFF));
            result.push_back(static_cast<uint8_t>(kyber_size & 0xFF));
            
            // Add Kyber ciphertext
            result.insert(result.end(), kyber_ciphertext.begin(), kyber_ciphertext.end());
            
            // Add nonce
            result.insert(result.end(), nonce.begin(), nonce.end());
            
            // Add encrypted data
            result.insert(result.end(), encrypted_data.begin(), encrypted_data.end());
            
            return result;
        } catch (const std::exception& e) {
            throw std::runtime_error(std::string("Encryption failed: ") + e.what());
        }
    }
    
    std::vector<uint8_t> encrypt(const std::string& data, 
                                const std::vector<uint8_t>& public_key) {
        return encrypt(std::vector<uint8_t>(data.begin(), data.end()), public_key);
    }
    
    std::vector<uint8_t> decrypt(const std::vector<uint8_t>& ciphertext,
                                const std::vector<uint8_t>& private_key) {
        try {
            // 1. Parse the ciphertext format:
            // [kyber_ciphertext_size(4 bytes)][kyber_ciphertext][nonce(12 bytes)][encrypted_data]
            if (ciphertext.size() < 4) {
                throw std::runtime_error("Invalid ciphertext: too short");
            }
            
            // Extract Kyber ciphertext size (4 bytes, big-endian)
            uint32_t kyber_size = 
                (static_cast<uint32_t>(ciphertext[0]) << 24) |
                (static_cast<uint32_t>(ciphertext[1]) << 16) |
                (static_cast<uint32_t>(ciphertext[2]) << 8) |
                static_cast<uint32_t>(ciphertext[3]);
            
            // Validate sizes
            if (ciphertext.size() < 4 + kyber_size + 12) {
                throw std::runtime_error("Invalid ciphertext: too short for header and nonce");
            }
            
            // Extract Kyber ciphertext
            std::vector<uint8_t> kyber_ciphertext(
                ciphertext.begin() + 4,
                ciphertext.begin() + 4 + kyber_size
            );
            
            // Extract nonce
            std::vector<uint8_t> nonce(
                ciphertext.begin() + 4 + kyber_size,
                ciphertext.begin() + 4 + kyber_size + 12
            );
            
            // Extract encrypted data
            Botan::secure_vector<uint8_t> encrypted_data(
                ciphertext.begin() + 4 + kyber_size + 12,
                ciphertext.end()
            );
            
            // 2. Decapsulate the shared secret using Kyber
            std::vector<uint8_t> shared_secret = m_kem->decapsulate(kyber_ciphertext, private_key);
            
            // 3. Create AES-GCM decryption object
            auto aes_gcm = Botan::Cipher_Mode::create("AES-256/GCM", Botan::Cipher_Dir::Decryption);
            if (!aes_gcm) {
                throw std::runtime_error("Failed to create AES-GCM decryption object");
            }
            
            // 4. Set key and nonce
            aes_gcm->set_key(shared_secret);
            aes_gcm->start(nonce);
            
            // 5. Decrypt the data
            aes_gcm->finish(encrypted_data);
            
            // 6. Return the decrypted data
            return std::vector<uint8_t>(encrypted_data.begin(), encrypted_data.end());
        } catch (const std::exception& e) {
            throw std::runtime_error(std::string("Decryption failed: ") + e.what());
        }
    }
    
    std::string decrypt_to_string(const std::vector<uint8_t>& ciphertext,
                                 const std::vector<uint8_t>& private_key) {
        auto decrypted = decrypt(ciphertext, private_key);
        return std::string(decrypted.begin(), decrypted.end());
    }
    
    size_t get_public_key_size() const {
        return m_kem->get_public_key_size();
    }
    
    size_t get_private_key_size() const {
        return m_kem->get_private_key_size();
    }
    
    size_t get_encryption_overhead() const {
        // 4 bytes for Kyber ciphertext size + Kyber ciphertext size + 12 bytes for nonce + 16 bytes for GCM tag
        return 4 + m_kem->get_ciphertext_size() + 12 + 16;
    }
    
    std::string get_name() const {
        return "Kyber-AES-GCM-" + m_kyber_mode;
    }
    
    int get_security_level() const {
        if (m_kyber_mode == "Kyber512" || m_kyber_mode == "Kyber512-90s") {
            return 1;
        } else if (m_kyber_mode == "Kyber768" || m_kyber_mode == "Kyber768-90s") {
            return 3;
        } else if (m_kyber_mode == "Kyber1024" || m_kyber_mode == "Kyber1024-90s") {
            return 5;
        } else {
            return 0;
        }
    }
    
private:
    std::string m_kyber_mode;
    std::unique_ptr<KyberKEM> m_kem;
    std::unique_ptr<Botan::RandomNumberGenerator> m_rng;
};

// Implementation of the public class methods

KyberAES::KyberAES(const std::string& kyber_mode)
    : impl_(std::make_unique<Impl>(kyber_mode)) {
}

KyberAES::~KyberAES() = default;

std::pair<std::vector<uint8_t>, std::vector<uint8_t>> KyberAES::generate_keypair() {
    return impl_->generate_keypair();
}

std::vector<uint8_t> KyberAES::encrypt(const std::vector<uint8_t>& data, 
                                      const std::vector<uint8_t>& public_key) {
    return impl_->encrypt(data, public_key);
}

std::vector<uint8_t> KyberAES::encrypt(const std::string& data, 
                                     const std::vector<uint8_t>& public_key) {
    return impl_->encrypt(data, public_key);
}

std::vector<uint8_t> KyberAES::decrypt(const std::vector<uint8_t>& ciphertext,
                                     const std::vector<uint8_t>& private_key) {
    return impl_->decrypt(ciphertext, private_key);
}

std::string KyberAES::decrypt_to_string(const std::vector<uint8_t>& ciphertext,
                                      const std::vector<uint8_t>& private_key) {
    return impl_->decrypt_to_string(ciphertext, private_key);
}

size_t KyberAES::get_public_key_size() const {
    return impl_->get_public_key_size();
}

size_t KyberAES::get_private_key_size() const {
    return impl_->get_private_key_size();
}

size_t KyberAES::get_encryption_overhead() const {
    return impl_->get_encryption_overhead();
}

std::string KyberAES::get_name() const {
    return impl_->get_name();
}

int KyberAES::get_security_level() const {
    return impl_->get_security_level();
}

void KyberAES::demo() {
    std::cout << "============================================" << std::endl;
    std::cout << "Kyber-AES Demo" << std::endl;
    std::cout << "============================================" << std::endl;
    
    try {
        // Create a Kyber-AES instance with Kyber-768
        KyberAES kyber_aes("Kyber768");
        std::cout << "Created " << kyber_aes.get_name() << " instance" << std::endl;
        std::cout << "Security level: " << kyber_aes.get_security_level() << std::endl;
        
        // Generate a key pair
        std::cout << "Generating key pair..." << std::endl;
        auto [public_key, private_key] = kyber_aes.generate_keypair();
        
        std::cout << "Public key size: " << public_key.size() << " bytes" << std::endl;
        std::cout << "Private key size: " << private_key.size() << " bytes" << std::endl;
        
        // Encrypt a message
        std::string message = "This is a secret message that needs to be encrypted securely.";
        std::cout << "Original message: " << message << std::endl;
        std::cout << "Original message size: " << message.size() << " bytes" << std::endl;
        
        std::cout << "Encrypting message..." << std::endl;
        auto ciphertext = kyber_aes.encrypt(message, public_key);
        
        std::cout << "Ciphertext size: " << ciphertext.size() << " bytes" << std::endl;
        std::cout << "Encryption overhead: " << kyber_aes.get_encryption_overhead() << " bytes" << std::endl;
        
        // Decrypt the message
        std::cout << "Decrypting message..." << std::endl;
        auto decrypted = kyber_aes.decrypt_to_string(ciphertext, private_key);
        
        std::cout << "Decrypted message: " << decrypted << std::endl;
        std::cout << "Decryption successful: " << (message == decrypted ? "YES" : "NO") << std::endl;
        
        std::cout << "============================================" << std::endl;
        std::cout << "Demo completed successfully" << std::endl;
        std::cout << "============================================" << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "ERROR: " << e.what() << std::endl;
    }
}

} // namespace crypto
} // namespace hydra
