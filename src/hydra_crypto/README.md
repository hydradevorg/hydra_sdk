# Hydra Crypto Library

A modern C++ cryptographic library for the Hydra SDK, focusing on post-quantum cryptography.

## Overview

The `hydra_crypto` library provides post-quantum secure cryptographic primitives for the Hydra SDK. It implements various post-quantum cryptographic algorithms that are resistant to attacks from quantum computers, making it suitable for applications requiring long-term security.

## Features

- **Post-Quantum Cryptography**: Implementations of NIST-standardized algorithms:
  - Kyber key encapsulation mechanism (KEM)
  - Dilithium (ML-DSA) digital signatures
  - Falcon digital signatures
- **Hybrid Encryption**: Combines post-quantum algorithms with traditional cryptography for defense-in-depth
- **Key Management**: Secure generation, storage, and rotation of cryptographic keys
- **Easy Integration**: Designed for seamless integration with Hydra SDK
- **Flexible API**: Simple yet powerful interface for cryptographic operations
- **Standards Compliant**: Follows NIST-approved post-quantum cryptography standards

## Key Components

### Key Encapsulation Mechanisms (KEM)

- **KyberKEM**: Implementation of the Kyber key encapsulation mechanism, a lattice-based post-quantum secure key exchange algorithm that has been standardized by NIST. Supports multiple security levels (Kyber512, Kyber768, Kyber1024).

- **KyberAES**: A hybrid encryption scheme that combines Kyber KEM with AES for authenticated encryption, providing both post-quantum security and efficient symmetric encryption.

### Digital Signature Algorithms

- **DilithiumSignature**: Implementation of the Dilithium (ML-DSA) digital signature algorithm, a lattice-based post-quantum secure signature scheme that has been standardized by NIST. Supports multiple security levels (ML-DSA-44, ML-DSA-65, ML-DSA-87).

- **FalconSignature**: Implementation of the Falcon digital signature algorithm, another lattice-based post-quantum secure signature scheme that offers smaller signatures compared to Dilithium but with more complex implementation.

### Key Management

- **RootKeyManager**: A secure key management system that handles the generation, storage, and rotation of root cryptographic keys, with support for hardware security modules (HSMs) where available.

## Usage Examples

### KyberKEM Example

```cpp
#include "hydra_crypto/kyber_kem.hpp"
#include <iostream>

using namespace hydra::crypto;

int main() {
    // Generate a keypair
    KyberKEM kem("Kyber768");
    auto [public_key, private_key] = kem.generate_keypair();
    
    // Encapsulate a shared key
    auto [ciphertext, shared_key] = kem.encapsulate(public_key);
    
    // Decapsulate the shared key
    auto decapsulated_key = kem.decapsulate(ciphertext, private_key);
    
    // Verify that both parties have the same shared key
    if (shared_key == decapsulated_key) {
        std::cout << "Key exchange successful!" << std::endl;
    }
    
    return 0;
}
```

### DilithiumSignature Example

```cpp
#include "hydra_crypto/dilithium_signature.hpp"
#include <iostream>
#include <string>

using namespace hydra::crypto;

int main() {
    // Create a signer with default security level (ML-DSA-65)
    DilithiumSignature signer;
    
    // Generate a key pair
    auto [public_key, private_key] = signer.generate_key_pair();
    
    // Set the keys
    signer.set_public_key(public_key);
    signer.set_private_key(private_key);
    
    // Sign a message
    std::string message = "Hello, post-quantum world!";
    auto signature = signer.sign_message(message);
    
    // Verify the signature
    bool is_valid = signer.verify_signature(message, signature);
    
    if (is_valid) {
        std::cout << "Signature verified successfully!" << std::endl;
    }
    
    return 0;
}
```

## Getting Started

### Prerequisites

- C++17 compatible compiler
- CMake 3.12 or higher
- Botan 2.x cryptography library
- OpenSSL (for hybrid encryption schemes)

### Building the Library

```bash
cd src/hydra_crypto
mkdir build && cd build
cmake ..
make
```

## Testing

The library includes a comprehensive test suite that verifies the correctness and interoperability of all cryptographic primitives:

```bash
cd build
ctest
```

## Security Considerations

- The library implements post-quantum cryptographic algorithms that are believed to be resistant to attacks from both classical and quantum computers.
- The implementations follow best practices for secure coding and constant-time operations to resist side-channel attacks.
- Regular security audits and updates are recommended to ensure continued protection against emerging threats.

## Integration with Hydra SDK

The `hydra_crypto` library is designed to work seamlessly with other components of the Hydra SDK:

- **hydra_vfs**: Secure storage of cryptographic keys and encrypted data
- **hydra_math**: Mathematical operations required for cryptographic algorithms
- **hydra_server**: Secure communication channels using post-quantum cryptography

## Future Developments

- Integration with hardware security modules (HSMs) for key protection
- Support for additional post-quantum algorithms as they are standardized
- Performance optimizations for resource-constrained environments

## References

- [NIST Post-Quantum Cryptography Standardization](https://csrc.nist.gov/projects/post-quantum-cryptography)
- [Kyber KEM Specification](https://pq-crystals.org/kyber/)
- [Dilithium Signature Specification](https://pq-crystals.org/dilithium/)
- [Falcon Signature Specification](https://falcon-sign.info/)
