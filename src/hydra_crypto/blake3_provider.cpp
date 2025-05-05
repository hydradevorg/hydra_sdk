#include "blake3_provider.hpp"
#include "hydra_crypto/blake3_hash.hpp"

namespace hydra {
namespace crypto {

Blake3Provider::Blake3Provider() {
    // Nothing to initialize here
}

Blake3Provider::~Blake3Provider() {
    // Nothing to clean up
}

std::vector<uint8_t> Blake3Provider::hash(const uint8_t* data, size_t length) {
    return Blake3Hash::hash(data, length);
}

std::vector<uint8_t> Blake3Provider::hash(const std::vector<uint8_t>& data) {
    return Blake3Hash::hash(data);
}

std::vector<uint8_t> Blake3Provider::hash(const std::string& data) {
    return Blake3Hash::hash(data);
}

size_t Blake3Provider::get_hash_size() const {
    return BLAKE3_HASH_SIZE;
}

std::string Blake3Provider::get_algorithm_name() const {
    return "BLAKE3";
}

// Factory function implementation
std::shared_ptr<IHashingProvider> create_blake3_provider() {
    return std::make_shared<Blake3Provider>();
}

} // namespace crypto
} // namespace hydra
