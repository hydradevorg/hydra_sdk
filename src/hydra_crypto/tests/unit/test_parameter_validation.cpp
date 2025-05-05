/**
 * @file test_parameter_validation.cpp
 * @brief Tests for parameter validation in the hydra_crypto library
 */

#include <catch2/catch.hpp>
#include <crypto/dilithium_signature.hpp>
#include <vector>
#include <string>
#include <stdexcept>

using namespace hydra_crypto;

TEST_CASE("Parameter validation", "[validation][dilithium]") {
    SECTION("DilithiumSignature constructor") {
        // Valid strengths
        REQUIRE_NOTHROW(DilithiumSignature("ML-DSA-44"));
        REQUIRE_NOTHROW(DilithiumSignature("ML-DSA-65"));
        REQUIRE_NOTHROW(DilithiumSignature("ML-DSA-87"));
        
        // Invalid strengths
        REQUIRE_THROWS_AS(DilithiumSignature(""), std::invalid_argument);
        REQUIRE_THROWS_AS(DilithiumSignature("ML-DSA"), std::invalid_argument);
        REQUIRE_THROWS_AS(DilithiumSignature("ML-DSA-11"), std::invalid_argument);
        REQUIRE_THROWS_AS(DilithiumSignature("INVALID"), std::invalid_argument);
        
        // Edge cases
        REQUIRE_THROWS_AS(DilithiumSignature("ml-dsa-65"), std::invalid_argument); // Case sensitive
        REQUIRE_THROWS_AS(DilithiumSignature("ML-DSA-65 "), std::invalid_argument); // Extra space
    }
    
    SECTION("Empty message signing") {
        DilithiumSignature signer;
        signer.generate_key_pair();
        
        // Empty message should be signable
        std::vector<uint8_t> empty_vec;
        std::string empty_str;
        
        REQUIRE_NOTHROW(signer.sign_message(empty_vec));
        REQUIRE_NOTHROW(signer.sign_message(empty_str));
        
        auto empty_sig = signer.sign_message(empty_vec);
        REQUIRE_FALSE(empty_sig.empty());
        
        // Should be verifiable
        REQUIRE(signer.verify_signature(empty_vec, empty_sig));
        REQUIRE(signer.verify_signature(empty_str, empty_sig));
    }
    
    SECTION("Large message signing") {
        DilithiumSignature signer;
        signer.generate_key_pair();
        
        // Create a 1MB message
        std::vector<uint8_t> large_message(1024 * 1024);
        for (size_t i = 0; i < large_message.size(); i++) {
            large_message[i] = static_cast<uint8_t>(i & 0xFF);
        }
        
        // Should handle large messages without issues
        REQUIRE_NOTHROW(signer.sign_message(large_message));
        
        auto signature = signer.sign_message(large_message);
        REQUIRE_FALSE(signature.empty());
        REQUIRE(signer.verify_signature(large_message, signature));
    }
    
    SECTION("Missing keys") {
        DilithiumSignature signer;
        std::vector<uint8_t> message = {1, 2, 3, 4, 5};
        
        // No keys generated yet
        REQUIRE_THROWS(signer.sign_message(message));
        REQUIRE_THROWS(signer.get_public_key());
        REQUIRE_THROWS(signer.get_private_key());
        
        // Generate only public key
        auto [pub, priv] = DilithiumSignature().generate_key_pair();
        
        DilithiumSignature pub_only;
        pub_only.set_public_key(pub);
        
        // Should not be able to sign without private key
        REQUIRE_THROWS(pub_only.sign_message(message));
        REQUIRE_NOTHROW(pub_only.get_public_key());
        REQUIRE_THROWS(pub_only.get_private_key());
    }
    
    SECTION("Invalid keys") {
        DilithiumSignature signer;
        
        // Totally invalid data
        std::vector<uint8_t> invalid_data = {1, 2, 3, 4, 5};
        REQUIRE_THROWS(signer.set_public_key(invalid_data));
        REQUIRE_THROWS(signer.set_private_key(invalid_data));
        
        // Empty data
        std::vector<uint8_t> empty_data;
        REQUIRE_THROWS(signer.set_public_key(empty_data));
        REQUIRE_THROWS(signer.set_private_key(empty_data));
        
        // Generate real keys but with wrong size
        auto [pub, priv] = DilithiumSignature("ML-DSA-44").generate_key_pair();
        DilithiumSignature signer87("ML-DSA-87");
        
        // Keys from different strength should be rejected
        REQUIRE_THROWS(signer87.set_public_key(pub));
        REQUIRE_THROWS(signer87.set_private_key(priv));
    }
    
    SECTION("Invalid signatures") {
        DilithiumSignature signer;
        signer.generate_key_pair();
        
        std::vector<uint8_t> message = {1, 2, 3, 4, 5};
        
        // Create some invalid signatures
        std::vector<uint8_t> empty_sig;
        std::vector<uint8_t> short_sig = {1, 2, 3, 4, 5};
        std::vector<uint8_t> random_sig(signer.get_signature_size(), 0xFF);
        
        // Verification should return false, not throw
        REQUIRE_FALSE(signer.verify_signature(message, empty_sig));
        REQUIRE_FALSE(signer.verify_signature(message, short_sig));
        REQUIRE_FALSE(signer.verify_signature(message, random_sig));
    }
}
