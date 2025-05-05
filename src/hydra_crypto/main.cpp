#include <iostream>
#include <string>
#include <vector>
#include <hydra_crypto/kyber_kem.hpp>
#include <hydra_crypto/kyber_aes.hpp>
#include <hydra_crypto/dilithium_signature.hpp>
#include <hydra_crypto/root_key_manager.hpp>

int main() {
    std::cout << "Hydra Crypto Library Demo" << std::endl;
    std::cout << "=========================" << std::endl;
    
    try {
        // Demonstrate Kyber KEM functionality
        std::cout << "\n1. Kyber KEM Demo:" << std::endl;
        std::cout << "------------------" << std::endl;
        
        hydra::crypto::KyberKEM kyber("Kyber768");
        std::cout << "Created Kyber KEM instance with mode: " << kyber.get_mode() << std::endl;
        
        auto [public_key, private_key] = kyber.generate_keypair();
        std::cout << "Generated key pair:" << std::endl;
        std::cout << "  Public key size: " << public_key.size() << " bytes" << std::endl;
        std::cout << "  Private key size: " << private_key.size() << " bytes" << std::endl;
        
        auto [ciphertext, shared_secret] = kyber.encapsulate(public_key);
        std::cout << "Encapsulated shared secret:" << std::endl;
        std::cout << "  Ciphertext size: " << ciphertext.size() << " bytes" << std::endl;
        std::cout << "  Shared secret size: " << shared_secret.size() << " bytes" << std::endl;
        
        auto decapsulated = kyber.decapsulate(ciphertext, private_key);
        std::cout << "Decapsulated shared secret size: " << decapsulated.size() << " bytes" << std::endl;
        
        bool success = (shared_secret == decapsulated);
        std::cout << "Shared secrets match: " << (success ? "YES" : "NO") << std::endl;
        
        // Demonstrate Kyber AES functionality
        std::cout << "\n2. Kyber AES Demo:" << std::endl;
        std::cout << "------------------" << std::endl;
        
        hydra::crypto::KyberAES kyber_aes("Kyber768");
        std::cout << "Created " << kyber_aes.get_name() << " instance" << std::endl;
        
        auto [aes_public_key, aes_private_key] = kyber_aes.generate_keypair();
        std::cout << "Generated key pair:" << std::endl;
        std::cout << "  Public key size: " << aes_public_key.size() << " bytes" << std::endl;
        std::cout << "  Private key size: " << aes_private_key.size() << " bytes" << std::endl;
        
        std::string message = "This is a secret message that needs to be encrypted securely.";
        std::cout << "Original message: " << message << std::endl;
        
        auto encrypted = kyber_aes.encrypt(message, aes_public_key);
        std::cout << "Encrypted message size: " << encrypted.size() << " bytes" << std::endl;
        
        auto decrypted = kyber_aes.decrypt_to_string(encrypted, aes_private_key);
        std::cout << "Decrypted message: " << decrypted << std::endl;
        std::cout << "Decryption successful: " << (message == decrypted ? "YES" : "NO") << std::endl;
        
        // Demonstrate Dilithium Signature functionality
        std::cout << "\n3. Dilithium Signature Demo:" << std::endl;
        std::cout << "----------------------------" << std::endl;
        
        hydra::crypto::DilithiumSignature dilithium("ML-DSA-65");
        std::cout << "Created " << dilithium.get_name() << " instance" << std::endl;
        
        auto [dilithium_public_key, dilithium_private_key] = dilithium.generate_key_pair();
        std::cout << "Generated key pair:" << std::endl;
        std::cout << "  Public key size: " << dilithium_public_key.size() << " bytes" << std::endl;
        std::cout << "  Private key size: " << dilithium_private_key.size() << " bytes" << std::endl;
        
        std::string sign_message = "This message needs to be signed to verify its authenticity.";
        std::cout << "Message to sign: " << sign_message << std::endl;
        
        dilithium.set_private_key(dilithium_private_key);
        auto signature = dilithium.sign_message(sign_message);
        std::cout << "Signature size: " << signature.size() << " bytes" << std::endl;
        
        dilithium.set_public_key(dilithium_public_key);
        bool verified = dilithium.verify_signature(sign_message, signature);
        std::cout << "Signature verification: " << (verified ? "VALID" : "INVALID") << std::endl;
        
        // Demonstrate Root Key Manager functionality
        std::cout << "\n4. Root Key Manager Demo:" << std::endl;
        std::cout << "--------------------------" << std::endl;
        
        hydra::crypto::RootKeyManager key_manager(30); // 30 days rotation interval
        std::cout << "Created Root Key Manager with 30-day rotation interval" << std::endl;
        
        std::string key_id = key_manager.generateRootKeyPair();
        std::cout << "Generated root key pair with ID: " << key_id << std::endl;
        
        auto key_pairs = key_manager.listRootKeyPairs();
        std::cout << "Number of key pairs: " << key_pairs.size() << std::endl;
        
        auto active_key = key_manager.getActiveRootKeyPair();
        std::cout << "Active key ID: " << active_key.id << std::endl;
        
        std::string test_key = "TestPublicKey123";
        std::string signature_b64 = key_manager.signPublicKey(test_key);
        std::cout << "Signed public key, signature size: " << signature_b64.size() << " bytes" << std::endl;
        
        bool verify_result = key_manager.verifyPublicKey(test_key, signature_b64, active_key.id);
        std::cout << "Signature verification: " << (verify_result ? "VALID" : "INVALID") << std::endl;
        
        std::cout << "\nAll demos completed successfully!" << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "ERROR: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
