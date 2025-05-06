#pragma once

#include <vector>
#include <stdexcept>
#include <memory>
#include <string>
#include <cmath>
#include <iostream>
#include <hydra_math/bigint.hpp>

namespace lmvs {

/**
 * @brief Class representing a vector of BigInt values
 * 
 * This class provides a vector of arbitrary-precision integers using
 * the hydra::math::BigInt class.
 */
class BigIntVector {
public:
    /**
     * @brief Construct a new BigInt Vector with specified dimension
     * 
     * @param dimension Dimension of the vector
     */
    explicit BigIntVector(size_t dimension);

    /**
     * @brief Construct a new BigInt Vector from existing data
     * 
     * @param data Vector of BigInt values
     */
    explicit BigIntVector(const std::vector<hydra::math::BigInt>& data);

    /**
     * @brief Construct a new BigInt Vector from double values
     * 
     * @param data Vector of double values
     * @param scale Scaling factor to convert doubles to integers
     */
    BigIntVector(const std::vector<double>& data, double scale = 1e6);

    /**
     * @brief Get the dimension of the vector
     * 
     * @return size_t Dimension
     */
    size_t getDimension() const { return m_values.size(); }

    /**
     * @brief Get a specific element of the vector
     * 
     * @param index Index of the element to retrieve
     * @return const hydra::math::BigInt& Reference to the element
     * @throws std::out_of_range if index is out of bounds
     */
    const hydra::math::BigInt& getValue(size_t index) const;

    /**
     * @brief Get a mutable reference to a specific element
     * 
     * @param index Index of the element to retrieve
     * @return hydra::math::BigInt& Mutable reference to the element
     * @throws std::out_of_range if index is out of bounds
     */
    hydra::math::BigInt& getValueMutable(size_t index);

    /**
     * @brief Set the value of a specific element
     * 
     * @param index Index of the element to set
     * @param value New value for the element
     * @throws std::out_of_range if index is out of bounds
     */
    void setValue(size_t index, const hydra::math::BigInt& value);

    /**
     * @brief Get all values of the vector
     * 
     * @return const std::vector<hydra::math::BigInt>& Reference to all values
     */
    const std::vector<hydra::math::BigInt>& getAllValues() const { return m_values; }

    /**
     * @brief Convert the BigInt vector to a vector of doubles
     * 
     * @param scale Scaling factor to convert integers to doubles
     * @return std::vector<double> Vector of double values
     */
    std::vector<double> toDoubleVector(double scale = 1e6) const;

    /**
     * @brief Serialize the vector to a byte array
     * 
     * @return std::vector<uint8_t> Serialized vector
     */
    std::vector<uint8_t> serialize() const;

    /**
     * @brief Deserialize a byte array to a BigInt vector
     * 
     * @param data Serialized data
     * @return BigIntVector Deserialized vector
     */
    static BigIntVector deserialize(const std::vector<uint8_t>& data);

    /**
     * @brief Compress the vector using a simple run-length encoding
     * 
     * @return std::vector<uint8_t> Compressed data
     */
    std::vector<uint8_t> compress() const;

    /**
     * @brief Decompress a compressed vector
     * 
     * @param compressed_data Compressed data
     * @return BigIntVector Decompressed vector
     */
    static BigIntVector decompress(const std::vector<uint8_t>& compressed_data);

    /**
     * @brief Print the vector to the console for debugging
     */
    void print() const;

private:
    std::vector<hydra::math::BigInt> m_values; // Vector of BigInt values
};

} // namespace lmvs
