#include "../include/commands/vfs_commands.h"
#include <iostream>
#include <iomanip>
#include <filesystem>
#include <hydra_vfs/vfs.h>
#include <hydra_vfs/container_vfs.h>

namespace fs = std::filesystem;

namespace hydra {
namespace cli {
namespace vfs {

int ListCommand::execute(const std::vector<std::string>& args) {
    if (is_help_requested(args)) {
        print_help();
        return 0;
    }

    // Need at least one argument (the container path)
    if (args.size() < 1) {
        std::cerr << "Error: Missing container path" << std::endl;
        print_help();
        return 1;
    }

    std::string container_path = args[0];
    std::string password = get_param_value(args, "--password");
    std::string key_file = get_param_value(args, "--key-file");
    std::string dir_path = get_param_value(args, "--path", "/");
    bool show_hidden = has_flag(args, "--all");
    bool long_format = has_flag(args, "--long");
    
    // Validate parameters
    if (password.empty() && key_file.empty()) {
        std::cerr << "Error: Either --password or --key-file must be specified" << std::endl;
        return 1;
    }
    
    // Open the container
    auto vfs = open_container(container_path, password.empty() ? key_file : password, !password.empty());
    if (!vfs) {
        return 1;
    }
    
    // List the directory
    auto result = vfs->list_directory(dir_path);
    if (!result.success()) {
        std::cerr << "Error: Failed to list directory: " << static_cast<int>(result.error()) << std::endl;
        return 1;
    }
    
    auto entries = result.value();
    
    // Filter hidden files if needed
    if (!show_hidden) {
        auto it = std::remove_if(entries.begin(), entries.end(), 
            [](const hydra::vfs::FileInfo& e) { 
                return !e.name.empty() && e.name[0] == '.'; 
            });
        entries.erase(it, entries.end());
    }
    
    // Sort entries - directories first, then files
    std::sort(entries.begin(), entries.end(), 
        [](const hydra::vfs::FileInfo& a, const hydra::vfs::FileInfo& b) {
            if (a.is_directory && !b.is_directory) return true;
            if (!a.is_directory && b.is_directory) return false;
            return a.name < b.name;
        });
    
    // Print header for long format
    if (long_format) {
        std::cout << std::left << std::setw(10) << "Type" 
                  << std::left << std::setw(12) << "Size" 
                  << std::left << std::setw(20) << "Modified" 
                  << "Name" << std::endl;
        std::cout << std::string(80, '-') << std::endl;
    }
    
    // Print entries
    for (const auto& entry : entries) {
        if (long_format) {
            // Type
            std::cout << std::left << std::setw(10) 
                      << (entry.is_directory ? "Directory" : "File");
            
            // Size
            std::cout << std::left << std::setw(12);
            if (entry.is_directory) {
                std::cout << "-";
            } else {
                std::cout << entry.size;
            }
            
            // Modified time - this is a simplified version as our API might not provide this
            std::cout << std::left << std::setw(20) << "-";
            
            // Name
            std::cout << entry.name << std::endl;
        } else {
            // Simple format
            if (entry.is_directory) {
                std::cout << entry.name << "/" << std::endl;
            } else {
                std::cout << entry.name << std::endl;
            }
        }
    }
    
    return 0;
}

void ListCommand::print_help() const {
    std::cout << "Hydra CLI - VFS List Command" << std::endl;
    std::cout << "Usage: hydra-cli vfs ls <container_path> [--path <dir_path>] [--password <password>|--key-file <key_file>] [--all] [--long]" << std::endl;
    std::cout << std::endl;
    std::cout << "Arguments:" << std::endl;
    std::cout << "  container_path       Path to the VFS container" << std::endl;
    std::cout << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  --path <dir_path>    Directory path within the container (default: '/')" << std::endl;
    std::cout << "  --password <pwd>     Use the specified password for decryption" << std::endl;
    std::cout << "  --key-file <file>    Use the decryption key from the specified file" << std::endl;
    std::cout << "  --all                Show hidden files (starting with '.')" << std::endl;
    std::cout << "  --long               Use long listing format" << std::endl;
}

std::shared_ptr<hydra::vfs::IVirtualFileSystem> ListCommand::open_container(
    const std::string& container_path,
    const std::string& key_source,
    bool key_is_file
) const {
    // Load encryption key
    hydra::vfs::EncryptionKey key = {};
    
    if (key_is_file) {
        // Load key from file
        std::ifstream key_file(key_source, std::ios::binary);
        if (!key_file) {
            std::cerr << "Error: Cannot open key file: " << key_source << std::endl;
            return nullptr;
        }
        
        key_file.read(reinterpret_cast<char*>(key.data()), key.size());
        if (key_file.gcount() != static_cast<std::streamsize>(key.size())) {
            std::cerr << "Warning: Key file size mismatch. Expected " << key.size() 
                      << " bytes, got " << key_file.gcount() << " bytes." << std::endl;
        }
    } else {
        // Derive key from password
        // In a real application, use a proper key derivation function like PBKDF2
        // This is a simplified implementation for demonstration
        
        // First zero the key
        for (size_t i = 0; i < key.size(); ++i) {
            key[i] = 0;
        }
        
        // Simple key derivation (NOT secure for production use)
        for (size_t i = 0; i < key_source.length(); ++i) {
            key[i % key.size()] ^= key_source[i];
        }
        
        // Additional mixing
        for (size_t i = 0; i < key.size() - 1; ++i) {
            key[i + 1] ^= key[i];
        }
    }
    
    // Open the container
    auto container = hydra::vfs::create_container_vfs(
        container_path,
        key,
        nullptr,  // No custom crypto provider
        hydra::vfs::SecurityLevel::STANDARD,
        {}  // Default limits
    );
    
    if (!container) {
        std::cerr << "Error: Failed to open container. Check the password/key and container path." << std::endl;
        return nullptr;
    }
    
    return container;
}

} // namespace vfs
} // namespace cli
} // namespace hydra
