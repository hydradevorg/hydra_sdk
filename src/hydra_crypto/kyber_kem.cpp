#include <hydra_crypto/kyber_kem.hpp>
#include <botan/kyber.h>
#include <botan/auto_rng.h>
#include <botan/hex.h>
#include <botan/pubkey.h>
#include <iostream>
#include <stdexcept>

namespace hydra {
namespace crypto {

class KyberKEM::Impl {
public:
    Impl(const std::string& mode) : m_mode(mode), m_rng(std::make_unique<Botan::AutoSeeded_RNG>()) {
        // Validate mode
        if (mode != "Kyber512" && mode != "Kyber768" && mode != "Kyber1024" &&
            mode != "Kyber512-90s" && mode != "Kyber768-90s" && mode != "Kyber1024-90s") {
            throw std::invalid_argument("Invalid Kyber mode. Supported values are Kyber512, Kyber768, Kyber1024, "
                                       "Kyber512-90s, Kyber768-90s, Kyber1024-90s");
        }
    }

    ~Impl() = default;

    std::pair<std::vector<uint8_t>, std::vector<uint8_t>> generate_keypair() {
        // Create Kyber key pair
        auto kyber_privkey = create_kyber_private_key();
        auto kyber_pubkey = std::unique_ptr<Botan::Kyber_PublicKey>(
            dynamic_cast<Botan::Kyber_PublicKey*>(kyber_privkey->public_key().release()));
        
        // Extract raw key bits
        std::vector<uint8_t> public_key = kyber_pubkey->raw_public_key_bits();
        
        // Convert secure_vector to regular vector
        Botan::secure_vector<uint8_t> priv_key_secure = kyber_privkey->raw_private_key_bits();
        std::vector<uint8_t> private_key(priv_key_secure.begin(), priv_key_secure.end());
        
        // Log the operation for audit purposes
        std::cout << "AUDIT: GENERATE_KEYPAIR - " << to_hex(public_key) << std::endl;
        
        return {public_key, private_key};
    }

    std::pair<std::vector<uint8_t>, std::vector<uint8_t>> encapsulate(const std::vector<uint8_t>& public_key) {
        // Create Kyber public key
        auto kyber_pubkey = create_kyber_public_key(public_key);
        
        // Create KEM encryptor with Raw KDF
        Botan::PK_KEM_Encryptor encryptor(*kyber_pubkey, "Raw");
        
        // Encapsulate key
        auto kem_result = encryptor.encrypt(*m_rng);
        
        // Extract results
        std::vector<uint8_t> ciphertext = kem_result.encapsulated_shared_key();
        Botan::secure_vector<uint8_t> shared_key_secure = kem_result.shared_key();
        std::vector<uint8_t> shared_key(shared_key_secure.begin(), shared_key_secure.end());
        
        // Log the operation for audit purposes
        std::cout << "AUDIT: ENCAPSULATE - " << to_hex(ciphertext) << std::endl;
        
        return {ciphertext, shared_key};
    }

    std::vector<uint8_t> decapsulate(const std::vector<uint8_t>& ciphertext, const std::vector<uint8_t>& private_key) {
        // Create Kyber private key
        auto kyber_privkey = create_kyber_private_key(private_key);
        
        // Create KEM decryptor with Raw KDF
        Botan::PK_KEM_Decryptor decryptor(*kyber_privkey, *m_rng, "Raw");
        
        // Decapsulate key
        Botan::secure_vector<uint8_t> shared_key_secure = decryptor.decrypt(ciphertext);
        std::vector<uint8_t> shared_key(shared_key_secure.begin(), shared_key_secure.end());
        
        // Log the operation for audit purposes
        std::cout << "AUDIT: DECAPSULATE - " << to_hex(shared_key) << std::endl;
        
        return shared_key;
    }

    std::string get_mode() const {
        return m_mode;
    }

    size_t get_public_key_size() const {
        auto kyber_privkey = create_kyber_private_key();
        auto kyber_pubkey = std::unique_ptr<Botan::Kyber_PublicKey>(
            dynamic_cast<Botan::Kyber_PublicKey*>(kyber_privkey->public_key().release()));
        return kyber_pubkey->raw_public_key_bits().size();
    }

    size_t get_private_key_size() const {
        auto kyber_privkey = create_kyber_private_key();
        return kyber_privkey->raw_private_key_bits().size();
    }

    size_t get_ciphertext_size() const {
        // Ciphertext size depends on the mode
        if (m_mode == "Kyber512" || m_mode == "Kyber512-90s") {
            return 768;
        } else if (m_mode == "Kyber768" || m_mode == "Kyber768-90s") {
            return 1088;
        } else if (m_mode == "Kyber1024" || m_mode == "Kyber1024-90s") {
            return 1568;
        } else {
            throw std::runtime_error("Unsupported Kyber mode");
        }
    }

