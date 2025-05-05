#include <hydra_server/api_server.hpp>
#include <iostream>
#include <string>
#include <random>
#include <iomanip>
#include <sstream>
#include <thread>
#include <chrono>

// Function to generate a random API key
std::string generate_random_api_key(size_t length = 32) {
    static const char charset[] = "0123456789"
                                  "abcdefghijklmnopqrstuvwxyz"
                                  "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(0, sizeof(charset) - 2);
    
    std::string key;
    key.reserve(length);
    
    for (size_t i = 0; i < length; ++i) {
        key += charset[dist(gen)];
    }
    
    return key;
}

int main() {
    // Generate a random API key
    std::string api_key = generate_random_api_key();
    std::cout << "Generated API key: " << api_key << std::endl;
    std::cout << "Store this key securely as it will be required for API access." << std::endl;
    
    // Create and start the API server
    hydra::server::ApiServer server(8888, api_key);
    
    std::cout << "Starting API server on localhost:8888..." << std::endl;
    std::cout << "The server is configured to only accept connections from localhost." << std::endl;
    std::cout << "All requests must include the API key in the Authorization header:" << std::endl;
    std::cout << "  Authorization: Bearer " << api_key << std::endl;
    
    // Start the server
    server.start();
    
    std::cout << "\nAPI Endpoints:" << std::endl;
    std::cout << "  GET  /health                    - Health check" << std::endl;
    std::cout << "  POST /api/kyber/generate        - Generate Kyber KEM key pair" << std::endl;
    std::cout << "  POST /api/kyber/encapsulate     - Encapsulate shared secret" << std::endl;
    std::cout << "  POST /api/kyber/decapsulate     - Decapsulate shared secret" << std::endl;
    std::cout << "  POST /api/dilithium/generate    - Generate Dilithium signature key pair" << std::endl;
    std::cout << "  POST /api/dilithium/sign        - Sign a message" << std::endl;
    std::cout << "  POST /api/dilithium/verify      - Verify a signature" << std::endl;
    
    std::cout << "\nPress Enter to stop the server..." << std::endl;
    std::cin.get();
    
    std::cout << "Stopping API server..." << std::endl;
    server.stop();
    std::cout << "Server stopped." << std::endl;
    return 0;
}
