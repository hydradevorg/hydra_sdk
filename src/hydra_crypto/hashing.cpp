#include "hydra_crypto/hashing.hpp"
#include "blake3_provider.hpp"

namespace hydra {
namespace crypto {

std::shared_ptr<IHashingProvider> create_blake3_provider() {
    return std::make_shared<Blake3Provider>();
}

} // namespace crypto
} // namespace hydra
