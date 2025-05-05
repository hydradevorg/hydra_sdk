#include <iostream>
#include <string>
#include <map>
#include <fstream>
#include <vector>
#include <sstream>

#include "../include/commands/crypto_commands.h"
#include <hydra_crypto/kyber_kem.hpp>

namespace hydra {
namespace cli {
namespace crypto {

int DecryptCommand::execute(const std::vector<std::string>& args) {
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
    char key_header[8] = {0};
    key.read(key_header, sizeof(key_header));
    
    if (!key) {
        std::cerr << "Error: Failed to read key file header" << std::endl;
        return 1;
    }
    
    // Check that the algorithm matches the key type
    if (algo == "kyber" && std::string(key_header, 4) != "KYBR") {
        std::cerr << "Error: Algorithm mismatch. Selected algorithm is kyber but key is not a Kyber key." << std::endl;
        return 1;
    }
    
    // Verify this is a private key
    if (std::string(key_header, 4) == "KYBR" && key_header[4] == 'P') {
        std::cerr << "Error: Cannot decrypt with a public key. Please provide a private key." << std::endl;
        return 1;
    }
    
    // Read encrypted file header
    char enc_header[8] = {0};
    input.read(enc_header, sizeof(enc_header));
    
    if (!input) {
        std::cerr << "Error: Failed to read encrypted file header" << std::endl;
        return 1;
    }
    
    // Verify encrypted file format
    std::string expected_enc_header = "KYBR_ENC";
    if (std::string(enc_header, expected_enc_header.size()) != expected_enc_header) {
        std::cerr << "Error: Invalid encrypted file format" << std::endl;
        return 1;
    }
    
    // Read timestamp and original file size from encrypted file
    time_t timestamp;
    size_t original_size;
    
    input.read(reinterpret_cast<char*>(&timestamp), sizeof(timestamp));
    input.read(reinterpret_cast<char*>(&original_size), sizeof(original_size));
    
    if (!input) {
        std::cerr << "Error: Failed to read encrypted file metadata" << std::endl;
        return 1;
    }
    
    std::cout << "Decrypting file: " << input_file << std::endl;
    std::cout << "Using key: " << key_file << std::endl;
    std::cout << "Output file: " << output_file << std::endl;
    std::cout << "Algorithm: " << algo << std::endl;
    std::cout << "File was encrypted on: " << ctime(&timestamp);
    std::cout << "Original file size: " << original_size << " bytes" << std::endl;
    
    // In a real implementation, you would use the crypto library to decrypt the data
    // For now, we'll simulate a decryption process
    
    // Read the encapsulated key (32 bytes in our mock format)
    char encapsulated_key[32];
    input.read(encapsulated_key, sizeof(encapsulated_key));
    
    if (!input) {
        std::cerr << "Error: Failed to read encapsulated key" << std::endl;
        return 1;
    }
    
    // Read the remaining encrypted data
    std::stringstream buffer;
    buffer << input.rdbuf();
    std::string encrypted_data = buffer.str();
    
    // Create output file
    std::ofstream out_file(output_file, std::ios::binary);
    if (!out_file) {
        std::cerr << "Error: Could not create output file: " << output_file << std::endl;
        return 1;
    }
    
    // In a real implementation, we would use the private key to decrypt the encapsulated key
    // and then use that to decrypt the data
    
    // For our mock implementation, we'll just xor the data with a fixed key
    // This isn't real decryption - just a placeholder for the actual implementation
    for (size_t i = 0; i < encrypted_data.size() && i < original_size; i++) {
        // Using a constant value for "decryption" to simulate the process
        // In a real implementation, this would use the decrypted shared secret
        char key_byte = encapsulated_key[i % sizeof(encapsulated_key)];
        out_file.put(static_cast<char>(encrypted_data[i] ^ key_byte));
    }
    
    out_file.close();
    
    if (out_file.good()) {
        std::cout << "Successfully decrypted file: " << output_file << std::endl;
        return 0;
    } else {
        std::cerr << "Error: Failed to write output file" << std::endl;
        return 1;
    }
}

void DecryptCommand::print_help() const {
    std::cout << "Usage: hydra crypto decrypt [OPTIONS] KEY_FILE INPUT_FILE OUTPUT_FILE" << std::endl;
    std::cout << "Decrypt a file using a private key" << std::endl;
    std::cout << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  --algorithm ALGO      Encryption algorithm (default: kyber)" << std::endl;
    std::cout << "  -h, --help            Show this help message" << std::endl;
}

} // namespace crypto
} // namespace cli
} // namespace hydra
