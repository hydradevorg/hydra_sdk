#include "hydra_math/rational.hpp"
#include <gmp.h>
#include <stdexcept>

namespace hydra {
    namespace math {

Rational::Rational() : numerator("0"), denominator("1") {}

Rational::Rational(const BigInt& num, const BigInt& denom)
    : numerator(num), denominator(denom) {
    if (denominator == BigInt("0")) throw std::invalid_argument("Divide by zero");
    simplify();
}

Rational::Rational(const std::string& str) {
    auto pos = str.find('/');
    if (pos == std::string::npos)
        throw std::invalid_argument("Invalid rational format");

    numerator = BigInt(str.substr(0, pos));
    denominator = BigInt(str.substr(pos + 1));
    simplify();
}

void Rational::simplify() {
    mpz_t gcd;
    mpz_init(gcd);
    // Get the raw mpz_t values and dereference them
    mpz_gcd(gcd, *numerator.raw(), *denominator.raw());
    mpz_tdiv_q(*numerator.raw(), *numerator.raw(), gcd);
    mpz_tdiv_q(*denominator.raw(), *denominator.raw(), gcd);
    mpz_clear(gcd);
}

Rational Rational::operator+(const Rational& rhs) const {
    BigInt num = numerator * rhs.denominator + rhs.numerator * denominator;
    BigInt denom = denominator * rhs.denominator;
    return Rational(num, denom);
}

Rational Rational::operator-(const Rational& rhs) const {
    BigInt num = numerator * rhs.denominator - rhs.numerator * denominator;
    BigInt denom = denominator * rhs.denominator;
    return Rational(num, denom);
}

Rational Rational::operator*(const Rational& rhs) const {
    return Rational(numerator * rhs.numerator, denominator * rhs.denominator);
}

Rational Rational::operator/(const Rational& rhs) const {
    return Rational(numerator * rhs.denominator, denominator * rhs.numerator);
}

bool Rational::operator==(const Rational& rhs) const {
    return numerator == rhs.numerator && denominator == rhs.denominator;
}

std::string Rational::toString() const {
    return numerator.to_string() + "/" + denominator.to_string();
}

} // namespace math
} // namespace hydra
