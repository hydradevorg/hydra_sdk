# Hydra Server Library

## Overview

The `hydra_server` library provides a secure API server implementation for the Hydra SDK, enabling remote access to cryptographic operations and other Hydra SDK functionality through a RESTful API interface. It is designed with security in mind, implementing post-quantum cryptographic algorithms for secure communication and authentication.

## Key Features

- **RESTful API**: Clean, well-documented API endpoints for accessing Hydra SDK functionality
- **Post-Quantum Security**: Integration with Hydra's post-quantum cryptographic algorithms
- **API Key Authentication**: Secure API key-based authentication system
- **JSON-based Communication**: Standardized JSON request and response formats
- **Localhost Binding**: Default configuration binds to localhost for enhanced security
- **Base64 Encoding/Decoding**: Built-in utilities for handling binary data in API requests and responses

## API Endpoints

### Health and Status

- `GET /health` - Health check endpoint to verify server status

### Kyber Key Encapsulation Mechanism (KEM)

- `POST /api/kyber/generate` - Generate a Kyber KEM key pair
- `POST /api/kyber/encapsulate` - Encapsulate a shared secret using a public key
- `POST /api/kyber/decapsulate` - Decapsulate a shared secret using a private key

### Dilithium Digital Signatures

- `POST /api/dilithium/generate` - Generate a Dilithium signature key pair
- `POST /api/dilithium/sign` - Sign a message using a private key
- `POST /api/dilithium/verify` - Verify a signature using a public key

### Hybrid Encryption (Kyber-AES)

- `POST /api/hybrid/encrypt` - Encrypt data using Kyber-AES hybrid encryption
- `POST /api/hybrid/decrypt` - Decrypt data using Kyber-AES hybrid encryption

### Key Management

- `POST /api/keys/rotate` - Rotate cryptographic keys
- `GET /api/keys/status` - Get key status information

## Usage Example

```cpp
#include <hydra_server/api_server.hpp>
#include <iostream>
#include <string>
#include <random>

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
    
    // Create and start the API server
    hydra::server::ApiServer server(8888, api_key);
    
    std::cout << "Starting API server on localhost:8888..." << std::endl;
    
    // Start the server
    server.start();
    
    // The server runs in a separate thread, so we can continue with other operations
    // or just wait for user input to stop the server
    std::cout << "Press Enter to stop the server..." << std::endl;
    std::cin.get();
    
    // Stop the server
    server.stop();
    
    return 0;
}
```

## Client Example (curl)

```bash
# Generate a Kyber key pair
curl -X POST http://localhost:8888/api/kyber/generate \
  -H "Authorization: Bearer YOUR_API_KEY" \
  -H "Content-Type: application/json" \
  -d '{"mode": "Kyber768"}'

# Encapsulate a shared secret
curl -X POST http://localhost:8888/api/kyber/encapsulate \
  -H "Authorization: Bearer YOUR_API_KEY" \
  -H "Content-Type: application/json" \
  -d '{"public_key": "BASE64_ENCODED_PUBLIC_KEY"}'

# Sign a message with Dilithium
curl -X POST http://localhost:8888/api/dilithium/sign \
  -H "Authorization: Bearer YOUR_API_KEY" \
  -H "Content-Type: application/json" \
  -d '{"private_key": "BASE64_ENCODED_PRIVATE_KEY", "message": "BASE64_ENCODED_MESSAGE"}'
```

## Security Considerations

- **API Key Protection**: Store the API key securely and rotate it regularly
- **Network Security**: By default, the server binds to localhost to prevent external access
- **TLS Encryption**: For production use, configure TLS encryption for all API communications
- **Input Validation**: All API inputs are validated to prevent injection attacks
- **Rate Limiting**: Consider implementing rate limiting for production deployments
- **Logging**: Enable secure logging for audit purposes

## Integration with Hydra SDK

The `hydra_server` library integrates with other components of the Hydra SDK:

- **hydra_crypto**: Leverages post-quantum cryptographic algorithms
- **hydra_vfs**: Can provide secure storage for keys and other sensitive data
- **hydra_kernel**: Can run the server in an isolated process environment

## Dependencies

- C++17 compatible compiler
- cpp-httplib for HTTP server functionality
- nlohmann/json for JSON parsing and generation
- Hydra SDK cryptographic libraries

## Building

```bash
cd src/hydra_server
mkdir build && cd build
cmake ..
make
```

## Future Developments

- WebSocket support for real-time applications
- OAuth2 authentication option
- Comprehensive API documentation using OpenAPI/Swagger
- Additional endpoints for other Hydra SDK functionality
- Cluster mode for high availability deployments
