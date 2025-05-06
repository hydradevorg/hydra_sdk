#include <hydra_address/layered_matrix.hpp>
#include <cmath>
#include <algorithm>
#include <stdexcept>
#include <random>
#include <sstream>

namespace hydra {
namespace address {

// Vector implementation
Vector::Vector() {}

Vector::Vector(size_t dimension) : m_data(dimension, hydra::math::BigInt(0)) {}

Vector::Vector(const std::vector<hydra::math::BigInt>& data) : m_data(data) {}

size_t Vector::getDimension() const {
    return m_data.size();
}

const std::vector<hydra::math::BigInt>& Vector::getData() const {
    return m_data;
}

void Vector::setElement(size_t index, const hydra::math::BigInt& value) {
    if (index >= m_data.size()) {
        throw std::out_of_range("Vector index out of range");
    }
    m_data[index] = value;
}

hydra::math::BigInt Vector::getElement(size_t index) const {
    if (index >= m_data.size()) {
        throw std::out_of_range("Vector index out of range");
    }
    return m_data[index];
}

void Vector::normalize() {
    // Calculate the sum of squares
    hydra::math::BigInt sum_of_squares(0);
    for (const auto& val : m_data) {
        sum_of_squares = sum_of_squares + (val * val);
    }

    // Calculate the square root (approximate for BigInt)
    double sqrt_val = std::sqrt(std::stod(sum_of_squares.to_string()));
    hydra::math::BigInt norm(static_cast<int64_t>(sqrt_val));

    // Check if norm is greater than zero
    hydra::math::BigInt zero(0);
    if (norm > zero) {
        // Divide each element by the norm
        for (auto& val : m_data) {
            val = val / norm;
        }
    }
}

std::vector<uint8_t> Vector::compress() const {
    // Simple serialization for now
    std::vector<uint8_t> result;

    // Add the dimension (4 bytes)
    uint32_t dimension = static_cast<uint32_t>(m_data.size());
    result.push_back((dimension >> 24) & 0xFF);
    result.push_back((dimension >> 16) & 0xFF);
    result.push_back((dimension >> 8) & 0xFF);
    result.push_back(dimension & 0xFF);

    // Add each element
    for (const auto& val : m_data) {
        // Convert BigInt to string
        std::string str = val.to_string();

        // Add string length (1 byte)
        result.push_back(static_cast<uint8_t>(str.length()));

        // Add string data
        result.insert(result.end(), str.begin(), str.end());
    }

    return result;
}

std::optional<Vector> Vector::decompress(const std::vector<uint8_t>& data) {
    if (data.size() < 4) {
        return std::nullopt;
    }

    // Extract dimension
    uint32_t dimension = (static_cast<uint32_t>(data[0]) << 24) |
                         (static_cast<uint32_t>(data[1]) << 16) |
                         (static_cast<uint32_t>(data[2]) << 8) |
                         static_cast<uint32_t>(data[3]);

    std::vector<hydra::math::BigInt> values;
    size_t pos = 4;

    // Extract each element
    for (uint32_t i = 0; i < dimension; ++i) {
        if (pos >= data.size()) {
            return std::nullopt;
        }

        // Extract string length
        uint8_t str_len = data[pos++];

        if (pos + str_len > data.size()) {
            return std::nullopt;
        }

        // Extract string data
        std::string str(data.begin() + pos, data.begin() + pos + str_len);
        pos += str_len;

        // Convert string to BigInt
        try {
            hydra::math::BigInt val(str);
            values.push_back(val);
        } catch (const std::exception&) {
            return std::nullopt;
        }
    }

    return Vector(values);
}

std::vector<uint8_t> Vector::toBinary() const {
    return compress();
}

std::optional<Vector> Vector::fromBinary(const std::vector<uint8_t>& data) {
    return decompress(data);
}

Vector Vector::operator+(const Vector& other) const {
    if (m_data.size() != other.m_data.size()) {
        throw std::invalid_argument("Vector dimensions must match for addition");
    }

    std::vector<hydra::math::BigInt> result(m_data.size());
    for (size_t i = 0; i < m_data.size(); ++i) {
        result[i] = m_data[i] + other.m_data[i];
    }

    return Vector(result);
}

Vector Vector::operator-(const Vector& other) const {
    if (m_data.size() != other.m_data.size()) {
        throw std::invalid_argument("Vector dimensions must match for subtraction");
    }

    std::vector<hydra::math::BigInt> result(m_data.size());
    for (size_t i = 0; i < m_data.size(); ++i) {
        result[i] = m_data[i] - other.m_data[i];
    }

    return Vector(result);
}

Vector Vector::operator*(const hydra::math::BigInt& scalar) const {
    std::vector<hydra::math::BigInt> result(m_data.size());
    for (size_t i = 0; i < m_data.size(); ++i) {
        result[i] = m_data[i] * scalar;
    }

    return Vector(result);
}

hydra::math::BigInt Vector::dot(const Vector& other) const {
    if (m_data.size() != other.m_data.size()) {
        throw std::invalid_argument("Vector dimensions must match for dot product");
    }

    hydra::math::BigInt result(0);
    for (size_t i = 0; i < m_data.size(); ++i) {
        result = result + (m_data[i] * other.m_data[i]);
    }

    return result;
}

bool Vector::operator==(const Vector& other) const {
    return m_data == other.m_data;
}

bool Vector::operator!=(const Vector& other) const {
    return !(*this == other);
}

// MatrixLayer implementation
MatrixLayer::MatrixLayer() : m_type(MatrixLayerType::IDENTITY) {}

MatrixLayer::MatrixLayer(size_t rows, size_t cols, MatrixLayerType type)
    : m_data(rows, std::vector<hydra::math::BigInt>(cols, hydra::math::BigInt(0))),
      m_type(type) {

    if (type == MatrixLayerType::IDENTITY) {
        if (rows != cols) {
            throw std::invalid_argument("Identity matrix must be square");
        }

        // Initialize as identity matrix
        for (size_t i = 0; i < rows; ++i) {
            m_data[i][i] = hydra::math::BigInt(1);
        }
    }
}

MatrixLayer::MatrixLayer(const std::vector<std::vector<hydra::math::BigInt>>& data, MatrixLayerType type)
    : m_data(data), m_type(type) {

    // Validate matrix dimensions
    if (m_data.empty() || m_data[0].empty()) {
        throw std::invalid_argument("Matrix cannot be empty");
    }

    size_t cols = m_data[0].size();
    for (const auto& row : m_data) {
        if (row.size() != cols) {
            throw std::invalid_argument("All rows must have the same number of columns");
        }
    }
}

size_t MatrixLayer::getRows() const {
    return m_data.size();
}

size_t MatrixLayer::getCols() const {
    return m_data.empty() ? 0 : m_data[0].size();
}

MatrixLayerType MatrixLayer::getType() const {
    return m_type;
}

const std::vector<std::vector<hydra::math::BigInt>>& MatrixLayer::getData() const {
    return m_data;
}

void MatrixLayer::setElement(size_t row, size_t col, const hydra::math::BigInt& value) {
    if (row >= m_data.size() || col >= m_data[0].size()) {
        throw std::out_of_range("Matrix indices out of range");
    }
    m_data[row][col] = value;
}

hydra::math::BigInt MatrixLayer::getElement(size_t row, size_t col) const {
    if (row >= m_data.size() || col >= m_data[0].size()) {
        throw std::out_of_range("Matrix indices out of range");
    }
    return m_data[row][col];
}

Vector MatrixLayer::apply(const Vector& vec) const {
    if (getCols() != vec.getDimension()) {
        throw std::invalid_argument("Matrix columns must match vector dimension");
    }

    std::vector<hydra::math::BigInt> result(getRows(), hydra::math::BigInt(0));

    for (size_t i = 0; i < getRows(); ++i) {
        for (size_t j = 0; j < getCols(); ++j) {
            result[i] = result[i] + (m_data[i][j] * vec.getElement(j));
        }
    }

    return Vector(result);
}

MatrixLayer MatrixLayer::createIdentity(size_t size) {
    return MatrixLayer(size, size, MatrixLayerType::IDENTITY);
}

MatrixLayer MatrixLayer::createPermutation(size_t size, const std::vector<size_t>& permutation) {
    if (permutation.size() != size) {
        throw std::invalid_argument("Permutation size must match matrix size");
    }

    // Validate permutation
    std::vector<bool> used(size, false);
    for (size_t idx : permutation) {
        if (idx >= size || used[idx]) {
            throw std::invalid_argument("Invalid permutation");
        }
        used[idx] = true;
    }

    // Create permutation matrix
    std::vector<std::vector<hydra::math::BigInt>> data(size, std::vector<hydra::math::BigInt>(size, hydra::math::BigInt(0)));

    for (size_t i = 0; i < size; ++i) {
        data[i][permutation[i]] = hydra::math::BigInt(1);
    }

    return MatrixLayer(data, MatrixLayerType::PERMUTATION);
}

MatrixLayer MatrixLayer::createProjection(size_t input_dim, size_t output_dim) {
    // Create a simple projection matrix that selects the first output_dim dimensions
    std::vector<std::vector<hydra::math::BigInt>> data(output_dim, std::vector<hydra::math::BigInt>(input_dim, hydra::math::BigInt(0)));

    for (size_t i = 0; i < output_dim && i < input_dim; ++i) {
        data[i][i] = hydra::math::BigInt(1);
    }

    return MatrixLayer(data, MatrixLayerType::PROJECTION);
}

MatrixLayer MatrixLayer::createRotation(size_t size, const std::vector<double>& angles) {
    if (size < 2) {
        throw std::invalid_argument("Rotation matrix size must be at least 2");
    }

    // Create identity matrix
    std::vector<std::vector<hydra::math::BigInt>> data(size, std::vector<hydra::math::BigInt>(size, hydra::math::BigInt(0)));
    for (size_t i = 0; i < size; ++i) {
        data[i][i] = hydra::math::BigInt(1);
    }

    // Apply rotations
    size_t angle_idx = 0;
    for (size_t i = 0; i < size - 1 && angle_idx < angles.size(); ++i) {
        for (size_t j = i + 1; j < size && angle_idx < angles.size(); ++j) {
            double angle = angles[angle_idx++];
            double cos_val = std::cos(angle);
            double sin_val = std::sin(angle);

            // Create rotation matrix for this plane
            std::vector<std::vector<hydra::math::BigInt>> rot(size, std::vector<hydra::math::BigInt>(size, hydra::math::BigInt(0)));
            for (size_t k = 0; k < size; ++k) {
                rot[k][k] = hydra::math::BigInt(1);
            }

            rot[i][i] = hydra::math::BigInt(static_cast<int64_t>(cos_val * 1000)) / hydra::math::BigInt(1000);
            rot[i][j] = hydra::math::BigInt(static_cast<int64_t>(-sin_val * 1000)) / hydra::math::BigInt(1000);
            rot[j][i] = hydra::math::BigInt(static_cast<int64_t>(sin_val * 1000)) / hydra::math::BigInt(1000);
            rot[j][j] = hydra::math::BigInt(static_cast<int64_t>(cos_val * 1000)) / hydra::math::BigInt(1000);

            // Apply this rotation
            MatrixLayer rot_layer(rot, MatrixLayerType::ROTATION);
            MatrixLayer current(data, MatrixLayerType::CUSTOM);
            MatrixLayer result = rot_layer * current;
            data = result.getData();
        }
    }

    return MatrixLayer(data, MatrixLayerType::ROTATION);
}

MatrixLayer MatrixLayer::createScaling(size_t size, const std::vector<hydra::math::BigInt>& factors) {
    if (factors.size() != size) {
        throw std::invalid_argument("Number of scaling factors must match matrix size");
    }

    std::vector<std::vector<hydra::math::BigInt>> data(size, std::vector<hydra::math::BigInt>(size, hydra::math::BigInt(0)));

    for (size_t i = 0; i < size; ++i) {
        data[i][i] = factors[i];
    }

    return MatrixLayer(data, MatrixLayerType::SCALING);
}

MatrixLayer MatrixLayer::operator*(const MatrixLayer& other) const {
    if (getCols() != other.getRows()) {
        throw std::invalid_argument("Matrix dimensions incompatible for multiplication");
    }

    size_t rows = getRows();
    size_t cols = other.getCols();
    size_t inner = getCols();

    std::vector<std::vector<hydra::math::BigInt>> result(rows, std::vector<hydra::math::BigInt>(cols, hydra::math::BigInt(0)));

    for (size_t i = 0; i < rows; ++i) {
        for (size_t j = 0; j < cols; ++j) {
            for (size_t k = 0; k < inner; ++k) {
                result[i][j] = result[i][j] + (m_data[i][k] * other.m_data[k][j]);
            }
        }
    }

    return MatrixLayer(result, MatrixLayerType::CUSTOM);
}

MatrixLayer MatrixLayer::operator+(const MatrixLayer& other) const {
    if (getRows() != other.getRows() || getCols() != other.getCols()) {
        throw std::invalid_argument("Matrix dimensions must match for addition");
    }

    std::vector<std::vector<hydra::math::BigInt>> result(getRows(), std::vector<hydra::math::BigInt>(getCols()));

    for (size_t i = 0; i < getRows(); ++i) {
        for (size_t j = 0; j < getCols(); ++j) {
            result[i][j] = m_data[i][j] + other.m_data[i][j];
        }
    }

    return MatrixLayer(result, MatrixLayerType::CUSTOM);
}

MatrixLayer MatrixLayer::operator-(const MatrixLayer& other) const {
    if (getRows() != other.getRows() || getCols() != other.getCols()) {
        throw std::invalid_argument("Matrix dimensions must match for subtraction");
    }

    std::vector<std::vector<hydra::math::BigInt>> result(getRows(), std::vector<hydra::math::BigInt>(getCols()));

    for (size_t i = 0; i < getRows(); ++i) {
        for (size_t j = 0; j < getCols(); ++j) {
            result[i][j] = m_data[i][j] - other.m_data[i][j];
        }
    }

    return MatrixLayer(result, MatrixLayerType::CUSTOM);
}

MatrixLayer MatrixLayer::transpose() const {
    std::vector<std::vector<hydra::math::BigInt>> result(getCols(), std::vector<hydra::math::BigInt>(getRows()));

    for (size_t i = 0; i < getRows(); ++i) {
        for (size_t j = 0; j < getCols(); ++j) {
            result[j][i] = m_data[i][j];
        }
    }

    return MatrixLayer(result, MatrixLayerType::CUSTOM);
}

std::optional<MatrixLayer> MatrixLayer::inverse() const {
    // Only implemented for identity and permutation matrices for now
    if (m_type == MatrixLayerType::IDENTITY) {
        return *this;
    } else if (m_type == MatrixLayerType::PERMUTATION) {
        return transpose();
    }

    // For other types, return empty
    return std::nullopt;
}

bool MatrixLayer::operator==(const MatrixLayer& other) const {
    return m_data == other.m_data && m_type == other.m_type;
}

bool MatrixLayer::operator!=(const MatrixLayer& other) const {
    return !(*this == other);
}

// LayeredMatrix implementation
LayeredMatrix::LayeredMatrix() : m_dimension(0) {}

LayeredMatrix::LayeredMatrix(size_t dimension) : m_dimension(dimension) {}

void LayeredMatrix::addLayer(const MatrixLayer& layer) {
    m_layers.push_back(layer);
}

size_t LayeredMatrix::getLayerCount() const {
    return m_layers.size();
}

const MatrixLayer& LayeredMatrix::getLayer(size_t index) const {
    if (index >= m_layers.size()) {
        throw std::out_of_range("Layer index out of range");
    }
    return m_layers[index];
}

Vector LayeredMatrix::apply(const Vector& vec) const {
    Vector result = vec;

    // Apply each layer in sequence
    for (const auto& layer : m_layers) {
        result = layer.apply(result);
    }

    return result;
}

std::vector<uint8_t> LayeredMatrix::compress() const {
    // Simple serialization for now
    std::vector<uint8_t> result;

    // Add the dimension (4 bytes)
    uint32_t dimension = static_cast<uint32_t>(m_dimension);
    result.push_back((dimension >> 24) & 0xFF);
    result.push_back((dimension >> 16) & 0xFF);
    result.push_back((dimension >> 8) & 0xFF);
    result.push_back(dimension & 0xFF);

    // Add the number of layers (4 bytes)
    uint32_t layer_count = static_cast<uint32_t>(m_layers.size());
    result.push_back((layer_count >> 24) & 0xFF);
    result.push_back((layer_count >> 16) & 0xFF);
    result.push_back((layer_count >> 8) & 0xFF);
    result.push_back(layer_count & 0xFF);

    // Add each layer
    for (const auto& layer : m_layers) {
        // Add layer type (1 byte)
        result.push_back(static_cast<uint8_t>(layer.getType()));

        // Add rows and columns (4 bytes each)
        uint32_t rows = static_cast<uint32_t>(layer.getRows());
        result.push_back((rows >> 24) & 0xFF);
        result.push_back((rows >> 16) & 0xFF);
        result.push_back((rows >> 8) & 0xFF);
        result.push_back(rows & 0xFF);

        uint32_t cols = static_cast<uint32_t>(layer.getCols());
        result.push_back((cols >> 24) & 0xFF);
        result.push_back((cols >> 16) & 0xFF);
        result.push_back((cols >> 8) & 0xFF);
        result.push_back(cols & 0xFF);

        // Add matrix data
        for (size_t i = 0; i < layer.getRows(); ++i) {
            for (size_t j = 0; j < layer.getCols(); ++j) {
                // Convert BigInt to string
                std::string str = layer.getElement(i, j).to_string();

                // Add string length (1 byte)
                result.push_back(static_cast<uint8_t>(str.length()));

                // Add string data
                result.insert(result.end(), str.begin(), str.end());
            }
        }
    }

    return result;
}

std::optional<LayeredMatrix> LayeredMatrix::decompress(const std::vector<uint8_t>& data) {
    if (data.size() < 8) {
        return std::nullopt;
    }

    // Extract dimension
    uint32_t dimension = (static_cast<uint32_t>(data[0]) << 24) |
                         (static_cast<uint32_t>(data[1]) << 16) |
                         (static_cast<uint32_t>(data[2]) << 8) |
                         static_cast<uint32_t>(data[3]);

    // Extract layer count
    uint32_t layer_count = (static_cast<uint32_t>(data[4]) << 24) |
                           (static_cast<uint32_t>(data[5]) << 16) |
                           (static_cast<uint32_t>(data[6]) << 8) |
                           static_cast<uint32_t>(data[7]);

    LayeredMatrix result(dimension);
    size_t pos = 8;

    // Extract each layer
    for (uint32_t l = 0; l < layer_count; ++l) {
        if (pos + 9 > data.size()) {
            return std::nullopt;
        }

        // Extract layer type
        MatrixLayerType type = static_cast<MatrixLayerType>(data[pos++]);

        // Extract rows and columns
        uint32_t rows = (static_cast<uint32_t>(data[pos]) << 24) |
                        (static_cast<uint32_t>(data[pos + 1]) << 16) |
                        (static_cast<uint32_t>(data[pos + 2]) << 8) |
                        static_cast<uint32_t>(data[pos + 3]);
        pos += 4;

        uint32_t cols = (static_cast<uint32_t>(data[pos]) << 24) |
                        (static_cast<uint32_t>(data[pos + 1]) << 16) |
                        (static_cast<uint32_t>(data[pos + 2]) << 8) |
                        static_cast<uint32_t>(data[pos + 3]);
        pos += 4;

        // Create matrix data
        std::vector<std::vector<hydra::math::BigInt>> matrix_data(rows, std::vector<hydra::math::BigInt>(cols));

        for (uint32_t i = 0; i < rows; ++i) {
            for (uint32_t j = 0; j < cols; ++j) {
                if (pos >= data.size()) {
                    return std::nullopt;
                }

                // Extract string length
                uint8_t str_len = data[pos++];

                if (pos + str_len > data.size()) {
                    return std::nullopt;
                }

                // Extract string data
                std::string str(data.begin() + pos, data.begin() + pos + str_len);
                pos += str_len;

                // Convert string to BigInt
                try {
                    hydra::math::BigInt val(str);
                    matrix_data[i][j] = val;
                } catch (const std::exception&) {
                    return std::nullopt;
                }
            }
        }

        // Add layer to result
        result.addLayer(MatrixLayer(matrix_data, type));
    }

    return result;
}

std::vector<uint8_t> LayeredMatrix::toBinary() const {
    return compress();
}

std::optional<LayeredMatrix> LayeredMatrix::fromBinary(const std::vector<uint8_t>& data) {
    return decompress(data);
}

bool LayeredMatrix::operator==(const LayeredMatrix& other) const {
    return m_dimension == other.m_dimension && m_layers == other.m_layers;
}

bool LayeredMatrix::operator!=(const LayeredMatrix& other) const {
    return !(*this == other);
}

} // namespace address
} // namespace hydra
