#pragma once

#include "lmvs/layered_vector.hpp"
#include <vector>
#include <stdexcept>
#include <memory>
#include <random>
#include <cmath>

namespace lmvs {

/**
 * @brief Class representing a projection matrix for dimensional reduction.
 * 
 * This class implements the projection matrix P ∈ ℝ^(k×n) that projects
 * vectors from ℝ^n to ℝ^k as described in the LMVS paper.
 */
class ProjectionMatrix {
public:
    /**
     * @brief Construct a new Projection Matrix with specified dimensions
     * 
     * @param input_dim Input dimension (n)
     * @param output_dim Output dimension (k)
     */
    ProjectionMatrix(size_t input_dim, size_t output_dim);

    /**
     * @brief Construct a new Projection Matrix from existing data
     * 
     * @param data 2D vector representing the projection matrix
     */
    explicit ProjectionMatrix(const std::vector<std::vector<double>>& data);

    /**
     * @brief Create a random projection matrix
     * 
     * @param input_dim Input dimension (n)
     * @param output_dim Output dimension (k)
     * @param seed Random seed (optional)
     * @return ProjectionMatrix Random projection matrix
     */
    static ProjectionMatrix createRandom(size_t input_dim, size_t output_dim, unsigned int seed = 0);

    /**
     * @brief Create an orthogonal projection matrix
     * 
     * @param input_dim Input dimension (n)
     * @param output_dim Output dimension (k)
     * @param seed Random seed (optional)
     * @return ProjectionMatrix Orthogonal projection matrix
     */
    static ProjectionMatrix createOrthogonal(size_t input_dim, size_t output_dim, unsigned int seed = 0);

    /**
     * @brief Get the input dimension
     * 
     * @return size_t Input dimension (n)
     */
    size_t getInputDimension() const { return m_matrix.empty() ? 0 : m_matrix[0].size(); }

    /**
     * @brief Get the output dimension
     * 
     * @return size_t Output dimension (k)
     */
    size_t getOutputDimension() const { return m_matrix.size(); }

    /**
     * @brief Get the projection matrix
     * 
     * @return const std::vector<std::vector<double>>& Reference to the matrix
     */
    const std::vector<std::vector<double>>& getMatrix() const { return m_matrix; }

    /**
     * @brief Project a vector from ℝ^n to ℝ^k
     * 
     * @param vector Input vector in ℝ^n
     * @return std::vector<double> Projected vector in ℝ^k
     * @throws std::invalid_argument if vector dimension doesn't match input dimension
     */
    std::vector<double> project(const std::vector<double>& vector) const;

    /**
     * @brief Project a layered vector, applying the projection to each layer
     * 
     * @param layered_vector Input layered vector
     * @return LayeredVector Projected layered vector
     * @throws std::invalid_argument if layer dimension doesn't match input dimension
     */
    LayeredVector project(const LayeredVector& layered_vector) const;

    /**
     * @brief Print the projection matrix to the console for debugging
     */
    void print() const;

private:
    std::vector<std::vector<double>> m_matrix; // Projection matrix P
};

} // namespace lmvs
