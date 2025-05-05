#include "hydra_math/matrixutils.hpp"
#include <unsupported/Eigen/MatrixFunctions> // for exp()
#include <Eigen/Eigenvalues>
#include <cmath>

namespace hydra { namespace math {

ComplexMatrix matrixExp(const ComplexMatrix& A) {
    return A.exp();  // requires unsupported module
}

ComplexMatrix matrixPow(const ComplexMatrix& A, int n) {
    ComplexMatrix result = ComplexMatrix::Identity(A.rows(), A.cols());
    for (int i = 0; i < n; ++i)
        result *= A;
    return result;
}

double frobeniusNorm(const ComplexMatrix& A) {
    return A.norm();  // same as Frobenius for complex
}

double matrixDistance(const ComplexMatrix& A, const ComplexMatrix& B) {
    return (A - B).norm();
}

bool isUnitary(const ComplexMatrix& A, double epsilon) {
    ComplexMatrix id = ComplexMatrix::Identity(A.rows(), A.cols());
    return ((A * A.adjoint() - id).norm() < epsilon);
}

bool isHermitian(const ComplexMatrix& A, double epsilon) {
    return ((A - A.adjoint()).norm() < epsilon);
}

ComplexVector basisState(int index, int dim) {
    ComplexVector v = ComplexVector::Zero(dim);
    v(index) = Complex(1.0, 0.0);
    return v;
}

ComplexVector uniformSuperposition(int dim) {
    return ComplexVector::Constant(dim, Complex(1.0 / std::sqrt(dim), 0));
}

ComplexMatrix identity(int dim) {
    return ComplexMatrix::Identity(dim, dim);
}

ComplexMatrix pauliX() {
    ComplexMatrix m(2, 2);
    m << 0, 1,
         1, 0;
    return m;
}

ComplexMatrix hadamard() {
    double s = 1.0 / std::sqrt(2);
    ComplexMatrix h(2, 2);
    h << s, s,
         s, -s;
    return h;
}

}} // namespace hydra::math
