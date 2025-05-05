#ifndef HYDRA_CLI_VFS_UTILS_H
#define HYDRA_CLI_VFS_UTILS_H

#include <string>
#include <fstream>
#include <stdexcept>
#include <vector>
#include <hydra_vfs/vfs.h>
#include <hydra_vfs/encrypted_vfs.h>

namespace hydra {
namespace cli {
namespace vfs {

// Utility function to derive an encryption key from a password
inline hydra::vfs::EncryptionKey derive_key_from_password(const std::string& password) {
    hydra::vfs::EncryptionKey key = {};
    
    // Zero the key first
    std::fill(key.begin(), key.end(), 0);
    
    if (password.empty()) {
        return key;
    }
    
    // Simple key derivation: first pass - copy password bytes
    for (size_t i = 0; i < key.size() && i < password.size(); ++i) {
        key[i] = password[i];
    }
    
    // If password is shorter than key, repeat it
    if (password.size() < key.size()) {
        for (size_t i = password.size(); i < key.size(); ++i) {
            key[i] = password[i % password.size()];
        }
    }
    
    // Second pass - mix the key bytes
    for (size_t i = 1; i < key.size(); ++i) {
        key[i] = key[i] ^ key[i-1];
    }
    
    // Final pass - ensure key has good entropy
    for (size_t i = 0; i < key.size(); ++i) {
        key[i] = key[i] ^ (key[key.size() - 1 - i] << 4);
    }
    
    return key;
}

// Utility function to load a key from a file
inline hydra::vfs::EncryptionKey load_key_from_file(const std::string& key_file) {
    hydra::vfs::EncryptionKey key = {};
    
    std::ifstream file(key_file, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Could not open key file");
    }
    
    // Read exactly 32 bytes or as many as available
    file.read(reinterpret_cast<char*>(key.data()), key.size());
    
    // If we didn't read a full key, zero-pad the rest
    if (file.gcount() < static_cast<std::streamsize>(key.size())) {
        std::fill(key.begin() + file.gcount(), key.end(), 0);
    }
    
    return key;
}

} // namespace vfs
} // namespace cli
} // namespace hydra

#endif // HYDRA_CLI_VFS_UTILS_H
