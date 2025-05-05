#pragma once

#include <vector>
#include <iostream>
// Temporarily comment out the SHA3 include for testing
// #include "../../vendors/sha3/include/sha3_256.hpp"

namespace hydra {
namespace vfs {

// Helper function to calculate SHA3-256 hash
inline std::vector<uint8_t> calculate_sha256(const std::vector<uint8_t>& data) {
    // For testing, return a simple dummy hash instead of using SHA3
    std::vector<uint8_t> hash(32, 0); // 32 bytes of zeros
    
    // Fill with some pattern for testing
    for (size_t i = 0; i < std::min(data.size(), hash.size()); ++i) {
        hash[i] = data[i % data.size()];
    }
    
    std::cout << "DEBUG: Using dummy hash function for testing" << std::endl;
    return hash;
}

} // namespace vfs
} // namespace hydra
