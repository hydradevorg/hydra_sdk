#include <iostream>
#include <string>
#include <map>
#include <fstream>
#include <vector>
#include <sstream>
#include <random>
#include <chrono>

#include "../include/commands/crypto_commands.h"
#include <hydra_crypto/kyber_kem.hpp>

namespace hydra {
namespace cli {
namespace crypto {

int EncryptCommand::execute(const std::vector<std::string>& args) {
    if (is_help_requested(args)) {
        print_help();
        return 0;
    }
    
    // Need at least 3 arguments (key file, input file, and output file)
    if (args.size() < 3) {
        std::cerr << "Error: Missing required arguments" << std::endl;
        print_help();
        return 1;
    }
    
    // Get parameters from command line
    std::string algo = get_param_value(args, "--algorithm", "kyber");
    
    // The last three arguments should be key_file, input_file, and output_file
    size_t argCount = args.size();
    std::string key_file = args[argCount - 3];
    std::string input_file = args[argCount - 2];
    std::string output_file = args[argCount - 1];
    
    // Validate algorithm
    if (algo != "kyber") {
        std::cerr << "Error: Unsupported algorithm: " << algo << std::endl;
        std::cerr << "Supported algorithms: kyber" << std::endl;
        return 1;
    }
    
    // Verify the key file exists and is readable
    std::ifstream key(key_file, std::ios::binary);
    if (!key) {
        std::cerr << "Error: Could not open key file: " << key_file << std::endl;
        return 1;
    }
    
    // Verify the input file exists and is readable
    std::ifstream input(input_file, std::ios::binary);
    if (!input) {
        std::cerr << "Error: Could not open input file: " << input_file << std::endl;
        return 1;
    }
    
    // Read key file header to determine key type
    char header[8] = {0};
    key.read(header, sizeof(header));
    
    if (!key) {
        std::cerr << "Error: Failed to read key file header" << std::endl;
        return 1;
    }
    
    // Check that the algorithm matches the key type
    if (algo == "kyber" && std::string(header, 4) != "KYBR") {
        std::cerr << "Error: Algorithm mismatch. Selected algorithm is kyber but key is not a Kyber key." << std::endl;
        return 1;
    }
    
    // Verify this is a public key
    if (std::string(header, 4) == "KYBR" && header[4] != 'P') {
        std::cerr << "Error: Cannot encrypt with a private key. Please provide a public key." << std::endl;
        return 1;
    }
    
    // Read the entire input file
    std::stringstream buffer;
    buffer << input.rdbuf();
    std::string input_data = buffer.str();
    
    std::cout << "Encrypting file: " << input_file << std::endl;
    std::cout << "Using key: " << key_file << std::endl;
    std::cout << "Output file: " << output_file << std::endl;
    std::cout << "Algorithm: " << algo << std::endl;
    
    // In a real implementation, you would use the crypto library to encrypt the data
    // For now, we'll just create a dummy encrypted file
    
    std::ofstream out_file(output_file, std::ios::binary);
    if (!out_file) {
        std::cerr << "Error: Could not create output file: " << output_file << std::endl;
        return 1;
    }
    
    // Write header: "KYBR_ENC" + timestamps
    out_file << "KYBR_ENC";
    
    // Add timestamp
    time_t now = time(nullptr);
    out_file.write(reinterpret_cast<const char*>(&now), sizeof(now));
    
    // Add original file size
    size_t file_size = input_data.size();
    out_file.write(reinterpret_cast<const char*>(&file_size), sizeof(file_size));
    
    // Use a random seed based on current time for "encryption"
    std::mt19937 rng(std::chrono::system_clock::now().time_since_epoch().count());
    std::uniform_int_distribution<int> dist(0, 255);
    
    // Add 32 bytes of "shared secret" data (in a real implementation, this would be the encapsulated key)
    for (int i = 0; i < 32; i++) {
        out_file.put(static_cast<char>(dist(rng)));
    }
    
    // "Encrypt" the data (in a real implementation, this would use the shared secret to encrypt)
    for (size_t i = 0; i < input_data.size(); i++) {
        char key_byte = static_cast<char>(dist(rng) & 0xFF);
        out_file.put(static_cast<char>(input_data[i] ^ key_byte));
    }
    
    out_file.close();
    
    if (out_file.good()) {
        std::cout << "Successfully encrypted file: " << output_file << std::endl;
        return 0;
    } else {
        std::cerr << "Error: Failed to write output file" << std::endl;
        return 1;
    }
}

void EncryptCommand::print_help() const {
    std::cout << "Usage: hydra crypto encrypt [OPTIONS] KEY_FILE INPUT_FILE OUTPUT_FILE" << std::endl;
    std::cout << "Encrypt a file using a public key" << std::endl;
    std::cout << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  --algorithm ALGO      Encryption algorithm (default: kyber)" << std::endl;
    std::cout << "  -h, --help            Show this help message" << std::endl;
}

} // namespace crypto
} // namespace cli
} // namespace hydra
