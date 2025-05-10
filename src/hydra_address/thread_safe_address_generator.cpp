#include <hydra_address/thread_safe_address_generator.hpp>
#include <mutex>
#include <thread>
#include <vector>
#include <algorithm>

namespace hydra {
namespace address {

ThreadSafeAddressGenerator::ThreadSafeAddressGenerator() : m_generator() {}

ThreadSafeAddressGenerator::ThreadSafeAddressGenerator(size_t security_level) 
    : m_generator(security_level) {}

Address ThreadSafeAddressGenerator::generateFromPublicKey(
    const std::vector<uint8_t>& public_key,
    AddressType type,
    AddressFormat format
) {
    // Use a shared lock for read-only operations
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    return m_generator.generateFromPublicKey(public_key, type, format);
}

Address ThreadSafeAddressGenerator::generateGeoAddress(
    const std::vector<uint8_t>& public_key,
    const Coordinates& coordinates,
    AddressType type
) {
    // Use a shared lock for read-only operations
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    return m_generator.generateGeoAddress(public_key, coordinates, type);
}

Address ThreadSafeAddressGenerator::generateQuantumAddress(
    const std::vector<uint8_t>& public_key,
    const std::vector<std::complex<double>>& quantum_state,
    AddressType type
) {
    // Use a shared lock for read-only operations
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    return m_generator.generateQuantumAddress(public_key, quantum_state, type);
}

Address ThreadSafeAddressGenerator::generateCompressedAddress(
    const std::vector<uint8_t>& public_key,
    AddressType type
) {
    // Use a shared lock for read-only operations
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    return m_generator.generateCompressedAddress(public_key, type);
}

bool ThreadSafeAddressGenerator::verifyAddress(const Address& address) {
    // Use a shared lock for read-only operations
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    return m_generator.verifyAddress(address);
}

void ThreadSafeAddressGenerator::setSecurityLevel(size_t security_level) {
    // Use an exclusive lock for write operations
    std::unique_lock<std::shared_mutex> lock(m_mutex);
    m_generator.setSecurityLevel(security_level);
}

size_t ThreadSafeAddressGenerator::getSecurityLevel() const {
    // Use a shared lock for read-only operations
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    return m_generator.getSecurityLevel();
}

size_t ThreadSafeAddressGenerator::getOptimalThreadCount(size_t suggested_threads) const {
    // If suggested_threads is 0, use the number of hardware threads
    if (suggested_threads == 0) {
        return std::thread::hardware_concurrency();
    }
    
    // Otherwise, use the suggested number of threads
    return suggested_threads;
}

std::vector<Address> ThreadSafeAddressGenerator::generateAddressesInParallel(
    const std::vector<uint8_t>& public_key,
    size_t count,
    AddressType type,
    AddressFormat format,
    size_t num_threads
) {
    // Determine the number of threads to use
    size_t thread_count = getOptimalThreadCount(num_threads);
    
    // Ensure at least one thread
    thread_count = std::max(thread_count, static_cast<size_t>(1));
    
    // Limit the number of threads to the number of addresses
    thread_count = std::min(thread_count, count);
    
    // Create a vector to store the results
    std::vector<Address> addresses(count);
    
    // Create a vector of threads
    std::vector<std::thread> threads;
    
    // Calculate the chunk size for each thread
    size_t chunk_size = count / thread_count;
    
    // Create a mutex for thread-safe access to the addresses vector
    std::mutex addresses_mutex;
    
    // Create a shared lock for read-only access to the generator
    std::shared_lock<std::shared_mutex> generator_lock(m_mutex);
    
    // Create a worker function
    auto worker = [&](size_t start, size_t end) {
        // Create a local copy of the generator for thread safety
        AddressGenerator local_generator(m_generator.getSecurityLevel());
        
        // Generate addresses for this chunk
        for (size_t i = start; i < end; ++i) {
            Address address = local_generator.generateFromPublicKey(public_key, type, format);
            
            // Store the address in the result vector
            std::lock_guard<std::mutex> lock(addresses_mutex);
            addresses[i] = address;
        }
    };
    
    // Launch the worker threads
    for (size_t i = 0; i < thread_count; ++i) {
        size_t start = i * chunk_size;
        size_t end = (i == thread_count - 1) ? count : (i + 1) * chunk_size;
        
        threads.emplace_back(worker, start, end);
    }
    
    // Wait for all threads to complete
    for (auto& thread : threads) {
        thread.join();
    }
    
    return addresses;
}

std::vector<Address> ThreadSafeAddressGenerator::generateCompressedAddressesInParallel(
    const std::vector<uint8_t>& public_key,
    size_t count,
    AddressType type,
    size_t num_threads
) {
    // Determine the number of threads to use
    size_t thread_count = getOptimalThreadCount(num_threads);
    
    // Ensure at least one thread
    thread_count = std::max(thread_count, static_cast<size_t>(1));
    
    // Limit the number of threads to the number of addresses
    thread_count = std::min(thread_count, count);
    
    // Create a vector to store the results
    std::vector<Address> addresses(count);
    
    // Create a vector of threads
    std::vector<std::thread> threads;
    
    // Calculate the chunk size for each thread
    size_t chunk_size = count / thread_count;
    
    // Create a mutex for thread-safe access to the addresses vector
    std::mutex addresses_mutex;
    
    // Create a shared lock for read-only access to the generator
    std::shared_lock<std::shared_mutex> generator_lock(m_mutex);
    
    // Create a worker function
    auto worker = [&](size_t start, size_t end) {
        // Create a local copy of the generator for thread safety
        AddressGenerator local_generator(m_generator.getSecurityLevel());
        
        // Generate addresses for this chunk
        for (size_t i = start; i < end; ++i) {
            Address address = local_generator.generateCompressedAddress(public_key, type);
            
            // Store the address in the result vector
            std::lock_guard<std::mutex> lock(addresses_mutex);
            addresses[i] = address;
        }
    };
    
    // Launch the worker threads
    for (size_t i = 0; i < thread_count; ++i) {
        size_t start = i * chunk_size;
        size_t end = (i == thread_count - 1) ? count : (i + 1) * chunk_size;
        
        threads.emplace_back(worker, start, end);
    }
    
    // Wait for all threads to complete
    for (auto& thread : threads) {
        thread.join();
    }
    
    return addresses;
}

} // namespace address
} // namespace hydra
