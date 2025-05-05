#include "hydra_math/pedersen.hpp"
#include "hydra_math/modular.hpp"

namespace hydra { namespace math {

BigInt pedersenCommit(const BigInt& m, const BigInt& r, const PedersenParams& params) {
    BigInt gm, hr;

    mpz_powm(gm.get_mpz_t(), params.g.get_mpz_t(), m.get_mpz_t(), params.p.get_mpz_t());
    mpz_powm(hr.get_mpz_t(), params.h.get_mpz_t(), r.get_mpz_t(), params.p.get_mpz_t());

    return modMul(gm, hr, params.p);
}

}} // namespace hydra::math
