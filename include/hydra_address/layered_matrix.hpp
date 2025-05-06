#pragma once

#include <vector>
#include <array>
#include <memory>
#include <optional>
#include <hydra_math/bigint.hpp>

namespace hydra {
namespace address {

/**
 * @brief Matrix layer type
 */
enum class MatrixLayerType {
    IDENTITY,      ///< Identity matrix
    PERMUTATION,   ///< Permutation matrix
    PROJECTION,    ///< Projection matrix
    ROTATION,      ///< Rotation matrix
    SCALING,       ///< Scaling matrix
    CUSTOM         ///< Custom matrix
};

/**
 * @brief Vector representation for the layered matrix system
 */
class Vector {
public:
    /**
     * @brief Default constructor
     */
    Vector();
    
    /**
     * @brief Constructor with dimension
     * @param dimension Vector dimension
     */
    explicit Vector(size_t dimension);
    
    /**
     * @brief Constructor with data
     * @param data Vector data as BigInt values
     */
    explicit Vector(const std::vector<hydra::math::BigInt>& data);
    
    /**
     * @brief Get the vector dimension
     * @return Vector dimension
     */
    size_t getDimension() const;
    
    /**
     * @brief Get the vector data
     * @return Vector data
     */
    const std::vector<hydra::math::BigInt>& getData() const;
    
    /**
     * @brief Set a vector element
     * @param index Element index
     * @param value Element value
     */
    void setElement(size_t index, const hydra::math::BigInt& value);
    
    /**
     * @brief Get a vector element
     * @param index Element index
     * @return Element value
     */
    hydra::math::BigInt getElement(size_t index) const;
    
    /**
     * @brief Normalize the vector
     */
    void normalize();
    
    /**
     * @brief Compress the vector
     * @return Compressed vector data
     */
    std::vector<uint8_t> compress() const;
    
    /**
     * @brief Decompress vector data
     * @param data Compressed vector data
     * @return Decompressed vector or empty if invalid
     */
    static std::optional<Vector> decompress(const std::vector<uint8_t>& data);
    
    /**
     * @brief Convert to binary representation
     * @return Binary representation
     */
    std::vector<uint8_t> toBinary() const;
    
    /**
     * @brief Create from binary representation
     * @param data Binary representation
     * @return Vector or empty if invalid
     */
    static std::optional<Vector> fromBinary(const std::vector<uint8_t>& data);
    
    /**
     * @brief Vector addition
     */
    Vector operator+(const Vector& other) const;
    
    /**
     * @brief Vector subtraction
     */
    Vector operator-(const Vector& other) const;
    
    /**
     * @brief Scalar multiplication
     */
    Vector operator*(const hydra::math::BigInt& scalar) const;
    
    /**
     * @brief Dot product
     */
    hydra::math::BigInt dot(const Vector& other) const;
    
    /**
     * @brief Equality operator
     */
    bool operator==(const Vector& other) const;
    
    /**
     * @brief Inequality operator
     */
    bool operator!=(const Vector& other) const;

private:
    std::vector<hydra::math::BigInt> m_data;
};

/**
 * @brief Matrix layer for the layered matrix system
 */
class MatrixLayer {
public:
    /**
     * @brief Default constructor
     */
    MatrixLayer();
    
    /**
     * @brief Constructor with dimensions
     * @param rows Number of rows
     * @param cols Number of columns
     * @param type Layer type
     */
    MatrixLayer(size_t rows, size_t cols, MatrixLayerType type = MatrixLayerType::IDENTITY);
    
    /**
     * @brief Constructor with data
     * @param data Matrix data as 2D vector of BigInt values
     * @param type Layer type
     */
    MatrixLayer(const std::vector<std::vector<hydra::math::BigInt>>& data, MatrixLayerType type = MatrixLayerType::CUSTOM);
    
    /**
     * @brief Get the number of rows
     * @return Number of rows
     */
    size_t getRows() const;
    
    /**
     * @brief Get the number of columns
     * @return Number of columns
     */
    size_t getCols() const;
    
    /**
     * @brief Get the layer type
     * @return Layer type
     */
    MatrixLayerType getType() const;
    
    /**
     * @brief Get the matrix data
     * @return Matrix data
     */
    const std::vector<std::vector<hydra::math::BigInt>>& getData() const;
    
    /**
     * @brief Set a matrix element
     * @param row Row index
     * @param col Column index
     * @param value Element value
     */
    void setElement(size_t row, size_t col, const hydra::math::BigInt& value);
    
