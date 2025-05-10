#include <hydra_crypto/thread_safe_blake3_hash.hpp>
#include <thread>
#include <vector>
#include <mutex>
#include <algorithm>

namespace hydra {
namespace crypto {

ThreadSafeBlake3Hash::ThreadSafeBlake3Hash() : m_hasher() {}

ThreadSafeBlake3Hash::ThreadSafeBlake3Hash(const std::array<uint8_t, KEY_SIZE>& key) 
    : m_hasher(key) {}

ThreadSafeBlake3Hash::ThreadSafeBlake3Hash(const std::string& context) 
    : m_hasher(context) {}

void ThreadSafeBlake3Hash::reset() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_hasher.reset();
}

void ThreadSafeBlake3Hash::update(const void* data, size_t size) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_hasher.update(data, size);
}

void ThreadSafeBlake3Hash::update(const std::vector<uint8_t>& data) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_hasher.update(data);
}

void ThreadSafeBlake3Hash::update(const std::string& data) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_hasher.update(data);
}

void ThreadSafeBlake3Hash::finalize(uint8_t* output, size_t output_size) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_hasher.finalize(output, output_size);
}

std::vector<uint8_t> ThreadSafeBlake3Hash::finalize(size_t output_size) {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_hasher.finalize(output_size);
}

std::string ThreadSafeBlake3Hash::finalizeHex(size_t output_size) {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_hasher.finalizeHex(output_size);
}

std::vector<uint8_t> ThreadSafeBlake3Hash::hash(const void* data, size_t size, size_t output_size) {
    // Static methods don't need locking as they create a new hasher instance
    return Blake3Hash::hash(data, size, output_size);
}

std::vector<uint8_t> ThreadSafeBlake3Hash::hash(const std::vector<uint8_t>& data, size_t output_size) {
    // Static methods don't need locking as they create a new hasher instance
    return Blake3Hash::hash(data, output_size);
}

std::vector<uint8_t> ThreadSafeBlake3Hash::hash(const std::string& data, size_t output_size) {
    // Static methods don't need locking as they create a new hasher instance
    return Blake3Hash::hash(data, output_size);
}

std::string ThreadSafeBlake3Hash::hashHex(const void* data, size_t size, size_t output_size) {
    // Static methods don't need locking as they create a new hasher instance
    return Blake3Hash::hashHex(data, size, output_size);
}

std::string ThreadSafeBlake3Hash::hashHex(const std::vector<uint8_t>& data, size_t output_size) {
    // Static methods don't need locking as they create a new hasher instance
    return Blake3Hash::hashHex(data, output_size);
}

std::string ThreadSafeBlake3Hash::hashHex(const std::string& data, size_t output_size) {
    // Static methods don't need locking as they create a new hasher instance
    return Blake3Hash::hashHex(data, output_size);
}

size_t ThreadSafeBlake3Hash::getOptimalThreadCount(size_t suggested_threads) {
    // If suggested_threads is 0, use the number of hardware threads
    if (suggested_threads == 0) {
        return std::thread::hardware_concurrency();
    }
    
    // Otherwise, use the suggested number of threads
    return suggested_threads;
}

std::vector<std::vector<uint8_t>> ThreadSafeBlake3Hash::hashVectorsInParallel(
    const std::vector<std::vector<uint8_t>>& data_vectors,
    size_t output_size,
    size_t num_threads
) {
    // Determine the number of threads to use
    size_t thread_count = getOptimalThreadCount(num_threads);
    
    // Ensure at least one thread
    thread_count = std::max(thread_count, static_cast<size_t>(1));
    
    // Limit the number of threads to the number of data vectors
    thread_count = std::min(thread_count, data_vectors.size());
    
    // Create a vector to store the results
    std::vector<std::vector<uint8_t>> hash_outputs(data_vectors.size());
    
    // Create a vector of threads
    std::vector<std::thread> threads;
    
    // Calculate the chunk size for each thread
    size_t chunk_size = data_vectors.size() / thread_count;
    
    // Create a mutex for thread-safe access to the hash_outputs vector
    std::mutex hash_outputs_mutex;
    
    // Create a worker function
    auto worker = [&](size_t start, size_t end) {
        // Hash data vectors for this chunk
        for (size_t i = start; i < end; ++i) {
            std::vector<uint8_t> hash = Blake3Hash::hash(data_vectors[i], output_size);
            
            // Store the hash in the result vector
            std::lock_guard<std::mutex> lock(hash_outputs_mutex);
            hash_outputs[i] = std::move(hash);
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
    
    return hash_outputs;
}

std::vector<std::vector<uint8_t>> ThreadSafeBlake3Hash::hashStringsInParallel(
    const std::vector<std::string>& strings,
    size_t output_size,
    size_t num_threads
) {
    // Determine the number of threads to use
    size_t thread_count = getOptimalThreadCount(num_threads);
    
    // Ensure at least one thread
    thread_count = std::max(thread_count, static_cast<size_t>(1));
    
    // Limit the number of threads to the number of strings
    thread_count = std::min(thread_count, strings.size());
    
    // Create a vector to store the results
    std::vector<std::vector<uint8_t>> hash_outputs(strings.size());
    
    // Create a vector of threads
    std::vector<std::thread> threads;
    
    // Calculate the chunk size for each thread
    size_t chunk_size = strings.size() / thread_count;
    
    // Create a mutex for thread-safe access to the hash_outputs vector
    std::mutex hash_outputs_mutex;
    
    // Create a worker function
    auto worker = [&](size_t start, size_t end) {
        // Hash strings for this chunk
        for (size_t i = start; i < end; ++i) {
            std::vector<uint8_t> hash = Blake3Hash::hash(strings[i], output_size);
            
            // Store the hash in the result vector
            std::lock_guard<std::mutex> lock(hash_outputs_mutex);
            hash_outputs[i] = std::move(hash);
        }
    };
    
    // Launch the worker threads
    for (size_t i = 0; i < thread_count; ++i) {
        size_t start = i * chunk_size;
        size_t end = (i == thread_count - 1) ? strings.size() : (i + 1) * chunk_size;
        
        threads.emplace_back(worker, start, end);
    }
    
    // Wait for all threads to complete
    for (auto& thread : threads) {
        thread.join();
    }
    
    return hash_outputs;
}

} // namespace crypto
} // namespace hydra
