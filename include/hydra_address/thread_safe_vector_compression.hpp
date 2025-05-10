#pragma once

#include <hydra_address/vector_compression.hpp>
#include <mutex>
#include <shared_mutex>
#include <vector>
#include <optional>
#include <hydra_math/bigint.hpp>
#include <hydra_address/layered_matrix.hpp>

namespace hydra {
namespace address {

/**
 * @brief Thread-safe wrapper for VectorCompression
 * 
 * This class provides thread-safe access to the VectorCompression functionality,
 * allowing multiple threads to compress and decompress vectors concurrently without conflicts.
 */
class ThreadSafeVectorCompression {
public:
    /**
     * @brief Default constructor
     */
    ThreadSafeVectorCompression();

    /**
     * @brief Constructor with compression method
     * @param method Compression method
     */
    explicit ThreadSafeVectorCompression(CompressionMethod method);

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
    
    /**
     * @brief Compress multiple vectors in parallel
     * @param vectors Vectors to compress
     * @param num_threads Number of threads to use (0 for auto-detection)
     * @return Vector of compressed data
     */
    std::vector<std::vector<uint8_t>> compressVectorsInParallel(
        const std::vector<Vector>& vectors,
        size_t num_threads = 0
    ) const;
    
    /**
     * @brief Decompress multiple data vectors in parallel
     * @param data_vectors Compressed data vectors
     * @param num_threads Number of threads to use (0 for auto-detection)
     * @return Vector of decompressed vectors
     */
    std::vector<std::optional<Vector>> decompressVectorsInParallel(
        const std::vector<std::vector<uint8_t>>& data_vectors,
        size_t num_threads = 0
    ) const;

private:
    // The underlying vector compression
    VectorCompression m_compressor;
    
    // Mutex for thread safety
    mutable std::shared_mutex m_mutex;
    
    // Helper method to determine the optimal number of threads
    size_t getOptimalThreadCount(size_t suggested_threads) const;
};

} // namespace address
} // namespace hydra
