#ifndef HYDRA_COMPRESSION_OST_H
#define HYDRA_COMPRESSION_OST_H

#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <memory>
#include <functional>
#include <cstdint>

namespace hydra {
namespace compression {

/**
 * @brief Compression strategy interface for OST algorithm
 * 
 * This interface defines the methods that any compression strategy
 * used with the OST algorithm must implement.
 */
class CompressionStrategy {
public:
    virtual ~CompressionStrategy() = default;
    
    /**
     * @brief Compress the given data
     * 
     * @param data Data to compress
     * @return Compressed data
     */
    virtual std::vector<uint8_t> compress(const std::vector<uint8_t>& data) = 0;
    
    /**
     * @brief Decompress the given data
     * 
     * @param data Data to decompress
     * @return Decompressed data
     */
    virtual std::vector<uint8_t> decompress(const std::vector<uint8_t>& data) = 0;
    
    /**
     * @brief Get the name of the compression strategy
     * 
     * @return Strategy name
     */
    virtual std::string getName() const = 0;
};

/**
 * @brief Classification strategy interface for OST algorithm
 * 
 * This interface defines the methods that any classification strategy
 * used with the OST algorithm must implement.
 */
class ClassificationStrategy {
public:
    virtual ~ClassificationStrategy() = default;
    
    /**
     * @brief Classify the given data into a label
     * 
     * @param data Data to classify
     * @return Label representing the classification
     */
    virtual std::string classify(const std::vector<uint8_t>& data) = 0;
    
    /**
     * @brief Get the name of the classification strategy
     * 
     * @return Strategy name
     */
    virtual std::string getName() const = 0;
};

/**
 * @brief Bin for storing similar sequences in the OST algorithm
 */
struct Bin {
    std::string label;                    ///< Label of the bin
    std::vector<std::vector<uint8_t>> sequences; ///< Sequences in the bin
    std::vector<uint8_t> compressed_data; ///< Compressed data of the bin
};

/**
 * @brief OST compression algorithm
 * 
 * Implementation of the Okaily-Srivastava-Tbakhi compression algorithm
 * that uses a divide-and-conquer approach by classifying subsequences
 * based on similarities in their content and binning similar subsequences together.
 */
class OSTCompressor {
public:
    /**
     * @brief Create a new OST compressor
     * 
     * @param classification_strategy Strategy for classifying subsequences
     * @param compression_strategy Strategy for compressing bins
     * @param window_length Length of the window for subsequences
     * @param label_length Length of the label for classification
     */
    OSTCompressor(
        std::shared_ptr<ClassificationStrategy> classification_strategy,
        std::shared_ptr<CompressionStrategy> compression_strategy,
        size_t window_length = 1000,
        size_t label_length = 4
    );
    
    /**
     * @brief Compress the given data
     * 
     * @param data Data to compress
     * @return Compressed data
     */
    std::vector<uint8_t> compress(const std::vector<uint8_t>& data);
    
    /**
     * @brief Decompress the given data
     * 
     * @param data Data to decompress
     * @return Decompressed data
     */
    std::vector<uint8_t> decompress(const std::vector<uint8_t>& data);
    
    /**
     * @brief Set the window length
     * 
     * @param window_length New window length
     */
    void setWindowLength(size_t window_length);
    
    /**
     * @brief Set the label length
     * 
     * @param label_length New label length
     */
    void setLabelLength(size_t label_length);
    
    /**
     * @brief Get the window length
     * 
     * @return Current window length
     */
    size_t getWindowLength() const;
    
    /**
     * @brief Get the label length
     * 
     * @return Current label length
     */
    size_t getLabelLength() const;
    
    /**
     * @brief Get the compression strategy
     * 
     * @return Current compression strategy
     */
    std::shared_ptr<CompressionStrategy> getCompressionStrategy() const;
    
    /**
     * @brief Get the classification strategy
     * 
     * @return Current classification strategy
     */
    std::shared_ptr<ClassificationStrategy> getClassificationStrategy() const;
    
    /**
     * @brief Set the compression strategy
     * 
     * @param compression_strategy New compression strategy
     */
    void setCompressionStrategy(std::shared_ptr<CompressionStrategy> compression_strategy);
    
    /**
     * @brief Set the classification strategy
     * 
     * @param classification_strategy New classification strategy
     */
    void setClassificationStrategy(std::shared_ptr<ClassificationStrategy> classification_strategy);
    
private:
    std::shared_ptr<ClassificationStrategy> m_classification_strategy;
    std::shared_ptr<CompressionStrategy> m_compression_strategy;
    size_t m_window_length;
    size_t m_label_length;
    
    /**
     * @brief Classify a subsequence and get its bin label
     * 
     * @param subsequence Subsequence to classify
     * @return Bin label
     */
    std::string getBinLabel(const std::vector<uint8_t>& subsequence);
    
    /**
     * @brief Encode bin labels using Huffman encoding
     * 
     * @param labels Labels to encode
     * @return Encoded labels
     */
    std::vector<uint8_t> encodeLabels(const std::vector<std::string>& labels);
    
    /**
     * @brief Decode bin labels from Huffman encoding
     * 
     * @param encoded_labels Encoded labels
     * @return Decoded labels
     */
    std::vector<std::string> decodeLabels(const std::vector<uint8_t>& encoded_labels);
};

} // namespace compression
} // namespace hydra

#endif // HYDRA_COMPRESSION_OST_H
