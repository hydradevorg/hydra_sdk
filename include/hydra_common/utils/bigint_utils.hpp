#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include "hydra_math/bigint.hpp"

namespace hydra { namespace common {



inline math::BigInt toBigInt(const std::string& str, int base = 10) {
    math::BigInt result;
    mpz_set_str(result.get_mpz_t(), str.c_str(), base);
    return result;
}

inline math::BigInt toBigInt(const std::vector<uint8_t>& bytes) {
    math::BigInt result;
    mpz_import(result.get_mpz_t(), bytes.size(), 1, 1, 0, 0, bytes.data());
    return result;
}

template<typename T>
inline math::BigInt toBigInt(T value) requires std::is_integral_v<T> {
    return math::BigInt(static_cast<int64_t>(value));
}

// --- From BigInt ---

inline std::string fromBigIntToString(const math::BigInt& value, int base = 10) {
    return value.to_string(base);
}

inline std::vector<uint8_t> fromBigIntToBytes(const math::BigInt& value, size_t min_size = 0) {
    size_t count;
    std::vector<uint8_t> buf((mpz_sizeinbase(value.get_mpz_t(), 2) + 7) / 8 + 1);
    mpz_export(buf.data(), &count, 1, 1, 0, 0, value.get_mpz_t());
    buf.resize(std::max(count, min_size));
    return buf;
}

template<typename T>
inline T fromBigIntToIntegral(const math::BigInt& value) requires std::is_integral_v<T> {
    return static_cast<T>(value.to_int());
}

}} // namespace hydra::common

