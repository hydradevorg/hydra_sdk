#pragma once

#include <hydra_crypto/blake3_hash.hpp>
#include <mutex>
#include <shared_mutex>
#include <vector>
#include <string>
#include <array>
#include <thread>

namespace hydra {
namespace crypto {

/**
 * @brief Thread-safe wrapper for Blake3Hash
 * 
 * This class provides thread-safe access to the Blake3Hash functionality,
 * allowing multiple threads to perform hashing operations concurrently without conflicts.
 */
class ThreadSafeBlake3Hash {
public:
    /// The size of a BLAKE3 hash output in bytes (256 bits)
    static constexpr size_t HASH_SIZE = Blake3Hash::HASH_SIZE;

    /// The size of a BLAKE3 key in bytes
    static constexpr size_t KEY_SIZE = Blake3Hash::KEY_SIZE;

    /**
     * @brief Creates a new ThreadSafeBlake3Hash instance
     */
    ThreadSafeBlake3Hash();

    /**
     * @brief Creates a new keyed ThreadSafeBlake3Hash instance
     *
     * @param key 32-byte key
     */
    explicit ThreadSafeBlake3Hash(const std::array<uint8_t, KEY_SIZE>& key);

    /**
     * @brief Creates a new keyed ThreadSafeBlake3Hash instance with key derivation
     *
     * This constructor derives a key from the given context string.
     *
     * @param context Context string for key derivation
     */
    explicit ThreadSafeBlake3Hash(const std::string& context);

    /**
     * @brief Reset the hasher to its initial state
     */
    void reset();

    /**
     * @brief Update the hash with input data
     *
     * @param data Input data to hash
     * @param size Size of the input data in bytes
     */
    void update(const void* data, size_t size);

    /**
     * @brief Update the hash with input data
     *
     * @param data Input data to hash
     */
    void update(const std::vector<uint8_t>& data);

    /**
     * @brief Update the hash with input string
     *
     * @param data Input string to hash
     */
    void update(const std::string& data);

    /**
     * @brief Finalize the hash and get the output
     *
     * @param output Buffer to store the hash output
     * @param output_size Size of the output buffer in bytes
     */
    void finalize(uint8_t* output, size_t output_size);

    /**
     * @brief Finalize the hash and get the output as a vector
     *
     * @param output_size Size of the output in bytes
     * @return std::vector<uint8_t> Hash output
     */
    std::vector<uint8_t> finalize(size_t output_size = HASH_SIZE);

    /**
     * @brief Finalize the hash and get the output as a hexadecimal string
     *
     * @param output_size Size of the output in bytes
     * @return std::string Hexadecimal representation of the hash
     */
    std::string finalizeHex(size_t output_size = HASH_SIZE);

    /**
     * @brief One-shot function to hash data
     *
     * @param data Input data to hash
     * @param size Size of the input data in bytes
     * @param output_size Size of the output in bytes
     * @return std::vector<uint8_t> Hash output
     */
    static std::vector<uint8_t> hash(const void* data, size_t size, size_t output_size = HASH_SIZE);

    /**
     * @brief One-shot function to hash data
     *
     * @param data Input data to hash
     * @param output_size Size of the output in bytes
     * @return std::vector<uint8_t> Hash output
     */
    static std::vector<uint8_t> hash(const std::vector<uint8_t>& data, size_t output_size = HASH_SIZE);

    /**
     * @brief One-shot function to hash a string
     *
     * @param data Input string to hash
     * @param output_size Size of the output in bytes
     * @return std::vector<uint8_t> Hash output
     */
    static std::vector<uint8_t> hash(const std::string& data, size_t output_size = HASH_SIZE);

    /**
     * @brief One-shot function to hash data and get a hexadecimal string
     *
     * @param data Input data to hash
     * @param size Size of the input data in bytes
     * @param output_size Size of the output in bytes
     * @return std::string Hexadecimal representation of the hash
     */
    static std::string hashHex(const void* data, size_t size, size_t output_size = HASH_SIZE);

    /**
     * @brief One-shot function to hash data and get a hexadecimal string
     *
     * @param data Input data to hash
     * @param output_size Size of the output in bytes
     * @return std::string Hexadecimal representation of the hash
     */
    static std::string hashHex(const std::vector<uint8_t>& data, size_t output_size = HASH_SIZE);

    /**
     * @brief One-shot function to hash a string and get a hexadecimal string
     *
     * @param data Input string to hash
     * @param output_size Size of the output in bytes
     * @return std::string Hexadecimal representation of the hash
     */
    static std::string hashHex(const std::string& data, size_t output_size = HASH_SIZE);
    
    /**
     * @brief Hash multiple data vectors in parallel
     * @param data_vectors Data vectors to hash
     * @param output_size Size of each hash output in bytes
     * @param num_threads Number of threads to use (0 for auto-detection)
     * @return Vector of hash outputs
     */
    static std::vector<std::vector<uint8_t>> hashVectorsInParallel(
        const std::vector<std::vector<uint8_t>>& data_vectors,
        size_t output_size = HASH_SIZE,
        size_t num_threads = 0
    );
    
    /**
     * @brief Hash multiple strings in parallel
     * @param strings Strings to hash
     * @param output_size Size of each hash output in bytes
     * @param num_threads Number of threads to use (0 for auto-detection)
     * @return Vector of hash outputs
     */
    static std::vector<std::vector<uint8_t>> hashStringsInParallel(
        const std::vector<std::string>& strings,
        size_t output_size = HASH_SIZE,
        size_t num_threads = 0
    );

private:
    // The underlying Blake3Hash
    Blake3Hash m_hasher;
    
    // Mutex for thread safety
    mutable std::mutex m_mutex;
    
    // Helper method to determine the optimal number of threads
    static size_t getOptimalThreadCount(size_t suggested_threads);
};

} // namespace crypto
} // namespace hydra
