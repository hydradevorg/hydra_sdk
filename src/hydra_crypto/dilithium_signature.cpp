#include <hydra_crypto/dilithium_signature.hpp>

#include <botan/auto_rng.h>
#include <botan/pubkey.h>
#include <botan/pk_keys.h>
#include <botan/data_src.h>
#include <botan/hex.h>
#include <botan/pkcs8.h>
#include <botan/x509_key.h>
#include <botan/dilithium.h>  // For Dilithium support

#include <iostream>
#include <stdexcept>
#include <fstream>
#include <iomanip>

namespace hydra {
namespace crypto {

class DilithiumSignature::Impl {
public:
    Impl(const std::string& strength) : 
        m_strength(strength),
        m_rng(std::make_unique<Botan::AutoSeeded_RNG>()) {
        // Validate strength selection
        if (strength != "ML-DSA-44" && strength != "ML-DSA-65" && strength != "ML-DSA-87") {
            throw std::invalid_argument("Invalid Dilithium strength. Supported values are ML-DSA-44, ML-DSA-65, and ML-DSA-87");
        }
    }

    std::pair<std::vector<uint8_t>, std::vector<uint8_t>> generate_key_pair() {
        try {
            // Map our API names to Botan's internal names
            std::string botan_mode;
            if (m_strength == "ML-DSA-44") {
                botan_mode = "ML-DSA-4x4";
            } else if (m_strength == "ML-DSA-65") {
                botan_mode = "ML-DSA-6x5";
            } else if (m_strength == "ML-DSA-87") {
                botan_mode = "ML-DSA-8x7";
            } else {
                throw std::runtime_error("Unsupported Dilithium mode");
            }
            
            // Create a new key pair using string-based mode constructor
            Botan::DilithiumMode mode(botan_mode);
            m_private_key = std::make_unique<Botan::Dilithium_PrivateKey>(*m_rng, mode);
            
            if (!m_private_key) {
                throw std::runtime_error("Failed to create Dilithium private key");
            }

            // Get corresponding public key
            m_public_key = m_private_key->public_key();
            if (!m_public_key) {
                throw std::runtime_error("Failed to extract public key");
            }

            // Export keys to raw bytes - convert secure_vector to std::vector
            std::vector<uint8_t> public_key_bytes;
            auto secure_pubkey = m_public_key->public_key_bits();
            public_key_bytes.assign(secure_pubkey.begin(), secure_pubkey.end());
            
            // For private key, use PKCS8 encoding without encryption
            auto secure_privkey = Botan::PKCS8::BER_encode(*m_private_key);
            std::vector<uint8_t> private_key_bytes;
            private_key_bytes.assign(secure_privkey.begin(), secure_privkey.end());

            return {public_key_bytes, private_key_bytes};
        }
        catch (const std::exception& e) {
            throw std::runtime_error(std::string("Key generation failed: ") + e.what());
        }
    }

    std::vector<uint8_t> sign_message(const std::vector<uint8_t>& message) {
        if (!m_private_key) {
            throw std::runtime_error("No private key available for signing");
        }

        try {
            // Create a signer object - use empty string for default parameters
            Botan::PK_Signer signer(*m_private_key, *m_rng, "");
            
            // Update with message and generate signature
            signer.update(message);
            auto secure_sig = signer.signature(*m_rng);
            
            // Convert to std::vector
            std::vector<uint8_t> signature;
            signature.assign(secure_sig.begin(), secure_sig.end());
            
            return signature;
        }
        catch (const std::exception& e) {
            throw std::runtime_error(std::string("Signing failed: ") + e.what());
        }
    }

    bool verify_signature(const std::vector<uint8_t>& message, const std::vector<uint8_t>& signature) {
        if (!m_public_key) {
            throw std::runtime_error("No public key available for verification");
        }

        try {
            // Create a verifier object - use empty string for default parameters
            Botan::PK_Verifier verifier(*m_public_key, "");
            
            // Update with message and check signature
            verifier.update(message);
            return verifier.check_signature(signature);
        }
        catch (const std::exception& e) {
            // Log error but return false for invalid signatures
            std::cerr << "Verification error: " << e.what() << std::endl;
            return false;
        }
    }

