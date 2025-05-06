#include <hydra_address/address_generator.hpp>
#include <hydra_address/geohash.hpp>
#include <hydra_address/vector_compression.hpp>

#include <hydra_crypto/blake3_hash.hpp>
#include <hydra_crypto/falcon_signature.hpp>
#include <hydra_crypto/kyber_aes.hpp>
#include <hydra_qzkp/qzkp.hpp>
#include <hydra_math/bigint.hpp>

#include <lmvs/lmvs.hpp>
#include <lmvs/bigint_vector.hpp>
#include <lmvs/layered_bigint_vector.hpp>
#include <lmvs/p2p_vfs/p2p_vfs.hpp>

#include <iostream>
#include <sstream>
#include <iomanip>
#include <random>
#include <algorithm>
#include <cstring>

namespace hydra {
namespace address {

// Base58 encoding alphabet
static const char* BASE58_CHARS = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";

// Helper function to convert bytes to base58
static std::string bytesToBase58(const std::vector<uint8_t>& data) {
    hydra::math::BigInt value(0);

    // Convert bytes to BigInt
    for (size_t i = 0; i < data.size(); ++i) {
        value = value * hydra::math::BigInt(256) + hydra::math::BigInt(data[i]);
    }

    // Convert BigInt to base58 string
    std::string result;
    hydra::math::BigInt base(58);
    hydra::math::BigInt zero(0);

    while (value > zero) {
        hydra::math::BigInt remainder;
        hydra::math::BigInt quotient;

        // Perform division and get remainder
        quotient = value / base;

        // Implement modulo operation manually since BigInt doesn't have a % operator
        remainder = value - (quotient * base);

        // Prepend the base58 character
        result = BASE58_CHARS[remainder.to_int()] + result;

        // Update value for next iteration
        value = quotient;
    }

    // Add leading '1's for leading zeros in the data
    for (size_t i = 0; i < data.size() && data[i] == 0; ++i) {
        result = '1' + result;
    }

    return result;
}

// Helper function to convert base58 to bytes
static std::vector<uint8_t> base58ToBytes(const std::string& str) {
    hydra::math::BigInt value(0);
    hydra::math::BigInt base(58);

    // Convert base58 string to BigInt
    for (char c : str) {
        const char* p = strchr(BASE58_CHARS, c);
        if (!p) {
            // Invalid character
            return {};
        }

        size_t index = p - BASE58_CHARS;
        value = value * base + hydra::math::BigInt(static_cast<int64_t>(index));
    }

    // Convert BigInt to bytes
    std::vector<uint8_t> result;
    hydra::math::BigInt zero(0);
    hydra::math::BigInt byte_max(256);

    while (value > zero) {
        // Calculate remainder manually
        hydra::math::BigInt quotient = value / byte_max;
        hydra::math::BigInt remainder = value - (quotient * byte_max);
        value = quotient;
        result.push_back(static_cast<uint8_t>(remainder.to_int()));
    }

    // Reverse the result
    std::reverse(result.begin(), result.end());

    // Add leading zeros
    for (size_t i = 0; i < str.size() && str[i] == '1'; ++i) {
        result.insert(result.begin(), 0);
    }

    return result;
}

// Helper function to convert a vector of bytes to a layered BigInt vector
static lmvs::LayeredBigIntVector bytesToLayeredBigIntVector(const std::vector<uint8_t>& data, size_t num_layers) {
    // Calculate the layer size
    size_t layer_size = (data.size() + num_layers - 1) / num_layers;

    // Create the layered vector
    lmvs::LayeredBigIntVector result(num_layers, layer_size);

    // Fill the layers
    for (size_t layer = 0; layer < num_layers; ++layer) {
        lmvs::BigIntVector& layer_vec = result.getLayerMutable(layer);

        for (size_t i = 0; i < layer_size; ++i) {
            size_t data_idx = layer * layer_size + i;
            if (data_idx < data.size()) {
                layer_vec.setValue(i, hydra::math::BigInt(data[data_idx]));
            } else {
                layer_vec.setValue(i, hydra::math::BigInt(0));
            }
        }
    }

    return result;
}

// Helper function to convert a layered BigInt vector to a vector of bytes
static std::vector<uint8_t> layeredBigIntVectorToBytes(const lmvs::LayeredBigIntVector& vec) {
    std::vector<uint8_t> result;

    // Get the dimensions
    size_t num_layers = vec.getNumLayers();
    size_t layer_size = vec.getDimension();

    // Reserve space
    result.reserve(num_layers * layer_size);

    // Extract the data
    for (size_t layer = 0; layer < num_layers; ++layer) {
        const lmvs::BigIntVector& layer_vec = vec.getLayer(layer);

        for (size_t i = 0; i < layer_size; ++i) {
            hydra::math::BigInt val = layer_vec.getValue(i);
            hydra::math::BigInt zero(0);
            hydra::math::BigInt byte_max(256);

            if (val >= zero && val < byte_max) {
                result.push_back(static_cast<uint8_t>(val.to_int()));
            } else {
                // Handle values outside the byte range
                result.push_back(0);
            }
        }
    }

    return result;
}

// Implementation of LMVS-based address generator
class LMVSAddressGenerator {
public:
    LMVSAddressGenerator(size_t security_level = 128, size_t num_layers = 3)
        : m_security_level(security_level),
          m_num_layers(num_layers),
          m_blake3_hash(),
          m_falcon_signature(security_level <= 128 ? 512 : 1024),
          m_kyber_aes(security_level <= 128 ? "Kyber512" : (security_level <= 192 ? "Kyber768" : "Kyber1024")),
          m_qzkp(8, security_level),
          m_lmvs(num_layers, 32, 5, 3) {

        // Generate key pairs
        auto [falcon_public, falcon_private] = m_falcon_signature.generate_key_pair();
        m_falcon_signature.set_public_key(falcon_public);
        m_falcon_signature.set_private_key(falcon_private);

        auto [kyber_public, kyber_private] = m_kyber_aes.generate_keypair();
        m_kyber_public_key = kyber_public;
        m_kyber_private_key = kyber_private;
    }

