#pragma once

#include "hydra_math/complexmatrix.hpp"

namespace hydra { namespace math {

ComplexMatrix matrixExp(const ComplexMatrix& A);
ComplexMatrix matrixPow(const ComplexMatrix& A, int n);

double frobeniusNorm(const ComplexMatrix& A);
double matrixDistance(const ComplexMatrix& A, const ComplexMatrix& B);

bool isUnitary(const ComplexMatrix& A, double epsilon = 1e-10);
bool isHermitian(const ComplexMatrix& A, double epsilon = 1e-10);

ComplexVector basisState(int index, int dim);
ComplexVector uniformSuperposition(int dim);

ComplexMatrix identity(int dim);
ComplexMatrix pauliX();
ComplexMatrix hadamard();

}} // namespace hydra::math
