#include <hydra_address/address_generator.hpp>
#include <hydra_crypto/blake3_hash.hpp>
#include <hydra_crypto/falcon_signature.hpp>
#include <hydra_crypto/kyber_aes.hpp>
#include <lmvs/lmvs.hpp>

#include <gtest/gtest.h>
#include <random>
#include <string>
#include <vector>

// Helper function to generate random bytes
std::vector<uint8_t> generateRandomBytes(size_t length) {
    std::vector<uint8_t> result(length);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(0, 255);
    
    for (size_t i = 0; i < length; ++i) {
        result[i] = static_cast<uint8_t>(distrib(gen));
    }
    
    return result;
}

class AddressGeneratorTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Generate a key pair using Falcon
        auto [public_key, private_key] = m_falcon.generate_key_pair();
        m_public_key = public_key;
        m_private_key = private_key;
    }
    
    hydra::crypto::FalconSignature m_falcon{512};
    std::vector<uint8_t> m_public_key;
    std::vector<uint8_t> m_private_key;
};

TEST_F(AddressGeneratorTest, GenerateStandardAddress) {
    hydra::address::AddressGenerator address_gen(128);
    
    auto address = address_gen.generateFromPublicKey(
        m_public_key,
        hydra::address::AddressType::USER,
        hydra::address::AddressFormat::STANDARD
    );
    
    EXPECT_FALSE(address.toString().empty());
    EXPECT_EQ(address.getType(), hydra::address::AddressType::USER);
    EXPECT_EQ(address.getFormat(), hydra::address::AddressFormat::STANDARD);
    EXPECT_TRUE(address.verify());
}

TEST_F(AddressGeneratorTest, GenerateGeohashedAddress) {
    hydra::address::AddressGenerator address_gen(128);
    
    hydra::address::Coordinates coords{37.7749, -122.4194, 0.0};  // San Francisco
    
    auto address = address_gen.generateGeoAddress(
        m_public_key,
        coords,
        hydra::address::AddressType::NODE
    );
    
    EXPECT_FALSE(address.toString().empty());
    EXPECT_EQ(address.getType(), hydra::address::AddressType::NODE);
    EXPECT_EQ(address.getFormat(), hydra::address::AddressFormat::GEOHASHED);
    EXPECT_TRUE(address.verify());
    
    // Check geohash
    auto geohash = address.getGeohash();
    EXPECT_TRUE(geohash.has_value());
    
    // Check coordinates
    auto extracted_coords = address.getCoordinates();
    EXPECT_TRUE(extracted_coords.has_value());
    if (extracted_coords) {
        EXPECT_NEAR(extracted_coords->latitude, coords.latitude, 0.01);
        EXPECT_NEAR(extracted_coords->longitude, coords.longitude, 0.01);
    }
}

TEST_F(AddressGeneratorTest, GenerateQuantumAddress) {
    hydra::address::AddressGenerator address_gen(128);
    
    // Create a quantum state vector
    std::vector<std::complex<double>> quantum_state(8);
    quantum_state[0] = {0.5, 0.0};
    quantum_state[1] = {0.5, 0.0};
    quantum_state[2] = {0.5, 0.0};
    quantum_state[3] = {0.5, 0.0};
    
    auto address = address_gen.generateQuantumAddress(
        m_public_key,
        quantum_state,
        hydra::address::AddressType::USER
    );
    
    EXPECT_FALSE(address.toString().empty());
    EXPECT_EQ(address.getType(), hydra::address::AddressType::USER);
    EXPECT_EQ(address.getFormat(), hydra::address::AddressFormat::QUANTUM_PROOF);
    EXPECT_TRUE(address.verify());
}

TEST_F(AddressGeneratorTest, GenerateCompressedAddress) {
    hydra::address::AddressGenerator address_gen(128);
    
    auto address = address_gen.generateCompressedAddress(
        m_public_key,
        hydra::address::AddressType::RESOURCE
    );
    
    EXPECT_FALSE(address.toString().empty());
    EXPECT_EQ(address.getType(), hydra::address::AddressType::RESOURCE);
    EXPECT_EQ(address.getFormat(), hydra::address::AddressFormat::COMPRESSED);
    EXPECT_TRUE(address.verify());
}

TEST_F(AddressGeneratorTest, VerifyAddress) {
    hydra::address::AddressGenerator address_gen(128);
    
    auto address = address_gen.generateFromPublicKey(
        m_public_key,
        hydra::address::AddressType::USER,
        hydra::address::AddressFormat::STANDARD
    );
    
    EXPECT_TRUE(address_gen.verifyAddress(address));
    
    // Modify the address data to make it invalid
    std::vector<uint8_t> data = address.getData();
    if (!data.empty()) {
        data[0] ^= 0xFF;  // Flip all bits in the first byte
        hydra::address::Address modified_address(data);
        EXPECT_FALSE(address_gen.verifyAddress(modified_address));
    }
}

TEST_F(AddressGeneratorTest, AddressEquality) {
    hydra::address::AddressGenerator address_gen(128);
    
    auto address1 = address_gen.generateFromPublicKey(
        m_public_key,
        hydra::address::AddressType::USER,
        hydra::address::AddressFormat::STANDARD
    );
    
    auto address2 = hydra::address::Address(address1.getData());
    
    EXPECT_EQ(address1, address2);
    EXPECT_EQ(address1.toString(), address2.toString());
    
    auto address3 = address_gen.generateFromPublicKey(
        m_public_key,
        hydra::address::AddressType::NODE,  // Different type
        hydra::address::AddressFormat::STANDARD
    );
    
    EXPECT_NE(address1, address3);
}

TEST_F(AddressGeneratorTest, SecurityLevelChange) {
    hydra::address::AddressGenerator address_gen(128);
    
    EXPECT_EQ(address_gen.getSecurityLevel(), 128);
    
    auto address1 = address_gen.generateFromPublicKey(
        m_public_key,
        hydra::address::AddressType::USER,
        hydra::address::AddressFormat::STANDARD
    );
    
    // Change security level
    address_gen.setSecurityLevel(256);
    EXPECT_EQ(address_gen.getSecurityLevel(), 256);
    
    auto address2 = address_gen.generateFromPublicKey(
        m_public_key,
        hydra::address::AddressType::USER,
        hydra::address::AddressFormat::STANDARD
    );
    
    // Addresses should be different due to different security parameters
    EXPECT_NE(address1.toString(), address2.toString());
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
