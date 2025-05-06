#include "lmvs/bigint_vector.hpp"
#include <sstream>
#include <iomanip>
#include <cstring>

namespace lmvs {

BigIntVector::BigIntVector(size_t dimension) {
    m_values.resize(dimension, hydra::math::BigInt(0));
}

BigIntVector::BigIntVector(const std::vector<hydra::math::BigInt>& data)
    : m_values(data) {
}

BigIntVector::BigIntVector(const std::vector<double>& data, double scale) {
    m_values.reserve(data.size());
    for (const auto& val : data) {
        // Convert double to BigInt by scaling and rounding
        int64_t scaled_val = static_cast<int64_t>(val * scale);
        m_values.emplace_back(scaled_val);
    }
}

const hydra::math::BigInt& BigIntVector::getValue(size_t index) const {
    if (index >= m_values.size()) {
        throw std::out_of_range("Index out of range");
    }
    return m_values[index];
}

hydra::math::BigInt& BigIntVector::getValueMutable(size_t index) {
    if (index >= m_values.size()) {
        throw std::out_of_range("Index out of range");
    }
    return m_values[index];
}

void BigIntVector::setValue(size_t index, const hydra::math::BigInt& value) {
    if (index >= m_values.size()) {
        throw std::out_of_range("Index out of range");
    }
    m_values[index] = value;
}

std::vector<double> BigIntVector::toDoubleVector(double scale) const {
    std::vector<double> result;
    result.reserve(m_values.size());
    
    for (const auto& val : m_values) {
        // Convert BigInt to double by dividing by scale
        double double_val = static_cast<double>(val.to_int()) / scale;
        result.push_back(double_val);
    }
    
    return result;
}

std::vector<uint8_t> BigIntVector::serialize() const {
    std::vector<uint8_t> result;
    
    // First, store the number of elements
    size_t size = m_values.size();
    result.resize(sizeof(size_t));
    std::memcpy(result.data(), &size, sizeof(size_t));
    
    // Then, store each BigInt as a string representation
    for (const auto& val : m_values) {
        std::string str = val.to_string();
        
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
    
    return result;
}

BigIntVector BigIntVector::deserialize(const std::vector<uint8_t>& data) {
    if (data.size() < sizeof(size_t)) {
        throw std::invalid_argument("Invalid serialized data");
    }
    
    // Read the number of elements
    size_t size;
    std::memcpy(&size, data.data(), sizeof(size_t));
    
    std::vector<hydra::math::BigInt> values;
    values.reserve(size);
    
    size_t offset = sizeof(size_t);
    for (size_t i = 0; i < size; ++i) {
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
        
        values.emplace_back(str);
    }
    
    return BigIntVector(values);
}

std::vector<uint8_t> BigIntVector::compress() const {
    // First, serialize the vector
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

BigIntVector BigIntVector::decompress(const std::vector<uint8_t>& compressed_data) {
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

void BigIntVector::print() const {
    std::cout << "BigIntVector with " << getDimension() << " elements:" << std::endl;
    std::cout << "[";
    for (size_t i = 0; i < m_values.size(); ++i) {
        std::cout << m_values[i].to_string();
        if (i < m_values.size() - 1) {
            std::cout << ", ";
        }
    }
    std::cout << "]" << std::endl;
}

} // namespace lmvs
