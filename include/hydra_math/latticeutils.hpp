#pragma once

#include "hydra_math/complexmatrix.hpp"
#include "hydra_math/bigint.hpp"
#include <vector>

namespace hydra { namespace math {

/**
 * @brief Performs the Gram-Schmidt orthogonalization process on a matrix
 *
 * The Gram-Schmidt process takes a set of vectors and creates an orthogonal
 * set of vectors that span the same subspace.
 *
 * @param B Input matrix whose columns are the vectors to orthogonalize
 * @return Orthogonalized matrix with the same dimensions as B
 */
Eigen::MatrixXd gramSchmidt(const Eigen::MatrixXd& B);

/**
 * @brief Computes the modulo of each element in a matrix
 *
 * This function applies the modulo operation to each element of the input matrix,
 * ensuring the result is in the range [0, q-1].
 *
 * @param A Input matrix to apply modulo operation to
 * @param q The modulus to use
 * @return Matrix with each element being the modulo q of the corresponding element in A
 */
Eigen::MatrixXi modMatrix(const Eigen::MatrixXi& A, int q);

}} // namespace hydra::math
