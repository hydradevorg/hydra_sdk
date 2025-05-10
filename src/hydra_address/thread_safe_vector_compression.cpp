#include <hydra_address/thread_safe_vector_compression.hpp>
#include <thread>
#include <vector>
#include <mutex>
#include <algorithm>

namespace hydra {
namespace address {

ThreadSafeVectorCompression::ThreadSafeVectorCompression() : m_compressor() {}

ThreadSafeVectorCompression::ThreadSafeVectorCompression(CompressionMethod method) 
    : m_compressor(method) {}

std::vector<uint8_t> ThreadSafeVectorCompression::compress(const Vector& vec) const {
    // Use a shared lock for read-only operations
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    return m_compressor.compress(vec);
}

std::optional<Vector> ThreadSafeVectorCompression::decompress(const std::vector<uint8_t>& data) const {
    // Use a shared lock for read-only operations
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    return m_compressor.decompress(data);
}

CompressionMethod ThreadSafeVectorCompression::getMethod() const {
    // Use a shared lock for read-only operations
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    return m_compressor.getMethod();
}

void ThreadSafeVectorCompression::setMethod(CompressionMethod method) {
    // Use an exclusive lock for write operations
    std::unique_lock<std::shared_mutex> lock(m_mutex);
    m_compressor.setMethod(method);
}

double ThreadSafeVectorCompression::getCompressionRatio(const Vector& vec) const {
    // Use a shared lock for read-only operations
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    return m_compressor.getCompressionRatio(vec);
}

std::vector<uint8_t> ThreadSafeVectorCompression::compressBigInts(
    const std::vector<hydra::math::BigInt>& values
) const {
    // Use a shared lock for read-only operations
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    return m_compressor.compressBigInts(values);
}

std::optional<std::vector<hydra::math::BigInt>> ThreadSafeVectorCompression::decompressBigInts(
    const std::vector<uint8_t>& data
) const {
    // Use a shared lock for read-only operations
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    return m_compressor.decompressBigInts(data);
}

std::vector<uint8_t> ThreadSafeVectorCompression::compressMatrix(const LayeredMatrix& matrix) const {
    // Use a shared lock for read-only operations
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    return m_compressor.compressMatrix(matrix);
}

std::optional<LayeredMatrix> ThreadSafeVectorCompression::decompressMatrix(
    const std::vector<uint8_t>& data
) const {
    // Use a shared lock for read-only operations
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    return m_compressor.decompressMatrix(data);
}

size_t ThreadSafeVectorCompression::getOptimalThreadCount(size_t suggested_threads) const {
    // If suggested_threads is 0, use the number of hardware threads
    if (suggested_threads == 0) {
        return std::thread::hardware_concurrency();
    }
    
    // Otherwise, use the suggested number of threads
    return suggested_threads;
}

std::vector<std::vector<uint8_t>> ThreadSafeVectorCompression::compressVectorsInParallel(
    const std::vector<Vector>& vectors,
    size_t num_threads
) const {
    // Determine the number of threads to use
    size_t thread_count = getOptimalThreadCount(num_threads);
    
    // Ensure at least one thread
    thread_count = std::max(thread_count, static_cast<size_t>(1));
    
    // Limit the number of threads to the number of vectors
    thread_count = std::min(thread_count, vectors.size());
    
    // Create a vector to store the results
    std::vector<std::vector<uint8_t>> compressed_vectors(vectors.size());
    
    // Create a vector of threads
    std::vector<std::thread> threads;
    
    // Calculate the chunk size for each thread
    size_t chunk_size = vectors.size() / thread_count;
    
    // Create a mutex for thread-safe access to the compressed_vectors vector
    std::mutex compressed_vectors_mutex;
    
    // Create a shared lock for read-only access to the compressor
    std::shared_lock<std::shared_mutex> compressor_lock(m_mutex);
    
    // Create a worker function
    auto worker = [&](size_t start, size_t end) {
        // Create a local copy of the compressor for thread safety
        VectorCompression local_compressor(m_compressor.getMethod());
        
        // Compress vectors for this chunk
        for (size_t i = start; i < end; ++i) {
            std::vector<uint8_t> compressed = local_compressor.compress(vectors[i]);
            
            // Store the compressed data in the result vector
            std::lock_guard<std::mutex> lock(compressed_vectors_mutex);
            compressed_vectors[i] = std::move(compressed);
        }
    };
    
    // Launch the worker threads
    for (size_t i = 0; i < thread_count; ++i) {
        size_t start = i * chunk_size;
        size_t end = (i == thread_count - 1) ? vectors.size() : (i + 1) * chunk_size;
        
        threads.emplace_back(worker, start, end);
    }
    
    // Wait for all threads to complete
    for (auto& thread : threads) {
        thread.join();
    }
    
    return compressed_vectors;
}

std::vector<std::optional<Vector>> ThreadSafeVectorCompression::decompressVectorsInParallel(
    const std::vector<std::vector<uint8_t>>& data_vectors,
    size_t num_threads
) const {
    // Determine the number of threads to use
    size_t thread_count = getOptimalThreadCount(num_threads);
    
    // Ensure at least one thread
    thread_count = std::max(thread_count, static_cast<size_t>(1));
    
    // Limit the number of threads to the number of data vectors
    thread_count = std::min(thread_count, data_vectors.size());
    
    // Create a vector to store the results
    std::vector<std::optional<Vector>> decompressed_vectors(data_vectors.size());
    
    // Create a vector of threads
    std::vector<std::thread> threads;
    
    // Calculate the chunk size for each thread
    size_t chunk_size = data_vectors.size() / thread_count;
    
    // Create a mutex for thread-safe access to the decompressed_vectors vector
    std::mutex decompressed_vectors_mutex;
    
    // Create a shared lock for read-only access to the compressor
    std::shared_lock<std::shared_mutex> compressor_lock(m_mutex);
    
    // Create a worker function
    auto worker = [&](size_t start, size_t end) {
        // Create a local copy of the compressor for thread safety
        VectorCompression local_compressor(m_compressor.getMethod());
        
        // Decompress data vectors for this chunk
        for (size_t i = start; i < end; ++i) {
            std::optional<Vector> decompressed = local_compressor.decompress(data_vectors[i]);
            
            // Store the decompressed vector in the result vector
            std::lock_guard<std::mutex> lock(decompressed_vectors_mutex);
            decompressed_vectors[i] = std::move(decompressed);
        }
    };
    
    // Launch the worker threads
    for (size_t i = 0; i < thread_count; ++i) {
        size_t start = i * chunk_size;
        size_t end = (i == thread_count - 1) ? data_vectors.size() : (i + 1) * chunk_size;
        
        threads.emplace_back(worker, start, end);
    }
    
    // Wait for all threads to complete
    for (auto& thread : threads) {
        thread.join();
    }
    
    return decompressed_vectors;
}

} // namespace address
} // namespace hydra
