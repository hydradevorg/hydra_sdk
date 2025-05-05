#include <hydra_compression/ost.h>
#include <hydra_compression/huffman.hpp>
#include <iostream>
#include <algorithm>
#include <sstream>
#include <queue>
#include <unordered_map>
#include <map>
#include <cassert>

namespace hydra {
namespace compression {

OSTCompressor::OSTCompressor(
    std::shared_ptr<ClassificationStrategy> classification_strategy,
    std::shared_ptr<CompressionStrategy> compression_strategy,
    size_t window_length,
    size_t label_length
) : m_classification_strategy(classification_strategy),
    m_compression_strategy(compression_strategy),
    m_window_length(window_length),
    m_label_length(label_length) {
}

std::vector<uint8_t> OSTCompressor::compress(const std::vector<uint8_t>& data) {
    // Check if data is empty
    if (data.empty()) {
        return {};
    }
    
    // Maps to store bins and labels
    std::unordered_map<std::string, Bin> bins;
    std::vector<std::string> label_sequence;
    
    // Step 1: Divide the data into non-overlapping windows and classify them
    for (size_t i = 0; i < data.size(); i += m_window_length) {
        // Extract the subsequence (window)
        size_t window_size = std::min(m_window_length, data.size() - i);
        std::vector<uint8_t> subsequence(data.begin() + i, data.begin() + i + window_size);
        
        // Classify the subsequence and get its bin label
        std::string label = getBinLabel(subsequence);
        label_sequence.push_back(label);
        
        // Create bin if it doesn't exist
        if (bins.find(label) == bins.end()) {
            bins[label] = Bin{label, {}, {}};
        }
        
        // Add subsequence to the appropriate bin
        bins[label].sequences.push_back(subsequence);
    }
    
    // Step 2: Compress each bin using the compression strategy
    for (auto& [label, bin] : bins) {
        // Concatenate all sequences in the bin
        std::vector<uint8_t> bin_data;
        for (const auto& seq : bin.sequences) {
            bin_data.insert(bin_data.end(), seq.begin(), seq.end());
        }
        
        // Compress the bin data
        bin.compressed_data = m_compression_strategy->compress(bin_data);
    }
    
    // Step 3: Encode the label sequence using Huffman encoding
    std::vector<uint8_t> encoded_labels = encodeLabels(label_sequence);
    
    // Step 4: Prepare the final compressed output
    std::vector<uint8_t> compressed_data;
    
    // First 4 bytes: Window length
    for (int i = 0; i < 4; ++i) {
        compressed_data.push_back((m_window_length >> (i * 8)) & 0xFF);
    }
    
    // Next 4 bytes: Label length
    for (int i = 0; i < 4; ++i) {
        compressed_data.push_back((m_label_length >> (i * 8)) & 0xFF);
    }
    
    // Next 4 bytes: Number of bins
    uint32_t num_bins = bins.size();
    for (int i = 0; i < 4; ++i) {
        compressed_data.push_back((num_bins >> (i * 8)) & 0xFF);
    }
    
    // Next 4 bytes: Length of encoded labels
    uint32_t encoded_labels_size = encoded_labels.size();
    for (int i = 0; i < 4; ++i) {
        compressed_data.push_back((encoded_labels_size >> (i * 8)) & 0xFF);
    }
    
    // Add encoded labels
    compressed_data.insert(compressed_data.end(), encoded_labels.begin(), encoded_labels.end());
    
    // For each bin, store: label, compressed data size, compressed data
    for (const auto& [label, bin] : bins) {
        // Add label (fixed size based on label_length)
        for (size_t i = 0; i < label.size() && i < 32; ++i) {
            compressed_data.push_back(label[i]);
        }
        // Pad label if needed
        for (size_t i = label.size(); i < 32; ++i) {
            compressed_data.push_back(0);
        }
        
        // Add compressed data size (4 bytes)
        uint32_t bin_size = bin.compressed_data.size();
        for (int i = 0; i < 4; ++i) {
            compressed_data.push_back((bin_size >> (i * 8)) & 0xFF);
        }
        
        // Add compressed data
        compressed_data.insert(compressed_data.end(), bin.compressed_data.begin(), bin.compressed_data.end());
    }
    
    return compressed_data;
}

std::vector<uint8_t> OSTCompressor::decompress(const std::vector<uint8_t>& data) {
    // Check if data is empty
    if (data.empty()) {
        return {};
    }
    
    // Extract header information
    size_t offset = 0;
    
    // Window length (4 bytes)
    uint32_t window_length = 0;
    for (int i = 0; i < 4; ++i) {
        window_length |= static_cast<uint32_t>(data[offset++]) << (i * 8);
    }
    m_window_length = window_length;
    
    // Label length (4 bytes)
    uint32_t label_length = 0;
    for (int i = 0; i < 4; ++i) {
        label_length |= static_cast<uint32_t>(data[offset++]) << (i * 8);
    }
    m_label_length = label_length;
    
    // Number of bins (4 bytes)
    uint32_t num_bins = 0;
    for (int i = 0; i < 4; ++i) {
        num_bins |= static_cast<uint32_t>(data[offset++]) << (i * 8);
    }
    
    // Length of encoded labels (4 bytes)
    uint32_t encoded_labels_size = 0;
    for (int i = 0; i < 4; ++i) {
        encoded_labels_size |= static_cast<uint32_t>(data[offset++]) << (i * 8);
    }
    
    // Extract encoded labels
    std::vector<uint8_t> encoded_labels(data.begin() + offset, data.begin() + offset + encoded_labels_size);
    offset += encoded_labels_size;
    
    // Decode the label sequence
    std::vector<std::string> label_sequence = decodeLabels(encoded_labels);
    
    // Extract bin data
    std::unordered_map<std::string, std::vector<uint8_t>> bin_compressed_data;
    for (uint32_t i = 0; i < num_bins; ++i) {
        // Extract label (fixed 32 bytes, but actual length is label_length)
        std::string label;
        for (size_t j = 0; j < 32; ++j) {
            if (data[offset + j] != 0) {
                label.push_back(data[offset + j]);
            }
        }
        offset += 32;
        
        // Extract compressed data size (4 bytes)
        uint32_t bin_size = 0;
        for (int j = 0; j < 4; ++j) {
            bin_size |= static_cast<uint32_t>(data[offset++]) << (j * 8);
        }
        
        // Extract compressed data
        bin_compressed_data[label] = std::vector<uint8_t>(data.begin() + offset, data.begin() + offset + bin_size);
        offset += bin_size;
    }
    
    // Decompress bins
    std::unordered_map<std::string, std::vector<uint8_t>> bin_data;
    for (const auto& [label, compressed] : bin_compressed_data) {
        bin_data[label] = m_compression_strategy->decompress(compressed);
    }
    
    // Reconstruct the original data using the label sequence
    std::vector<uint8_t> decompressed_data;
    std::unordered_map<std::string, size_t> bin_offsets;
    
    for (const std::string& label : label_sequence) {
        // If this is the first time we're using this bin, initialize its offset
        if (bin_offsets.find(label) == bin_offsets.end()) {
            bin_offsets[label] = 0;
        }
        
        // Get the current offset for this bin
        size_t offset = bin_offsets[label];
        
        // Extract a window-sized chunk from the bin data
        size_t window_size = std::min(m_window_length, bin_data[label].size() - offset);
        std::vector<uint8_t> window(bin_data[label].begin() + offset, bin_data[label].begin() + offset + window_size);
        
        // Add the window to the decompressed data
        decompressed_data.insert(decompressed_data.end(), window.begin(), window.end());
        
        // Update the offset for this bin
        bin_offsets[label] += window_size;
    }
    
    return decompressed_data;
}

std::string OSTCompressor::getBinLabel(const std::vector<uint8_t>& subsequence) {
    // Use the classification strategy to classify the subsequence
    std::string full_label = m_classification_strategy->classify(subsequence);
    
    // Truncate the label to the specified label length if needed
    if (full_label.length() > m_label_length) {
        return full_label.substr(0, m_label_length);
    }
    
    return full_label;
}

std::vector<uint8_t> OSTCompressor::encodeLabels(const std::vector<std::string>& labels) {
    // Count frequencies of each label
    std::unordered_map<std::string, int> label_frequencies;
    for (const auto& label : labels) {
        label_frequencies[label]++;
    }
    
    // Convert to format needed for Huffman encoding
    std::vector<std::pair<std::string, double>> frequencies;
    for (const auto& [label, freq] : label_frequencies) {
        frequencies.push_back({label, static_cast<double>(freq)});
    }
    
    // Create Huffman tree
    auto huffman_tree = hydra::compression::HuffmanTree<std::string>(frequencies);
    auto huffman_codes = huffman_tree.getCodes();
    
    // Encode the Huffman table for later decoding
    std::vector<uint8_t> encoded_table;
    
    // First byte: Number of entries in the Huffman table
    encoded_table.push_back(huffman_codes.size());
    
    // For each entry, store: label length, label, code length, code
    for (const auto& [label, code] : huffman_codes) {
        // Label length (1 byte)
        encoded_table.push_back(label.size());
        
        // Label
        for (char c : label) {
            encoded_table.push_back(c);
        }
        
        // Code length in bits (1 byte)
        encoded_table.push_back(code.size());
        
        // Code (packed into bytes)
        size_t code_bytes = (code.size() + 7) / 8;
        for (size_t i = 0; i < code_bytes; ++i) {
            uint8_t byte = 0;
            for (size_t j = 0; j < 8 && (i * 8 + j) < code.size(); ++j) {
                if (code[i * 8 + j] == '1') {
                    byte |= (1 << j);
                }
            }
            encoded_table.push_back(byte);
        }
    }
    
    // Encode the sequence of labels
    std::vector<uint8_t> encoded_sequence;
    std::string bit_buffer;
    
    for (const auto& label : labels) {
        bit_buffer += huffman_codes[label];
    }
    
    // Pack bits into bytes
    for (size_t i = 0; i < bit_buffer.size(); i += 8) {
        uint8_t byte = 0;
        for (size_t j = 0; j < 8 && (i + j) < bit_buffer.size(); ++j) {
            if (bit_buffer[i + j] == '1') {
                byte |= (1 << j);
            }
        }
        encoded_sequence.push_back(byte);
    }
    
    // Combine the encoded table and sequence
    std::vector<uint8_t> encoded_data;
    
    // First 4 bytes: Length of encoded table
    uint32_t table_size = encoded_table.size();
    for (int i = 0; i < 4; ++i) {
        encoded_data.push_back((table_size >> (i * 8)) & 0xFF);
    }
    
    // Add encoded table and sequence
    encoded_data.insert(encoded_data.end(), encoded_table.begin(), encoded_table.end());
    encoded_data.insert(encoded_data.end(), encoded_sequence.begin(), encoded_sequence.end());
    
    return encoded_data;
}

std::vector<std::string> OSTCompressor::decodeLabels(const std::vector<uint8_t>& encoded_labels) {
    // Extract header information
    size_t offset = 0;
    
    // Length of encoded table (4 bytes)
    uint32_t table_size = 0;
    for (int i = 0; i < 4; ++i) {
        table_size |= static_cast<uint32_t>(encoded_labels[offset++]) << (i * 8);
    }
    
    // Extract the encoded table
    std::vector<uint8_t> encoded_table(encoded_labels.begin() + offset, 
                                     encoded_labels.begin() + offset + table_size);
    offset += table_size;
    
    // Decode the Huffman table
    size_t table_offset = 0;
    
    // Number of entries in the Huffman table
    uint8_t num_entries = encoded_table[table_offset++];
    
    // Rebuild the Huffman codes
    std::unordered_map<std::string, std::string> huffman_codes;
    std::unordered_map<std::string, std::string> reverse_codes;
    
    for (uint8_t i = 0; i < num_entries; ++i) {
        // Label length
        uint8_t label_length = encoded_table[table_offset++];
        
        // Label
        std::string label;
        for (uint8_t j = 0; j < label_length; ++j) {
            label.push_back(encoded_table[table_offset++]);
        }
        
        // Code length in bits
        uint8_t code_length = encoded_table[table_offset++];
        
        // Code (packed into bytes)
        std::string code;
        size_t code_bytes = (code_length + 7) / 8;
        for (size_t j = 0; j < code_bytes; ++j) {
            uint8_t byte = encoded_table[table_offset++];
            for (size_t k = 0; k < 8 && (j * 8 + k) < code_length; ++k) {
                code.push_back((byte & (1 << k)) ? '1' : '0');
            }
        }
        
        huffman_codes[label] = code;
        reverse_codes[code] = label;
    }
    
    // Decode the sequence of labels
    std::vector<std::string> labels;
    std::string current_code;
    
    // Extract bits from remaining bytes
    for (size_t i = offset; i < encoded_labels.size(); ++i) {
        uint8_t byte = encoded_labels[i];
        for (size_t j = 0; j < 8; ++j) {
            current_code.push_back((byte & (1 << j)) ? '1' : '0');
            
            // Check if the current code matches a label
            if (reverse_codes.find(current_code) != reverse_codes.end()) {
                labels.push_back(reverse_codes[current_code]);
                current_code.clear();
            }
        }
    }
    
    return labels;
}

void OSTCompressor::setWindowLength(size_t window_length) {
    m_window_length = window_length;
}

void OSTCompressor::setLabelLength(size_t label_length) {
    m_label_length = label_length;
}

size_t OSTCompressor::getWindowLength() const {
    return m_window_length;
}

size_t OSTCompressor::getLabelLength() const {
    return m_label_length;
}

std::shared_ptr<CompressionStrategy> OSTCompressor::getCompressionStrategy() const {
    return m_compression_strategy;
}

std::shared_ptr<ClassificationStrategy> OSTCompressor::getClassificationStrategy() const {
    return m_classification_strategy;
}

void OSTCompressor::setCompressionStrategy(std::shared_ptr<CompressionStrategy> compression_strategy) {
    m_compression_strategy = compression_strategy;
}

void OSTCompressor::setClassificationStrategy(std::shared_ptr<ClassificationStrategy> classification_strategy) {
    m_classification_strategy = classification_strategy;
}

} // namespace compression
} // namespace hydra
