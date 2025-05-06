#include "lmvs/projection_matrix.hpp"
#include <iostream>
#include <algorithm>
#include <numeric>

namespace lmvs {

ProjectionMatrix::ProjectionMatrix(size_t input_dim, size_t output_dim) {
    if (output_dim > input_dim) {
        throw std::invalid_argument("Output dimension cannot be larger than input dimension");
    }
    
    m_matrix.resize(output_dim);
    for (auto& row : m_matrix) {
        row.resize(input_dim, 0.0);
    }
    
    // Initialize as identity matrix (truncated if output_dim < input_dim)
    for (size_t i = 0; i < output_dim; ++i) {
        m_matrix[i][i] = 1.0;
    }
}

ProjectionMatrix::ProjectionMatrix(const std::vector<std::vector<double>>& data) {
    if (data.empty()) {
        throw std::invalid_argument("Projection matrix cannot be empty");
    }
    
    const size_t output_dim = data.size();
    const size_t input_dim = data[0].size();
    
    // Check that all rows have the same dimension
    for (const auto& row : data) {
        if (row.size() != input_dim) {
            throw std::invalid_argument("All rows must have the same dimension");
        }
    }
    
    m_matrix = data;
}

ProjectionMatrix ProjectionMatrix::createRandom(size_t input_dim, size_t output_dim, unsigned int seed) {
    if (output_dim > input_dim) {
        throw std::invalid_argument("Output dimension cannot be larger than input dimension");
    }
    
    std::mt19937 gen(seed);
    std::normal_distribution<double> dist(0.0, 1.0 / std::sqrt(input_dim));
    
    std::vector<std::vector<double>> matrix(output_dim, std::vector<double>(input_dim));
    for (auto& row : matrix) {
        for (auto& val : row) {
            val = dist(gen);
        }
    }
    
    return ProjectionMatrix(matrix);
}

ProjectionMatrix ProjectionMatrix::createOrthogonal(size_t input_dim, size_t output_dim, unsigned int seed) {
    if (output_dim > input_dim) {
        throw std::invalid_argument("Output dimension cannot be larger than input dimension");
    }
    
    // Start with a random matrix
    auto proj = createRandom(input_dim, output_dim, seed);
    auto& matrix = proj.m_matrix;
    
    // Gram-Schmidt orthogonalization
    for (size_t i = 0; i < output_dim; ++i) {
        // Normalize the current row
        double norm = 0.0;
        for (size_t j = 0; j < input_dim; ++j) {
            norm += matrix[i][j] * matrix[i][j];
        }
        norm = std::sqrt(norm);
        
        for (size_t j = 0; j < input_dim; ++j) {
            matrix[i][j] /= norm;
        }
        
        // Make all subsequent rows orthogonal to this one
        for (size_t k = i + 1; k < output_dim; ++k) {
            // Calculate dot product
            double dot = 0.0;
            for (size_t j = 0; j < input_dim; ++j) {
                dot += matrix[i][j] * matrix[k][j];
            }
            
            // Subtract projection
            for (size_t j = 0; j < input_dim; ++j) {
                matrix[k][j] -= dot * matrix[i][j];
            }
        }
    }
    
    return proj;
}

std::vector<double> ProjectionMatrix::project(const std::vector<double>& vector) const {
    const size_t input_dim = getInputDimension();
    const size_t output_dim = getOutputDimension();
    
    if (vector.size() != input_dim) {
        throw std::invalid_argument("Vector dimension mismatch");
    }
    
    std::vector<double> result(output_dim, 0.0);
    
    // Matrix-vector multiplication
    for (size_t i = 0; i < output_dim; ++i) {
        for (size_t j = 0; j < input_dim; ++j) {
            result[i] += m_matrix[i][j] * vector[j];
        }
    }
    
    return result;
}

LayeredVector ProjectionMatrix::project(const LayeredVector& layered_vector) const {
    const size_t num_layers = layered_vector.getNumLayers();
    const size_t input_dim = getInputDimension();
    
    if (layered_vector.getDimension() != input_dim) {
        throw std::invalid_argument("Layer dimension mismatch");
    }
    
    std::vector<std::vector<double>> projected_layers;
    projected_layers.reserve(num_layers);
    
    for (size_t i = 0; i < num_layers; ++i) {
        projected_layers.push_back(project(layered_vector.getLayer(i)));
    }
    
    return LayeredVector(projected_layers);
}

void ProjectionMatrix::print() const {
    std::cout << "ProjectionMatrix " << getOutputDimension() << "x" << getInputDimension() << std::endl;
    
    for (const auto& row : m_matrix) {
        std::cout << "  [";
        for (size_t i = 0; i < row.size(); ++i) {
            std::cout << row[i];
            if (i < row.size() - 1) {
                std::cout << ", ";
            }
        }
        std::cout << "]" << std::endl;
    }
}

} // namespace lmvs
