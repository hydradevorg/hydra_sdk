#include <hydra_crypto/falcon_signature.hpp>
#include <falcon/falcon.hpp>
#include <falcon/utils.hpp>

#include <iostream>
#include <stdexcept>
#include <cstring>
#include <memory>
#include <vector>

namespace hydra {
namespace crypto {

class FalconSignature::Impl {
public:
    Impl(int degree) : m_degree(degree) {
        // Validate degree selection
        if (degree != 512 && degree != 1024) {
            throw std::invalid_argument("Invalid Falcon degree. Supported values are 512 or 1024");
        }
        
        // Set algorithm name based on degree
        m_name = "Falcon-" + std::to_string(degree);
    }
    
    ~Impl() = default;

    std::pair<std::vector<uint8_t>, std::vector<uint8_t>> generate_key_pair() {
        try {
            // Allocate memory for keys
            size_t pklen = falcon_utils::compute_pkey_len<512>();
            size_t sklen = falcon_utils::compute_skey_len<512>();
            
            if (m_degree == 1024) {
                pklen = falcon_utils::compute_pkey_len<1024>();
                sklen = falcon_utils::compute_skey_len<1024>();
            }
            
            std::vector<uint8_t> public_key(pklen);
            std::vector<uint8_t> private_key(sklen);
            
            // Generate key pair using the appropriate template specialization
            if (m_degree == 512) {
                falcon::keygen<512>(public_key.data(), private_key.data());
            } else { // m_degree == 1024
                falcon::keygen<1024>(public_key.data(), private_key.data());
            }
            
            // Store the keys
            m_public_key = public_key;
            m_private_key = private_key;
            
            return {public_key, private_key};
        }
        catch (const std::exception& e) {
            throw std::runtime_error(std::string("Key generation failed: ") + e.what());
        }
    }

    std::vector<uint8_t> get_public_key() const {
        return m_public_key;
    }
    
    std::vector<uint8_t> get_private_key() const {
        return m_private_key;
    }

    std::vector<uint8_t> sign_message(const std::vector<uint8_t>& message) {
        try {
            if (m_private_key.empty()) {
                throw std::runtime_error("No private key available for signing");
            }
            
            // Determine signature size based on degree
            size_t sig_len = (m_degree == 512) ? 666 : 1280;
            std::vector<uint8_t> signature(sig_len);
            
            // Sign the message using the appropriate template specialization
            bool success = false;
            if (m_degree == 512) {
                success = falcon::sign<512>(
                    m_private_key.data(),
                    message.data(),
                    message.size(),
                    signature.data()
                );
            } else { // m_degree == 1024
                success = falcon::sign<1024>(
                    m_private_key.data(),
                    message.data(),
                    message.size(),
                    signature.data()
                );
            }
            
            if (!success) {
                throw std::runtime_error("Signature generation failed");
            }
            
            return signature;
        }
        catch (const std::exception& e) {
            throw std::runtime_error(std::string("Signing failed: ") + e.what());
        }
    }
    
    std::vector<uint8_t> sign_message(const std::string& message) {
        return sign_message(std::vector<uint8_t>(message.begin(), message.end()));
    }

    bool verify_signature(const std::vector<uint8_t>& message, const std::vector<uint8_t>& signature) {
        try {
            if (m_public_key.empty()) {
                throw std::runtime_error("No public key available for verification");
            }
            
            // Create a mutable copy of the signature for the verify function
            std::vector<uint8_t> sig_copy = signature;
            
            // Verify the signature using the appropriate template specialization
            bool result = false;
            if (m_degree == 512) {
                result = falcon::verify<512>(
                    m_public_key.data(),
                    message.data(),
                    message.size(),
                    sig_copy.data()
                );
            } else { // m_degree == 1024
                result = falcon::verify<1024>(
                    m_public_key.data(),
                    message.data(),
                    message.size(),
                    sig_copy.data()
                );
            }
            
            return result;
        }
        catch (const std::exception& e) {
            throw std::runtime_error(std::string("Verification failed: ") + e.what());
        }
    }
    
    bool verify_signature(const std::string& message, const std::vector<uint8_t>& signature) {
        return verify_signature(std::vector<uint8_t>(message.begin(), message.end()), signature);
    }

    void set_public_key(const std::vector<uint8_t>& key) {
        m_public_key = key;
    }
    
    void set_private_key(const std::vector<uint8_t>& key) {
        m_private_key = key;
    }

