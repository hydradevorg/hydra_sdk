# Hydra Address Generator

The Hydra Address Generator is a C++ library for generating cryptographic addresses for the P2P VFS (Virtual File System). It implements the Layered Matrix and Vector System (LMVS) for secure consensus, fault tolerance, and data projection.

## Features

- **Multiple Address Formats**:
  - Standard addresses
  - Geohashed addresses (with location data)
  - Quantum-resistant addresses (with QZKP)
  - Compressed addresses

- **Cryptographic Security**:
  - BLAKE3 hashing for address generation
  - Falcon signatures for data integrity
  - Kyber KEM and AES for encryption and vector authentication
  - Quantum Zero-Knowledge Proofs (QZKP) for quantum resistance

- **Layered Matrix and Vector System**:
  - Integration with the LMVS module
  - Support for layered matrices and vectors
  - Vector compression techniques

- **Geohashing**:
  - Conversion between coordinates and geohashes
  - Support for different precision levels
  - Geohash-based addressing for location-aware applications

## Usage

### Basic Address Generation

```cpp
#include <hydra_address/address_generator.hpp>
#include <hydra_crypto/falcon_signature.hpp>

// Generate a key pair
hydra::crypto::FalconSignature falcon(512);
auto [public_key, private_key] = falcon.generate_key_pair();

// Create an address generator
hydra::address::AddressGenerator address_gen(128);

// Generate a standard address
auto address = address_gen.generateFromPublicKey(
    public_key,
    hydra::address::AddressType::USER,
    hydra::address::AddressFormat::STANDARD
);

// Get the address as a string
std::string address_str = address.toString();
```

### Geohashed Address Generation

```cpp
// Define coordinates
hydra::address::Coordinates coords{37.7749, -122.4194, 0.0};  // San Francisco

// Generate a geohashed address
auto geo_address = address_gen.generateGeoAddress(
    public_key,
    coords,
    hydra::address::AddressType::NODE
);

// Extract geohash from the address
auto geohash = geo_address.getGeohash();
if (geohash) {
    std::cout << "Geohash: " << *geohash << std::endl;
}

// Extract coordinates from the address
auto extracted_coords = geo_address.getCoordinates();
if (extracted_coords) {
    std::cout << "Coordinates: " << extracted_coords->latitude << ", " 
              << extracted_coords->longitude << std::endl;
}
```

### Quantum-Resistant Address Generation

```cpp
// Create a quantum state vector
std::vector<std::complex<double>> quantum_state(8);
quantum_state[0] = {0.5, 0.0};
quantum_state[1] = {0.5, 0.0};
quantum_state[2] = {0.5, 0.0};
quantum_state[3] = {0.5, 0.0};

// Generate a quantum-resistant address
auto quantum_address = address_gen.generateQuantumAddress(
    public_key,
    quantum_state,
    hydra::address::AddressType::USER
);
```

### Compressed Address Generation

```cpp
// Generate a compressed address
auto compressed_address = address_gen.generateCompressedAddress(
    public_key,
    hydra::address::AddressType::RESOURCE
);
```

### Address Verification

```cpp
// Verify an address
bool is_valid = address_gen.verifyAddress(address);
```

## Integration with LMVS

The address generator integrates with the Layered Matrix and Vector System (LMVS) for secure consensus, fault tolerance, and data projection:

```cpp
// Create an LMVS instance
lmvs::LMVS lmvs_system(3, 32, 5, 3);

// Create a layered vector
std::vector<std::vector<double>> vector_data = {
    {1.0, 2.0, 3.0, 4.0},
    {5.0, 6.0, 7.0, 8.0},
    {9.0, 10.0, 11.0, 12.0}
};

lmvs::LayeredVector layered_vector(vector_data);

// Project the vector
lmvs::LayeredVector projected_vector = lmvs_system.projectVector(layered_vector, 2);

// Split the vector for secure distribution
auto shares = lmvs_system.splitVector(layered_vector, 5, 3);
```

## Building and Testing

### Building the Library

```bash
mkdir build && cd build
cmake ..
make
```

### Running the Tests

```bash
make test
```

### Running the Examples

```bash
./bin/examples/address_generation_example
```

## Dependencies

- Hydra Crypto (BLAKE3, Falcon, Kyber)
- Hydra QZKP (Quantum Zero-Knowledge Proofs)
- Hydra Math (BigInt)
- Hydra LMVS (Layered Matrix and Vector System)
- OpenSSL
- GMP (GNU Multiple Precision Arithmetic Library)
- nlohmann_json (for JSON serialization)

## License

This library is part of the Hydra SDK and is subject to the same license terms.