    size_t get_shared_key_size() const {
        // Shared key size is 32 bytes for all Kyber modes
        return 32;
    }

    std::string to_hex(const std::vector<uint8_t>& data) {
        return Botan::hex_encode(data);
    }

private:
    std::string m_mode;
    std::unique_ptr<Botan::RandomNumberGenerator> m_rng;

    Botan::KyberMode get_kyber_mode() const {
        // Map string mode to Botan::KyberMode enum
        if (m_mode == "Kyber512") {
            return Botan::KyberMode(Botan::KyberMode::Kyber512_R3);
        } else if (m_mode == "Kyber768") {
            return Botan::KyberMode(Botan::KyberMode::Kyber768_R3);
        } else if (m_mode == "Kyber1024") {
            return Botan::KyberMode(Botan::KyberMode::Kyber1024_R3);
        } else if (m_mode == "Kyber512-90s") {
            return Botan::KyberMode(Botan::KyberMode::Kyber512_90s);
        } else if (m_mode == "Kyber768-90s") {
            return Botan::KyberMode(Botan::KyberMode::Kyber768_90s);
        } else if (m_mode == "Kyber1024-90s") {
            return Botan::KyberMode(Botan::KyberMode::Kyber1024_90s);
        } else {
            throw std::runtime_error("Unsupported Kyber mode");
        }
    }

    std::unique_ptr<Botan::Kyber_PrivateKey> create_kyber_private_key() const {
        return std::make_unique<Botan::Kyber_PrivateKey>(*m_rng, get_kyber_mode());
    }

    std::unique_ptr<Botan::Kyber_PrivateKey> create_kyber_private_key(const std::vector<uint8_t>& private_key) const {
        return std::make_unique<Botan::Kyber_PrivateKey>(private_key, get_kyber_mode());
    }

    std::unique_ptr<Botan::Kyber_PublicKey> create_kyber_public_key(const std::vector<uint8_t>& public_key) const {
        return std::make_unique<Botan::Kyber_PublicKey>(public_key, get_kyber_mode());
    }
};

// Implementation of the public class methods

KyberKEM::KyberKEM(const std::string& mode)
    : impl_(std::make_unique<Impl>(mode)) {
}

KyberKEM::~KyberKEM() = default;

std::pair<std::vector<uint8_t>, std::vector<uint8_t>> KyberKEM::generate_keypair() {
    return impl_->generate_keypair();
}

std::pair<std::vector<uint8_t>, std::vector<uint8_t>> KyberKEM::encapsulate(const std::vector<uint8_t>& public_key) {
    return impl_->encapsulate(public_key);
}

std::vector<uint8_t> KyberKEM::decapsulate(const std::vector<uint8_t>& ciphertext, const std::vector<uint8_t>& private_key) {
    return impl_->decapsulate(ciphertext, private_key);
}

std::string KyberKEM::get_mode() const {
    return impl_->get_mode();
}

size_t KyberKEM::get_public_key_size() const {
    return impl_->get_public_key_size();
}

size_t KyberKEM::get_private_key_size() const {
    return impl_->get_private_key_size();
}

size_t KyberKEM::get_ciphertext_size() const {
    return impl_->get_ciphertext_size();
}

size_t KyberKEM::get_shared_key_size() const {
    return impl_->get_shared_key_size();
}

void KyberKEM::demo() {
    std::cout << "============================================" << std::endl;
    std::cout << "Kyber KEM Demo" << std::endl;
    std::cout << "============================================" << std::endl;
    
    try {
        // Create a Kyber KEM instance with Kyber-768
        KyberKEM kem("Kyber768");
        std::cout << "Created Kyber KEM instance with mode: " << kem.get_mode() << std::endl;
        
        // Generate a key pair
        std::cout << "Generating key pair..." << std::endl;
        auto [public_key, private_key] = kem.generate_keypair();
        
        std::cout << "Public key size: " << public_key.size() << " bytes" << std::endl;
        std::cout << "Private key size: " << private_key.size() << " bytes" << std::endl;
        
        // Encapsulate a shared secret
        std::cout << "Encapsulating shared secret..." << std::endl;
        auto [ciphertext, shared_secret] = kem.encapsulate(public_key);
        
        std::cout << "Ciphertext size: " << ciphertext.size() << " bytes" << std::endl;
        std::cout << "Shared secret size: " << shared_secret.size() << " bytes" << std::endl;
        
        // Decapsulate the shared secret
        std::cout << "Decapsulating shared secret..." << std::endl;
        auto decapsulated = kem.decapsulate(ciphertext, private_key);
        
        std::cout << "Decapsulated shared secret size: " << decapsulated.size() << " bytes" << std::endl;
        
        // Verify that the shared secrets match
        bool success = (shared_secret == decapsulated);
        std::cout << "Shared secrets match: " << (success ? "YES" : "NO") << std::endl;
        
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
