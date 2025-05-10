// Mock Botan Hex header for WebAssembly build
#pragma once

#include <string>
#include <vector>

namespace Botan {

std::string hex_encode(const uint8_t data[], size_t length) {
    return "";
}

std::string hex_encode(const std::vector<uint8_t>& data) {
    return "";
}

std::vector<uint8_t> hex_decode(const std::string& hex) {
    return {};
}

} // namespace Botan
