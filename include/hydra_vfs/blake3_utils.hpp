#pragma once

#include <vector>
#include <string>
#include <hydra_crypto/blake3_hash.hpp>

namespace hydra {
namespace vfs {

/**
 * @brief Utility functions for BLAKE3 hashing in the VFS module
 */
class Blake3Utils {
public:
    /**
     * @brief Calculate a BLAKE3 hash of the provided data
     * 
     * @param data The data to hash
     * @return std::vector<uint8_t> The resulting hash (32 bytes)
     */
    static std::vector<uint8_t> calculateHash(const std::vector<uint8_t>& data) {
        return hydra::crypto::Blake3Hash::hash(data);
    }
    
    /**
     * @brief Calculate a BLAKE3 hash of the provided string
     * 
     * @param data The string to hash
     * @return std::vector<uint8_t> The resulting hash (32 bytes)
     */
    static std::vector<uint8_t> calculateHash(const std::string& data) {
        return hydra::crypto::Blake3Hash::hash(data);
    }
    
    /**
     * @brief Calculate a BLAKE3 hash of the provided data with a specific output size
     * 
     * @param data The data to hash
     * @param output_size The desired output size in bytes
     * @return std::vector<uint8_t> The resulting hash
     */
    static std::vector<uint8_t> calculateHash(const std::vector<uint8_t>& data, size_t output_size) {
        return hydra::crypto::Blake3Hash::hash(data, output_size);
    }
    
    /**
     * @brief Calculate a BLAKE3 hash of the provided data and return it as a hex string
     * 
     * @param data The data to hash
     * @return std::string The resulting hash as a hexadecimal string
     */
    static std::string calculateHashHex(const std::vector<uint8_t>& data) {
        return hydra::crypto::Blake3Hash::hashHex(data);
    }
    
    /**
     * @brief Verify if a hash matches the expected hash for the given data
     * 
     * @param data The data to verify
     * @param expected_hash The expected hash value
     * @return bool True if the hash matches, false otherwise
     */
    static bool verifyHash(const std::vector<uint8_t>& data, const std::vector<uint8_t>& expected_hash) {
        auto calculated_hash = calculateHash(data);
        
        if (calculated_hash.size() != expected_hash.size()) {
            return false;
        }
        
        for (size_t i = 0; i < calculated_hash.size(); ++i) {
            if (calculated_hash[i] != expected_hash[i]) {
                return false;
            }
        }
        
        return true;
    }
};

} // namespace vfs
} // namespace hydra
