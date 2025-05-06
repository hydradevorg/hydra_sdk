#include "lmvs/security/secure_vector_transport.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <iomanip>
#include <fstream>

// Helper function to print a byte vector as hex
void printHex(const std::vector<uint8_t>& data, size_t max_bytes = 32) {
    std::cout << std::hex << std::setfill('0');
    for (size_t i = 0; i < std::min(data.size(), max_bytes); ++i) {
        std::cout << std::setw(2) << static_cast<int>(data[i]);
        if ((i + 1) % 16 == 0 && i + 1 < std::min(data.size(), max_bytes)) {
            std::cout << std::endl << "   ";
        } else if (i + 1 < std::min(data.size(), max_bytes)) {
            std::cout << " ";
        }
    }
    if (data.size() > max_bytes) {
        std::cout << "... (" << std::dec << data.size() << " bytes total)";
    }
    std::cout << std::dec;
}

int main() {
    std::cout << "Secure Vector Transport Example using Kyber and Falcon" << std::endl;
    std::cout << "=====================================================" << std::endl;
    
    // Create a secure vector transport instance
    lmvs::security::SecureVectorTransport transport("Kyber768", 512);
    
    // Generate keys for two nodes
    std::string alice_id = "alice";
    std::string bob_id = "bob";
    
    std::cout << "\n1. Generating keys for Alice and Bob..." << std::endl;
    auto [alice_public_key, alice_private_key] = transport.generate_node_keys(alice_id);
    auto [bob_public_key, bob_private_key] = transport.generate_node_keys(bob_id);
    
    std::cout << "   Alice's public key bundle: ";
    printHex(alice_public_key);
    std::cout << std::endl;
    
    std::cout << "   Bob's public key bundle: ";
    printHex(bob_public_key);
    std::cout << std::endl;
    
    // Create a layered vector
    std::vector<std::vector<double>> data = {
        {1.0, 2.0, 3.0, 4.0},     // Layer 1
        {5.0, 6.0, 7.0, 8.0},     // Layer 2
        {9.0, 10.0, 11.0, 12.0}   // Layer 3
    };
    
    lmvs::LayeredBigIntVector vector(data);
    
    std::cout << "\n2. Created Layered Vector:" << std::endl;
    vector.print();
    
    // Alice packages the vector for Bob
    std::cout << "\n3. Alice packages the vector for Bob..." << std::endl;
    lmvs::security::SecureVectorPackage package = transport.package_vector(
        vector, bob_id, alice_private_key);
    
    std::cout << "   Encrypted data: ";
    printHex(package.encrypted_data);
    std::cout << std::endl;
    
    std::cout << "   Signature: ";
    printHex(package.signature);
    std::cout << std::endl;
    
    std::cout << "   Authentication tag: ";
    printHex(package.auth_tag);
    std::cout << std::endl;
    
    // Serialize the package
    std::vector<uint8_t> serialized_package = package.serialize();
    
    std::cout << "\n4. Serialized package size: " << serialized_package.size() << " bytes" << std::endl;
    
    // Save the package to a file
    std::string package_file = "vector_package.dat";
    std::ofstream outfile(package_file, std::ios::binary);
    outfile.write(reinterpret_cast<const char*>(serialized_package.data()), serialized_package.size());
    outfile.close();
    
    std::cout << "   Saved package to " << package_file << std::endl;
    
    // Load the package from file
    std::ifstream infile(package_file, std::ios::binary);
    std::vector<uint8_t> loaded_data((std::istreambuf_iterator<char>(infile)), std::istreambuf_iterator<char>());
    infile.close();
    
    // Deserialize the package
    lmvs::security::SecureVectorPackage loaded_package = lmvs::security::SecureVectorPackage::deserialize(loaded_data);
    
    std::cout << "\n5. Loaded and deserialized package from file" << std::endl;
    
    // Bob unpacks the vector
    std::cout << "\n6. Bob unpacks the vector..." << std::endl;
    auto [unpacked_vector, is_valid] = transport.unpackage_vector(
        loaded_package, alice_id, bob_private_key);
    
    std::cout << "   Signature and authentication valid: " << (is_valid ? "Yes" : "No") << std::endl;
    
    if (is_valid) {
        std::cout << "   Unpacked vector:" << std::endl;
        unpacked_vector.print();
        
        // Convert back to double values for comparison
        std::vector<std::vector<double>> unpacked_data = unpacked_vector.toDoubleVector();
        
        std::cout << "\n7. Comparing original and unpacked data:" << std::endl;
        bool data_matches = true;
        for (size_t i = 0; i < data.size(); ++i) {
            std::cout << "   Layer " << i << ": ";
            if (data[i].size() != unpacked_data[i].size()) {
                std::cout << "Size mismatch!" << std::endl;
                data_matches = false;
                continue;
            }
            
            bool layer_matches = true;
            for (size_t j = 0; j < data[i].size(); ++j) {
                if (std::abs(data[i][j] - unpacked_data[i][j]) > 1e-6) {
                    layer_matches = false;
                    break;
                }
            }
            
            std::cout << (layer_matches ? "Match" : "Mismatch") << std::endl;
            data_matches = data_matches && layer_matches;
        }
        
        std::cout << "   Overall data integrity: " << (data_matches ? "Preserved" : "Compromised") << std::endl;
    }
    
    // Try to tamper with the package
    std::cout << "\n8. Testing tamper detection..." << std::endl;
    
    lmvs::security::SecureVectorPackage tampered_package = loaded_package;
    if (!tampered_package.encrypted_data.empty()) {
        // Modify a byte in the encrypted data
        tampered_package.encrypted_data[tampered_package.encrypted_data.size() / 2] ^= 0xFF;
        
        std::cout << "   Tampered with encrypted data" << std::endl;
        
        // Bob tries to unpack the tampered vector
        auto [tampered_vector, tampered_valid] = transport.unpackage_vector(
            tampered_package, alice_id, bob_private_key);
        
        std::cout << "   Signature and authentication valid: " << (tampered_valid ? "Yes (BAD!)" : "No (GOOD!)") << std::endl;
    }
    
    // Try with wrong sender
    std::cout << "\n9. Testing wrong sender detection..." << std::endl;
    
    try {
        auto [wrong_sender_vector, wrong_sender_valid] = transport.unpackage_vector(
            loaded_package, "mallory", bob_private_key);
        
        std::cout << "   Unpacking with wrong sender: " 
                  << (wrong_sender_valid ? "Succeeded (BAD!)" : "Failed (GOOD!)") << std::endl;
    } catch (const std::exception& e) {
        std::cout << "   Unpacking with wrong sender failed as expected: " << e.what() << std::endl;
    }
    
    // Try with wrong recipient
    std::cout << "\n10. Testing wrong recipient detection..." << std::endl;
    
    try {
        auto [wrong_recipient_vector, wrong_recipient_valid] = transport.unpackage_vector(
            loaded_package, alice_id, alice_private_key); // Alice tries to unpack her own message
        
        std::cout << "   Unpacking with wrong recipient: " 
                  << (wrong_recipient_valid ? "Succeeded (BAD!)" : "Failed (GOOD!)") << std::endl;
    } catch (const std::exception& e) {
        std::cout << "   Unpacking with wrong recipient failed as expected: " << e.what() << std::endl;
    }
    
    std::cout << "\nSecure Vector Transport Example Completed Successfully!" << std::endl;
    
    return 0;
}
