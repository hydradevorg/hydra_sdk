#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <memory>

namespace hydra {
namespace crypto {

/**
 * Interface for hashing providers.
 */
class IHashingProvider {
public:
    virtual ~IHashingProvider() = default;

    /**
     * Hash a data buffer using the provider's hashing algorithm.
     * 
     * @param data Pointer to the data to hash
     * @param length Length of the data in bytes
     * @return A vector containing the hash value
     */
    virtual std::vector<uint8_t> hash(const uint8_t* data, size_t length) = 0;

    /**
     * Hash a vector of data using the provider's hashing algorithm.
     * 
     * @param data Vector containing the data to hash
     * @return A vector containing the hash value
     */
    virtual std::vector<uint8_t> hash(const std::vector<uint8_t>& data) = 0;

    /**
     * Hash a string using the provider's hashing algorithm.
     * 
     * @param data String containing the data to hash
     * @return A vector containing the hash value
     */
    virtual std::vector<uint8_t> hash(const std::string& data) = 0;

    /**
     * Get the size of the hash output in bytes.
     * 
     * @return Size of the hash output in bytes
     */
    virtual size_t get_hash_size() const = 0;

    /**
     * Get the name of the hashing algorithm.
     * 
     * @return Name of the hashing algorithm
     */
    virtual std::string get_algorithm_name() const = 0;
};

/**
 * Create a BLAKE3 hashing provider.
 * 
 * @return A shared pointer to a BLAKE3 hashing provider
 */
std::shared_ptr<IHashingProvider> create_blake3_provider();

} // namespace crypto
} // namespace hydra
