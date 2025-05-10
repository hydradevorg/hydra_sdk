# Hydra SDK Examples

This document provides examples of common tasks and use cases for the Hydra SDK.

## Address Generation Examples

### Generating and Verifying Addresses

This example demonstrates how to generate different types of addresses and verify them.

```bash
#!/bin/bash
# Generate a key pair
hydra-cli crypto keygen --algorithm falcon --type public public_key.bin
hydra-cli crypto keygen --algorithm falcon --type private private_key.bin

# Generate a standard address
STANDARD_ADDR=$(hydra-cli crypto address standard --type user public_key.bin | grep "Standard Address:" | cut -d' ' -f3)
echo "Generated standard address: $STANDARD_ADDR"

# Generate a compressed address
COMPRESSED_ADDR=$(hydra-cli crypto address compressed --type resource public_key.bin | grep "Compressed Address:" | cut -d' ' -f3)
echo "Generated compressed address: $COMPRESSED_ADDR"

# Generate a geohashed address (San Francisco coordinates)
GEO_ADDR=$(hydra-cli crypto address geo --type node 37.7749 -122.4194 public_key.bin | grep "Geohashed Address:" | cut -d' ' -f3)
echo "Generated geohashed address: $GEO_ADDR"

# Verify the addresses
hydra-cli crypto address verify "$STANDARD_ADDR"
hydra-cli crypto address verify "$COMPRESSED_ADDR"
hydra-cli crypto address verify "$GEO_ADDR"
```

### Programmatic Address Generation

This example shows how to use the Hydra SDK API to generate addresses programmatically.

```cpp
#include <hydra_address/address_generator.hpp>
#include <hydra_crypto/falcon_signature.hpp>
#include <iostream>
#include <fstream>
#include <vector>

int main() {
    // Generate a key pair
    hydra::crypto::FalconSignature falcon(512);
    auto [public_key, private_key] = falcon.generate_key_pair();
    
    // Create an address generator
    hydra::address::AddressGenerator address_gen(128);
    
    // Generate a standard address
    auto standard_address = address_gen.generateFromPublicKey(
        public_key,
        hydra::address::AddressType::USER,
        hydra::address::AddressFormat::STANDARD
    );
    
    std::cout << "Standard Address: " << standard_address.toString() << std::endl;
    
    // Generate a compressed address
    auto compressed_address = address_gen.generateCompressedAddress(
        public_key,
        hydra::address::AddressType::RESOURCE
    );
    
    std::cout << "Compressed Address: " << compressed_address.toString() << std::endl;
    std::cout << "Address Size: " << compressed_address.getData().size() << " bytes" << std::endl;
    
    // Generate a geohashed address
    hydra::address::Coordinates coords{37.7749, -122.4194, 0.0};  // San Francisco
    
    auto geo_address = address_gen.generateGeoAddress(
        public_key,
        coords,
        hydra::address::AddressType::NODE
    );
    
    std::cout << "Geohashed Address: " << geo_address.toString() << std::endl;
    
    // Extract geohash from the address
    auto geohash = geo_address.getGeohash();
    if (geohash) {
        std::cout << "Geohash: " << *geohash << std::endl;
    }
    
    // Verify the addresses
    std::cout << "Standard Address Verification: " 
              << (address_gen.verifyAddress(standard_address) ? "Valid" : "Invalid") << std::endl;
    
    std::cout << "Compressed Address Verification: " 
              << (address_gen.verifyAddress(compressed_address) ? "Valid" : "Invalid") << std::endl;
    
    std::cout << "Geohashed Address Verification: " 
              << (address_gen.verifyAddress(geo_address) ? "Valid" : "Invalid") << std::endl;
    
    return 0;
}
```

## Secure File Storage Examples

### Creating and Using an Encrypted Container

This example demonstrates how to create and use an encrypted VFS container.

```bash
#!/bin/bash
# Create a new encrypted container
hydra-cli vfs container create --password mysecretpassword mycontainer.vfs

# Create a test file
echo "This is a test file" > test.txt

# Add the file to the container
hydra-cli vfs put --password mysecretpassword mycontainer.vfs test.txt /test.txt

# List files in the container
hydra-cli vfs ls --password mysecretpassword mycontainer.vfs /

# Display the content of the file
hydra-cli vfs cat --password mysecretpassword mycontainer.vfs /test.txt

# Extract the file with a different name
hydra-cli vfs get --password mysecretpassword mycontainer.vfs /test.txt extracted.txt

# Compare the original and extracted files
diff test.txt extracted.txt

# Remove the file from the container
hydra-cli vfs rm --password mysecretpassword mycontainer.vfs /test.txt

# Verify the file is gone
hydra-cli vfs ls --password mysecretpassword mycontainer.vfs /
```

