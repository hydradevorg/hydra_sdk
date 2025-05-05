#pragma once

#include <Eigen/Dense>
#include <type_traits>

namespace hydra { namespace math {

/**
 * @brief Performs Lenstra-Lenstra-Lovász (LLL) lattice basis reduction
 *
 * LLL is a polynomial-time lattice reduction algorithm that produces a basis with
 * vectors that are relatively short and somewhat orthogonal. It is widely used in
 * cryptography, cryptanalysis, and various computational problems involving lattices.
 *
 * @tparam MatrixType The matrix type (typically Eigen::MatrixXd or Eigen::MatrixXi)
 * @param B The input lattice basis matrix, with each column representing a basis vector
 * @param delta The LLL parameter (typically between 0.5 and 1.0)
 * @return The reduced lattice basis
 */
template<typename MatrixType>
MatrixType lllReduce(const MatrixType& B, double delta = 0.99);

// Specializations declared here (implemented in .cpp)
/**
 * @brief Integer matrix specialization of LLL reduction
 * @param B Integer lattice basis matrix
 * @param delta LLL parameter
 * @return Reduced integer lattice basis
 */
extern template Eigen::MatrixXi lllReduce(const Eigen::MatrixXi&, double);

/**
 * @brief Double-precision floating-point matrix specialization of LLL reduction
 * @param B Double-precision lattice basis matrix
 * @param delta LLL parameter
 * @return Reduced double-precision lattice basis
 */
extern template Eigen::MatrixXd lllReduce(const Eigen::MatrixXd&, double);

}} // namespace hydra::math