    void set_public_key(const std::vector<uint8_t>& key) {
        try {
            // Map our API names to Botan's internal names
            std::string botan_mode;
            if (m_strength == "ML-DSA-44") {
                botan_mode = "ML-DSA-4x4";
            } else if (m_strength == "ML-DSA-65") {
                botan_mode = "ML-DSA-6x5";
            } else if (m_strength == "ML-DSA-87") {
                botan_mode = "ML-DSA-8x7";
            } else {
                throw std::runtime_error("Unsupported Dilithium mode");
            }
            
            // Create Dilithium mode
            Botan::DilithiumMode mode(botan_mode);
            
            // Load public key from raw bytes
            m_public_key = std::make_unique<Botan::Dilithium_PublicKey>(key, mode);
            
            if (!m_public_key) {
                throw std::runtime_error("Failed to load public key");
            }
        }
        catch (const std::exception& e) {
            throw std::runtime_error(std::string("Setting public key failed: ") + e.what());
        }
    }

    void set_private_key(const std::vector<uint8_t>& key) {
        try {
            // Map our API names to Botan's internal names
            std::string botan_mode;
            if (m_strength == "ML-DSA-44") {
                botan_mode = "ML-DSA-4x4";
            } else if (m_strength == "ML-DSA-65") {
                botan_mode = "ML-DSA-6x5";
            } else if (m_strength == "ML-DSA-87") {
                botan_mode = "ML-DSA-8x7";
            } else {
                throw std::runtime_error("Unsupported Dilithium mode");
            }
            
            // Create Dilithium mode
            Botan::DilithiumMode mode(botan_mode);
            
            // Load private key from raw bytes
            Botan::DataSource_Memory source(key.data(), key.size());
            m_private_key = Botan::PKCS8::load_key(source);
            
            if (!m_private_key) {
                throw std::runtime_error("Failed to load private key");
            }
            
            // Also extract the public key
            m_public_key = m_private_key->public_key();
        }
        catch (const std::exception& e) {
            throw std::runtime_error(std::string("Setting private key failed: ") + e.what());
        }
    }

    std::vector<uint8_t> get_public_key() const {
        if (!m_public_key) {
            throw std::runtime_error("No public key available");
        }
        auto secure_pubkey = m_public_key->public_key_bits();
        std::vector<uint8_t> public_key_bytes;
        public_key_bytes.assign(secure_pubkey.begin(), secure_pubkey.end());
        return public_key_bytes;
    }

    std::vector<uint8_t> get_private_key() const {
        if (!m_private_key) {
            throw std::runtime_error("No private key available");
        }
        auto secure_privkey = Botan::PKCS8::BER_encode(*m_private_key);
        std::vector<uint8_t> private_key_bytes;
        private_key_bytes.assign(secure_privkey.begin(), secure_privkey.end());
        return private_key_bytes;
    }

    size_t get_public_key_size() const {
        if (!m_public_key) {
            // Return estimated size based on strength
            if (m_strength == "ML-DSA-44") return 1312;
            if (m_strength == "ML-DSA-65") return 1952;
            if (m_strength == "ML-DSA-87") return 2592;
            return 0;
        }
        return m_public_key->public_key_bits().size();
    }

    size_t get_private_key_size() const {
        if (!m_private_key) {
            // Return estimated size based on strength
            if (m_strength == "ML-DSA-44") return 2528;
            if (m_strength == "ML-DSA-65") return 4000;
            if (m_strength == "ML-DSA-87") return 4864;
            return 0;
        }
        return Botan::PKCS8::BER_encode(*m_private_key).size();
    }

    size_t get_signature_size() const {
        // Return expected signature size based on strength
        if (m_strength == "ML-DSA-44") return 2420;
        if (m_strength == "ML-DSA-65") return 3293;
        if (m_strength == "ML-DSA-87") return 4595;
        return 0;
    }

    std::string get_name() const {
        return m_strength;
    }

