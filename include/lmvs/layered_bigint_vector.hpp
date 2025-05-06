#pragma once

#include "lmvs/bigint_vector.hpp"
#include <vector>
#include <stdexcept>
#include <memory>
#include <string>
#include <cmath>
#include <iostream>

namespace lmvs {

/**
 * @brief Class representing a layered vector of BigInt values
 *
 * A layered BigInt vector consists of multiple layers, each being a vector of BigInt values.
 */
class LayeredBigIntVector {
public:
    /**
     * @brief Default constructor
     */
    LayeredBigIntVector() = default;

    /**
     * @brief Construct a new Layered BigInt Vector with specified dimensions
     *
     * @param num_layers Number of layers in the vector
     * @param dimension Dimension of each layer
     */
    LayeredBigIntVector(size_t num_layers, size_t dimension);

    /**
     * @brief Construct a new Layered BigInt Vector from existing data
     *
     * @param layers Vector of BigIntVector, where each represents a layer
     */
    explicit LayeredBigIntVector(const std::vector<BigIntVector>& layers);

    /**
     * @brief Construct a new Layered BigInt Vector from double values
     *
     * @param data Vector of vectors of doubles, where each inner vector represents a layer
     * @param scale Scaling factor to convert doubles to integers
     */
    LayeredBigIntVector(const std::vector<std::vector<double>>& data, double scale = 1e6);

    /**
     * @brief Get the number of layers in the vector
     *
     * @return size_t Number of layers
     */
    size_t getNumLayers() const { return m_layers.size(); }

    /**
     * @brief Get the dimension of each layer
     *
     * @return size_t Dimension of each layer
     */
    size_t getDimension() const { return m_layers.empty() ? 0 : m_layers[0].getDimension(); }

    /**
     * @brief Get a specific layer of the vector
     *
     * @param layer_idx Index of the layer to retrieve
     * @return const BigIntVector& Reference to the layer
     * @throws std::out_of_range if layer_idx is out of bounds
     */
    const BigIntVector& getLayer(size_t layer_idx) const;

    /**
     * @brief Get a mutable reference to a specific layer
     *
     * @param layer_idx Index of the layer to retrieve
     * @return BigIntVector& Mutable reference to the layer
     * @throws std::out_of_range if layer_idx is out of bounds
     */
    BigIntVector& getLayerMutable(size_t layer_idx);

    /**
     * @brief Set the values of a specific layer
     *
     * @param layer_idx Index of the layer to set
     * @param values New values for the layer
     * @throws std::out_of_range if layer_idx is out of bounds
     * @throws std::invalid_argument if values.getDimension() != getDimension()
     */
    void setLayer(size_t layer_idx, const BigIntVector& values);

    /**
     * @brief Get all layers of the vector
     *
     * @return const std::vector<BigIntVector>& Reference to all layers
     */
    const std::vector<BigIntVector>& getAllLayers() const { return m_layers; }

    /**
     * @brief Add a new layer to the vector
     *
     * @param layer Values for the new layer
     * @throws std::invalid_argument if layer.getDimension() != getDimension() and vector is not empty
     */
    void addLayer(const BigIntVector& layer);

    /**
     * @brief Remove a layer from the vector
     *
     * @param layer_idx Index of the layer to remove
     * @throws std::out_of_range if layer_idx is out of bounds
     */
    void removeLayer(size_t layer_idx);

    /**
     * @brief Convert the layered BigInt vector to a layered double vector
     *
     * @param scale Scaling factor to convert integers to doubles
     * @return std::vector<std::vector<double>> Vector of vectors of doubles
     */
    std::vector<std::vector<double>> toDoubleVector(double scale = 1e6) const;

    /**
     * @brief Serialize the layered vector to a byte array
     *
     * @return std::vector<uint8_t> Serialized vector
     */
    std::vector<uint8_t> serialize() const;

    /**
     * @brief Deserialize a byte array to a layered BigInt vector
     *
     * @param data Serialized data
     * @return LayeredBigIntVector Deserialized vector
     */
    static LayeredBigIntVector deserialize(const std::vector<uint8_t>& data);

    /**
     * @brief Compress the layered vector
     *
     * @return std::vector<uint8_t> Compressed data
     */
    std::vector<uint8_t> compress() const;

    /**
     * @brief Decompress a compressed layered vector
     *
     * @param compressed_data Compressed data
     * @return LayeredBigIntVector Decompressed vector
     */
    static LayeredBigIntVector decompress(const std::vector<uint8_t>& compressed_data);

    /**
     * @brief Print the layered vector to the console for debugging
     */
    void print() const;

private:
    std::vector<BigIntVector> m_layers; // Vector of layers
};

} // namespace lmvs
