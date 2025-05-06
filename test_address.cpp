#include <iostream>
#include <vector>
#include <string>
#include <random>

// Function to generate a random public key
std::vector<uint8_t> generateRandomPublicKey(size_t length) {
    std::vector<uint8_t> key(length);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(0, 255);
    
    for (size_t i = 0; i < length; ++i) {
        key[i] = static_cast<uint8_t>(distrib(gen));
    }
    
    return key;
}

// Simple address class
class Address {
public:
    Address(const std::vector<uint8_t>& data) : m_data(data) {}
    
    std::string toString() const {
        std::string result;
        for (auto byte : m_data) {
            result += std::to_string(byte) + " ";
        }
        return result;
    }
    
private:
    std::vector<uint8_t> m_data;
};

// Simple address generator
class AddressGenerator {
public:
    Address generateAddress(const std::vector<uint8_t>& publicKey) {
        // Simple hash function (XOR all bytes)
        std::vector<uint8_t> hash(32, 0);
        for (size_t i = 0; i < publicKey.size(); ++i) {
            hash[i % 32] ^= publicKey[i];
        }
        return Address(hash);
    }
};

int main() {
    std::cout << "Testing Address Generation" << std::endl;
    std::cout << "=========================" << std::endl;
    
    // Generate a random public key
    auto publicKey = generateRandomPublicKey(64);
    
    std::cout << "Public Key: ";
    for (size_t i = 0; i < 8; ++i) {
        std::cout << static_cast<int>(publicKey[i]) << " ";
    }
    std::cout << "..." << std::endl;
    
    // Generate an address
    AddressGenerator generator;
    auto address = generator.generateAddress(publicKey);
    
    std::cout << "Address: " << address.toString() << std::endl;
    
    return 0;
}
