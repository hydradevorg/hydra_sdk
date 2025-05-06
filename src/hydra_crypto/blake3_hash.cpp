#include "hydra_crypto/blake3_hash.hpp"
#include "../../lib/blake3/c/blake3.h"
#include <memory>
#include <stdexcept>
#include <sstream>
#include <iomanip>
#include <cstring>

namespace hydra {
namespace crypto {

Blake3Hash::Blake3Hash() {
    m_hasher = new blake3_hasher();
    blake3_hasher_init(static_cast<blake3_hasher*>(m_hasher));
}

Blake3Hash::Blake3Hash(const std::array<uint8_t, KEY_SIZE>& key) {
    m_hasher = new blake3_hasher();
    blake3_hasher_init_keyed(static_cast<blake3_hasher*>(m_hasher), key.data());
}

Blake3Hash::Blake3Hash(const std::string& context) {
    m_hasher = new blake3_hasher();
    blake3_hasher_init_derive_key(static_cast<blake3_hasher*>(m_hasher), context.c_str());
}

Blake3Hash::~Blake3Hash() {
    if (m_hasher) {
        delete static_cast<blake3_hasher*>(m_hasher);
        m_hasher = nullptr;
    }
}

void Blake3Hash::reset() {
    blake3_hasher_reset(static_cast<blake3_hasher*>(m_hasher));
}

void Blake3Hash::update(const void* data, size_t size) {
    blake3_hasher_update(static_cast<blake3_hasher*>(m_hasher), data, size);
}

// std::span version removed due to compatibility issues

void Blake3Hash::update(const std::vector<uint8_t>& data) {
    update(data.data(), data.size());
}

void Blake3Hash::update(const std::string& data) {
    update(data.c_str(), data.size());
}

void Blake3Hash::finalize(uint8_t* output, size_t output_size) {
    blake3_hasher_finalize(static_cast<blake3_hasher*>(m_hasher), output, output_size);
}

std::vector<uint8_t> Blake3Hash::finalize(size_t output_size) {
    std::vector<uint8_t> output(output_size);
    finalize(output.data(), output_size);
    return output;
}

std::string Blake3Hash::finalizeHex(size_t output_size) {
    auto binary = finalize(output_size);
    return toHex(binary);
}

std::vector<uint8_t> Blake3Hash::hash(const void* data, size_t size, size_t output_size) {
    Blake3Hash hasher;
    hasher.update(data, size);
    return hasher.finalize(output_size);
}

// std::span version removed due to compatibility issues

std::vector<uint8_t> Blake3Hash::hash(const std::vector<uint8_t>& data, size_t output_size) {
    return hash(data.data(), data.size(), output_size);
}

std::vector<uint8_t> Blake3Hash::hash(const std::string& data, size_t output_size) {
    return hash(data.c_str(), data.size(), output_size);
}

std::string Blake3Hash::hashHex(const void* data, size_t size, size_t output_size) {
    auto binary = hash(data, size, output_size);
    return toHex(binary);
}

// std::span version removed due to compatibility issues

std::string Blake3Hash::hashHex(const std::vector<uint8_t>& data, size_t output_size) {
    return hashHex(data.data(), data.size(), output_size);
}

std::string Blake3Hash::hashHex(const std::string& data, size_t output_size) {
    return hashHex(data.c_str(), data.size(), output_size);
}

std::string Blake3Hash::toHex(const std::vector<uint8_t>& data) {
    std::stringstream ss;
    ss << std::hex << std::setfill('0');

    for (const auto& byte : data) {
        ss << std::setw(2) << static_cast<int>(byte);
    }

    return ss.str();
}

} // namespace crypto
} // namespace hydra
