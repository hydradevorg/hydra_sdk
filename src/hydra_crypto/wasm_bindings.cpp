#include <emscripten/bind.h>
#include <emscripten/val.h>
#include <hydra_crypto/blake3_hash.hpp>

using namespace emscripten;

// Simple Blake3 hash implementation for testing
class Blake3HashWrapper {
public:
    Blake3HashWrapper() {}

    std::vector<uint8_t> hash(const std::string& input) {
        hydra::crypto::Blake3Hash hasher;
        return hasher.hash(
            reinterpret_cast<const uint8_t*>(input.c_str()),
            input.size()
        );
    }
};

EMSCRIPTEN_BINDINGS(hydra_crypto) {
    class_<Blake3HashWrapper>("Blake3Hash")
        .constructor<>()
        .function("hash", &Blake3HashWrapper::hash);

    register_vector<uint8_t>("Vector<uint8_t>");
}
