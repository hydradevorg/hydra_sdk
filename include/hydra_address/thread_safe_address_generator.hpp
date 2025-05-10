#pragma once

#include <hydra_address/address_generator.hpp>
#include <mutex>
#include <shared_mutex>
#include <memory>
#include <vector>
#include <optional>
#include <complex>
#include <thread>

namespace hydra {
namespace address {

/**
 * @brief Thread-safe wrapper for AddressGenerator
 * 
 * This class provides thread-safe access to the AddressGenerator functionality,
 * allowing multiple threads to generate addresses concurrently without conflicts.
 */
class ThreadSafeAddressGenerator {
public:
    /**
     * @brief Default constructor
     */
    ThreadSafeAddressGenerator();
    
    /**
     * @brief Constructor with specific security parameters
     * @param security_level Security level in bits
     */
    explicit ThreadSafeAddressGenerator(size_t security_level);
    
    /**
     * @brief Destructor
     */
    ~ThreadSafeAddressGenerator() = default;
    
    /**
     * @brief Generate an address from a public key
     * @param public_key Public key data
     * @param type Address type
     * @param format Address format
     * @return Generated address
     */
    Address generateFromPublicKey(
        const std::vector<uint8_t>& public_key,
        AddressType type = AddressType::USER,
        AddressFormat format = AddressFormat::STANDARD
    );
    
    /**
     * @brief Generate an address with geolocation data
     * @param public_key Public key data
     * @param coordinates Geographic coordinates
     * @param type Address type
     * @return Generated address with geohash
     */
    Address generateGeoAddress(
        const std::vector<uint8_t>& public_key,
        const Coordinates& coordinates,
        AddressType type = AddressType::NODE
    );
    
    /**
     * @brief Generate a quantum-resistant address
     * @param public_key Public key data
     * @param quantum_state Quantum state vector
     * @param type Address type
     * @return Generated quantum-resistant address
     */
    Address generateQuantumAddress(
        const std::vector<uint8_t>& public_key,
        const std::vector<std::complex<double>>& quantum_state,
        AddressType type = AddressType::USER
    );
    
    /**
     * @brief Generate a compressed address
     * @param public_key Public key data
     * @param type Address type
     * @return Generated compressed address
     */
    Address generateCompressedAddress(
        const std::vector<uint8_t>& public_key,
        AddressType type = AddressType::RESOURCE
    );
    
    /**
     * @brief Verify an address
     * @param address Address to verify
     * @return True if the address is valid
     */
    bool verifyAddress(const Address& address);
    
    /**
     * @brief Set the security level
     * @param security_level Security level in bits
     */
    void setSecurityLevel(size_t security_level);
    
    /**
     * @brief Get the current security level
     * @return Security level in bits
     */
    size_t getSecurityLevel() const;
    
    /**
     * @brief Generate multiple addresses in parallel
     * @param public_key Public key data
     * @param count Number of addresses to generate
     * @param type Address type
     * @param format Address format
     * @param num_threads Number of threads to use (0 for auto-detection)
     * @return Vector of generated addresses
     */
    std::vector<Address> generateAddressesInParallel(
        const std::vector<uint8_t>& public_key,
        size_t count,
        AddressType type = AddressType::USER,
        AddressFormat format = AddressFormat::STANDARD,
        size_t num_threads = 0
    );
    
    /**
     * @brief Generate multiple compressed addresses in parallel
     * @param public_key Public key data
     * @param count Number of addresses to generate
     * @param type Address type
     * @param num_threads Number of threads to use (0 for auto-detection)
     * @return Vector of generated addresses
     */
    std::vector<Address> generateCompressedAddressesInParallel(
        const std::vector<uint8_t>& public_key,
        size_t count,
        AddressType type = AddressType::RESOURCE,
        size_t num_threads = 0
    );

private:
    // The underlying address generator
    AddressGenerator m_generator;
    
    // Mutex for thread safety
    mutable std::shared_mutex m_mutex;
    
    // Helper method to determine the optimal number of threads
    size_t getOptimalThreadCount(size_t suggested_threads) const;
};

} // namespace address
} // namespace hydra