### Programmatic VFS Usage

This example shows how to use the Hydra SDK API to work with the VFS programmatically.

```cpp
#include <hydra_vfs/container_vfs.h>
#include <iostream>
#include <string>
#include <vector>

int main() {
    // Create a new encrypted container
    hydra::vfs::ContainerVFS vfs;
    std::string password = "mysecretpassword";
    std::string container_path = "programmatic.vfs";
    
    // Create the container
    if (!vfs.create(container_path, password)) {
        std::cerr << "Failed to create container" << std::endl;
        return 1;
    }
    
    // Open the container
    if (!vfs.open(container_path, password)) {
        std::cerr << "Failed to open container" << std::endl;
        return 1;
    }
    
    // Create a test file
    std::string content = "This is a test file created programmatically";
    std::vector<uint8_t> data(content.begin(), content.end());
    
    // Write the file to the container
    if (!vfs.write_file("/test.txt", data)) {
        std::cerr << "Failed to write file" << std::endl;
        return 1;
    }
    
    // List files in the container
    auto files = vfs.list_files("/");
    std::cout << "Files in container:" << std::endl;
    for (const auto& file : files) {
        std::cout << "- " << file << std::endl;
    }
    
    // Read the file from the container
    auto read_data = vfs.read_file("/test.txt");
    if (read_data.empty()) {
        std::cerr << "Failed to read file" << std::endl;
        return 1;
    }
    
    // Convert the data back to a string
    std::string read_content(read_data.begin(), read_data.end());
    std::cout << "File content: " << read_content << std::endl;
    
    // Delete the file
    if (!vfs.delete_file("/test.txt")) {
        std::cerr << "Failed to delete file" << std::endl;
        return 1;
    }
    
    // Close the container
    vfs.close();
    
    return 0;
}
```

## Quantum-Resistant Cryptography Examples

### Using Quantum-Resistant Encryption

This example demonstrates how to use quantum-resistant encryption algorithms.

```bash
#!/bin/bash
# Generate Kyber key pair
hydra-cli crypto keygen --algorithm kyber --type private kyber_private.bin
hydra-cli crypto keygen --algorithm kyber --type public kyber_public.bin

# Create a test file
echo "This is a secret message that needs quantum-resistant encryption" > secret.txt

# Encrypt the file using Kyber
hydra-cli crypto encrypt --algorithm kyber --key kyber_public.bin secret.txt secret.enc

# Decrypt the file using Kyber
hydra-cli crypto decrypt --algorithm kyber --key kyber_private.bin secret.enc decrypted.txt

# Compare the original and decrypted files
diff secret.txt decrypted.txt
```

### Using Quantum-Resistant Signatures

This example demonstrates how to use quantum-resistant signature algorithms.

```bash
#!/bin/bash
# Generate Dilithium key pair
hydra-cli crypto keygen --algorithm dilithium --type private dilithium_private.bin
hydra-cli crypto keygen --algorithm dilithium --type public dilithium_public.bin

# Create a test file
echo "This message needs to be signed with a quantum-resistant algorithm" > message.txt

# Sign the file using Dilithium
hydra-cli crypto sign --algorithm dilithium --key dilithium_private.bin message.txt message.sig

# Verify the signature using Dilithium
hydra-cli crypto verify --algorithm dilithium --key dilithium_public.bin message.txt message.sig
```

## More Examples

For more examples, check out the example applications included with the Hydra SDK:

- `address_gen_example`: Demonstrates address generation
- `vfs_example`: Demonstrates VFS operations
- `p2p_vfs_example`: Demonstrates P2P VFS operations
- `secure_vector_transport_example`: Demonstrates secure vector transport
- `lmvs_example`: Demonstrates the Layered Matrix and Vector System
- `kernel_example`: Demonstrates container isolation

These examples can be found in the `examples` directory of the Hydra SDK source code.
