#include <hydra_address/thread_safe_address_generator.hpp>
#include <hydra_address/thread_safe_vector_compression.hpp>
#include <hydra_crypto/thread_safe_blake3_hash.hpp>
#include <gtest/gtest.h>
#include <thread>
#include <vector>
#include <atomic>
#include <mutex>
#include <random>
#include <chrono>

using namespace hydra::address;
using namespace hydra::crypto;

// Helper function to generate random bytes
std::vector<uint8_t> generateRandomBytes(size_t length) {
    std::vector<uint8_t> result(length);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(0, 255);
    
    for (size_t i = 0; i < length; ++i) {
        result[i] = static_cast<uint8_t>(distrib(gen));
    }
    
    return result;
}

// Test fixture for thread safety tests
class ThreadSafetyTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Generate a random public key for testing
        m_public_key = generateRandomBytes(32);
    }
    
    std::vector<uint8_t> m_public_key;
};

// Test thread-safe address generation
TEST_F(ThreadSafetyTest, ThreadSafeAddressGenerator) {
    ThreadSafeAddressGenerator address_gen;
    
    // Number of threads to use
    const size_t num_threads = 8;
    
    // Number of addresses to generate per thread
    const size_t addresses_per_thread = 10;
    
    // Total number of addresses to generate
    const size_t total_addresses = num_threads * addresses_per_thread;
    
    // Vector to store the generated addresses
    std::vector<Address> addresses(total_addresses);
    
    // Atomic counter for thread synchronization
    std::atomic<size_t> counter(0);
    
    // Create threads
    std::vector<std::thread> threads;
    for (size_t i = 0; i < num_threads; ++i) {
        threads.emplace_back([&, i]() {
            // Generate addresses
            for (size_t j = 0; j < addresses_per_thread; ++j) {
                // Generate a compressed address
                Address address = address_gen.generateCompressedAddress(
                    m_public_key,
                    AddressType::RESOURCE
                );
                
                // Store the address
                size_t index = i * addresses_per_thread + j;
                addresses[index] = address;
                
                // Increment the counter
                counter.fetch_add(1);
            }
        });
    }
    
    // Wait for all threads to complete
    for (auto& thread : threads) {
        thread.join();
    }
    
    // Verify that all addresses were generated
    EXPECT_EQ(counter.load(), total_addresses);
    
    // Verify that all addresses are valid
    for (const auto& address : addresses) {
        EXPECT_TRUE(address_gen.verifyAddress(address));
    }
}

// Test parallel address generation
TEST_F(ThreadSafetyTest, ParallelAddressGeneration) {
    ThreadSafeAddressGenerator address_gen;
    
    // Number of addresses to generate
    const size_t num_addresses = 50;
    
    // Generate addresses in parallel
    auto addresses = address_gen.generateCompressedAddressesInParallel(
        m_public_key,
        num_addresses,
        AddressType::RESOURCE,
        8  // Use 8 threads
    );
    
    // Verify that the correct number of addresses were generated
    EXPECT_EQ(addresses.size(), num_addresses);
    
    // Verify that all addresses are valid
    for (const auto& address : addresses) {
        EXPECT_TRUE(address_gen.verifyAddress(address));
    }
}

// Test thread-safe vector compression
TEST_F(ThreadSafetyTest, ThreadSafeVectorCompression) {
    ThreadSafeVectorCompression compressor(CompressionMethod::HYBRID);
    
    // Number of threads to use
    const size_t num_threads = 8;
    
    // Number of vectors to compress per thread
    const size_t vectors_per_thread = 10;
    
    // Total number of vectors to compress
    const size_t total_vectors = num_threads * vectors_per_thread;
    
    // Create vectors to compress
    std::vector<Vector> vectors;
    for (size_t i = 0; i < total_vectors; ++i) {
        // Create a vector with random data
        Vector vec(32);
        for (size_t j = 0; j < 32; ++j) {
            vec.setElement(j, hydra::math::BigInt(rand() % 256));
        }
        vectors.push_back(vec);
    }
    
    // Vector to store the compressed data
    std::vector<std::vector<uint8_t>> compressed_data(total_vectors);
    
    // Atomic counter for thread synchronization
    std::atomic<size_t> counter(0);
    
    // Create threads
    std::vector<std::thread> threads;
    for (size_t i = 0; i < num_threads; ++i) {
        threads.emplace_back([&, i]() {
            // Compress vectors
            for (size_t j = 0; j < vectors_per_thread; ++j) {
                // Compress the vector
                size_t index = i * vectors_per_thread + j;
                compressed_data[index] = compressor.compress(vectors[index]);
                
                // Increment the counter
                counter.fetch_add(1);
            }
        });
    }
    
    // Wait for all threads to complete
    for (auto& thread : threads) {
        thread.join();
    }
    
    // Verify that all vectors were compressed
    EXPECT_EQ(counter.load(), total_vectors);
    
    // Verify that all compressed data can be decompressed
    for (size_t i = 0; i < total_vectors; ++i) {
        auto decompressed = compressor.decompress(compressed_data[i]);
        EXPECT_TRUE(decompressed.has_value());
    }
}

