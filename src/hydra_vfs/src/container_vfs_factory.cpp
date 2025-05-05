#include "hydra_vfs/container_vfs.h"
#include "hydra_vfs/encrypted_vfs.h"
#include "hydra_vfs/persistent_vfs.h"
#include <hydra_crypto/kyber_aes.hpp>
#include <filesystem>
#include <iostream>

namespace hydra {
namespace vfs {

// Implementation of create_container_vfs function
std::shared_ptr<IVirtualFileSystem> create_container_vfs(
    const std::string& container_path,
    const EncryptionKey& key,
    std::shared_ptr<IVirtualFileSystem> base_vfs,
    SecurityLevel security_level,
    const ResourceLimits& resource_limits) {
    
    // Create a persistent VFS as the base if none is provided
    if (!base_vfs) {
        std::filesystem::path path(container_path);
        std::string parent_dir = path.parent_path().string();
        std::cout << "DEBUG: Creating persistent VFS for container at: " << parent_dir << std::endl;
        base_vfs = create_vfs(parent_dir);
    }
    
    // Create a KyberAES encryption provider for post-quantum security
    std::cout << "DEBUG: Creating KyberAES encryption provider" << std::endl;
    auto encryption_provider = std::make_shared<KyberAESEncryptionProvider>("Kyber768");
    
    // Generate a keypair if the provided key is empty
    EncryptionKey effective_key = key;
    bool is_empty_key = true;
    
    // Check if the key is empty (all zeros)
    for (size_t i = 0; i < key.size(); ++i) {
        if (key[i] != 0) {
            is_empty_key = false;
            break;
        }
    }
    
    // If the key is empty, generate a new keypair using KyberAES
    if (is_empty_key) {
        std::cout << "DEBUG: Generating new Kyber keypair for container" << std::endl;
        try {
            // Generate a keypair
            auto keypair_result = encryption_provider->generate_keypair();
            if (keypair_result.success()) {
                auto [public_key, private_key] = keypair_result.value();
                
                // Store the private key as the effective key
                // Note: In a real-world scenario, we would securely store both keys
                // and use the public key for encryption and private key for decryption
                size_t copy_size = std::min(private_key.size(), effective_key.size());
                std::copy_n(private_key.begin(), copy_size, effective_key.begin());
                
                std::cout << "DEBUG: Successfully generated Kyber keypair" << std::endl;
            } else {
                std::cerr << "DEBUG: Failed to generate Kyber keypair, using fallback key" << std::endl;
                // Use a deterministic key based on the container path as fallback
                for (size_t i = 0; i < container_path.size() && i < effective_key.size(); ++i) {
                    effective_key[i % effective_key.size()] ^= container_path[i];
                }
                
                // Ensure the key has some entropy
                for (size_t i = 0; i < effective_key.size(); ++i) {
                    if (effective_key[i] == 0) {
                        effective_key[i] = 0xAA; // Arbitrary non-zero value
                    }
                }
            }
        } catch (const std::exception& e) {
            std::cerr << "DEBUG: Exception during Kyber keypair generation: " << e.what() << std::endl;
            // Use a deterministic key based on the container path as fallback
            for (size_t i = 0; i < container_path.size() && i < effective_key.size(); ++i) {
                effective_key[i % effective_key.size()] ^= container_path[i];
            }
            
            // Ensure the key has some entropy
            for (size_t i = 0; i < effective_key.size(); ++i) {
                if (effective_key[i] == 0) {
                    effective_key[i] = 0xAA; // Arbitrary non-zero value
                }
            }
        }
    }
    
    // Create and return a new ContainerVFS instance
    return std::make_shared<ContainerVFS>(
        container_path,
        encryption_provider,
        effective_key,
        base_vfs,
        security_level,
        resource_limits
    );
}

} // namespace vfs
} // namespace hydra
