#include "lmvs/layered_bigint_matrix.hpp"
#include <sstream>
#include <iomanip>
#include <cstring>

namespace lmvs {

LayeredBigIntMatrix::LayeredBigIntMatrix(size_t num_layers, size_t row_dimension, size_t col_dimension) {
    m_blocks.resize(num_layers);
    for (auto& row_layer : m_blocks) {
        row_layer.resize(num_layers);
        for (auto& block : row_layer) {
            block.resize(row_dimension);
            for (auto& row : block) {
                row.resize(col_dimension, hydra::math::BigInt(0));
            }
        }
    }
}

LayeredBigIntMatrix::LayeredBigIntMatrix(const LayeredBigIntVector& vec_a, const LayeredBigIntVector& vec_b) {
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
            std::vector<std::vector<hydra::math::BigInt>> outer_product(
                layer_a.getDimension(), 
                std::vector<hydra::math::BigInt>(layer_b.getDimension())
            );
            
            for (size_t r = 0; r < layer_a.getDimension(); ++r) {
                for (size_t c = 0; c < layer_b.getDimension(); ++c) {
                    outer_product[r][c] = layer_a.getValue(r) * layer_b.getValue(c);
                }
            }
            
            m_blocks[i][j] = outer_product;
        }
    }
}

const std::vector<std::vector<hydra::math::BigInt>>& LayeredBigIntMatrix::getBlock(size_t row_layer, size_t col_layer) const {
    if (row_layer >= m_blocks.size() || col_layer >= m_blocks[0].size()) {
        throw std::out_of_range("Block indices out of range");
    }
    return m_blocks[row_layer][col_layer];
}

std::vector<std::vector<hydra::math::BigInt>>& LayeredBigIntMatrix::getBlockMutable(size_t row_layer, size_t col_layer) {
    if (row_layer >= m_blocks.size() || col_layer >= m_blocks[0].size()) {
        throw std::out_of_range("Block indices out of range");
    }
    return m_blocks[row_layer][col_layer];
}

