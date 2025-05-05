/**
 * @file dilithium_example.cpp
 * @brief Example usage of the hydra_crypto Dilithium signature functionality
 */

#include <crypto/dilithium_signature.hpp>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <stdexcept>

// Helper function to save data to file
void save_to_file(const std::string& filename, const std::vector<uint8_t>& data) {
    std::ofstream file(filename, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Failed to open file for writing: " + filename);
    }
    file.write(reinterpret_cast<const char*>(data.data()), data.size());
}

// Helper function to load data from file
std::vector<uint8_t> load_from_file(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file) {
        throw std::runtime_error("Failed to open file for reading: " + filename);
    }
    
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    
    std::vector<uint8_t> buffer(size);
    if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
        throw std::runtime_error("Failed to read file: " + filename);
    }
    
    return buffer;
}

// Example: Document signing and verification
void example_document_signing() {
    std::cout << "\n===== Document Signing Example =====\n" << std::endl;
    
    try {
        // 1. Create a signer with high security level
        hydra_crypto::DilithiumSignature signer("ML-DSA-87");
        std::cout << "Created signer with " << signer.get_name() 
                  << " (" << signer.get_security_level() << "-bit security)" << std::endl;
        
        // 2. Generate a new key pair
        std::cout << "Generating key pair..." << std::endl;
        auto [public_key, private_key] = signer.generate_key_pair();
        
        std::cout << "Public key size: " << public_key.size() << " bytes" << std::endl;
        std::cout << "Private key size: " << private_key.size() << " bytes" << std::endl;
        
        // 3. Save keys to files (in a real application)
        save_to_file("example_public.key", public_key);
        save_to_file("example_private.key", private_key);
        std::cout << "Keys saved to example_public.key and example_private.key" << std::endl;
        
        // 4. "Document" to be signed
        std::string document = "This is a legally binding contract between Party A and Party B.\n"
                               "Party A agrees to deliver 100 widgets to Party B by December 31, 2023.\n"
                               "Party B agrees to pay $10,000 upon delivery of the widgets.";
        
        std::cout << "\nDocument to sign:\n" << document << std::endl;
        
        // 5. Sign the document
        std::cout << "\nSigning document..." << std::endl;
        auto signature = signer.sign_message(document);
        std::cout << "Signature size: " << signature.size() << " bytes" << std::endl;
        
        // 6. Save signature to file
        save_to_file("example_signature.sig", signature);
        std::cout << "Signature saved to example_signature.sig" << std::endl;
        
        // 7. Verification (by another party)
        std::cout << "\nVerifying signature (as receiver)..." << std::endl;
        
        // 7.1 Load public key and signature
        auto loaded_public_key = load_from_file("example_public.key");
        auto loaded_signature = load_from_file("example_signature.sig");
        
        // 7.2 Create verifier with public key
        hydra_crypto::DilithiumSignature verifier("ML-DSA-87");
        verifier.set_public_key(loaded_public_key);
        
        // 7.3 Verify the document
        bool is_authentic = verifier.verify_signature(document, loaded_signature);
        
        if (is_authentic) {
            std::cout << "✓ Document signature is VALID!" << std::endl;
            std::cout << "  The document is authentic and has not been tampered with." << std::endl;
        } else {
            std::cout << "✗ Document signature is INVALID!" << std::endl;
            std::cout << "  The document may have been tampered with or the signature is incorrect." << std::endl;
        }
        
        // 8. Try verifying a tampered document
        std::string tampered_document = document;
        tampered_document.replace(tampered_document.find("$10,000"), 7, "$20,000");
        
        std::cout << "\nVerifying tampered document..." << std::endl;
        std::cout << "Original: $10,000" << std::endl;
        std::cout << "Tampered: $20,000" << std::endl;
        
        bool tampered_authentic = verifier.verify_signature(tampered_document, loaded_signature);
        
        if (tampered_authentic) {
            std::cout << "✗ ERROR: Tampered document signature reported as valid!" << std::endl;
        } else {
            std::cout << "✓ Correctly detected tampered document (invalid signature)" << std::endl;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

int main() {
    std::cout << "Hydra Crypto - Dilithium Signature Example" << std::endl;
    std::cout << "==========================================" << std::endl;
    
    // Run the document signing example
    example_document_signing();
    
    // Optional: Run the built-in demo as well
    std::cout << "\nRunning built-in demo:" << std::endl;
    hydra_crypto::DilithiumSignature::demo();
    
    return 0;
}
