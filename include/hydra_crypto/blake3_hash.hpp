#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <array>

namespace hydra {
namespace crypto {

/**
 * @brief BLAKE3 cryptographic hash function implementation
 *
 * BLAKE3 is a cryptographic hash function that is:
 * - Much faster than MD5, SHA-1, SHA-2, and SHA-3, and on par with BLAKE2
 * - Secure against length extension attacks
 * - One algorithm with no variants
 * - A single implementation for all platforms, with runtime CPU feature detection
 * - Supports extendable output (XOF), keyed hashing, and key derivation
 */
class Blake3Hash {
public:
    /// The size of a BLAKE3 hash output in bytes (256 bits)
    static constexpr size_t HASH_SIZE = 32;

    /// The size of a BLAKE3 key in bytes
    static constexpr size_t KEY_SIZE = 32;

    /**
     * @brief Creates a new Blake3Hash instance
     */
    Blake3Hash();

    /**
     * @brief Creates a new keyed Blake3Hash instance
     *
     * @param key 32-byte key
     */
    explicit Blake3Hash(const std::array<uint8_t, KEY_SIZE>& key);

    /**
     * @brief Creates a new keyed Blake3Hash instance with key derivation
     *
     * This constructor derives a key from the given context string.
     *
     * @param context Context string for key derivation
     */
    explicit Blake3Hash(const std::string& context);

    /**
     * @brief Destroys the Blake3Hash instance
     */
    ~Blake3Hash();

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

    // std::span version removed due to compatibility issues

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

    // std::span version removed due to compatibility issues

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

    // std::span version removed due to compatibility issues

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

private:
    /// Pointer to the internal BLAKE3 hasher
    void* m_hasher;

    /// Convert a binary hash to a hexadecimal string
    static std::string toHex(const std::vector<uint8_t>& data);
};

} // namespace crypto
} // namespace hydra