void LayeredBigIntMatrix::setBlock(size_t row_layer, size_t col_layer, const std::vector<std::vector<hydra::math::BigInt>>& values) {
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

void LayeredBigIntMatrix::setBlockFromOuterProduct(size_t row_layer, size_t col_layer, 
                                                 const BigIntVector& vec_a, 
                                                 const BigIntVector& vec_b) {
    if (row_layer >= m_blocks.size() || col_layer >= m_blocks[0].size()) {
        throw std::out_of_range("Block indices out of range");
    }
    
    if (vec_a.getDimension() != m_blocks[row_layer][col_layer].size()) {
        throw std::invalid_argument("Vector A dimension mismatch");
    }
    
    if (!m_blocks[row_layer][col_layer].empty() && vec_b.getDimension() != m_blocks[row_layer][col_layer][0].size()) {
        throw std::invalid_argument("Vector B dimension mismatch");
    }
    
    // Compute outer product
    std::vector<std::vector<hydra::math::BigInt>> outer_product(
        vec_a.getDimension(), 
        std::vector<hydra::math::BigInt>(vec_b.getDimension())
    );
    
    for (size_t i = 0; i < vec_a.getDimension(); ++i) {
        for (size_t j = 0; j < vec_b.getDimension(); ++j) {
            outer_product[i][j] = vec_a.getValue(i) * vec_b.getValue(j);
        }
    }
    
    m_blocks[row_layer][col_layer] = outer_product;
}

std::vector<uint8_t> LayeredBigIntMatrix::serialize() const {
    std::vector<uint8_t> result;
    
    // First, store the dimensions
    size_t num_layers = m_blocks.size();
    size_t row_dim = getRowDimension();
    size_t col_dim = getColDimension();
    
    result.resize(3 * sizeof(size_t));
    std::memcpy(result.data(), &num_layers, sizeof(size_t));
    std::memcpy(result.data() + sizeof(size_t), &row_dim, sizeof(size_t));
    std::memcpy(result.data() + 2 * sizeof(size_t), &col_dim, sizeof(size_t));
    
    // Then, store each block
    for (size_t i = 0; i < num_layers; ++i) {
        for (size_t j = 0; j < num_layers; ++j) {
            const auto& block = m_blocks[i][j];
            
            for (size_t r = 0; r < row_dim; ++r) {
                for (size_t c = 0; c < col_dim; ++c) {
                    // Store each BigInt as a string
                    std::string str = block[r][c].to_string();
                    
                    // Store the length of the string
                    size_t str_len = str.size();
                    size_t offset = result.size();
                    result.resize(offset + sizeof(size_t));
                    std::memcpy(result.data() + offset, &str_len, sizeof(size_t));
                    
                    // Store the string itself
                    offset = result.size();
                    result.resize(offset + str_len);
                    std::memcpy(result.data() + offset, str.data(), str_len);
                }
            }
        }
    }
    
    return result;
}

LayeredBigIntMatrix LayeredBigIntMatrix::deserialize(const std::vector<uint8_t>& data) {
    if (data.size() < 3 * sizeof(size_t)) {
        throw std::invalid_argument("Invalid serialized data");
    }
    
    // Read the dimensions
    size_t num_layers, row_dim, col_dim;
    std::memcpy(&num_layers, data.data(), sizeof(size_t));
    std::memcpy(&row_dim, data.data() + sizeof(size_t), sizeof(size_t));
    std::memcpy(&col_dim, data.data() + 2 * sizeof(size_t), sizeof(size_t));
    
    LayeredBigIntMatrix matrix(num_layers, row_dim, col_dim);
    
    size_t offset = 3 * sizeof(size_t);
    for (size_t i = 0; i < num_layers; ++i) {
        for (size_t j = 0; j < num_layers; ++j) {
            auto& block = matrix.getBlockMutable(i, j);
            
            for (size_t r = 0; r < row_dim; ++r) {
                for (size_t c = 0; c < col_dim; ++c) {
                    if (offset + sizeof(size_t) > data.size()) {
                        throw std::invalid_argument("Invalid serialized data");
                    }
                    
                    // Read the length of the string
                    size_t str_len;
                    std::memcpy(&str_len, data.data() + offset, sizeof(size_t));
                    offset += sizeof(size_t);
                    
                    if (offset + str_len > data.size()) {
                        throw std::invalid_argument("Invalid serialized data");
                    }
                    
                    // Read the string and convert to BigInt
                    std::string str(reinterpret_cast<const char*>(data.data() + offset), str_len);
                    offset += str_len;
                    
                    block[r][c] = hydra::math::BigInt(str);
                }
            }
        }
    }
    
    return matrix;
}

std::vector<uint8_t> LayeredBigIntMatrix::compress() const {
    // First, serialize the matrix
    std::vector<uint8_t> serialized = serialize();
    
    // Simple run-length encoding for compression
    std::vector<uint8_t> compressed;
    compressed.reserve(serialized.size()); // Worst case: no compression
    
    size_t i = 0;
    while (i < serialized.size()) {
        uint8_t current = serialized[i];
        uint8_t count = 1;
        
        // Count consecutive identical bytes
        while (i + count < serialized.size() && 
               serialized[i + count] == current && 
               count < 255) {
            ++count;
        }
        
        // If run length is at least 4, use RLE
        if (count >= 4) {
            compressed.push_back(0); // Marker for RLE
            compressed.push_back(count);
            compressed.push_back(current);
            i += count;
        } else {
            // Otherwise, store the literal bytes
            compressed.push_back(count); // Literal count
            for (uint8_t j = 0; j < count; ++j) {
                compressed.push_back(serialized[i + j]);
            }
            i += count;
        }
    }
    
    return compressed;
}

LayeredBigIntMatrix LayeredBigIntMatrix::decompress(const std::vector<uint8_t>& compressed_data) {
    std::vector<uint8_t> decompressed;
    
    size_t i = 0;
    while (i < compressed_data.size()) {
        uint8_t marker = compressed_data[i++];
        
        if (marker == 0) {
            // RLE block
            if (i + 1 >= compressed_data.size()) {
                throw std::invalid_argument("Invalid compressed data");
            }
            
            uint8_t count = compressed_data[i++];
            uint8_t value = compressed_data[i++];
            
            for (uint8_t j = 0; j < count; ++j) {
                decompressed.push_back(value);
            }
        } else {
            // Literal block
            uint8_t count = marker;
            
            if (i + count > compressed_data.size()) {
                throw std::invalid_argument("Invalid compressed data");
            }
            
            for (uint8_t j = 0; j < count; ++j) {
                decompressed.push_back(compressed_data[i + j]);
            }
            
            i += count;
        }
    }
    
    return deserialize(decompressed);
}

void LayeredBigIntMatrix::print() const {
    std::cout << "LayeredBigIntMatrix with " << getNumLayers() << " layers, each block of size " 
              << getRowDimension() << "x" << getColDimension() << std::endl;
    
    for (size_t i = 0; i < getNumLayers(); ++i) {
        for (size_t j = 0; j < getNumLayers(); ++j) {
            std::cout << "Block [" << i << "][" << j << "]:" << std::endl;
            const auto& block = getBlock(i, j);
            for (const auto& row : block) {
                std::cout << "  [";
                for (size_t k = 0; k < row.size(); ++k) {
                    std::cout << row[k].to_string();
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