    Address generateFromPublicKey(
        const std::vector<uint8_t>& public_key,
        AddressType type,
        AddressFormat format
    ) {
        // Create the address data
        std::vector<uint8_t> address_data;

        // Add type and format (1 byte)
        uint8_t type_format = (static_cast<uint8_t>(type) << 4) | static_cast<uint8_t>(format);
        address_data.push_back(type_format);

        // Hash the public key using BLAKE3
        auto key_hash = m_blake3_hash.hash(public_key);

        // Convert the key hash to a layered BigInt vector
        lmvs::LayeredBigIntVector layered_vec = bytesToLayeredBigIntVector(key_hash, m_num_layers);

        // Apply LMVS transformations
        lmvs::LayeredBigIntVector transformed_vec = applyLMVSTransformations(layered_vec);

        // Convert back to bytes
        std::vector<uint8_t> transformed_hash = layeredBigIntVectorToBytes(transformed_vec);

        // Add the transformed hash
        address_data.insert(address_data.end(), transformed_hash.begin(), transformed_hash.end());

        // Sign the transformed hash using Falcon
        auto signature = m_falcon_signature.sign_message(transformed_hash);

        // Add a compressed version of the signature (variable length)
        VectorCompression compressor(CompressionMethod::HUFFMAN);
        std::vector<hydra::math::BigInt> sig_bigints;
        for (auto byte : signature) {
            sig_bigints.push_back(hydra::math::BigInt(byte));
        }
        auto compressed_sig = compressor.compressBigInts(sig_bigints);

        // Add the compressed signature length (2 bytes)
        uint16_t sig_length = static_cast<uint16_t>(compressed_sig.size());
        address_data.push_back(sig_length >> 8);
        address_data.push_back(sig_length & 0xFF);

        // Add the compressed signature
        address_data.insert(address_data.end(), compressed_sig.begin(), compressed_sig.end());

        // Calculate checksum (4 bytes)
        auto checksum = m_blake3_hash.hash(address_data, 4);

        // Add the checksum
        address_data.insert(address_data.end(), checksum.begin(), checksum.end());

        return Address(address_data);
    }

