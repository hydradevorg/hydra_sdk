#pragma once

#include "hydra_math/bigint.hpp"

namespace hydra { namespace math {

struct PedersenParams {
    BigInt g;
    BigInt h;
    BigInt p; // large prime modulus
};

BigInt pedersenCommit(const BigInt& m, const BigInt& r, const PedersenParams& params);

}} // namespace hydra::math
