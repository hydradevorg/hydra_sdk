#include "hydra_math/lll.hpp"

namespace hydra { namespace math {

template<typename MatrixType>
MatrixType lllReduce(const MatrixType& B_in, double delta) {
    using Scalar = typename MatrixType::Scalar;
    const int n = B_in.rows();
    const int m = B_in.cols();

    MatrixType B = B_in;
    Eigen::Matrix<Scalar, Eigen::Dynamic, Eigen::Dynamic> mu(m, m);
    Eigen::Matrix<Scalar, Eigen::Dynamic, 1> Bnorm(m);

    auto gramSchmidt = [&]() {
        for (int i = 0; i < m; ++i) {
            auto vi = B.col(i);
            auto proj = vi;
            for (int j = 0; j < i; ++j) {
                Scalar dot = proj.dot(B.col(j));
                mu(i, j) = dot / Bnorm(j);
                proj -= mu(i, j) * B.col(j);
            }
            Bnorm(i) = proj.squaredNorm();
        }
    };

    gramSchmidt();

    int k = 1;
    while (k < m) {
        for (int j = k - 1; j >= 0; --j) {
            Scalar q = std::round(mu(k, j));
            B.col(k) -= q * B.col(j);
        }

        gramSchmidt();

        Scalar lhs = Bnorm(k);
        Scalar rhs = (delta - mu(k, k - 1) * mu(k, k - 1)) * Bnorm(k - 1);
        if (lhs < rhs) {
            B.col(k).swap(B.col(k - 1));
            gramSchmidt();
            k = std::max(1, k - 1);
        } else {
            ++k;
        }
    }

    return B;
}

// Explicit instantiations
template Eigen::MatrixXi lllReduce(const Eigen::MatrixXi&, double);
template Eigen::MatrixXd lllReduce(const Eigen::MatrixXd&, double);

}} // namespace hydra::math