    size_t get_public_key_size() const {
        if (m_degree == 512) {
            return falcon_utils::compute_pkey_len<512>();
        } else { // m_degree == 1024
            return falcon_utils::compute_pkey_len<1024>();
        }
    }
    
    size_t get_private_key_size() const {
        if (m_degree == 512) {
            return falcon_utils::compute_skey_len<512>();
        } else { // m_degree == 1024
            return falcon_utils::compute_skey_len<1024>();
        }
    }
    
    size_t get_signature_size() const {
        // Approximate signature size based on degree
        return m_degree == 512 ? 666 : 1280;
    }
    
    std::string get_name() const {
        return m_name;
    }
    
    int get_security_level() const {
        return m_degree == 512 ? 128 : 256;
    }

private:
    int m_degree;
    std::string m_name;
    std::vector<uint8_t> m_public_key;
    std::vector<uint8_t> m_private_key;
};

// Implementation of the public class methods

FalconSignature::FalconSignature(int degree)
    : impl_(std::make_unique<Impl>(degree)) {
}

FalconSignature::~FalconSignature() = default;

std::pair<std::vector<uint8_t>, std::vector<uint8_t>> FalconSignature::generate_key_pair() {
    return impl_->generate_key_pair();
}

std::vector<uint8_t> FalconSignature::get_public_key() const {
    return impl_->get_public_key();
}

std::vector<uint8_t> FalconSignature::get_private_key() const {
    return impl_->get_private_key();
}

std::vector<uint8_t> FalconSignature::sign_message(const std::vector<uint8_t>& message) {
    return impl_->sign_message(message);
}

std::vector<uint8_t> FalconSignature::sign_message(const std::string& message) {
    return impl_->sign_message(message);
}

bool FalconSignature::verify_signature(const std::vector<uint8_t>& message, const std::vector<uint8_t>& signature) {
    return impl_->verify_signature(message, signature);
}

bool FalconSignature::verify_signature(const std::string& message, const std::vector<uint8_t>& signature) {
    return impl_->verify_signature(message, signature);
}

void FalconSignature::set_public_key(const std::vector<uint8_t>& key) {
    impl_->set_public_key(key);
}

void FalconSignature::set_private_key(const std::vector<uint8_t>& key) {
    impl_->set_private_key(key);
}

size_t FalconSignature::get_public_key_size() const {
    return impl_->get_public_key_size();
}

size_t FalconSignature::get_private_key_size() const {
    return impl_->get_private_key_size();
}

size_t FalconSignature::get_signature_size() const {
    return impl_->get_signature_size();
}

std::string FalconSignature::get_name() const {
    return impl_->get_name();
}

int FalconSignature::get_security_level() const {
    return impl_->get_security_level();
}

void FalconSignature::demo() {
    std::cout << "============================================" << std::endl;
    std::cout << "Falcon Signature Demo" << std::endl;
    std::cout << "============================================" << std::endl;
    
    try {
        // Create a Falcon-512 instance
        FalconSignature falcon(512);
        std::cout << "Created " << falcon.get_name() << " instance" << std::endl;
        std::cout << "Security level: " << falcon.get_security_level() << " bits" << std::endl;
        
        // Generate a key pair
        std::cout << "Generating key pair..." << std::endl;
        auto [public_key, private_key] = falcon.generate_key_pair();
        
        std::cout << "Public key size: " << public_key.size() << " bytes" << std::endl;
        std::cout << "Private key size: " << private_key.size() << " bytes" << std::endl;
        
        // Sign a message
        std::string message = "Hello, post-quantum world!";
        std::cout << "Signing message: \"" << message << "\"" << std::endl;
        
        auto signature = falcon.sign_message(message);
        std::cout << "Signature size: " << signature.size() << " bytes" << std::endl;
        
        // Verify the signature
        std::cout << "Verifying signature..." << std::endl;
        bool is_valid = falcon.verify_signature(message, signature);
        
        std::cout << "Signature is " << (is_valid ? "valid" : "invalid") << std::endl;
        
        // Try with a tampered message
        std::string tampered_message = "Hello, post-quantum world?";
        std::cout << "Verifying with tampered message: \"" << tampered_message << "\"" << std::endl;
        
        bool tampered_valid = falcon.verify_signature(tampered_message, signature);
        std::cout << "Tampered signature is " << (tampered_valid ? "valid (BAD!)" : "invalid (GOOD!)") << std::endl;
        
        std::cout << "============================================" << std::endl;
        std::cout << "Demo completed successfully" << std::endl;
        std::cout << "============================================" << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Error in demo: " << e.what() << std::endl;
    }
}

} // namespace crypto
} // namespace hydra
