#pragma once

#include <Eigen/Dense>
#include <string>

namespace hydra { namespace math {

// Shortest vector problem (placeholder)
Eigen::VectorXd svpBabai(const Eigen::MatrixXd& basis);

// BKZ placeholder
Eigen::MatrixXd bkzReduce(const Eigen::MatrixXd& basis, int blockSize);

}} // namespace hydra::math
