#pragma once

#include <Eigen/Dense>

namespace hydra { namespace math {

/**
 * @brief Performs Block Korkine-Zolotarev (BKZ) lattice basis reduction
 *
 * BKZ is a lattice reduction algorithm that improves upon LLL by incorporating
 * stronger reduction techniques within local blocks of the basis. It produces
 * bases that are shorter and more orthogonal than those produced by LLL.
 *
 * @tparam MatrixType The matrix type (typically Eigen::MatrixXd or Eigen::MatrixXi)
 * @param B The input lattice basis matrix, with each column representing a basis vector
 * @param beta The block size parameter (larger values produce better bases but are more expensive)
 * @param delta The LLL delta parameter (typically between 0.5 and 1.0)
 * @return The reduced lattice basis
 */
template<typename MatrixType>
MatrixType bkzReduce(const MatrixType& B, int beta = 20, double delta = 0.99);

// Explicit instantiations
/**
 * @brief Integer matrix specialization of BKZ reduction
 * @param B Integer lattice basis matrix
 * @param beta Block size parameter
 * @param delta LLL delta parameter
 * @return Reduced integer lattice basis
 */
extern template Eigen::MatrixXi bkzReduce(const Eigen::MatrixXi&, int, double);

/**
 * @brief Double-precision floating-point matrix specialization of BKZ reduction
 * @param B Double-precision lattice basis matrix
 * @param beta Block size parameter
 * @param delta LLL delta parameter
 * @return Reduced double-precision lattice basis
 */
extern template Eigen::MatrixXd bkzReduce(const Eigen::MatrixXd&, int, double);

}} // namespace hydra::math
