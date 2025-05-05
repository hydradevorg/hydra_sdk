#include "hydra_math/complexmatrix.hpp"
#include <unsupported/Eigen/KroneckerProduct>
#include <sstream>
#include <iomanip>


namespace hydra { namespace math {

ComplexMatrix tensor(const ComplexMatrix& A, const ComplexMatrix& B) {
    return Eigen::kroneckerProduct(A, B).eval();
}

ComplexMatrix dagger(const ComplexMatrix& A) {
    return A.adjoint();  // Hermitian transpose
}

std::string toString(const ComplexMatrix& mat) {
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(3);
    for (int i = 0; i < mat.rows(); ++i) {
        ss << "[ ";
        for (int j = 0; j < mat.cols(); ++j) {
            ss << mat(i, j) << " ";
        }
        ss << "]\n";
    }
    return ss.str();
}

std::string toString(const ComplexVector& vec) {
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(3);
    ss << "[ ";
    for (int i = 0; i < vec.size(); ++i) {
        ss << vec(i) << " ";
    }
    ss << "]";
    return ss.str();
}

}} // namespace hydra::math
