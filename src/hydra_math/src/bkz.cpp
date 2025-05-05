#include "hydra_math/bkz.hpp"
#include "hydra_math/lll.hpp"

namespace hydra { namespace math {

template<typename MatrixType>
MatrixType bkzReduce(const MatrixType& B_in, int beta, double delta) {
    using Scalar = typename MatrixType::Scalar;
    const int n = B_in.rows();
    const int m = B_in.cols();

    MatrixType B = lllReduce(B_in, delta); // Preprocessing
    Eigen::Matrix<Scalar, Eigen::Dynamic, 1> Bnorm(m);
    for (int i = 0; i < m; ++i) {
        Bnorm(i) = B.col(i).squaredNorm();
    }

    for (int k = 0; k <= m - beta; ++k) {
        int end = std::min(k + beta, m);
        MatrixType sub = B.middleCols(k, end - k);
        sub = lllReduce(sub, delta);  // Local block LLL

        // Reinsert reduced block
        B.middleCols(k, end - k) = sub;

        // TODO: ENUM phase here (placeholder)
        // For now, skip real enumeration
    }

    return B;
}

template Eigen::MatrixXi bkzReduce(const Eigen::MatrixXi&, int, double);
template Eigen::MatrixXd bkzReduce(const Eigen::MatrixXd&, int, double);

}} // namespace hydra::math
