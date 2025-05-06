#pragma once

#include <string>
#include <vector>
#include <array>
#include <memory>
#include <optional>
#include <complex>

#include <hydra_crypto/blake3_hash.hpp>
#include <hydra_crypto/falcon_signature.hpp>
#include <hydra_crypto/kyber_aes.hpp>
#include <hydra_qzkp/qzkp.hpp>
#include <hydra_math/bigint.hpp>

namespace hydra {
namespace address {

/**
 * @brief Address format options
 */
enum class AddressFormat {
    STANDARD,       ///< Standard format (base58 encoded)
    GEOHASHED,      ///< Geohashed format (includes location data)
    QUANTUM_PROOF,  ///< Quantum-resistant format (includes QZKP)
    COMPRESSED      ///< Compressed format (for efficient storage)
};

/**
 * @brief Address type options
 */
enum class AddressType {
    USER,           ///< User address
    NODE,           ///< Node address
    RESOURCE,       ///< Resource address
    SERVICE         ///< Service address
};

/**
 * @brief Coordinate structure for geohashing
 */
struct Coordinates {
    double latitude;
    double longitude;
    double altitude;
    
    bool operator==(const Coordinates& other) const {
        return latitude == other.latitude && 
               longitude == other.longitude && 
               altitude == other.altitude;
    }
};

/**
 * @brief Address class representing a cryptographic address in the P2P VFS
 */
class Address {
public:
    /**
     * @brief Default constructor
     */
    Address();
    
    /**
     * @brief Constructor with raw address data
     * @param data Raw address data
     */
    explicit Address(const std::vector<uint8_t>& data);
    
    /**
     * @brief Constructor with address string
     * @param address_str Address string
     */
    explicit Address(const std::string& address_str);
    
    /**
     * @brief Get the raw address data
     * @return Raw address data
     */
    std::vector<uint8_t> getData() const;
    
    /**
     * @brief Get the address as a string
     * @return Address string
     */
    std::string toString() const;
    
    /**
     * @brief Get the address type
     * @return Address type
     */
    AddressType getType() const;
    
    /**
     * @brief Get the address format
     * @return Address format
     */
    AddressFormat getFormat() const;
    
    /**
     * @brief Get the geohash if available
     * @return Geohash string or empty if not available
     */
    std::optional<std::string> getGeohash() const;
    
    /**
     * @brief Get the coordinates if available
     * @return Coordinates or empty if not available
     */
    std::optional<Coordinates> getCoordinates() const;
    
    /**
     * @brief Verify the address integrity
     * @return True if the address is valid
     */
    bool verify() const;
    
    /**
     * @brief Equality operator
     */
    bool operator==(const Address& other) const;
    
    /**
     * @brief Inequality operator
     */
    bool operator!=(const Address& other) const;

private:
    std::vector<uint8_t> m_data;
    AddressType m_type;
    AddressFormat m_format;
    std::optional<std::string> m_geohash;
    std::optional<Coordinates> m_coordinates;
    
    void parseData();
};

/**
 * @brief AddressGenerator class for generating cryptographic addresses
 * 
 * This class implements the address generation system for the P2P VFS,
 * using the Layered Matrix and Vector System for secure consensus,
 * fault tolerance, and data projection.
 */
class AddressGenerator {
public:
    /**
     * @brief Default constructor
     */
    AddressGenerator();
    
    /**
     * @brief Constructor with specific security parameters
     * @param security_level Security level in bits
     */
    explicit AddressGenerator(size_t security_level);
    
    /**
     * @brief Destructor
     */
    ~AddressGenerator();
    
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

private:
    class Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace address
} // namespace hydra
