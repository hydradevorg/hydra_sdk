#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <fstream>
#include <random>
#include <ctime>
#include <complex>

#include "../include/commands/crypto_commands.h"
#include <hydra_crypto/kyber_kem.hpp>
#include <hydra_crypto/dilithium_signature.hpp>
#include <hydra_crypto/falcon_signature.hpp>
#include <hydra_address/address_generator.hpp>

namespace hydra {
namespace cli {
namespace crypto {

int AddressCommand::execute(const std::vector<std::string>& args) {
    if (is_help_requested(args)) {
        print_help();
        return 0;
    }
    
    // Need at least one argument (operation)
    if (args.empty()) {
        std::cerr << "Error: Missing operation argument" << std::endl;
        print_help();
        return 1;
    }
    
    // Get the operation
    std::string operation = args[0];
    
    // Create a new vector without the operation
    std::vector<std::string> operation_args(args.begin() + 1, args.end());
    
    // Execute the appropriate operation
    if (operation == "standard") {
        return generate_standard_address(operation_args);
    } else if (operation == "geo") {
        return generate_geo_address(operation_args);
    } else if (operation == "quantum") {
        return generate_quantum_address(operation_args);
    } else if (operation == "compressed") {
        return generate_compressed_address(operation_args);
    } else if (operation == "verify") {
        return verify_address(operation_args);
    } else {
        std::cerr << "Error: Unknown operation '" << operation << "'" << std::endl;
        print_help();
        return 1;
    }
}

int AddressCommand::generate_standard_address(const std::vector<std::string>& args) {
    if (args.empty()) {
        std::cerr << "Error: Missing public key file" << std::endl;
        std::cerr << "Usage: hydra-cli crypto address standard [--type TYPE] PUBLIC_KEY_FILE" << std::endl;
        return 1;
    }
    
    // Get parameters
    std::string type_str = get_param_value(args, "--type", "user");
    std::string key_file = args.back();
    
    // Parse address type
    hydra::address::AddressType type = parse_address_type(type_str);
    
    try {
        // Load the public key
        std::vector<uint8_t> public_key = load_public_key(key_file);
        
        // Create address generator
        hydra::address::AddressGenerator address_gen;
        
        // Generate the address
        auto address = address_gen.generateFromPublicKey(
            public_key,
            type,
            hydra::address::AddressFormat::STANDARD
        );
        
        // Output the address
        std::cout << "Standard Address: " << address.toString() << std::endl;
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}

int AddressCommand::generate_geo_address(const std::vector<std::string>& args) {
    if (args.size() < 3) {
        std::cerr << "Error: Missing required arguments" << std::endl;
        std::cerr << "Usage: hydra-cli crypto address geo [--type TYPE] LATITUDE LONGITUDE PUBLIC_KEY_FILE" << std::endl;
        return 1;
    }
    
    // Get parameters
    std::string type_str = get_param_value(args, "--type", "node");
    
    // Get latitude and longitude
    double latitude, longitude;
    try {
        latitude = std::stod(args[0]);
        longitude = std::stod(args[1]);
    } catch (const std::exception& e) {
        std::cerr << "Error: Invalid coordinates format" << std::endl;
        return 1;
    }
    
    // Get key file
    std::string key_file = args[2];
    
    // Parse address type
    hydra::address::AddressType type = parse_address_type(type_str);
    
    try {
        // Load the public key
        std::vector<uint8_t> public_key = load_public_key(key_file);
        
        // Create address generator
        hydra::address::AddressGenerator address_gen;
        
        // Create coordinates
        hydra::address::Coordinates coords{latitude, longitude, 0.0};
        
        // Generate the address
        auto address = address_gen.generateGeoAddress(
            public_key,
            coords,
            type
        );
        
        // Output the address
        std::cout << "Geohashed Address: " << address.toString() << std::endl;
        
        // Output the geohash
        auto geohash = address.getGeohash();
        if (geohash) {
            std::cout << "Geohash: " << *geohash << std::endl;
        }
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}

int AddressCommand::generate_quantum_address(const std::vector<std::string>& args) {
    if (args.empty()) {
        std::cerr << "Error: Missing public key file" << std::endl;
        std::cerr << "Usage: hydra-cli crypto address quantum [--type TYPE] PUBLIC_KEY_FILE" << std::endl;
        return 1;
    }
    
    // Get parameters
    std::string type_str = get_param_value(args, "--type", "user");
    std::string key_file = args.back();
    
    // Parse address type
    hydra::address::AddressType type = parse_address_type(type_str);
    
    try {
        // Load the public key
        std::vector<uint8_t> public_key = load_public_key(key_file);
        
        // Create address generator
        hydra::address::AddressGenerator address_gen;
        
        // Create a simple quantum state vector (8 dimensions)
        std::vector<std::complex<double>> quantum_state(8);
        quantum_state[0] = {0.5, 0.0};
        quantum_state[1] = {0.5, 0.0};
        quantum_state[2] = {0.5, 0.0};
        quantum_state[3] = {0.5, 0.0};
        quantum_state[4] = {0.0, 0.0};
        quantum_state[5] = {0.0, 0.0};
        quantum_state[6] = {0.0, 0.0};
        quantum_state[7] = {0.0, 0.0};
        
        // Generate the address
        auto address = address_gen.generateQuantumAddress(
            public_key,
            quantum_state,
            type
        );
        
        // Output the address
        std::cout << "Quantum-Resistant Address: " << address.toString() << std::endl;
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}

int AddressCommand::generate_compressed_address(const std::vector<std::string>& args) {
    if (args.empty()) {
        std::cerr << "Error: Missing public key file" << std::endl;
        std::cerr << "Usage: hydra-cli crypto address compressed [--type TYPE] PUBLIC_KEY_FILE" << std::endl;
        return 1;
    }
    
    // Get parameters
    std::string type_str = get_param_value(args, "--type", "resource");
    std::string key_file = args.back();
    
    // Parse address type
    hydra::address::AddressType type = parse_address_type(type_str);
    
    try {
        // Load the public key
        std::vector<uint8_t> public_key = load_public_key(key_file);
        
        // Create address generator
        hydra::address::AddressGenerator address_gen;
        
        // Generate the address
        auto address = address_gen.generateCompressedAddress(
            public_key,
            type
        );
        
        // Output the address
        std::cout << "Compressed Address: " << address.toString() << std::endl;
        std::cout << "Address Size: " << address.getData().size() << " bytes" << std::endl;
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}

int AddressCommand::verify_address(const std::vector<std::string>& args) {
    if (args.empty()) {
        std::cerr << "Error: Missing address" << std::endl;
        std::cerr << "Usage: hydra-cli crypto address verify ADDRESS" << std::endl;
        return 1;
    }
    
    // Get the address
    std::string address_str = args[0];
    
    try {
        // Create address from string
        hydra::address::Address address(address_str);
        
        // Create address generator
        hydra::address::AddressGenerator address_gen;
        
        // Verify the address
        bool is_valid = address_gen.verifyAddress(address);
        
        // Output the result
        std::cout << "Address Verification: " << (is_valid ? "Valid" : "Invalid") << std::endl;
        
        // Output address details
        std::cout << "Address Type: ";
        switch (address.getType()) {
            case hydra::address::AddressType::USER:
                std::cout << "User";
                break;
            case hydra::address::AddressType::NODE:
                std::cout << "Node";
                break;
            case hydra::address::AddressType::RESOURCE:
                std::cout << "Resource";
                break;
            case hydra::address::AddressType::SERVICE:
                std::cout << "Service";
                break;
            default:
                std::cout << "Unknown";
        }
        std::cout << std::endl;
        
        std::cout << "Address Format: ";
        switch (address.getFormat()) {
            case hydra::address::AddressFormat::STANDARD:
                std::cout << "Standard";
                break;
            case hydra::address::AddressFormat::GEOHASHED:
                std::cout << "Geohashed";
                break;
            case hydra::address::AddressFormat::QUANTUM_PROOF:
                std::cout << "Quantum-Resistant";
                break;
            case hydra::address::AddressFormat::COMPRESSED:
                std::cout << "Compressed";
                break;
            default:
                std::cout << "Unknown";
        }
        std::cout << std::endl;
        
        // If it's a geohashed address, output the geohash and coordinates
        if (address.getFormat() == hydra::address::AddressFormat::GEOHASHED) {
            auto geohash = address.getGeohash();
            if (geohash) {
                std::cout << "Geohash: " << *geohash << std::endl;
            }
            
            auto coords = address.getCoordinates();
            if (coords) {
                std::cout << "Coordinates: " << coords->latitude << ", " << coords->longitude << std::endl;
            }
        }
        
        return is_valid ? 0 : 1;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}

std::vector<uint8_t> AddressCommand::load_public_key(const std::string& key_file) {
    // Open the file
    std::ifstream file(key_file, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Failed to open key file: " + key_file);
    }
    
    // Read the file
    std::vector<uint8_t> key_data;
    file.seekg(0, std::ios::end);
    size_t file_size = file.tellg();
    file.seekg(0, std::ios::beg);
    
    key_data.resize(file_size);
    file.read(reinterpret_cast<char*>(key_data.data()), file_size);
    
    if (!file) {
        throw std::runtime_error("Failed to read key file: " + key_file);
    }
    
    return key_data;
}

hydra::address::AddressType AddressCommand::parse_address_type(const std::string& type_str) {
    if (type_str == "user") {
        return hydra::address::AddressType::USER;
    } else if (type_str == "node") {
        return hydra::address::AddressType::NODE;
    } else if (type_str == "resource") {
        return hydra::address::AddressType::RESOURCE;
    } else if (type_str == "service") {
        return hydra::address::AddressType::SERVICE;
    } else {
        throw std::runtime_error("Invalid address type: " + type_str);
    }
}

void AddressCommand::print_help() const {
    std::cout << "Usage: hydra-cli crypto address OPERATION [OPTIONS]" << std::endl;
    std::cout << "Generate and manage cryptographic addresses" << std::endl;
    std::cout << std::endl;
    std::cout << "Operations:" << std::endl;
    std::cout << "  standard [--type TYPE] PUBLIC_KEY_FILE" << std::endl;
    std::cout << "    Generate a standard address from a public key" << std::endl;
    std::cout << std::endl;
    std::cout << "  geo [--type TYPE] LATITUDE LONGITUDE PUBLIC_KEY_FILE" << std::endl;
    std::cout << "    Generate a geohashed address with location data" << std::endl;
    std::cout << std::endl;
    std::cout << "  quantum [--type TYPE] PUBLIC_KEY_FILE" << std::endl;
    std::cout << "    Generate a quantum-resistant address" << std::endl;
    std::cout << std::endl;
    std::cout << "  compressed [--type TYPE] PUBLIC_KEY_FILE" << std::endl;
    std::cout << "    Generate a compressed address (< 100 bytes)" << std::endl;
    std::cout << std::endl;
    std::cout << "  verify ADDRESS" << std::endl;
    std::cout << "    Verify an address and display its details" << std::endl;
    std::cout << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  --type TYPE           Address type (user, node, resource, service)" << std::endl;
    std::cout << "  -h, --help            Show this help message" << std::endl;
}

} // namespace crypto
} // namespace cli
} // namespace hydra
