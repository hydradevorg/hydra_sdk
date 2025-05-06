#pragma once

#include "hydra_crypto/hashing.hpp"

namespace hydra {
namespace crypto {

/**
 * BLAKE3 hashing provider implementation.
 */
class Blake3Provider : public IHashingProvider {
public:
    Blake3Provider();
    ~Blake3Provider() override;

    std::vector<uint8_t> hash(const uint8_t* data, size_t length) override;
    std::vector<uint8_t> hash(const std::vector<uint8_t>& data) override;
    std::vector<uint8_t> hash(const std::string& data) override;
    
    size_t get_hash_size() const override;
    std::string get_algorithm_name() const override;

private:
    static constexpr size_t BLAKE3_HASH_SIZE = 32; // BLAKE3 produces 32-byte digests
};

} // namespace crypto
} // namespace hydra