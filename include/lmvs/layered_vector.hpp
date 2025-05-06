#pragma once

#include <vector>
#include <stdexcept>
#include <memory>
#include <string>
#include <cmath>
#include <iostream>

namespace lmvs {

/**
 * @brief Class representing a layered vector as described in the LMVS paper.
 *
 * A layered vector consists of multiple layers, each being an n-dimensional vector.
 */
class LayeredVector {
public:
    /**
     * @brief Default constructor
     */
    LayeredVector() = default;

    /**
     * @brief Construct a new Layered Vector with specified dimensions
     *
     * @param num_layers Number of layers in the vector
     * @param dimension Dimension of each layer
     */
    LayeredVector(size_t num_layers, size_t dimension);

    /**
     * @brief Construct a new Layered Vector from existing data
     *
     * @param data Vector of vectors, where each inner vector represents a layer
     */
    explicit LayeredVector(const std::vector<std::vector<double>>& data);

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
    size_t getDimension() const { return m_layers.empty() ? 0 : m_layers[0].size(); }

    /**
     * @brief Get a specific layer of the vector
     *
     * @param layer_idx Index of the layer to retrieve
     * @return const std::vector<double>& Reference to the layer
     * @throws std::out_of_range if layer_idx is out of bounds
     */
    const std::vector<double>& getLayer(size_t layer_idx) const;

    /**
     * @brief Get a mutable reference to a specific layer
     *
     * @param layer_idx Index of the layer to retrieve
     * @return std::vector<double>& Mutable reference to the layer
     * @throws std::out_of_range if layer_idx is out of bounds
     */
    std::vector<double>& getLayerMutable(size_t layer_idx);

    /**
     * @brief Set the values of a specific layer
     *
     * @param layer_idx Index of the layer to set
     * @param values New values for the layer
     * @throws std::out_of_range if layer_idx is out of bounds
     * @throws std::invalid_argument if values.size() != getDimension()
     */
    void setLayer(size_t layer_idx, const std::vector<double>& values);

    /**
     * @brief Get all layers of the vector
     *
     * @return const std::vector<std::vector<double>>& Reference to all layers
     */
    const std::vector<std::vector<double>>& getAllLayers() const { return m_layers; }

    /**
     * @brief Add a new layer to the vector
     *
     * @param layer Values for the new layer
     * @throws std::invalid_argument if layer.size() != getDimension() and vector is not empty
     */
    void addLayer(const std::vector<double>& layer);

    /**
     * @brief Remove a layer from the vector
     *
     * @param layer_idx Index of the layer to remove
     * @throws std::out_of_range if layer_idx is out of bounds
     */
    void removeLayer(size_t layer_idx);

    /**
     * @brief Calculate the squared Euclidean distance between this vector and another
     *
     * @param other The other layered vector
     * @return double The squared distance
     * @throws std::invalid_argument if vectors have different dimensions or number of layers
     */
    double squaredDistance(const LayeredVector& other) const;

    /**
     * @brief Print the vector to the console for debugging
     */
    void print() const;

private:
    std::vector<std::vector<double>> m_layers; // Vector of layers
};

} // namespace lmvs
