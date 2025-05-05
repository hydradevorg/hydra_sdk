#include "hydra_math/latticesolver.hpp"
#include "hydra_math/latticeutils.hpp"

namespace hydra { namespace math {

Eigen::VectorXd svpBabai(const Eigen::MatrixXd& basis) {
    Eigen::MatrixXd G = gramSchmidt(basis);
    return G.col(0); // placeholder: shortest basis vector
}

Eigen::MatrixXd bkzReduce(const Eigen::MatrixXd& basis, int blockSize) {
    // Future: LLL + BKZ implementation
    return basis; // placeholder
}

}} // namespace hydra::math