// Test parallel vector compression
TEST_F(ThreadSafetyTest, ParallelVectorCompression) {
    ThreadSafeVectorCompression compressor(CompressionMethod::HYBRID);
    
    // Number of vectors to compress
    const size_t num_vectors = 50;
    
    // Create vectors to compress
    std::vector<Vector> vectors;
    for (size_t i = 0; i < num_vectors; ++i) {
        // Create a vector with random data
        Vector vec(32);
        for (size_t j = 0; j < 32; ++j) {
            vec.setElement(j, hydra::math::BigInt(rand() % 256));
        }
        vectors.push_back(vec);
    }
    
    // Compress vectors in parallel
    auto compressed_data = compressor.compressVectorsInParallel(
        vectors,
        8  // Use 8 threads
    );
    
    // Verify that the correct number of vectors were compressed
    EXPECT_EQ(compressed_data.size(), num_vectors);
    
    // Verify that all compressed data can be decompressed
    auto decompressed_vectors = compressor.decompressVectorsInParallel(
        compressed_data,
        8  // Use 8 threads
    );
    
    // Verify that the correct number of vectors were decompressed
    EXPECT_EQ(decompressed_vectors.size(), num_vectors);
    
    // Verify that all vectors were decompressed successfully
    for (const auto& decompressed : decompressed_vectors) {
        EXPECT_TRUE(decompressed.has_value());
    }
}

// Test thread-safe Blake3 hashing
TEST_F(ThreadSafetyTest, ThreadSafeBlake3Hash) {
    ThreadSafeBlake3Hash hasher;
    
    // Number of threads to use
    const size_t num_threads = 8;
    
    // Number of updates per thread
    const size_t updates_per_thread = 10;
    
    // Create random data for each update
    std::vector<std::vector<uint8_t>> update_data;
    for (size_t i = 0; i < num_threads * updates_per_thread; ++i) {
        update_data.push_back(generateRandomBytes(64));
    }
    
    // Create threads
    std::vector<std::thread> threads;
    for (size_t i = 0; i < num_threads; ++i) {
        threads.emplace_back([&, i]() {
            // Update the hasher
            for (size_t j = 0; j < updates_per_thread; ++j) {
                size_t index = i * updates_per_thread + j;
                hasher.update(update_data[index]);
            }
        });
    }
    
    // Wait for all threads to complete
    for (auto& thread : threads) {
        thread.join();
    }
    
    // Finalize the hash
    auto hash = hasher.finalize();
    
    // Verify that the hash is not empty
    EXPECT_FALSE(hash.empty());
    EXPECT_EQ(hash.size(), ThreadSafeBlake3Hash::HASH_SIZE);
}

// Test parallel Blake3 hashing
TEST_F(ThreadSafetyTest, ParallelBlake3Hashing) {
    // Number of data vectors to hash
    const size_t num_vectors = 50;
    
    // Create random data vectors
    std::vector<std::vector<uint8_t>> data_vectors;
    for (size_t i = 0; i < num_vectors; ++i) {
        data_vectors.push_back(generateRandomBytes(64));
    }
    
    // Hash data vectors in parallel
    auto hashes = ThreadSafeBlake3Hash::hashVectorsInParallel(
        data_vectors,
        ThreadSafeBlake3Hash::HASH_SIZE,
        8  // Use 8 threads
    );
    
    // Verify that the correct number of hashes were generated
    EXPECT_EQ(hashes.size(), num_vectors);
    
    // Verify that all hashes have the correct size
    for (const auto& hash : hashes) {
        EXPECT_EQ(hash.size(), ThreadSafeBlake3Hash::HASH_SIZE);
    }
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
