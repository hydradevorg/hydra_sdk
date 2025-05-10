// Mock Botan Auto RNG header for WebAssembly build
#pragma once

#include <vector>
#include <memory>
#include <cstdint>

namespace Botan {

class RandomNumberGenerator {
public:
    virtual ~RandomNumberGenerator() = default;
    virtual void randomize(uint8_t output[], size_t length) {}
};

class AutoSeeded_RNG : public RandomNumberGenerator {
public:
    AutoSeeded_RNG() = default;
    ~AutoSeeded_RNG() override = default;

    void randomize(uint8_t output[], size_t length) override {}
};

} // namespace Botan
