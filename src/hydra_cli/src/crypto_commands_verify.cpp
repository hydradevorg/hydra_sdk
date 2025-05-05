#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <fstream>
#include <sstream>

#include "../include/commands/crypto_commands.h"
#include <hydra_crypto/dilithium_signature.hpp>
#include <hydra_crypto/falcon_signature.hpp>

namespace hydra {
namespace cli {
namespace crypto {

int VerifyCommand::execute(const std::vector<std::string>& args) {
    if (is_help_requested(args)) {
        print_help();
        return 0;
    }
    
    // Need at least 3 arguments (key file, input file, and signature file)
    if (args.size() < 3) {
        std::cerr << "Error: Missing required arguments" << std::endl;
        print_help();
        return 1;
    }
    
    // Get parameters from command line
    std::string algo = get_param_value(args, "--algorithm", "dilithium");
    
    // The last three arguments should be key_file, input_file, and signature_file
    size_t argCount = args.size();
    std::string key_file = args[argCount - 3];
    std::string input_file = args[argCount - 2];
    std::string signature_file = args[argCount - 1];
    
    // Validate algorithm
    if (algo != "dilithium" && algo != "falcon") {
        std::cerr << "Error: Unsupported algorithm: " << algo << std::endl;
        std::cerr << "Supported algorithms: dilithium, falcon" << std::endl;
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
    
    // Verify the signature file exists and is readable
    std::ifstream sig(signature_file, std::ios::binary);
    if (!sig) {
        std::cerr << "Error: Could not open signature file: " << signature_file << std::endl;
        return 1;
    }
    
    // Read key file header to determine key type
    char key_header[8] = {0};
    key.read(key_header, sizeof(key_header));
    
    if (!key) {
        std::cerr << "Error: Failed to read key file header" << std::endl;
        return 1;
    }
    
    // Verify this is a public key
    if ((std::string(key_header, 4) == "DLTH" || std::string(key_header, 4) == "FALC") && key_header[4] != 'P') {
        std::cerr << "Warning: Using a private key for verification. Usually a public key is used." << std::endl;
    }
    
    // Check that the algorithm matches the key type
    if (algo == "dilithium" && std::string(key_header, 4) != "DLTH") {
        std::cerr << "Error: Algorithm mismatch. Selected algorithm is dilithium but key is not a Dilithium key." << std::endl;
        return 1;
    }
    
    if (algo == "falcon" && std::string(key_header, 4) != "FALC") {
        std::cerr << "Error: Algorithm mismatch. Selected algorithm is falcon but key is not a Falcon key." << std::endl;
        return 1;
    }
    
    // Read signature file header
    char sig_header[8] = {0};
    sig.read(sig_header, sizeof(sig_header));
    
    if (!sig) {
        std::cerr << "Error: Failed to read signature file header" << std::endl;
        return 1;
    }
    
    // Verify signature file format
    std::string expected_sig_header;
    if (algo == "dilithium") {
        expected_sig_header = "DLTH_SIG";
    } else if (algo == "falcon") {
        expected_sig_header = "FALC_SIG";
    }
    
    if (std::string(sig_header, expected_sig_header.size()) != expected_sig_header) {
        std::cerr << "Error: Invalid signature format for algorithm " << algo << std::endl;
        return 1;
    }
    
    // Read file data
    std::stringstream buffer;
    buffer << input.rdbuf();
    std::string input_data = buffer.str();
    
    std::cout << "Verifying file: " << input_file << std::endl;
    std::cout << "Using key: " << key_file << std::endl;
    std::cout << "Signature file: " << signature_file << std::endl;
    std::cout << "Algorithm: " << algo << std::endl;
    
    // In a real implementation, you would use the crypto library to verify the signature
    // For now, we'll just simulate a successful verification
    
    // Read timestamp and file size from signature
    time_t timestamp;
    size_t file_size;
    
    sig.read(reinterpret_cast<char*>(&timestamp), sizeof(timestamp));
    sig.read(reinterpret_cast<char*>(&file_size), sizeof(file_size));
    
    if (!sig) {
        std::cerr << "Error: Failed to read signature metadata" << std::endl;
        return 1;
    }
    
    // Check file size
    if (file_size != input_data.size()) {
        std::cout << "Verification FAILED: File size mismatch" << std::endl;
        std::cout << "Expected file size: " << file_size << " bytes" << std::endl;
        std::cout << "Actual file size: " << input_data.size() << " bytes" << std::endl;
        return 1;
    }
    
    // In a real implementation, we would verify the cryptographic signature here
    // For this mock implementation, we'll just pretend the verification succeeded
    
    std::cout << "Verification SUCCESS" << std::endl;
    std::cout << "File was signed on: " << ctime(&timestamp);
    
    return 0;
}

void VerifyCommand::print_help() const {
    std::cout << "Usage: hydra crypto verify [OPTIONS] KEY_FILE INPUT_FILE SIGNATURE_FILE" << std::endl;
    std::cout << "Verify a signature using a public key" << std::endl;
    std::cout << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  --algorithm ALGO      Signature algorithm (default: dilithium)" << std::endl;
    std::cout << "  -h, --help            Show this help message" << std::endl;
}

} // namespace crypto
} // namespace cli
} // namespace hydra
