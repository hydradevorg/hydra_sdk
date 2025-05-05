#include <iostream>
#include <string>
#include <map>
#include <fstream>
#include <vector>
#include <iomanip>

#include "../include/commands/crypto_commands.h"
#include <hydra_crypto/kyber_kem.hpp>
#include <hydra_crypto/dilithium_signature.hpp>
#include <hydra_crypto/falcon_signature.hpp>

namespace hydra {
namespace cli {
namespace crypto {

int KeyInfoCommand::execute(const std::vector<std::string>& args) {
    if (is_help_requested(args)) {
        print_help();
        return 0;
    }
    
    // We need at least one argument (key file path)
    if (args.size() < 1) {
        std::cerr << "Error: Missing key file argument" << std::endl;
        print_help();
        return 1;
    }
    
    // Get key file path (last argument)
    std::string key_file = args[args.size() - 1];
    
    // Try to read the key file
    std::ifstream file(key_file, std::ios::binary);
    if (!file) {
        std::cerr << "Error: Could not open key file: " << key_file << std::endl;
        return 1;
    }
    
    // Read file header to determine key type
    char header[8] = {0};
    file.read(header, sizeof(header));
    
    if (!file) {
        std::cerr << "Error: Failed to read key file header" << std::endl;
        return 1;
    }
    
    // Display key information
    std::cout << "Key File: " << key_file << std::endl;
    
    // Determine key type based on header
    // This is a simplified implementation - in a real system,
    // you would have a proper key format with headers
    if (std::string(header, 4) == "KYBR") {
        std::cout << "Algorithm: Kyber" << std::endl;
        std::cout << "Type: " << (header[4] == 'P' ? "Public" : "Private") << " Key" << std::endl;
        std::cout << "Strength: Kyber-" << static_cast<int>(static_cast<unsigned char>(header[5])) * 256 + static_cast<int>(static_cast<unsigned char>(header[6])) << std::endl;
    } else if (std::string(header, 4) == "DLTH") {
        std::cout << "Algorithm: Dilithium" << std::endl;
        std::cout << "Type: " << (header[4] == 'P' ? "Public" : "Private") << " Key" << std::endl;
        std::cout << "Strength: Level " << static_cast<int>(static_cast<unsigned char>(header[5])) << std::endl;
    } else if (std::string(header, 4) == "FALC") {
        std::cout << "Algorithm: Falcon" << std::endl;
        std::cout << "Type: " << (header[4] == 'P' ? "Public" : "Private") << " Key" << std::endl;
        std::cout << "Degree: " << static_cast<int>(static_cast<unsigned char>(header[5])) * 256 + static_cast<int>(static_cast<unsigned char>(header[6])) << std::endl;
    } else {
        std::cout << "Unknown key format" << std::endl;
        // Display raw header bytes for debugging
        std::cout << "Header bytes (hex): ";
        for (int i = 0; i < sizeof(header); i++) {
            std::cout << std::hex << std::setw(2) << std::setfill('0')
                      << static_cast<int>(static_cast<unsigned char>(header[i])) << " ";
        }
        std::cout << std::dec << std::endl;
    }
    
    return 0;
}

void KeyInfoCommand::print_help() const {
    std::cout << "Usage: hydra crypto keyinfo [OPTIONS] KEY_FILE" << std::endl;
    std::cout << "Display information about a crypto key" << std::endl;
    std::cout << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  -h, --help            Show this help message" << std::endl;
}

} // namespace crypto
} // namespace cli
} // namespace hydra
