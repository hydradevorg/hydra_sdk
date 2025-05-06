#include "lmvs/layered_bigint_vector.hpp"
#include <sstream>
#include <iomanip>
#include <cstring>

namespace lmvs {

LayeredBigIntVector::LayeredBigIntVector(size_t num_layers, size_t dimension) {
    m_layers.resize(num_layers, BigIntVector(dimension));
}

LayeredBigIntVector::LayeredBigIntVector(const std::vector<BigIntVector>& layers)
    : m_layers(layers) {
    
    if (layers.empty()) {
        return;
    }
    
    // Check that all layers have the same dimension
    const size_t dimension = layers[0].getDimension();
    for (size_t i = 1; i < layers.size(); ++i) {
        if (layers[i].getDimension() != dimension) {
            throw std::invalid_argument("All layers must have the same dimension");
        }
    }
}

LayeredBigIntVector::LayeredBigIntVector(const std::vector<std::vector<double>>& data, double scale) {
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
    
    // Convert each layer to a BigIntVector
    m_layers.reserve(data.size());
    for (const auto& layer : data) {
        m_layers.emplace_back(layer, scale);
    }
}

const BigIntVector& LayeredBigIntVector::getLayer(size_t layer_idx) const {
    if (layer_idx >= m_layers.size()) {
        throw std::out_of_range("Layer index out of range");
    }
    return m_layers[layer_idx];
}

BigIntVector& LayeredBigIntVector::getLayerMutable(size_t layer_idx) {
    if (layer_idx >= m_layers.size()) {
        throw std::out_of_range("Layer index out of range");
    }
    return m_layers[layer_idx];
}

void LayeredBigIntVector::setLayer(size_t layer_idx, const BigIntVector& values) {
    if (layer_idx >= m_layers.size()) {
        throw std::out_of_range("Layer index out of range");
    }
    
    if (values.getDimension() != m_layers[layer_idx].getDimension()) {
        throw std::invalid_argument("Layer dimension mismatch");
    }
    
    m_layers[layer_idx] = values;
}

void LayeredBigIntVector::addLayer(const BigIntVector& layer) {
    if (!m_layers.empty() && layer.getDimension() != m_layers[0].getDimension()) {
        throw std::invalid_argument("Layer dimension mismatch");
    }
    
    m_layers.push_back(layer);
}

void LayeredBigIntVector::removeLayer(size_t layer_idx) {
    if (layer_idx >= m_layers.size()) {
        throw std::out_of_range("Layer index out of range");
    }
    
    m_layers.erase(m_layers.begin() + layer_idx);
}

std::vector<std::vector<double>> LayeredBigIntVector::toDoubleVector(double scale) const {
    std::vector<std::vector<double>> result;
    result.reserve(m_layers.size());
    
    for (const auto& layer : m_layers) {
        result.push_back(layer.toDoubleVector(scale));
    }
    
    return result;
}

std::vector<uint8_t> LayeredBigIntVector::serialize() const {
    std::vector<uint8_t> result;
    
    // First, store the number of layers
    size_t num_layers = m_layers.size();
    result.resize(sizeof(size_t));
    std::memcpy(result.data(), &num_layers, sizeof(size_t));
    
    // Then, serialize each layer
    for (const auto& layer : m_layers) {
        std::vector<uint8_t> layer_data = layer.serialize();
        
        // Store the size of the layer data
        size_t layer_size = layer_data.size();
        size_t offset = result.size();
        result.resize(offset + sizeof(size_t));
        std::memcpy(result.data() + offset, &layer_size, sizeof(size_t));
        
        // Store the layer data
        offset = result.size();
        result.resize(offset + layer_size);
        std::memcpy(result.data() + offset, layer_data.data(), layer_size);
    }
    
    return result;
}

LayeredBigIntVector LayeredBigIntVector::deserialize(const std::vector<uint8_t>& data) {
    if (data.size() < sizeof(size_t)) {
        throw std::invalid_argument("Invalid serialized data");
    }
    
    // Read the number of layers
    size_t num_layers;
    std::memcpy(&num_layers, data.data(), sizeof(size_t));
    
    std::vector<BigIntVector> layers;
    layers.reserve(num_layers);
    
    size_t offset = sizeof(size_t);
    for (size_t i = 0; i < num_layers; ++i) {
        if (offset + sizeof(size_t) > data.size()) {
            throw std::invalid_argument("Invalid serialized data");
        }
        
        // Read the size of the layer data
        size_t layer_size;
        std::memcpy(&layer_size, data.data() + offset, sizeof(size_t));
        offset += sizeof(size_t);
        
        if (offset + layer_size > data.size()) {
            throw std::invalid_argument("Invalid serialized data");
        }
        
        // Read the layer data and deserialize
        std::vector<uint8_t> layer_data(data.begin() + offset, data.begin() + offset + layer_size);
        offset += layer_size;
        
        layers.push_back(BigIntVector::deserialize(layer_data));
    }
    
    return LayeredBigIntVector(layers);
}

std::vector<uint8_t> LayeredBigIntVector::compress() const {
    // First, serialize the layered vector
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

LayeredBigIntVector LayeredBigIntVector::decompress(const std::vector<uint8_t>& compressed_data) {
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

void LayeredBigIntVector::print() const {
    std::cout << "LayeredBigIntVector with " << getNumLayers() << " layers, each of dimension " << getDimension() << std::endl;
    
    for (size_t i = 0; i < m_layers.size(); ++i) {
        std::cout << "Layer " << i << ": ";
        m_layers[i].print();
    }
}

} // namespace lmvs