    Address generateGeoAddress(
        const std::vector<uint8_t>& public_key,
        const Coordinates& coordinates,
        AddressType type
    ) {
        // Create the address data
        std::vector<uint8_t> address_data;

        // Add type and format (1 byte)
        uint8_t type_format = (static_cast<uint8_t>(type) << 4) | static_cast<uint8_t>(AddressFormat::GEOHASHED);
        address_data.push_back(type_format);

        // Convert coordinates to geohash
        Geohash geohash_converter(GeohashPrecision::LEVEL_9);  // ~5m precision
        std::string geohash = geohash_converter.encode(coordinates.latitude, coordinates.longitude);

        // Add geohash length (1 byte)
        address_data.push_back(static_cast<uint8_t>(geohash.length()));

        // Add geohash
        address_data.insert(address_data.end(), geohash.begin(), geohash.end());

        // Hash the public key using BLAKE3
        auto key_hash = m_blake3_hash.hash(public_key);

        // Convert the key hash to a layered BigInt vector
        lmvs::LayeredBigIntVector layered_vec = bytesToLayeredBigIntVector(key_hash, m_num_layers);

        // Apply LMVS transformations
        lmvs::LayeredBigIntVector transformed_vec = applyLMVSTransformations(layered_vec);

        // Convert back to bytes
        std::vector<uint8_t> transformed_hash = layeredBigIntVectorToBytes(transformed_vec);

        // Add the transformed hash
        address_data.insert(address_data.end(), transformed_hash.begin(), transformed_hash.end());

        // Sign the combined data using Falcon
        std::vector<uint8_t> combined_data = transformed_hash;
        combined_data.insert(combined_data.end(), geohash.begin(), geohash.end());
        auto signature = m_falcon_signature.sign_message(combined_data);

        // Add a compressed version of the signature (variable length)
        VectorCompression compressor(CompressionMethod::HUFFMAN);
        std::vector<hydra::math::BigInt> sig_bigints;
        for (auto byte : signature) {
            sig_bigints.push_back(hydra::math::BigInt(byte));
        }
        auto compressed_sig = compressor.compressBigInts(sig_bigints);

        // Add the compressed signature length (2 bytes)
        uint16_t sig_length = static_cast<uint16_t>(compressed_sig.size());
        address_data.push_back(sig_length >> 8);
        address_data.push_back(sig_length & 0xFF);

        // Add the compressed signature
        address_data.insert(address_data.end(), compressed_sig.begin(), compressed_sig.end());

        // Calculate checksum (4 bytes)
        auto checksum = m_blake3_hash.hash(address_data, 4);

        // Add the checksum
        address_data.insert(address_data.end(), checksum.begin(), checksum.end());

        return Address(address_data);
    }

    bool verifyAddress(const Address& address) {
        return address.verify();
    }

    size_t getSecurityLevel() const {
        return m_security_level;
    }

    void setSecurityLevel(size_t security_level) {
        m_security_level = security_level;

        // Since these classes don't support assignment, we need to recreate them

        // Create a new FalconSignature
        m_falcon_signature.~FalconSignature();
        new (&m_falcon_signature) hydra::crypto::FalconSignature(security_level <= 128 ? 512 : 1024);

        // Generate new key pair
        auto [falcon_public, falcon_private] = m_falcon_signature.generate_key_pair();
        m_falcon_signature.set_public_key(falcon_public);
        m_falcon_signature.set_private_key(falcon_private);

        // Create a new KyberAES
        m_kyber_aes.~KyberAES();
        new (&m_kyber_aes) hydra::crypto::KyberAES(security_level <= 128 ? "Kyber512" :
                                                 (security_level <= 192 ? "Kyber768" : "Kyber1024"));

        // Generate new key pair
        auto [kyber_public, kyber_private] = m_kyber_aes.generate_keypair();
        m_kyber_public_key = kyber_public;
        m_kyber_private_key = kyber_private;

        // Create a new QuantumZKP
        m_qzkp.~QuantumZKP();
        new (&m_qzkp) hydra::qzkp::QuantumZKP(8, security_level);
    }

private:
    // Apply LMVS transformations to a layered BigInt vector
    lmvs::LayeredBigIntVector applyLMVSTransformations(const lmvs::LayeredBigIntVector& vec) {
        // Create a projection matrix
        size_t input_dim = vec.getDimension();
        size_t output_dim = input_dim;  // Keep the same dimension for now

        // Convert to double vector for LMVS operations
        auto double_vec = vec.toDoubleVector();

        // Create a layered vector
        lmvs::LayeredVector layered_vec(double_vec);

        // Project the vector
        lmvs::LayeredVector projected_vec = m_lmvs.projectVector(layered_vec, output_dim);

        // Convert back to LayeredBigIntVector
        return lmvs::LayeredBigIntVector(projected_vec.getAllLayers());
    }

    size_t m_security_level;
    size_t m_num_layers;
    hydra::crypto::Blake3Hash m_blake3_hash;
    hydra::crypto::FalconSignature m_falcon_signature;
    hydra::crypto::KyberAES m_kyber_aes;
    hydra::qzkp::QuantumZKP m_qzkp;
    std::vector<uint8_t> m_kyber_public_key;
    std::vector<uint8_t> m_kyber_private_key;
    lmvs::LMVS m_lmvs;
};

} // namespace address
} // namespace hydra
