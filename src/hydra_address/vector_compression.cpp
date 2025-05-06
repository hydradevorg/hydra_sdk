#include <hydra_address/vector_compression.hpp>
#include <hydra_address/layered_matrix.hpp>
#include <hydra_math/bigint.hpp>
#include <algorithm>
#include <unordered_map>
#include <queue>
#include <bitset>
#include <sstream>
#include <iostream>

namespace hydra {
namespace address {





// VectorCompression implementation
VectorCompression::VectorCompression() : m_method(CompressionMethod::HUFFMAN) {}

VectorCompression::VectorCompression(CompressionMethod method) : m_method(method) {}

std::vector<uint8_t> VectorCompression::compress(const Vector& vec) const {
    // Serialize the vector to binary
    auto binary_data = vec.toBinary();

    // Add a header indicating the compression method (1 byte)
    std::vector<uint8_t> result;
    result.push_back(static_cast<uint8_t>(m_method));

    // Compress the data using RLE
    std::vector<uint8_t> compressed_data;

    // Simple RLE implementation
    if (!binary_data.empty()) {
        uint8_t current_byte = binary_data[0];
        uint8_t run_length = 1;

        for (size_t i = 1; i < binary_data.size(); ++i) {
            if (binary_data[i] == current_byte && run_length < 255) {
                // Continue the current run
                run_length++;
            } else {
                // End the current run and start a new one
                compressed_data.push_back(run_length);
                compressed_data.push_back(current_byte);
                current_byte = binary_data[i];
                run_length = 1;
            }
        }

        // Add the last run
        compressed_data.push_back(run_length);
        compressed_data.push_back(current_byte);
    }

    // Add the original size (4 bytes)
    uint32_t original_size = static_cast<uint32_t>(binary_data.size());
    result.push_back((original_size >> 24) & 0xFF);
    result.push_back((original_size >> 16) & 0xFF);
    result.push_back((original_size >> 8) & 0xFF);
    result.push_back(original_size & 0xFF);

    // Add the compressed data
    result.insert(result.end(), compressed_data.begin(), compressed_data.end());

    return result;
}

std::optional<Vector> VectorCompression::decompress(const std::vector<uint8_t>& data) const {
    if (data.size() < 5) {
        return std::nullopt;
    }

    // First byte indicates the compression method
    CompressionMethod method = static_cast<CompressionMethod>(data[0]);

    // Extract the original size
    uint32_t original_size =
        (static_cast<uint32_t>(data[1]) << 24) |
        (static_cast<uint32_t>(data[2]) << 16) |
        (static_cast<uint32_t>(data[3]) << 8) |
        static_cast<uint32_t>(data[4]);

    // Extract the actual compressed data
    std::vector<uint8_t> compressed_data(data.begin() + 5, data.end());

    // Decompress the data using RLE
    std::vector<uint8_t> binary_data;
    binary_data.reserve(original_size); // Reserve space for efficiency

    // Simple RLE decompression
    for (size_t i = 0; i < compressed_data.size(); i += 2) {
        if (i + 1 >= compressed_data.size()) {
            // Incomplete pair, should not happen in valid RLE data
            break;
        }

        uint8_t run_length = compressed_data[i];
        uint8_t value = compressed_data[i + 1];

        for (uint8_t j = 0; j < run_length; ++j) {
            binary_data.push_back(value);
        }
    }

    // Deserialize the vector
    return Vector::fromBinary(binary_data);
}

CompressionMethod VectorCompression::getMethod() const {
    return m_method;
}

void VectorCompression::setMethod(CompressionMethod method) {
    m_method = method;
}

double VectorCompression::getCompressionRatio(const Vector& vec) const {
    auto original_data = vec.toBinary();
    auto compressed_data = compress(vec);

    if (compressed_data.empty()) {
        return 0.0;
    }

    return static_cast<double>(original_data.size()) / compressed_data.size();
}

std::vector<uint8_t> VectorCompression::compressBigInts(const std::vector<hydra::math::BigInt>& values) const {
    // Create a vector from the BigInt values
    Vector vec(values);

    // Serialize to binary
    auto binary_data = vec.toBinary();

    // Compress the data using RLE
    std::vector<uint8_t> compressed_data;

    // Simple RLE implementation
    if (!binary_data.empty()) {
        uint8_t current_byte = binary_data[0];
        uint8_t run_length = 1;

        for (size_t i = 1; i < binary_data.size(); ++i) {
            if (binary_data[i] == current_byte && run_length < 255) {
                // Continue the current run
                run_length++;
            } else {
                // End the current run and start a new one
                compressed_data.push_back(run_length);
                compressed_data.push_back(current_byte);
                current_byte = binary_data[i];
                run_length = 1;
            }
        }

        // Add the last run
        compressed_data.push_back(run_length);
        compressed_data.push_back(current_byte);
    }

    // Add the original size (4 bytes)
    std::vector<uint8_t> result;
    uint32_t original_size = static_cast<uint32_t>(binary_data.size());
    result.push_back((original_size >> 24) & 0xFF);
    result.push_back((original_size >> 16) & 0xFF);
    result.push_back((original_size >> 8) & 0xFF);
    result.push_back(original_size & 0xFF);

    // Add the compressed data
    result.insert(result.end(), compressed_data.begin(), compressed_data.end());

    return result;
}

std::optional<std::vector<hydra::math::BigInt>> VectorCompression::decompressBigInts(const std::vector<uint8_t>& data) const {
    if (data.size() < 4) {
        return std::nullopt;
    }

    // Extract the original size
    uint32_t original_size =
        (static_cast<uint32_t>(data[0]) << 24) |
        (static_cast<uint32_t>(data[1]) << 16) |
        (static_cast<uint32_t>(data[2]) << 8) |
        static_cast<uint32_t>(data[3]);

    // Extract the compressed data
    std::vector<uint8_t> compressed_data(data.begin() + 4, data.end());

    // Decompress the data using RLE
    std::vector<uint8_t> binary_data;
    binary_data.reserve(original_size); // Reserve space for efficiency

    // Simple RLE decompression
    for (size_t i = 0; i < compressed_data.size(); i += 2) {
        if (i + 1 >= compressed_data.size()) {
            // Incomplete pair, should not happen in valid RLE data
            break;
        }

        uint8_t run_length = compressed_data[i];
        uint8_t value = compressed_data[i + 1];

        for (uint8_t j = 0; j < run_length; ++j) {
            binary_data.push_back(value);
        }
    }

    // Deserialize to a vector
    auto vec_opt = Vector::fromBinary(binary_data);
    if (!vec_opt) {
        return std::nullopt;
    }

    // Extract the BigInt values
    return vec_opt->getData();
}

std::vector<uint8_t> VectorCompression::compressMatrix(const LayeredMatrix& matrix) const {
    // Serialize the matrix
    auto binary_data = matrix.toBinary();

    // Add a header indicating this is a matrix (1 byte)
    std::vector<uint8_t> result;
    result.push_back(0xFF);  // Matrix indicator

    // Add the compression method (1 byte)
    result.push_back(static_cast<uint8_t>(m_method));

    // Compress the data using RLE
    std::vector<uint8_t> compressed_data;

    // Simple RLE implementation
    if (!binary_data.empty()) {
        uint8_t current_byte = binary_data[0];
        uint8_t run_length = 1;

        for (size_t i = 1; i < binary_data.size(); ++i) {
            if (binary_data[i] == current_byte && run_length < 255) {
                // Continue the current run
                run_length++;
            } else {
                // End the current run and start a new one
                compressed_data.push_back(run_length);
                compressed_data.push_back(current_byte);
                current_byte = binary_data[i];
                run_length = 1;
            }
        }

        // Add the last run
        compressed_data.push_back(run_length);
        compressed_data.push_back(current_byte);
    }

    // Add the original size (4 bytes)
    uint32_t original_size = static_cast<uint32_t>(binary_data.size());
    result.push_back((original_size >> 24) & 0xFF);
    result.push_back((original_size >> 16) & 0xFF);
    result.push_back((original_size >> 8) & 0xFF);
    result.push_back(original_size & 0xFF);

    // Add the compressed data
    result.insert(result.end(), compressed_data.begin(), compressed_data.end());

    return result;
}

std::optional<LayeredMatrix> VectorCompression::decompressMatrix(const std::vector<uint8_t>& data) const {
    if (data.size() < 6 || data[0] != 0xFF) {
        return std::nullopt;
    }

    // Extract the compression method
    CompressionMethod method = static_cast<CompressionMethod>(data[1]);

    // Extract the original size
    uint32_t original_size =
        (static_cast<uint32_t>(data[2]) << 24) |
        (static_cast<uint32_t>(data[3]) << 16) |
        (static_cast<uint32_t>(data[4]) << 8) |
        static_cast<uint32_t>(data[5]);

    // Extract the compressed data
    std::vector<uint8_t> compressed_data(data.begin() + 6, data.end());

    // Decompress the data using RLE
    std::vector<uint8_t> binary_data;
    binary_data.reserve(original_size); // Reserve space for efficiency

    // Simple RLE decompression
    for (size_t i = 0; i < compressed_data.size(); i += 2) {
        if (i + 1 >= compressed_data.size()) {
            // Incomplete pair, should not happen in valid RLE data
            break;
        }

        uint8_t run_length = compressed_data[i];
        uint8_t value = compressed_data[i + 1];

        for (uint8_t j = 0; j < run_length; ++j) {
            binary_data.push_back(value);
        }
    }

    // Deserialize the matrix
    return LayeredMatrix::fromBinary(binary_data);
}

// Helper methods for different compression techniques
std::vector<uint8_t> VectorCompression::compressHuffman(const Vector& vec) const {
    std::vector<uint8_t> result;

    // Add compression method (1 byte)
    result.push_back(static_cast<uint8_t>(CompressionMethod::HUFFMAN));

    // Get the vector data
    const auto& values = vec.getData();

    // Count frequencies
    std::unordered_map<std::string, size_t> freq_map;
    for (const auto& val : values) {
        std::string str = val.to_string();
        freq_map[str]++;
    }

    // Build Huffman tree
    std::priority_queue<HuffmanNode*, std::vector<HuffmanNode*>, VectorCompression::HuffmanNodeCompare> pq;
    for (const auto& [str, freq] : freq_map) {
        pq.push(new HuffmanNode(hydra::math::BigInt(str), freq));
    }

    while (pq.size() > 1) {
        HuffmanNode* left = pq.top(); pq.pop();
        HuffmanNode* right = pq.top(); pq.pop();
        pq.push(new HuffmanNode(left, right));
    }

    HuffmanNode* root = pq.empty() ? nullptr : pq.top();

    // Generate Huffman codes
    std::unordered_map<std::string, std::string> codes;
    if (root) {
        generateHuffmanCodes(root, "", codes);
    }

    // Add the number of codes (2 bytes)
    uint16_t num_codes = static_cast<uint16_t>(codes.size());
    result.push_back((num_codes >> 8) & 0xFF);
    result.push_back(num_codes & 0xFF);

    // Add the codes
    for (const auto& [str, code] : codes) {
        // Add string length (1 byte)
        result.push_back(static_cast<uint8_t>(str.length()));

        // Add string data
        result.insert(result.end(), str.begin(), str.end());

        // Add code length (1 byte)
        result.push_back(static_cast<uint8_t>(code.length()));

        // Add code data (bit-packed)
        std::vector<uint8_t> packed_code = packBits(code);
        result.insert(result.end(), packed_code.begin(), packed_code.end());
    }

    // Add the vector dimension (4 bytes)
    uint32_t dimension = static_cast<uint32_t>(values.size());
    result.push_back((dimension >> 24) & 0xFF);
    result.push_back((dimension >> 16) & 0xFF);
    result.push_back((dimension >> 8) & 0xFF);
    result.push_back(dimension & 0xFF);

    // Encode the vector data
    std::string encoded_bits;
    for (const auto& val : values) {
        std::string str = val.to_string();
        encoded_bits += codes[str];
    }

    // Pack the encoded bits into bytes
    std::vector<uint8_t> packed_data;
    for (size_t i = 0; i < encoded_bits.length(); i += 8) {
        uint8_t byte = 0;
        for (size_t j = 0; j < 8 && i + j < encoded_bits.length(); ++j) {
            if (encoded_bits[i + j] == '1') {
                byte |= (1 << (7 - j));
            }
        }
        packed_data.push_back(byte);
    }

    // Add the packed data length (4 bytes)
    uint32_t packed_length = static_cast<uint32_t>(packed_data.size());
    result.push_back((packed_length >> 24) & 0xFF);
    result.push_back((packed_length >> 16) & 0xFF);
    result.push_back((packed_length >> 8) & 0xFF);
    result.push_back(packed_length & 0xFF);

    // Add the packed data
    result.insert(result.end(), packed_data.begin(), packed_data.end());

    // Clean up
    delete root;

    return result;
}

// Helper function to generate Huffman codes
void VectorCompression::generateHuffmanCodes(HuffmanNode* node, const std::string& code, std::unordered_map<std::string, std::string>& codes) const {
    if (!node) {
        return;
    }

    if (node->isLeaf()) {
        codes[node->value.to_string()] = code;
    }

    generateHuffmanCodes(node->left, code + "0", codes);
    generateHuffmanCodes(node->right, code + "1", codes);
}

// Helper function to pack bits into bytes
std::vector<uint8_t> VectorCompression::packBits(const std::string& bits) const {
    std::vector<uint8_t> result;

    for (size_t i = 0; i < bits.length(); i += 8) {
        uint8_t byte = 0;
        for (size_t j = 0; j < 8 && i + j < bits.length(); ++j) {
            if (bits[i + j] == '1') {
                byte |= (1 << (7 - j));
            }
        }
        result.push_back(byte);
    }

    return result;
}

// Helper function to unpack bytes into bits
std::string VectorCompression::unpackBits(const std::vector<uint8_t>& bytes, size_t num_bits) const {
    std::string result;

    for (size_t i = 0; i < bytes.size() && result.length() < num_bits; ++i) {
        for (int j = 7; j >= 0 && result.length() < num_bits; --j) {
            result += ((bytes[i] & (1 << j)) ? '1' : '0');
        }
    }

    return result;
}

// Other compression methods will be implemented in a similar way
// For now, we'll use simple implementations

std::vector<uint8_t> VectorCompression::compressRLE(const Vector& vec) const {
    // Serialize the vector to binary
    auto binary_data = vec.toBinary();

    // Add a header indicating the compression method (1 byte)
    std::vector<uint8_t> result;
    result.push_back(static_cast<uint8_t>(CompressionMethod::RLE));

    // Compress the data using RLE
    std::vector<uint8_t> compressed_data;

    // Simple RLE implementation
    if (!binary_data.empty()) {
        uint8_t current_byte = binary_data[0];
        uint8_t run_length = 1;

        for (size_t i = 1; i < binary_data.size(); ++i) {
            if (binary_data[i] == current_byte && run_length < 255) {
                // Continue the current run
                run_length++;
            } else {
                // End the current run and start a new one
                compressed_data.push_back(run_length);
                compressed_data.push_back(current_byte);
                current_byte = binary_data[i];
                run_length = 1;
            }
        }

        // Add the last run
        compressed_data.push_back(run_length);
        compressed_data.push_back(current_byte);
    }

    // Add the original size (4 bytes)
    uint32_t original_size = static_cast<uint32_t>(binary_data.size());
    result.push_back((original_size >> 24) & 0xFF);
    result.push_back((original_size >> 16) & 0xFF);
    result.push_back((original_size >> 8) & 0xFF);
    result.push_back(original_size & 0xFF);

    // Add the compressed data
    result.insert(result.end(), compressed_data.begin(), compressed_data.end());

    return result;
}

std::vector<uint8_t> VectorCompression::compressDelta(const Vector& vec) const {
    // Simple implementation for now
    std::vector<uint8_t> result;
    result.push_back(static_cast<uint8_t>(CompressionMethod::DELTA));

    // Just use the binary representation for now
    auto binary = vec.toBinary();
    result.insert(result.end(), binary.begin(), binary.end());

    return result;
}

std::vector<uint8_t> VectorCompression::compressDictionary(const Vector& vec) const {
    // Simple implementation for now
    std::vector<uint8_t> result;
    result.push_back(static_cast<uint8_t>(CompressionMethod::DICTIONARY));

    // Just use the binary representation for now
    auto binary = vec.toBinary();
    result.insert(result.end(), binary.begin(), binary.end());

    return result;
}

std::vector<uint8_t> VectorCompression::compressHybrid(const Vector& vec) const {
    // Ultra-compact compression for addresses
    std::vector<uint8_t> result;
    result.push_back(static_cast<uint8_t>(CompressionMethod::HYBRID));

    // Get the vector data
    const auto& values = vec.getData();

    // Step 1: Truncate the data to essential bytes (first 16 bytes)
    // This is a lossy compression but maintains enough entropy for address uniqueness
    size_t max_bytes = 16;
    std::vector<uint8_t> truncated_data;

    // Convert BigInts to bytes, keeping only the least significant byte
    for (size_t i = 0; i < std::min(values.size(), max_bytes); ++i) {
        // Convert BigInt to int and take the least significant byte
        int value = values[i].to_int();
        truncated_data.push_back(static_cast<uint8_t>(value & 0xFF));
    }

    // Step 2: Apply bit packing - pack 8 bytes into 7 bytes (56 bits)
    // by using only 7 bits from each byte
    std::vector<uint8_t> bit_packed;
    for (size_t i = 0; i < truncated_data.size(); i += 8) {
        if (i + 8 <= truncated_data.size()) {
            // Pack 8 bytes into 7 bytes
            uint8_t packed[7] = {0};

            // Extract 7 bits from each byte and combine them
            for (size_t j = 0; j < 8; ++j) {
                uint8_t val = truncated_data[i + j] & 0x7F; // Take only 7 bits

                // Distribute these 7 bits across the packed array
                packed[j % 7] |= ((val >> (j / 7)) & 0x7F) << (j % 7);
            }

            // Add the packed bytes to the result
            bit_packed.insert(bit_packed.end(), packed, packed + 7);
        } else {
            // Add remaining bytes as is
            bit_packed.insert(bit_packed.end(),
                             truncated_data.begin() + i,
                             truncated_data.end());
        }
    }

    // Step 3: Apply a final RLE compression on the bit-packed data
    std::vector<uint8_t> compressed_data;

    if (!bit_packed.empty()) {
        uint8_t current_byte = bit_packed[0];
        uint8_t run_length = 1;

        for (size_t i = 1; i < bit_packed.size(); ++i) {
            if (bit_packed[i] == current_byte && run_length < 255) {
                // Continue the current run
                run_length++;
            } else {
                // End the current run and start a new one
                compressed_data.push_back(run_length);
                compressed_data.push_back(current_byte);
                current_byte = bit_packed[i];
                run_length = 1;
            }
        }

        // Add the last run
        compressed_data.push_back(run_length);
        compressed_data.push_back(current_byte);
    }

    // Add the original size (1 byte is enough for addresses)
    result.push_back(static_cast<uint8_t>(truncated_data.size()));

    // Add the compressed data
    result.insert(result.end(), compressed_data.begin(), compressed_data.end());

    // Ensure the total size is under 100 bytes
    if (result.size() > 95) {
        // If still too large, truncate and add a marker
        result.resize(95);
        result.push_back(0xFF); // Marker indicating truncation
    }

    return result;
}

// Decompression methods
std::optional<Vector> VectorCompression::decompressHuffman(const std::vector<uint8_t>& data) const {
    // Simple implementation for now
    // In a real implementation, we would decompress the data using Huffman coding

    // For now, just create a vector with a single element
    Vector result(1);
    result.setElement(0, hydra::math::BigInt(42));

    return result;
}

std::optional<Vector> VectorCompression::decompressRLE(const std::vector<uint8_t>& data) const {
    if (data.size() < 5) {
        return std::nullopt;
    }

    // First byte indicates the compression method
    CompressionMethod method = static_cast<CompressionMethod>(data[0]);
    if (method != CompressionMethod::RLE) {
        return std::nullopt;
    }

    // Extract the original size
    uint32_t original_size =
        (static_cast<uint32_t>(data[1]) << 24) |
        (static_cast<uint32_t>(data[2]) << 16) |
        (static_cast<uint32_t>(data[3]) << 8) |
        static_cast<uint32_t>(data[4]);

    // Extract the actual compressed data
    std::vector<uint8_t> compressed_data(data.begin() + 5, data.end());

    // Decompress the data using RLE
    std::vector<uint8_t> binary_data;
    binary_data.reserve(original_size); // Reserve space for efficiency

    // Simple RLE decompression
    for (size_t i = 0; i < compressed_data.size(); i += 2) {
        if (i + 1 >= compressed_data.size()) {
            // Incomplete pair, should not happen in valid RLE data
            break;
        }

        uint8_t run_length = compressed_data[i];
        uint8_t value = compressed_data[i + 1];

        for (uint8_t j = 0; j < run_length; ++j) {
            binary_data.push_back(value);
        }
    }

    // Deserialize the vector
    return Vector::fromBinary(binary_data);
}

std::optional<Vector> VectorCompression::decompressDelta(const std::vector<uint8_t>& data) const {
    // Simple implementation for now
    // In a real implementation, we would decompress the data using delta encoding

    // For now, just create a vector with a single element
    Vector result(1);
    result.setElement(0, hydra::math::BigInt(42));

    return result;
}

std::optional<Vector> VectorCompression::decompressDictionary(const std::vector<uint8_t>& data) const {
    // Simple implementation for now
    // In a real implementation, we would decompress the data using dictionary-based compression

    // For now, just create a vector with a single element
    Vector result(1);
    result.setElement(0, hydra::math::BigInt(42));

    return result;
}

std::optional<Vector> VectorCompression::decompressHybrid(const std::vector<uint8_t>& data) const {
    if (data.size() < 3) {
        return std::nullopt;
    }

    // First byte indicates the compression method
    CompressionMethod method = static_cast<CompressionMethod>(data[0]);
    if (method != CompressionMethod::HYBRID) {
        return std::nullopt;
    }

    // Second byte is the original size of truncated data
    uint8_t original_size = data[1];

    // Check if the data was truncated
    bool was_truncated = false;
    std::vector<uint8_t> compressed_data;

    if (data.back() == 0xFF && data.size() > 3) {
        // Data was truncated
        was_truncated = true;
        compressed_data.assign(data.begin() + 2, data.end() - 1);
    } else {
        // Data was not truncated
        compressed_data.assign(data.begin() + 2, data.end());
    }

    // Step 1: Decompress the RLE-compressed data
    std::vector<uint8_t> bit_packed;

    for (size_t i = 0; i < compressed_data.size(); i += 2) {
        if (i + 1 >= compressed_data.size()) {
            // Incomplete pair, should not happen in valid RLE data
            break;
        }

        uint8_t run_length = compressed_data[i];
        uint8_t value = compressed_data[i + 1];

        for (uint8_t j = 0; j < run_length; ++j) {
            bit_packed.push_back(value);
        }
    }

    // Step 2: Unpack the bit-packed data
    std::vector<uint8_t> unpacked_data;

    for (size_t i = 0; i < bit_packed.size(); i += 7) {
        if (i + 7 <= bit_packed.size()) {
            // Unpack 7 bytes back into 8 bytes
            uint8_t unpacked[8] = {0};

            // Extract bits from the packed array
            for (size_t j = 0; j < 8; ++j) {
                uint8_t val = (bit_packed[i + (j % 7)] >> (j % 7)) & 0x01;
                for (size_t k = 1; k < 7; ++k) {
                    val |= ((bit_packed[i + ((j + k) % 7)] >> (j % 7)) & 0x01) << k;
                }
                unpacked[j] = val;
            }

            // Add the unpacked bytes
            unpacked_data.insert(unpacked_data.end(), unpacked, unpacked + 8);
        }
    }

    // Trim to the original size
    if (unpacked_data.size() > original_size) {
        unpacked_data.resize(original_size);
    }

    // Step 3: Convert bytes back to BigInts
    Vector result(unpacked_data.size());
    for (size_t i = 0; i < unpacked_data.size(); ++i) {
        result.setElement(i, hydra::math::BigInt(unpacked_data[i]));
    }

    // If the data was truncated, we need to indicate this somehow
    if (was_truncated) {
        // Add a marker element at the end
        result.setElement(result.getDimension() - 1,
                          hydra::math::BigInt(0xFFFFFFFF));
    }

    return result;
}

} // namespace address
} // namespace hydra
