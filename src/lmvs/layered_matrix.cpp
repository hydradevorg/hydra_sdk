#include "lmvs/layered_matrix.hpp"

namespace lmvs {

LayeredMatrix::LayeredMatrix(size_t num_layers, size_t row_dimension, size_t col_dimension) {
    m_blocks.resize(num_layers);
    for (auto& row_layer : m_blocks) {
        row_layer.resize(num_layers);
        for (auto& block : row_layer) {
            block.resize(row_dimension);
            for (auto& row : block) {
                row.resize(col_dimension, 0.0);
            }
        }
    }
}

LayeredMatrix::LayeredMatrix(const std::vector<std::vector<std::vector<std::vector<double>>>>& data) {
    if (data.empty()) {
        return;
    }

    // Check that all blocks have the same dimensions
    const size_t num_layers = data.size();
    for (const auto& row_layer : data) {
        if (row_layer.size() != num_layers) {
            throw std::invalid_argument("All row layers must have the same number of column layers");
        }
    }

    // Check that all blocks have the same row dimension
    const size_t row_dim = data[0][0].size();
    // Check that all blocks have the same column dimension
    const size_t col_dim = data[0][0].empty() ? 0 : data[0][0][0].size();

    for (size_t i = 0; i < num_layers; ++i) {
        for (size_t j = 0; j < num_layers; ++j) {
            if (data[i][j].size() != row_dim) {
                throw std::invalid_argument("All blocks must have the same row dimension");
            }
            for (const auto& row : data[i][j]) {
                if (row.size() != col_dim) {
                    throw std::invalid_argument("All rows must have the same column dimension");
                }
            }
        }
    }

    m_blocks = data;
}

LayeredMatrix::LayeredMatrix(const LayeredVector& vec_a, const LayeredVector& vec_b) {
    const size_t num_layers = vec_a.getNumLayers();
    
    if (num_layers != vec_b.getNumLayers()) {
        throw std::invalid_argument("Both vectors must have the same number of layers");
    }
    
    m_blocks.resize(num_layers);
    for (size_t i = 0; i < num_layers; ++i) {
        m_blocks[i].resize(num_layers);
        for (size_t j = 0; j < num_layers; ++j) {
            const auto& layer_a = vec_a.getLayer(i);
            const auto& layer_b = vec_b.getLayer(j);
            
            // Compute outer product of layer_a and layer_b
            std::vector<std::vector<double>> outer_product(layer_a.size(), std::vector<double>(layer_b.size()));
            for (size_t r = 0; r < layer_a.size(); ++r) {
                for (size_t c = 0; c < layer_b.size(); ++c) {
                    outer_product[r][c] = layer_a[r] * layer_b[c];
                }
            }
            
            m_blocks[i][j] = outer_product;
        }
    }
}

const std::vector<std::vector<double>>& LayeredMatrix::getBlock(size_t row_layer, size_t col_layer) const {
    if (row_layer >= m_blocks.size() || col_layer >= m_blocks[0].size()) {
        throw std::out_of_range("Block indices out of range");
    }
    return m_blocks[row_layer][col_layer];
}

std::vector<std::vector<double>>& LayeredMatrix::getBlockMutable(size_t row_layer, size_t col_layer) {
    if (row_layer >= m_blocks.size() || col_layer >= m_blocks[0].size()) {
        throw std::out_of_range("Block indices out of range");
    }
    return m_blocks[row_layer][col_layer];
}

void LayeredMatrix::setBlock(size_t row_layer, size_t col_layer, const std::vector<std::vector<double>>& values) {
    if (row_layer >= m_blocks.size() || col_layer >= m_blocks[0].size()) {
        throw std::out_of_range("Block indices out of range");
    }
    
    if (values.size() != m_blocks[row_layer][col_layer].size()) {
        throw std::invalid_argument("Block row dimension mismatch");
    }
    
    for (size_t i = 0; i < values.size(); ++i) {
        if (values[i].size() != m_blocks[row_layer][col_layer][i].size()) {
            throw std::invalid_argument("Block column dimension mismatch");
        }
    }
    
    m_blocks[row_layer][col_layer] = values;
}

void LayeredMatrix::setBlockFromOuterProduct(size_t row_layer, size_t col_layer, 
                                           const std::vector<double>& vec_a, 
                                           const std::vector<double>& vec_b) {
    if (row_layer >= m_blocks.size() || col_layer >= m_blocks[0].size()) {
        throw std::out_of_range("Block indices out of range");
    }
    
    if (vec_a.size() != m_blocks[row_layer][col_layer].size()) {
        throw std::invalid_argument("Vector A dimension mismatch");
    }
    
    if (!m_blocks[row_layer][col_layer].empty() && vec_b.size() != m_blocks[row_layer][col_layer][0].size()) {
        throw std::invalid_argument("Vector B dimension mismatch");
    }
    
    // Compute outer product
    std::vector<std::vector<double>> outer_product(vec_a.size(), std::vector<double>(vec_b.size()));
    for (size_t i = 0; i < vec_a.size(); ++i) {
        for (size_t j = 0; j < vec_b.size(); ++j) {
            outer_product[i][j] = vec_a[i] * vec_b[j];
        }
    }
    
    m_blocks[row_layer][col_layer] = outer_product;
}

void LayeredMatrix::print() const {
    std::cout << "LayeredMatrix with " << getNumLayers() << " layers, each block of size " 
              << getRowDimension() << "x" << getColDimension() << std::endl;
    
    for (size_t i = 0; i < getNumLayers(); ++i) {
        for (size_t j = 0; j < getNumLayers(); ++j) {
            std::cout << "Block [" << i << "][" << j << "]:" << std::endl;
            const auto& block = getBlock(i, j);
            for (const auto& row : block) {
                std::cout << "  [";
                for (size_t k = 0; k < row.size(); ++k) {
                    std::cout << row[k];
                    if (k < row.size() - 1) {
                        std::cout << ", ";
                    }
                }
                std::cout << "]" << std::endl;
            }
        }
    }
}

} // namespace lmvs
