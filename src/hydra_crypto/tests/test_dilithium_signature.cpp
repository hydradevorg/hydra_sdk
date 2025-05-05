#include "hydra_crypto/dilithium_signature.hpp"
#include <gtest/gtest.h>
#include <vector>
#include <string>
#include <iostream>
#include <botan/hex.h>

using hydra::crypto::DilithiumSignature;

class DilithiumSignatureTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup is called before each test
    }

    void TearDown() override {
        // TearDown is called after each test
    }
};

TEST_F(DilithiumSignatureTest, TestKeyGeneration) {
    // Test key generation for each security level
    const std::vector<std::string> levels = {"ML-DSA-44", "ML-DSA-65", "ML-DSA-87"};
    
    for (const auto& level : levels) {
        DilithiumSignature dilithium(level);
        auto [pk, sk] = dilithium.generate_key_pair();
        
        // Verify keys are not empty
        EXPECT_FALSE(pk.empty());
        EXPECT_FALSE(sk.empty());
        
        // Verify key sizes are as expected
        EXPECT_EQ(pk.size(), dilithium.get_public_key_size());
        EXPECT_EQ(sk.size(), dilithium.get_private_key_size());
        
        // Verify retrieved keys match
        auto pk2 = dilithium.get_public_key();
        auto sk2 = dilithium.get_private_key();
        EXPECT_EQ(pk, pk2);
        EXPECT_EQ(sk, sk2);
    }
}

TEST_F(DilithiumSignatureTest, TestSignAndVerify) {
    DilithiumSignature dilithium("ML-DSA-65");
    dilithium.generate_key_pair();
    
    // Test signing and verification
    std::string message = "This is a test message for ML-DSA signature verification.";
    auto signature = dilithium.sign_message(message);
    
    // Signature should not be empty
    EXPECT_FALSE(signature.empty());
    
    // Verify the signature
    bool verification = dilithium.verify_signature(message, signature);
    EXPECT_TRUE(verification);
    
    // Test with modified message
    std::string modified_message = message + ".";
    verification = dilithium.verify_signature(modified_message, signature);
    EXPECT_FALSE(verification);
    
    // Test with modified signature
    if (!signature.empty()) {
        signature[0] ^= 0x01;  // Flip a bit
        verification = dilithium.verify_signature(message, signature);
        EXPECT_FALSE(verification);
    }
}

TEST_F(DilithiumSignatureTest, TestKeyImportExport) {
    DilithiumSignature dilithium1("ML-DSA-65");
    auto [pk1, sk1] = dilithium1.generate_key_pair();
    
    // Create a second instance and import keys
    DilithiumSignature dilithium2("ML-DSA-65");
    dilithium2.set_public_key(pk1);
    dilithium2.set_private_key(sk1);
    
    // Verify keys match
    auto pk2 = dilithium2.get_public_key();
    auto sk2 = dilithium2.get_private_key();
    EXPECT_EQ(pk1, pk2);
    EXPECT_EQ(sk1, sk2);
    
    // Test sign and verify with imported keys
    std::string message = "Testing with imported keys";
    auto signature = dilithium2.sign_message(message);
    bool verification = dilithium2.verify_signature(message, signature);
    EXPECT_TRUE(verification);
    
    // Test cross-verification (sign with one, verify with another)
    auto signature1 = dilithium1.sign_message(message);
    verification = dilithium2.verify_signature(message, signature1);
    EXPECT_TRUE(verification);
}

TEST_F(DilithiumSignatureTest, TestVectorSignature) {
    // Create a new signer
    DilithiumSignature dilithium("ML-DSA-65");
    dilithium.generate_key_pair();
    
    // Test with binary data
    std::vector<uint8_t> binary_data = {
        0x01, 0x02, 0x03, 0x04, 0x05, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE,
        0xFF, 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88
    };
    
    auto signature = dilithium.sign_message(binary_data);
    EXPECT_FALSE(signature.empty());
    
    bool verification = dilithium.verify_signature(binary_data, signature);
    EXPECT_TRUE(verification);
}

TEST_F(DilithiumSignatureTest, TestParameterInfo) {
    // Test parameter info for different security levels
    const std::vector<std::tuple<std::string, int>> levels = {
        {"ML-DSA-44", 128},
        {"ML-DSA-65", 192},
        {"ML-DSA-87", 256}
    };
    
    for (const auto& [level, security] : levels) {
        DilithiumSignature dilithium(level);
        
        // Check algorithm name
        EXPECT_EQ(dilithium.get_name(), level);
        
        // Check security level
        EXPECT_EQ(dilithium.get_security_level(), security);
        
        // Key and signature sizes should be reasonable values
        EXPECT_GT(dilithium.get_public_key_size(), 0);
        EXPECT_GT(dilithium.get_private_key_size(), 0);
        EXPECT_GT(dilithium.get_signature_size(), 0);
    }
}

TEST_F(DilithiumSignatureTest, TestInvalidParameters) {
    // Test with invalid security level
    EXPECT_THROW(DilithiumSignature("ML-DSA-100"), std::invalid_argument);
    EXPECT_THROW(DilithiumSignature("Invalid"), std::invalid_argument);
    
    DilithiumSignature dilithium("ML-DSA-65");
    
    // Test verification with no keys
    std::string message = "Test message";
    std::vector<uint8_t> fake_signature(64, 0);
    EXPECT_THROW(dilithium.verify_signature(message, fake_signature), std::runtime_error);
    
    // Test signing with no keys
    EXPECT_THROW(dilithium.sign_message(message), std::runtime_error);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}