#pragma once

#include "lmvs/layered_vector.hpp"
#include <vector>
#include <stdexcept>
#include <memory>
#include <string>
#include <cmath>
#include <iostream>

namespace lmvs {

/**
 * @brief Class representing a layered matrix as described in the LMVS paper.
 *
 * A layered matrix consists of block matrices, each representing interactions
 * between specific layers of two vectors.
 */
class LayeredMatrix {
public:
    /**
     * @brief Construct a new Layered Matrix with specified dimensions
     *
     * @param num_layers Number of layers (both rows and columns)
     * @param row_dimension Dimension of each row layer
     * @param col_dimension Dimension of each column layer
     */
    LayeredMatrix(size_t num_layers, size_t row_dimension, size_t col_dimension);

    /**
     * @brief Construct a new Layered Matrix from existing data
     *
     * @param data 4D vector representing the block matrices
     */
    explicit LayeredMatrix(const std::vector<std::vector<std::vector<std::vector<double>>>>& data);

    /**
     * @brief Construct a Layered Matrix from two Layered Vectors
     *
     * @param vec_a First layered vector
     * @param vec_b Second layered vector
     */
    LayeredMatrix(const LayeredVector& vec_a, const LayeredVector& vec_b);

    /**
     * @brief Get the number of layers in the matrix
     *
     * @return size_t Number of layers
     */
    size_t getNumLayers() const { return m_blocks.size(); }

    /**
     * @brief Get the row dimension of each block
     *
     * @return size_t Row dimension
     */
    size_t getRowDimension() const {
        return m_blocks.empty() || m_blocks[0].empty() ? 0 : m_blocks[0][0].size();
    }

    /**
     * @brief Get the column dimension of each block
     *
     * @return size_t Column dimension
     */
    size_t getColDimension() const {
        return m_blocks.empty() || m_blocks[0].empty() || m_blocks[0][0].empty() ? 0 : m_blocks[0][0][0].size();
    }

    /**
     * @brief Get a specific block of the matrix
     *
     * @param row_layer Row layer index
     * @param col_layer Column layer index
     * @return const std::vector<std::vector<double>>& Reference to the block
     * @throws std::out_of_range if indices are out of bounds
     */
    const std::vector<std::vector<double>>& getBlock(size_t row_layer, size_t col_layer) const;

    /**
     * @brief Get a mutable reference to a specific block
     *
     * @param row_layer Row layer index
     * @param col_layer Column layer index
     * @return std::vector<std::vector<double>>& Mutable reference to the block
     * @throws std::out_of_range if indices are out of bounds
     */
    std::vector<std::vector<double>>& getBlockMutable(size_t row_layer, size_t col_layer);

    /**
     * @brief Set the values of a specific block
     *
     * @param row_layer Row layer index
     * @param col_layer Column layer index
     * @param values New values for the block
     * @throws std::out_of_range if indices are out of bounds
     * @throws std::invalid_argument if dimensions don't match
     */
    void setBlock(size_t row_layer, size_t col_layer, const std::vector<std::vector<double>>& values);

    /**
     * @brief Calculate the outer product of two vectors and store as a block
     *
     * @param row_layer Row layer index
     * @param col_layer Column layer index
     * @param vec_a First vector
     * @param vec_b Second vector
     * @throws std::out_of_range if indices are out of bounds
     * @throws std::invalid_argument if dimensions don't match
     */
    void setBlockFromOuterProduct(size_t row_layer, size_t col_layer,
                                 const std::vector<double>& vec_a,
                                 const std::vector<double>& vec_b);

    /**
     * @brief Print the matrix to the console for debugging
     */
    void print() const;

private:
    // 3D vector: [row_layer][col_layer][row][col]
    std::vector<std::vector<std::vector<std::vector<double>>>> m_blocks;
};

} // namespace lmvs
