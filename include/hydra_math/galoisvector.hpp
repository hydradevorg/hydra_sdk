#pragma once

#include "hydra_math/bigint.hpp"
#include "hydra_math/modular.hpp"
#include <vector>

namespace hydra { namespace math {

template<typename T = BigInt>
class GaloisVector {
public:
    GaloisVector(const std::vector<T>& data, const T& modulus)
        : vec(data), q(modulus) {}

    GaloisVector<T> operator+(const GaloisVector<T>& other) const {
        std::vector<T> result;
        for (size_t i = 0; i < vec.size(); ++i) {
            result.push_back(modAdd(vec[i], other.vec[i], q));
        }
        return GaloisVector<T>(result, q);
    }

    GaloisVector<T> operator*(const T& scalar) const {
        std::vector<T> result;
        for (const auto& v : vec) {
            result.push_back(modMul(v, scalar, q));
        }
        return GaloisVector<T>(result, q);
    }

    const std::vector<T>& values() const { return vec; }
    const T& mod() const { return q; }

private:
    std::vector<T> vec;
    T q;
};

}} // namespace hydra::math
