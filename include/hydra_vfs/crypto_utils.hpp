#pragma once

#include <vector>
#include <iostream>
#include <hydra_vfs/blake3_utils.hpp>

namespace hydra {
namespace vfs {

/**
 * @brief Helper function to calculate hash using BLAKE3
 * 
 * @param data The data to hash
 * @return std::vector<uint8_t> The resulting hash (32 bytes)
 */
inline std::vector<uint8_t> calculate_sha256(const std::vector<uint8_t>& data) {
    // Use BLAKE3 from our utility class
    std::cout << "DEBUG: Using BLAKE3 hash function" << std::endl;
    return Blake3Utils::calculateHash(data);
}

} // namespace vfs
} // namespace hydra
