#include <hydra_crypto/root_key_manager.hpp>
#include <botan/base64.h>
#include <botan/aead.h>
#include <botan/cipher_mode.h>
#include <stdexcept>
#include <iostream>
#include <chrono>
#include <algorithm>
#include <random>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <nlohmann/json.hpp>

namespace hydra {
namespace crypto {

RootKeyManager::RootKeyManager(int rotation_interval_days)
    : rotation_interval_days_(rotation_interval_days),
      rng_(std::make_unique<Botan::AutoSeeded_RNG>()) {
    // Generate an initial root key pair if none exists
    if (root_key_pairs_.empty()) {
        generateRootKeyPair();
    }
}

std::string RootKeyManager::generateRootKeyPair(const std::string& strength) {
    // Create Dilithium and Kyber instances
    DilithiumSignature dilithium;
    KyberKEM kyber;
    
    // Generate key pairs
    auto [dilithium_public_key, dilithium_private_key] = dilithium.generate_key_pair();
    auto [kyber_public_key, kyber_private_key] = kyber.generate_keypair();
    
    // Base64 encode the keys
    std::string dilithium_public_key_b64 = Botan::base64_encode(dilithium_public_key);
    std::string dilithium_private_key_b64 = Botan::base64_encode(dilithium_private_key);
    std::string kyber_public_key_b64 = Botan::base64_encode(kyber_public_key);
    std::string kyber_private_key_b64 = Botan::base64_encode(kyber_private_key);
    
    // Create a new root key pair
    RootKeyPair key_pair;
    key_pair.id = generateUniqueId();
    key_pair.dilithium_private_key = dilithium_private_key_b64;
    key_pair.dilithium_public_key = dilithium_public_key_b64;
    key_pair.kyber_private_key = kyber_private_key_b64;
    key_pair.kyber_public_key = kyber_public_key_b64;
    key_pair.strength = strength;
    key_pair.created_at = std::chrono::system_clock::now();
    key_pair.expires_at = key_pair.created_at + std::chrono::hours(24 * rotation_interval_days_);
    
    // If this is the first key, make it active
    if (root_key_pairs_.empty()) {
        key_pair.is_active = true;
    } else {
        // Otherwise, it's not active by default
        key_pair.is_active = false;
    }
    
    // Add the key pair to the list
    root_key_pairs_.push_back(key_pair);
    
    return key_pair.id;
}

RootKeyManager::RootKeyPair RootKeyManager::getActiveRootKeyPair() const {
    auto it = findActiveRootKeyPair();
    if (it == root_key_pairs_.end()) {
        throw std::runtime_error("No active root key pair found");
    }
    return *it;
}

RootKeyManager::RootKeyPair RootKeyManager::getRootKeyPairById(const std::string& id) const {
    auto it = findRootKeyPairById(id);
    if (it == root_key_pairs_.end()) {
        throw std::runtime_error("Root key pair not found: " + id);
    }
    return *it;
}

std::vector<RootKeyManager::RootKeyPair> RootKeyManager::listRootKeyPairs() const {
    return root_key_pairs_;
}

std::string RootKeyManager::rotateRootKeyPair(bool force) {
    // Check if rotation is needed
    if (!force && !needsRotation()) {
        // Get the current active key
        auto it = findActiveRootKeyPair();
        if (it != root_key_pairs_.end()) {
            return it->id;
        }
    }
    
    // Generate a new key pair
    std::string new_key_id = generateRootKeyPair();
    
    // Deactivate all existing keys
    for (auto& key_pair : root_key_pairs_) {
        key_pair.is_active = false;
    }
    
    // Activate the new key
    auto it = findRootKeyPairById(new_key_id);
    if (it != root_key_pairs_.end()) {
        // Since we're using a const_iterator, we need to find the non-const element
        auto non_const_it = std::find_if(root_key_pairs_.begin(), root_key_pairs_.end(),
                                        [&new_key_id](const RootKeyPair& key_pair) {
                                            return key_pair.id == new_key_id;
                                        });
        if (non_const_it != root_key_pairs_.end()) {
            non_const_it->is_active = true;
        }
    }
    
    return new_key_id;
}

std::string RootKeyManager::signPublicKey(const std::string& public_key) const {
    // Get the active root key pair
    auto key_pair = getActiveRootKeyPair();
    
    // Decode the Dilithium private key
    Botan::secure_vector<uint8_t> dilithium_private_key_secure = Botan::base64_decode(key_pair.dilithium_private_key);
    std::vector<uint8_t> dilithium_private_key(dilithium_private_key_secure.begin(), dilithium_private_key_secure.end());
    
    // Create a Dilithium instance
    DilithiumSignature dilithium;
    
    // Set the private key
    dilithium.set_private_key(dilithium_private_key);
    
    // Sign the public key
    std::vector<uint8_t> public_key_bytes(public_key.begin(), public_key.end());
    std::vector<uint8_t> signature = dilithium.sign_message(public_key_bytes);
    
    // Encode the signature in base64
    return Botan::base64_encode(signature);
}

bool RootKeyManager::verifyPublicKey(const std::string& public_key, const std::string& signature, 
                                     const std::string& root_key_id) const {
    // Get the root key pair by ID
    auto key_pair = getRootKeyPairById(root_key_id);
    
    // Decode the Dilithium public key
    Botan::secure_vector<uint8_t> dilithium_public_key_secure = Botan::base64_decode(key_pair.dilithium_public_key);
    std::vector<uint8_t> dilithium_public_key(dilithium_public_key_secure.begin(), dilithium_public_key_secure.end());
    
    // Decode the signature
    Botan::secure_vector<uint8_t> signature_bytes_secure = Botan::base64_decode(signature);
    std::vector<uint8_t> signature_bytes(signature_bytes_secure.begin(), signature_bytes_secure.end());
    
    // Create a Dilithium instance
    DilithiumSignature dilithium;
    
    // Set the public key
    dilithium.set_public_key(dilithium_public_key);
    
    // Verify the signature
    std::vector<uint8_t> public_key_bytes(public_key.begin(), public_key.end());
    return dilithium.verify_signature(public_key_bytes, signature_bytes);
}

std::string RootKeyManager::encryptPublicKey(const std::string& public_key) const {
    // Get the active root key pair
    auto key_pair = getActiveRootKeyPair();
    
    // Decode the Kyber public key
    Botan::secure_vector<uint8_t> kyber_public_key_secure = Botan::base64_decode(key_pair.kyber_public_key);
    std::vector<uint8_t> kyber_public_key(kyber_public_key_secure.begin(), kyber_public_key_secure.end());
    
    // Create a Kyber instance
    KyberKEM kyber;
    
    // Encapsulate a shared key
    auto [ciphertext, shared_key] = kyber.encapsulate(kyber_public_key);
    
    // Create an AES-GCM cipher for encryption
    auto cipher = Botan::Cipher_Mode::create("AES-256/GCM", Botan::Cipher_Dir::Encryption);
    
    // Generate a random IV
    std::vector<uint8_t> iv(12);
    rng_->randomize(iv.data(), iv.size());
    
    // Initialize the cipher
    cipher->set_key(shared_key);
    cipher->start(iv);
    
    // Encrypt the public key
    Botan::secure_vector<uint8_t> public_key_bytes(public_key.begin(), public_key.end());
    cipher->finish(public_key_bytes);
    
    // Create the encrypted package
    nlohmann::json package;
    package["root_key_id"] = key_pair.id;
    package["ciphertext"] = Botan::base64_encode(ciphertext);
    package["iv"] = Botan::base64_encode(iv);
    package["encrypted_data"] = Botan::base64_encode(public_key_bytes);
    
    return package.dump();
}

std::string RootKeyManager::decryptPublicKey(const std::string& encrypted_public_key, 
                                           const std::string& key_id) const {
    // Parse the encrypted package
    nlohmann::json package = nlohmann::json::parse(encrypted_public_key);
    
    // Get the root key pair by ID
    auto key_pair = getRootKeyPairById(key_id);
    
    // Decode the Kyber private key
    Botan::secure_vector<uint8_t> kyber_private_key_secure = Botan::base64_decode(key_pair.kyber_private_key);
    std::vector<uint8_t> kyber_private_key(kyber_private_key_secure.begin(), kyber_private_key_secure.end());
    
    // Decode the ciphertext
    Botan::secure_vector<uint8_t> ciphertext_secure = Botan::base64_decode(package["ciphertext"].get<std::string>());
    std::vector<uint8_t> ciphertext(ciphertext_secure.begin(), ciphertext_secure.end());
    
    // Create a Kyber instance
    KyberKEM kyber;
    
    // Decapsulate the shared key
    std::vector<uint8_t> shared_key = kyber.decapsulate(ciphertext, kyber_private_key);
    
    // Decode the IV and encrypted data
    Botan::secure_vector<uint8_t> iv_secure = Botan::base64_decode(package["iv"].get<std::string>());
    std::vector<uint8_t> iv(iv_secure.begin(), iv_secure.end());
    Botan::secure_vector<uint8_t> encrypted_data = Botan::base64_decode(package["encrypted_data"].get<std::string>());
    
    // Create an AES-GCM cipher for decryption
    auto cipher = Botan::Cipher_Mode::create("AES-256/GCM", Botan::Cipher_Dir::Decryption);
    
    // Initialize the cipher
    cipher->set_key(shared_key);
    cipher->start(iv);
    
    // Decrypt the data
    cipher->finish(encrypted_data);
    
    // Convert the decrypted data to a string
    return std::string(encrypted_data.begin(), encrypted_data.end());
}

bool RootKeyManager::needsRotation() const {
    auto it = findActiveRootKeyPair();
    if (it == root_key_pairs_.end()) {
        // No active key, so rotation is needed
        return true;
    }
    
    // Check if the key has expired
    auto now = std::chrono::system_clock::now();
    return now >= it->expires_at;
}

std::string RootKeyManager::generateUniqueId() const {
    // Generate a UUID-like identifier
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 15);
    std::uniform_int_distribution<> dis2(8, 11);
    
