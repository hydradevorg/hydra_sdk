# Thread Safety in Hydra SDK

The Hydra SDK provides thread-safe implementations of key components to enable concurrent operations in multi-threaded applications. This document describes the thread safety features and best practices for using them.

## Thread-Safe Components

The following components have thread-safe implementations:

### 1. Address Generator

The `ThreadSafeAddressGenerator` class provides thread-safe access to address generation functionality:

```cpp
#include <hydra_address/thread_safe_address_generator.hpp>

// Create a thread-safe address generator
hydra::address::ThreadSafeAddressGenerator address_gen;

// Generate addresses concurrently from multiple threads
auto address = address_gen.generateCompressedAddress(public_key, type);

// Generate multiple addresses in parallel
auto addresses = address_gen.generateCompressedAddressesInParallel(
    public_key,
    100,  // Generate 100 addresses
    hydra::address::AddressType::RESOURCE,
    8     // Use 8 threads
);
```

### 2. Vector Compression

The `ThreadSafeVectorCompression` class provides thread-safe access to vector compression functionality:

```cpp
#include <hydra_address/thread_safe_vector_compression.hpp>

// Create a thread-safe vector compressor
hydra::address::ThreadSafeVectorCompression compressor(
    hydra::address::CompressionMethod::HYBRID
);

// Compress vectors concurrently from multiple threads
auto compressed = compressor.compress(vector);

// Compress multiple vectors in parallel
auto compressed_vectors = compressor.compressVectorsInParallel(
    vectors,
    8  // Use 8 threads
);

// Decompress multiple vectors in parallel
auto decompressed_vectors = compressor.decompressVectorsInParallel(
    compressed_vectors,
    8  // Use 8 threads
);
```

### 3. Blake3 Hashing

The `ThreadSafeBlake3Hash` class provides thread-safe access to Blake3 hashing functionality:

```cpp
#include <hydra_crypto/thread_safe_blake3_hash.hpp>

// Create a thread-safe Blake3 hasher
hydra::crypto::ThreadSafeBlake3Hash hasher;

// Update the hash concurrently from multiple threads
hasher.update(data);

// Finalize the hash
auto hash = hasher.finalize();

// Hash multiple data vectors in parallel
auto hashes = hydra::crypto::ThreadSafeBlake3Hash::hashVectorsInParallel(
    data_vectors,
    hydra::crypto::ThreadSafeBlake3Hash::HASH_SIZE,
    8  // Use 8 threads
);
```

## Thread Safety Implementation

The thread-safe implementations use the following techniques to ensure thread safety:

1. **Shared Mutexes**: For operations that don't modify the internal state, shared locks are used to allow concurrent read access.

2. **Exclusive Locks**: For operations that modify the internal state, exclusive locks are used to prevent concurrent access.

3. **Thread-Local Copies**: For parallel operations, thread-local copies of the underlying objects are created to avoid contention.

4. **Atomic Operations**: For thread synchronization, atomic operations are used to ensure visibility of changes across threads.

## Performance Considerations

When using thread-safe components, consider the following performance implications:

1. **Contention**: High contention on shared resources can lead to reduced performance due to lock waiting times.

2. **Thread Count**: Using too many threads can lead to diminishing returns due to context switching overhead. The optimal number of threads is typically close to the number of available CPU cores.

3. **Batch Processing**: For better performance, process data in batches rather than making many small calls to thread-safe methods.

4. **Parallel Methods**: Use the parallel methods (e.g., `generateAddressesInParallel`, `compressVectorsInParallel`) for bulk operations rather than calling the regular methods from multiple threads.

## Thread Safety Best Practices

Follow these best practices to ensure thread safety in your applications:

1. **Use Thread-Safe Classes**: Always use the thread-safe versions of classes in multi-threaded applications.

2. **Minimize Lock Duration**: Keep critical sections as short as possible to reduce contention.

3. **Avoid Nested Locks**: Avoid acquiring locks while holding other locks to prevent deadlocks.

4. **Consider Lock-Free Alternatives**: For high-performance applications, consider using lock-free data structures and algorithms.

5. **Test Thoroughly**: Test your multi-threaded code thoroughly to ensure it behaves correctly under concurrent access.

## Example: Multi-Threaded Address Generation

Here's a complete example of multi-threaded address generation:

```cpp
#include <hydra_address/thread_safe_address_generator.hpp>
#include <thread>
#include <vector>
#include <mutex>
#include <iostream>

int main() {
    // Create a thread-safe address generator
    hydra::address::ThreadSafeAddressGenerator address_gen;
    
    // Generate a public key
    std::vector<uint8_t> public_key = generatePublicKey();
    
    // Number of threads to use
    const size_t num_threads = 8;
    
    // Number of addresses to generate per thread
    const size_t addresses_per_thread = 10;
    
    // Vector to store the generated addresses
    std::vector<hydra::address::Address> addresses(num_threads * addresses_per_thread);
    
    // Mutex for thread-safe access to cout
    std::mutex cout_mutex;
    
    // Create threads
    std::vector<std::thread> threads;
    for (size_t i = 0; i < num_threads; ++i) {
        threads.emplace_back([&, i]() {
            // Generate addresses
            for (size_t j = 0; j < addresses_per_thread; ++j) {
                // Generate a compressed address
                hydra::address::Address address = address_gen.generateCompressedAddress(
                    public_key,
                    hydra::address::AddressType::RESOURCE
                );
                
                // Store the address
                size_t index = i * addresses_per_thread + j;
                addresses[index] = address;
                
                // Print the address
                std::lock_guard<std::mutex> lock(cout_mutex);
                std::cout << "Thread " << i << " generated address " << j << ": "
                          << address.toString() << std::endl;
            }
        });
    }
    
    // Wait for all threads to complete
    for (auto& thread : threads) {
        thread.join();
    }
    
    // Verify all addresses
    for (const auto& address : addresses) {
        bool is_valid = address_gen.verifyAddress(address);
        std::cout << "Address " << address.toString() << " is "
                  << (is_valid ? "valid" : "invalid") << std::endl;
    }
    
    return 0;
}
```

## Example: Parallel Vector Compression

Here's a complete example of parallel vector compression:

```cpp
#include <hydra_address/thread_safe_vector_compression.hpp>
#include <iostream>
#include <vector>

int main() {
    // Create a thread-safe vector compressor
    hydra::address::ThreadSafeVectorCompression compressor(
        hydra::address::CompressionMethod::HYBRID
    );
    
    // Create vectors to compress
    std::vector<hydra::address::Vector> vectors;
    for (size_t i = 0; i < 100; ++i) {
        // Create a vector with random data
        hydra::address::Vector vec(32);
        for (size_t j = 0; j < 32; ++j) {
            vec.setElement(j, hydra::math::BigInt(rand() % 256));
        }
        vectors.push_back(vec);
    }
    
    // Compress vectors in parallel
    auto compressed_vectors = compressor.compressVectorsInParallel(
        vectors,
        8  // Use 8 threads
    );
    
    // Print compression results
    for (size_t i = 0; i < vectors.size(); ++i) {
        double compression_ratio = static_cast<double>(vectors[i].getSize()) /
                                  static_cast<double>(compressed_vectors[i].size());
        
        std::cout << "Vector " << i << " compression ratio: " << compression_ratio << std::endl;
    }
    
    return 0;
}
```

For more information on thread safety in the Hydra SDK, refer to the API documentation for each thread-safe class.
