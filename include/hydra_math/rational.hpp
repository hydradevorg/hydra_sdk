#pragma once

#include "hydra_math/bigint.hpp"
#include <string>
#include <iostream>

namespace hydra { 
    namespace math {

class Rational {
public:
    Rational();
    Rational(const BigInt& num, const BigInt& denom);
    Rational(const std::string& str);

    Rational operator+(const Rational& rhs) const;
    Rational operator-(const Rational& rhs) const;
    Rational operator*(const Rational& rhs) const;
    Rational operator/(const Rational& rhs) const;

    bool operator==(const Rational& rhs) const;
    bool operator!=(const Rational& rhs) const;
    bool operator<(const Rational& rhs) const;
    bool operator>(const Rational& rhs) const;
    bool operator<=(const Rational& rhs) const;
    bool operator>=(const Rational& rhs) const;

    std::string toString() const;

    const BigInt& getNumerator() const;
    const BigInt& getDenominator() const;

private:
    BigInt numerator;
    BigInt denominator;
    void simplify();
};

std::ostream& operator<<(std::ostream& os, const Rational& val);

}} // namespace hydra::math
