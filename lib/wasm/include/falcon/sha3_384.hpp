#pragma once
#include "sponge.hpp"

// SHA3-384 Hash Function : Keccak[768](M || 01, 384)
namespace sha3_384 {

// Given N -bytes input message, this routine consumes it into keccak[768]
// sponge state and squeezes out 48 -bytes digest | N >= 0
//
// See SHA3 hash function definition in section 6.1 of SHA3 specification
// https://dx.doi.org/10.6028/NIST.FIPS.202
static void
hash(const uint8_t* const __restrict msg, const size_t mlen, uint8_t* const __restrict dig)
{
  constexpr size_t dlen = 384;
  constexpr size_t capacity = 2 * dlen;
  constexpr size_t rate = 1600 - capacity;

  uint64_t state[25]{};

  sponge::absorb<0b00000010, 2, rate>(state, msg, mlen);
  sponge::squeeze<rate>(state, dig, dlen >> 3);
}

}
