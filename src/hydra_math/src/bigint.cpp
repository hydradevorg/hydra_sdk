#include "hydra_math/bigint.hpp"

namespace hydra {
namespace math {

// This file is intentionally empty as BigInt is fully implemented in the header file
// with inline methods. This stub file is kept to maintain the build structure.

// Add any non-inline methods here if needed in the future

}} // namespace hydra::math

bool BigInt::operator<=(const BigInt& rhs) const {
    return mpz_cmp(value, rhs.value) <= 0;
}

bool BigInt::operator>=(const BigInt& rhs) const {
    return mpz_cmp(value, rhs.value) >= 0;
}

std::string BigInt::toString(int base) const {
    char* cstr = mpz_get_str(nullptr, base, value);
    std::string result(cstr);
    void (*freefunc)(void*, size_t);
    mp_get_memory_functions(nullptr, nullptr, &freefunc);
    freefunc(cstr, std::strlen(cstr) + 1);
    return result;
}

const mpz_t& BigInt::raw() const {
    return value;
}

mpz_t& BigInt::raw() {
    return value;
}

std::ostream& operator<<(std::ostream& os, const BigInt& val) {
    return os << val.toString();
}

} // namespace math
} // namespace hydra