    std::stringstream ss;
    ss << std::hex;
    
    for (int i = 0; i < 8; i++) {
        ss << dis(gen);
    }
    ss << "-";
    for (int i = 0; i < 4; i++) {
        ss << dis(gen);
    }
    ss << "-4";
    for (int i = 0; i < 3; i++) {
        ss << dis(gen);
    }
    ss << "-";
    ss << dis2(gen);
    for (int i = 0; i < 3; i++) {
        ss << dis(gen);
    }
    ss << "-";
    for (int i = 0; i < 12; i++) {
        ss << dis(gen);
    }
    
    return ss.str();
}

std::vector<RootKeyManager::RootKeyPair>::const_iterator RootKeyManager::findRootKeyPairById(const std::string& id) const {
    return std::find_if(root_key_pairs_.begin(), root_key_pairs_.end(),
                       [&id](const RootKeyPair& key_pair) {
                           return key_pair.id == id;
                       });
}

std::vector<RootKeyManager::RootKeyPair>::const_iterator RootKeyManager::findActiveRootKeyPair() const {
    return std::find_if(root_key_pairs_.begin(), root_key_pairs_.end(),
                       [](const RootKeyPair& key_pair) {
                           return key_pair.is_active;
                       });
}

} // namespace crypto
} // namespace hydra
