#pragma once

#include <vector>
#include <optional>
#include <hydra_math/bigint.hpp>
#include <hydra_address/layered_matrix.hpp>

namespace hydra {
namespace address {

/**
 * @brief Compression method for vectors
 */
enum class CompressionMethod {
    HUFFMAN,       ///< Huffman coding
    RLE,           ///< Run-length encoding
    DELTA,         ///< Delta encoding
    DICTIONARY,    ///< Dictionary-based compression
    HYBRID         ///< Hybrid approach combining multiple methods
};

/**
 * @brief Vector compression utility class
 */
class VectorCompression {
public:
    /**
     * @brief Default constructor
     */
    VectorCompression();

    /**
     * @brief Constructor with compression method
     * @param method Compression method
     */
    explicit VectorCompression(CompressionMethod method);

    /**
     * @brief Compress a vector
     * @param vec Vector to compress
     * @return Compressed data
     */
    std::vector<uint8_t> compress(const Vector& vec) const;

    /**
     * @brief Decompress data to a vector
     * @param data Compressed data
     * @return Decompressed vector or empty if invalid
     */
    std::optional<Vector> decompress(const std::vector<uint8_t>& data) const;

    /**
     * @brief Get the compression method
     * @return Compression method
     */
    CompressionMethod getMethod() const;

    /**
     * @brief Set the compression method
     * @param method Compression method
     */
    void setMethod(CompressionMethod method);

    /**
     * @brief Get the compression ratio for a vector
     * @param vec Vector to analyze
     * @return Compression ratio (original size / compressed size)
     */
    double getCompressionRatio(const Vector& vec) const;

    /**
     * @brief Compress a vector of BigInt values
     * @param values BigInt values
     * @return Compressed data
     */
    std::vector<uint8_t> compressBigInts(const std::vector<hydra::math::BigInt>& values) const;

    /**
     * @brief Decompress data to a vector of BigInt values
     * @param data Compressed data
     * @return Decompressed BigInt values or empty if invalid
     */
    std::optional<std::vector<hydra::math::BigInt>> decompressBigInts(const std::vector<uint8_t>& data) const;

    /**
     * @brief Compress a layered matrix
     * @param matrix Layered matrix to compress
     * @return Compressed data
     */
    std::vector<uint8_t> compressMatrix(const LayeredMatrix& matrix) const;

    /**
     * @brief Decompress data to a layered matrix
     * @param data Compressed data
     * @return Decompressed layered matrix or empty if invalid
     */
    std::optional<LayeredMatrix> decompressMatrix(const std::vector<uint8_t>& data) const;

private:
    CompressionMethod m_method;

    // Definition of Huffman node
    struct HuffmanNode {
        hydra::math::BigInt value;
        size_t frequency;
        HuffmanNode* left;
        HuffmanNode* right;

        HuffmanNode(const hydra::math::BigInt& val, size_t freq)
            : value(val), frequency(freq), left(nullptr), right(nullptr) {}

        HuffmanNode(HuffmanNode* l, HuffmanNode* r)
            : frequency(l->frequency + r->frequency), left(l), right(r) {}

        ~HuffmanNode() {
            delete left;
            delete right;
        }

        bool isLeaf() const {
            return left == nullptr && right == nullptr;
        }
    };

    // Comparator for priority queue
    struct HuffmanNodeCompare {
        bool operator()(const HuffmanNode* lhs, const HuffmanNode* rhs) const {
            return lhs->frequency > rhs->frequency;
        }
    };

    // Helper methods for different compression techniques
    std::vector<uint8_t> compressHuffman(const Vector& vec) const;
    std::optional<Vector> decompressHuffman(const std::vector<uint8_t>& data) const;

    std::vector<uint8_t> compressRLE(const Vector& vec) const;
    std::optional<Vector> decompressRLE(const std::vector<uint8_t>& data) const;

    std::vector<uint8_t> compressDelta(const Vector& vec) const;
    std::optional<Vector> decompressDelta(const std::vector<uint8_t>& data) const;

    std::vector<uint8_t> compressDictionary(const Vector& vec) const;
    std::optional<Vector> decompressDictionary(const std::vector<uint8_t>& data) const;

    std::vector<uint8_t> compressHybrid(const Vector& vec) const;
    std::optional<Vector> decompressHybrid(const std::vector<uint8_t>& data) const;

    // Helper methods for Huffman coding
    void generateHuffmanCodes(HuffmanNode* node, const std::string& code, std::unordered_map<std::string, std::string>& codes) const;
    std::vector<uint8_t> packBits(const std::string& bits) const;
    std::string unpackBits(const std::vector<uint8_t>& bytes, size_t num_bits) const;
};

} // namespace address
} // namespace hydra
