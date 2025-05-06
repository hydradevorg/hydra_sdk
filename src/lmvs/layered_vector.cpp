#include "lmvs/layered_vector.hpp"

namespace lmvs {

LayeredVector::LayeredVector(size_t num_layers, size_t dimension) {
    m_layers.resize(num_layers);
    for (auto& layer : m_layers) {
        layer.resize(dimension, 0.0);
    }
}

LayeredVector::LayeredVector(const std::vector<std::vector<double>>& data) {
    if (data.empty()) {
        return;
    }

    // Check that all layers have the same dimension
    const size_t dimension = data[0].size();
    for (size_t i = 1; i < data.size(); ++i) {
        if (data[i].size() != dimension) {
            throw std::invalid_argument("All layers must have the same dimension");
        }
    }

    m_layers = data;
}

const std::vector<double>& LayeredVector::getLayer(size_t layer_idx) const {
    if (layer_idx >= m_layers.size()) {
        throw std::out_of_range("Layer index out of range");
    }
    return m_layers[layer_idx];
}

std::vector<double>& LayeredVector::getLayerMutable(size_t layer_idx) {
    if (layer_idx >= m_layers.size()) {
        throw std::out_of_range("Layer index out of range");
    }
    return m_layers[layer_idx];
}

void LayeredVector::setLayer(size_t layer_idx, const std::vector<double>& values) {
    if (layer_idx >= m_layers.size()) {
        throw std::out_of_range("Layer index out of range");
    }
    
    if (values.size() != m_layers[layer_idx].size()) {
        throw std::invalid_argument("Layer dimension mismatch");
    }
    
    m_layers[layer_idx] = values;
}

void LayeredVector::addLayer(const std::vector<double>& layer) {
    if (!m_layers.empty() && layer.size() != m_layers[0].size()) {
        throw std::invalid_argument("Layer dimension mismatch");
    }
    
    m_layers.push_back(layer);
}

void LayeredVector::removeLayer(size_t layer_idx) {
    if (layer_idx >= m_layers.size()) {
        throw std::out_of_range("Layer index out of range");
    }
    
    m_layers.erase(m_layers.begin() + layer_idx);
}

double LayeredVector::squaredDistance(const LayeredVector& other) const {
    if (getNumLayers() != other.getNumLayers()) {
        throw std::invalid_argument("Vectors must have the same number of layers");
    }
    
    if (getDimension() != other.getDimension()) {
        throw std::invalid_argument("Vectors must have the same dimension");
    }
    
    double sum_squared_diff = 0.0;
    
    for (size_t i = 0; i < getNumLayers(); ++i) {
        const auto& layer1 = getLayer(i);
        const auto& layer2 = other.getLayer(i);
        
        for (size_t j = 0; j < getDimension(); ++j) {
            double diff = layer1[j] - layer2[j];
            sum_squared_diff += diff * diff;
        }
    }
    
    return sum_squared_diff;
}

void LayeredVector::print() const {
    std::cout << "LayeredVector with " << getNumLayers() << " layers, each of dimension " << getDimension() << std::endl;
    
    for (size_t i = 0; i < getNumLayers(); ++i) {
        std::cout << "Layer " << i << ": [";
        const auto& layer = getLayer(i);
        for (size_t j = 0; j < layer.size(); ++j) {
            std::cout << layer[j];
            if (j < layer.size() - 1) {
                std::cout << ", ";
            }
        }
        std::cout << "]" << std::endl;
    }
}

} // namespace lmvs
