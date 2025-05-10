#include <emscripten/bind.h>
#include <emscripten/val.h>

using namespace emscripten;

// Basic address generator for testing
class AddressGenerator {
public:
    AddressGenerator(int bitStrength) : bitStrength(bitStrength) {}

    std::string generateAddress(const std::vector<uint8_t>& publicKey) {
        // Simple address generation for testing
        std::string result = "addr_";
        for (size_t i = 0; i < 6 && i < publicKey.size(); i++) {
            char hex[3];
            snprintf(hex, sizeof(hex), "%02x", publicKey[i]);
            result += hex;
        }
        result += "_" + std::to_string(bitStrength);
        return result;
    }

private:
    int bitStrength;
};

EMSCRIPTEN_BINDINGS(hydra_address) {
    class_<AddressGenerator>("AddressGenerator")
        .constructor<int>()
        .function("generateAddress", &AddressGenerator::generateAddress);

    register_vector<uint8_t>("Vector<uint8_t>");
}
