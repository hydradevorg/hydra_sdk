/**
 * @file test_hydra_sdk_integration.cpp
 * @brief Tests integration between hydra_crypto and the rest of hydra_sdk
 */

#include <catch2/catch.hpp>
#include <crypto/dilithium_signature.hpp>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>

using namespace hydra_crypto;

// Helper function to hex encode for display
std::string hex_encode(const std::vector<uint8_t>& data, size_t max_length = 0) {
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    
    size_t display_size = (max_length > 0 && max_length < data.size()) ? max_length : data.size();
    
    for(size_t i = 0; i < display_size; i++) {
        ss << std::setw(2) << static_cast<int>(data[i]);
    }
    
    if (display_size < data.size()) {
        ss << "...";
    }
    
    return ss.str();
}

// Simulate hydra_sdk usage of the library
class MockHydraSDK {
public:
    MockHydraSDK() : 
        m_signer("ML-DSA-65"),
        m_last_error("") {
        // Initialize the crypto library
    }
    
    bool initialize() {
        try {
            // Generate keys for the session
            auto [pub_key, priv_key] = m_signer.generate_key_pair();
            m_public_key = pub_key;
            m_private_key = priv_key;
            return true;
        }
        catch (const std::exception& e) {
            m_last_error = e.what();
            return false;
        }
    }
    
    std::vector<uint8_t> sign_transaction(const std::string& transaction_data) {
        try {
            return m_signer.sign_message(transaction_data);
        }
        catch (const std::exception& e) {
            m_last_error = e.what();
            return {};
        }
    }
    
    bool verify_transaction(const std::string& transaction_data, 
                           const std::vector<uint8_t>& signature,
                           const std::vector<uint8_t>& public_key) {
        try {
            // Create a temporary verifier with the provided public key
            DilithiumSignature verifier("ML-DSA-65");
            verifier.set_public_key(public_key);
            return verifier.verify_signature(transaction_data, signature);
        }
        catch (const std::exception& e) {
            m_last_error = e.what();
            return false;
        }
    }
    
    std::string get_last_error() const {
        return m_last_error;
    }
    
    std::vector<uint8_t> get_public_key() const {
        return m_public_key;
    }

private:
    DilithiumSignature m_signer;
    std::vector<uint8_t> m_public_key;
    std::vector<uint8_t> m_private_key;
    std::string m_last_error;
};

TEST_CASE("Hydra SDK Integration", "[integration][hydra_sdk]") {
    SECTION("Basic SDK integration") {
        // Create mock SDK
        MockHydraSDK sdk;
        
        // Initialize
        REQUIRE(sdk.initialize());
        
        // Get the public key
        auto public_key = sdk.get_public_key();
        REQUIRE_FALSE(public_key.empty());
        
        INFO("Public key: " << hex_encode(public_key, 32));
        
        // Create a transaction
        std::string transaction = R"({
            "sender": "0x1234567890abcdef",
            "recipient": "0xfedcba0987654321",
            "amount": 100.50,
            "timestamp": 1620000000
        })";
        
        // Sign the transaction
        auto signature = sdk.sign_transaction(transaction);
        REQUIRE_FALSE(signature.empty());
        
        INFO("Signature: " << hex_encode(signature, 64));
        
        // Verify the transaction
        bool is_valid = sdk.verify_transaction(transaction, signature, public_key);
        REQUIRE(is_valid);
        
        // Tamper with the transaction
        std::string tampered_transaction = transaction;
        tampered_transaction.replace(tampered_transaction.find("100.50"), 6, "999.99");
        
        // Verify should fail
        bool tampered_valid = sdk.verify_transaction(tampered_transaction, signature, public_key);
        REQUIRE_FALSE(tampered_valid);
    }
    
    SECTION("Multiple instances") {
        // Create multiple SDK instances to ensure no static state issues
        MockHydraSDK sdk1;
        MockHydraSDK sdk2;
        
        REQUIRE(sdk1.initialize());
        REQUIRE(sdk2.initialize());
        
        auto pk1 = sdk1.get_public_key();
        auto pk2 = sdk2.get_public_key();
        
        // Public keys should be different
        REQUIRE(pk1 != pk2);
        
        // Each instance should validate its own signatures
        std::string msg = "Test message";
        
        auto sig1 = sdk1.sign_transaction(msg);
        auto sig2 = sdk2.sign_transaction(msg);
        
        REQUIRE(sdk1.verify_transaction(msg, sig1, pk1));
        REQUIRE(sdk2.verify_transaction(msg, sig2, pk2));
        
        // Cross-verification should work too
        REQUIRE(sdk1.verify_transaction(msg, sig2, pk2));
        REQUIRE(sdk2.verify_transaction(msg, sig1, pk1));
        
        // But signatures don't work with wrong keys
        REQUIRE_FALSE(sdk1.verify_transaction(msg, sig1, pk2));
        REQUIRE_FALSE(sdk2.verify_transaction(msg, sig2, pk1));
    }
    
    SECTION("Error handling") {
        MockHydraSDK sdk;
        REQUIRE(sdk.initialize());
        
        // Try to verify with invalid data
        std::vector<uint8_t> invalid_signature(10, 0);  // Too short
        std::vector<uint8_t> invalid_key(10, 0);        // Too short
        
        bool result = sdk.verify_transaction("test", invalid_signature, sdk.get_public_key());
        REQUIRE_FALSE(result);
        REQUIRE_FALSE(sdk.get_last_error().empty());
        
        result = sdk.verify_transaction("test", sdk.sign_transaction("test"), invalid_key);
        REQUIRE_FALSE(result);
        REQUIRE_FALSE(sdk.get_last_error().empty());
    }
}
