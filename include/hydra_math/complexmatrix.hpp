#pragma once

#include <Eigen/Dense>
#include <complex>
#include <string>

namespace hydra { namespace math {

using Complex = std::complex<double>;
using ComplexMatrix = Eigen::MatrixXcd;
using ComplexVector = Eigen::VectorXcd;

// Matrix ops
ComplexMatrix tensor(const ComplexMatrix& A, const ComplexMatrix& B);
ComplexMatrix dagger(const ComplexMatrix& A);
std::string toString(const ComplexMatrix& mat);
std::string toString(const ComplexVector& vec);

}} // namespace hydra::math