    /**
     * @brief Get a matrix element
     * @param row Row index
     * @param col Column index
     * @return Element value
     */
    hydra::math::BigInt getElement(size_t row, size_t col) const;
    
    /**
     * @brief Apply the matrix to a vector
     * @param vec Input vector
     * @return Transformed vector
     */
    Vector apply(const Vector& vec) const;
    
    /**
     * @brief Create an identity matrix
     * @param size Matrix size
     * @return Identity matrix
     */
    static MatrixLayer createIdentity(size_t size);
    
    /**
     * @brief Create a permutation matrix
     * @param size Matrix size
     * @param permutation Permutation indices
     * @return Permutation matrix
     */
    static MatrixLayer createPermutation(size_t size, const std::vector<size_t>& permutation);
    
    /**
     * @brief Create a projection matrix
     * @param input_dim Input dimension
     * @param output_dim Output dimension
     * @return Projection matrix
     */
    static MatrixLayer createProjection(size_t input_dim, size_t output_dim);
    
    /**
     * @brief Create a rotation matrix
     * @param size Matrix size
     * @param angles Rotation angles
     * @return Rotation matrix
     */
    static MatrixLayer createRotation(size_t size, const std::vector<double>& angles);
    
    /**
     * @brief Create a scaling matrix
     * @param size Matrix size
     * @param factors Scaling factors
     * @return Scaling matrix
     */
    static MatrixLayer createScaling(size_t size, const std::vector<hydra::math::BigInt>& factors);
    
    /**
     * @brief Matrix multiplication
     */
    MatrixLayer operator*(const MatrixLayer& other) const;
    
    /**
     * @brief Matrix addition
     */
    MatrixLayer operator+(const MatrixLayer& other) const;
    
    /**
     * @brief Matrix subtraction
     */
    MatrixLayer operator-(const MatrixLayer& other) const;
    
    /**
     * @brief Transpose the matrix
     * @return Transposed matrix
     */
    MatrixLayer transpose() const;
    
    /**
     * @brief Invert the matrix if possible
     * @return Inverted matrix or empty if not invertible
     */
    std::optional<MatrixLayer> inverse() const;
    
    /**
     * @brief Equality operator
     */
    bool operator==(const MatrixLayer& other) const;
    
    /**
     * @brief Inequality operator
     */
    bool operator!=(const MatrixLayer& other) const;

private:
    std::vector<std::vector<hydra::math::BigInt>> m_data;
    MatrixLayerType m_type;
};

/**
 * @brief Layered matrix system for the P2P VFS
 */
class LayeredMatrix {
public:
    /**
     * @brief Default constructor
     */
    LayeredMatrix();
    
    /**
     * @brief Constructor with dimension
     * @param dimension Matrix dimension
     */
    explicit LayeredMatrix(size_t dimension);
    
    /**
     * @brief Add a layer to the matrix
     * @param layer Matrix layer
     */
    void addLayer(const MatrixLayer& layer);
    
    /**
     * @brief Get the number of layers
     * @return Number of layers
     */
    size_t getLayerCount() const;
    
    /**
     * @brief Get a specific layer
     * @param index Layer index
     * @return Matrix layer
     */
    const MatrixLayer& getLayer(size_t index) const;
    
    /**
     * @brief Apply the layered matrix to a vector
     * @param vec Input vector
     * @return Transformed vector
     */
    Vector apply(const Vector& vec) const;
    
    /**
     * @brief Compress the layered matrix
     * @return Compressed matrix data
     */
    std::vector<uint8_t> compress() const;
    
    /**
     * @brief Decompress matrix data
     * @param data Compressed matrix data
     * @return Decompressed matrix or empty if invalid
     */
    static std::optional<LayeredMatrix> decompress(const std::vector<uint8_t>& data);
    
    /**
     * @brief Convert to binary representation
     * @return Binary representation
     */
    std::vector<uint8_t> toBinary() const;
    
    /**
     * @brief Create from binary representation
     * @param data Binary representation
     * @return Layered matrix or empty if invalid
     */
    static std::optional<LayeredMatrix> fromBinary(const std::vector<uint8_t>& data);
    
    /**
     * @brief Equality operator
     */
    bool operator==(const LayeredMatrix& other) const;
    
    /**
     * @brief Inequality operator
     */
    bool operator!=(const LayeredMatrix& other) const;

private:
    std::vector<MatrixLayer> m_layers;
    size_t m_dimension;
};

} // namespace address
} // namespace hydra
