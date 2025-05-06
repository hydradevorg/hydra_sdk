#include <hydra_address/address_generator.hpp>
#include <hydra_crypto/blake3_hash.hpp>
#include <hydra_crypto/falcon_signature.hpp>
#include <hydra_crypto/kyber_aes.hpp>
#include <lmvs/lmvs.hpp>
#include <iostream>
#include <iomanip>
#include <random>
#include <string>

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

// Helper function to print bytes as hex
void printHex(const std::vector<uint8_t>& data, const std::string& label) {
    std::cout << label << ": ";
    for (auto byte : data) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte);
    }
    std::cout << std::dec << std::endl;
}

int main() {
    std::cout << "Hydra Address Generation Example" << std::endl;
    std::cout << "===============================" << std::endl;
    
    try {
        // Generate a key pair using Falcon
        std::cout << "\n1. Generating Key Pair" << std::endl;
        hydra::crypto::FalconSignature falcon(512);
        auto [public_key, private_key] = falcon.generate_key_pair();
        
        printHex(public_key, "Public Key");
        printHex(private_key, "Private Key (first 32 bytes)");
        
        // Create an address generator
        std::cout << "\n2. Creating Address Generator" << std::endl;
        hydra::address::AddressGenerator address_gen(128);
        
        // Generate a standard address
        std::cout << "\n3. Generating Standard Address" << std::endl;
        auto standard_address = address_gen.generateFromPublicKey(
            public_key,
            hydra::address::AddressType::USER,
            hydra::address::AddressFormat::STANDARD
        );
        
        std::cout << "Standard Address: " << standard_address.toString() << std::endl;
        
        // Generate a geohashed address
        std::cout << "\n4. Generating Geohashed Address" << std::endl;
        hydra::address::Coordinates coords{37.7749, -122.4194, 0.0};  // San Francisco
        
        auto geo_address = address_gen.generateGeoAddress(
            public_key,
            coords,
            hydra::address::AddressType::NODE
        );
        
        std::cout << "Geohashed Address: " << geo_address.toString() << std::endl;
        
        // Extract geohash from the address
        auto geohash = geo_address.getGeohash();
        if (geohash) {
            std::cout << "Geohash: " << *geohash << std::endl;
        }
        
        // Extract coordinates from the address
        auto extracted_coords = geo_address.getCoordinates();
        if (extracted_coords) {
            std::cout << "Extracted Coordinates: " << std::fixed << std::setprecision(6)
                      << extracted_coords->latitude << ", " << extracted_coords->longitude << std::endl;
        }
        
        // Generate a quantum-resistant address
        std::cout << "\n5. Generating Quantum-Resistant Address" << std::endl;
        
        // Create a quantum state vector
        std::vector<std::complex<double>> quantum_state(8);
        quantum_state[0] = {0.5, 0.0};
        quantum_state[1] = {0.5, 0.0};
        quantum_state[2] = {0.5, 0.0};
        quantum_state[3] = {0.5, 0.0};
        
        auto quantum_address = address_gen.generateQuantumAddress(
            public_key,
            quantum_state,
            hydra::address::AddressType::USER
        );
        
        std::cout << "Quantum-Resistant Address: " << quantum_address.toString() << std::endl;
        
        // Generate a compressed address
        std::cout << "\n6. Generating Compressed Address" << std::endl;
        auto compressed_address = address_gen.generateCompressedAddress(
            public_key,
            hydra::address::AddressType::RESOURCE
        );
        
        std::cout << "Compressed Address: " << compressed_address.toString() << std::endl;
        
        // Verify addresses
        std::cout << "\n7. Verifying Addresses" << std::endl;
        std::cout << "Standard Address Verification: " << (address_gen.verifyAddress(standard_address) ? "Valid" : "Invalid") << std::endl;
        std::cout << "Geohashed Address Verification: " << (address_gen.verifyAddress(geo_address) ? "Valid" : "Invalid") << std::endl;
        std::cout << "Quantum-Resistant Address Verification: " << (address_gen.verifyAddress(quantum_address) ? "Valid" : "Invalid") << std::endl;
        std::cout << "Compressed Address Verification: " << (address_gen.verifyAddress(compressed_address) ? "Valid" : "Invalid") << std::endl;
        
        // Demonstrate LMVS integration
        std::cout << "\n8. LMVS Integration Example" << std::endl;
        
        // Create an LMVS instance
        lmvs::LMVS lmvs_system(3, 32, 5, 3);
        
        // Create a layered vector
        std::vector<std::vector<double>> vector_data = {
            {1.0, 2.0, 3.0, 4.0},
            {5.0, 6.0, 7.0, 8.0},
            {9.0, 10.0, 11.0, 12.0}
        };
        
        lmvs::LayeredVector layered_vector(vector_data);
        
        // Project the vector
        lmvs::LayeredVector projected_vector = lmvs_system.projectVector(layered_vector, 2);
        
        std::cout << "Original Vector Dimensions: " << layered_vector.getNumLayers() << " layers, "
                  << layered_vector.getDimension() << " dimensions per layer" << std::endl;
        
        std::cout << "Projected Vector Dimensions: " << projected_vector.getNumLayers() << " layers, "
                  << projected_vector.getDimension() << " dimensions per layer" << std::endl;
        
        // Split the vector for secure distribution
        auto shares = lmvs_system.splitVector(layered_vector, 5, 3);
        
        std::cout << "Vector Split into " << shares.size() << " shares, threshold = 3" << std::endl;
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