    int get_security_level() const {
        // Return security level in bits
        if (m_strength == "ML-DSA-44") return 128;
        if (m_strength == "ML-DSA-65") return 192;
        if (m_strength == "ML-DSA-87") return 256;
        return 0;
    }

private:
    std::string m_strength;
    std::unique_ptr<Botan::Private_Key> m_private_key;
    std::unique_ptr<Botan::Public_Key> m_public_key;
    std::unique_ptr<Botan::RandomNumberGenerator> m_rng;
};

// Implementation of the public class methods

DilithiumSignature::DilithiumSignature(const std::string& strength)
    : impl_(std::make_unique<Impl>(strength)) {
}

DilithiumSignature::~DilithiumSignature() = default;

std::pair<std::vector<uint8_t>, std::vector<uint8_t>> DilithiumSignature::generate_key_pair() {
    return impl_->generate_key_pair();
}

std::vector<uint8_t> DilithiumSignature::get_public_key() const {
    return impl_->get_public_key();
}

std::vector<uint8_t> DilithiumSignature::get_private_key() const {
    return impl_->get_private_key();
}

std::vector<uint8_t> DilithiumSignature::sign_message(const std::vector<uint8_t>& message) {
    return impl_->sign_message(message);
}

std::vector<uint8_t> DilithiumSignature::sign_message(const std::string& message) {
    return sign_message(std::vector<uint8_t>(message.begin(), message.end()));
}

bool DilithiumSignature::verify_signature(const std::vector<uint8_t>& message, const std::vector<uint8_t>& signature) {
    return impl_->verify_signature(message, signature);
}

bool DilithiumSignature::verify_signature(const std::string& message, const std::vector<uint8_t>& signature) {
    return verify_signature(std::vector<uint8_t>(message.begin(), message.end()), signature);
}

void DilithiumSignature::set_public_key(const std::vector<uint8_t>& key) {
    impl_->set_public_key(key);
}

void DilithiumSignature::set_private_key(const std::vector<uint8_t>& key) {
    impl_->set_private_key(key);
}

size_t DilithiumSignature::get_public_key_size() const {
    return impl_->get_public_key_size();
}

size_t DilithiumSignature::get_private_key_size() const {
    return impl_->get_private_key_size();
}

size_t DilithiumSignature::get_signature_size() const {
    return impl_->get_signature_size();
}

std::string DilithiumSignature::get_name() const {
    return impl_->get_name();
}

int DilithiumSignature::get_security_level() const {
    return impl_->get_security_level();
}

void DilithiumSignature::demo() {
    std::cout << "============================================" << std::endl;
    std::cout << "        ML-DSA (Dilithium) Demo            " << std::endl;
    std::cout << "============================================" << std::endl;
    
    try {
        // Create a Dilithium signer with ML-DSA-65 (medium security level)
        DilithiumSignature signer("ML-DSA-65");
        
        std::cout << "Algorithm: " << signer.get_name() << std::endl;
        std::cout << "Security level: " << signer.get_security_level() << " bits" << std::endl;
        
        // Generate a key pair
        std::cout << "\nGenerating key pair..." << std::endl;
        auto [public_key, private_key] = signer.generate_key_pair();
        
        std::cout << "Public key size: " << public_key.size() << " bytes" << std::endl;
        std::cout << "Public key (first 32 bytes): " 
                  << Botan::hex_encode(public_key.data(), std::min(size_t(32), public_key.size())) 
                  << "..." << std::endl;
        
        std::cout << "Private key size: " << private_key.size() << " bytes" << std::endl;
        std::cout << "Private key (first 32 bytes): " 
                  << Botan::hex_encode(private_key.data(), std::min(size_t(32), private_key.size())) 
                  << "..." << std::endl;
        
        // Sign a message
        std::string message = "Hello, post-quantum world!";
        std::cout << "\nSigning message: \"" << message << "\"" << std::endl;
        std::vector<uint8_t> signature = signer.sign_message(message);
        
        std::cout << "Signature size: " << signature.size() << " bytes" << std::endl;
        std::cout << "Signature (first 64 bytes): " 
                  << Botan::hex_encode(signature.data(), std::min(size_t(64), signature.size())) 
                  << "..." << std::endl;
        
        // Verify the signature
        std::cout << "\nVerifying signature..." << std::endl;
        bool valid = signer.verify_signature(message, signature);
        std::cout << "Signature is " << (valid ? "valid ✓" : "invalid ✗") << std::endl;
        
        // Try with a tampered message
        std::string tampered_message = "Hello, post-quantum world?";
        std::cout << "\nVerifying signature with tampered message..." << std::endl;
        bool invalid = signer.verify_signature(tampered_message, signature);
        std::cout << "Signature is " << (invalid ? "valid ✗" : "invalid ✓") << std::endl;
        
        std::cout << "\nDemo completed successfully!" << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Demo failed: " << e.what() << std::endl;
    }
}

} // namespace crypto
} // namespace hydra