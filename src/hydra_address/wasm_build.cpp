#include <emscripten/bind.h>
#include <emscripten/val.h>
#include <emscripten/emscripten.h>
#include <string>
#include <vector>
#include <cstdint>
#include <cstring>
#include <random>

// Export a simple function to ensure the JavaScript file is generated
extern "C" {
    EMSCRIPTEN_KEEPALIVE
    int get_version() {
        return 1;
    }
}

using namespace emscripten;

// Simple hash function for WebAssembly
std::vector<uint8_t> simple_hash(const std::vector<uint8_t>& data) {
    std::vector<uint8_t> hash(32, 0);

    // Simple hash algorithm for demonstration
    uint32_t h = 0x811c9dc5;
    for (uint8_t byte : data) {
        h ^= byte;
        h *= 0x01000193;
    }

    // Fill hash with derived values
    for (size_t i = 0; i < 8; ++i) {
        uint32_t value = h + i * 0x9e3779b9;
        hash[i*4] = (value >> 24) & 0xFF;
        hash[i*4+1] = (value >> 16) & 0xFF;
        hash[i*4+2] = (value >> 8) & 0xFF;
        hash[i*4+3] = value & 0xFF;
    }

    return hash;
}

// Mock address generator for WebAssembly
class AddressGenerator {
public:
    AddressGenerator(int bitStrength = 256) : m_bitStrength(bitStrength) {}

    std::string generateAddress(const std::vector<uint8_t>& publicKey) {
        // Simple address generation for WebAssembly
        std::vector<uint8_t> hash = simple_hash(publicKey);

        std::string result = "hydra_";
        for (size_t i = 0; i < 16 && i < hash.size(); i++) {
            char hex[3];
            snprintf(hex, sizeof(hex), "%02x", hash[i]);
            result += hex;
        }
        result += "_" + std::to_string(m_bitStrength);
        return result;
    }

    bool validateAddress(const std::string& address) {
        // Simple validation for WebAssembly
        return address.substr(0, 6) == "hydra_" && address.length() > 10;
    }

    std::string generateFromSeed(const std::string& seed) {
        // Convert seed to bytes
        std::vector<uint8_t> seedBytes(seed.begin(), seed.end());
        return generateAddress(seedBytes);
    }

    std::vector<uint8_t> generateRandomBytes(size_t length) {
        std::vector<uint8_t> result(length);
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> distrib(0, 255);

        for (size_t i = 0; i < length; ++i) {
            result[i] = static_cast<uint8_t>(distrib(gen));
        }

        return result;
    }

private:
    int m_bitStrength;
};

// Mock layered matrix for WebAssembly
class LayeredMatrix {
public:
    LayeredMatrix(int rows, int cols)
        : m_rows(rows), m_cols(cols), m_data(rows * cols, 0) {}

    void setValue(int row, int col, int value) {
        if (row >= 0 && row < m_rows && col >= 0 && col < m_cols) {
            m_data[row * m_cols + col] = value;
        }
    }

    int getValue(int row, int col) const {
        if (row >= 0 && row < m_rows && col >= 0 && col < m_cols) {
            return m_data[row * m_cols + col];
        }
        return 0;
    }

    std::vector<int> getRow(int row) const {
        std::vector<int> result;
        if (row >= 0 && row < m_rows) {
            for (int col = 0; col < m_cols; ++col) {
                result.push_back(m_data[row * m_cols + col]);
            }
        }
        return result;
    }

    std::vector<int> getColumn(int col) const {
        std::vector<int> result;
        if (col >= 0 && col < m_cols) {
            for (int row = 0; row < m_rows; ++row) {
                result.push_back(m_data[row * m_cols + col]);
            }
        }
        return result;
    }

private:
    int m_rows;
    int m_cols;
    std::vector<int> m_data;
};

// Bindings
EMSCRIPTEN_BINDINGS(hydra_address) {
    class_<AddressGenerator>("AddressGenerator")
        .constructor<>()
        .constructor<int>()
        .function("generateAddress", &AddressGenerator::generateAddress)
        .function("validateAddress", &AddressGenerator::validateAddress)
        .function("generateFromSeed", &AddressGenerator::generateFromSeed)
        .function("generateRandomBytes", &AddressGenerator::generateRandomBytes);

    class_<LayeredMatrix>("LayeredMatrix")
        .constructor<int, int>()
        .function("setValue", &LayeredMatrix::setValue)
        .function("getValue", &LayeredMatrix::getValue)
        .function("getRow", &LayeredMatrix::getRow)
        .function("getColumn", &LayeredMatrix::getColumn);

    register_vector<uint8_t>("Vector<uint8_t>");
    register_vector<int>("Vector<int>");

    function("simpleHash", &simple_hash);
}
