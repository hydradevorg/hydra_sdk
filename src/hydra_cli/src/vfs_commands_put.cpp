#include <iostream>
#include <fstream>
#include <vector>
#include <filesystem>
#include <sstream>

#include "../include/commands/vfs_commands.h"
#include <hydra_vfs/vfs.h>
#include <hydra_vfs/container_vfs.h>

namespace fs = std::filesystem;

namespace hydra {
namespace cli {
namespace vfs {

int PutCommand::execute(const std::vector<std::string>& args) {
    if (is_help_requested(args)) {
        print_help();
        return 0;
    }

    // Need at least two arguments (the container path and source file)
    if (args.size() < 2) {
        std::cerr << "Error: Missing required arguments" << std::endl;
        print_help();
        return 1;
    }

    std::string container_path = args[0];
    std::string source_file = args[1];
    std::string password = get_param_value(args, "--password");
    std::string key_file = get_param_value(args, "--key-file");
    std::string dest_path = get_param_value(args, "--dest");
    bool force = has_flag(args, "--force");
    
    // Validate parameters
    if (password.empty() && key_file.empty()) {
        std::cerr << "Error: Either --password or --key-file must be specified" << std::endl;
        return 1;
    }
    
    // Check if source file exists
    if (!fs::exists(source_file)) {
        std::cerr << "Error: Source file does not exist: " << source_file << std::endl;
        return 1;
    }
    
    // Determine destination path if not specified
    if (dest_path.empty()) {
        dest_path = "/" + fs::path(source_file).filename().string();
    }
    
    // Open the container
    auto vfs = open_container(container_path, password.empty() ? key_file : password, password.empty());
    if (!vfs) {
        return 1;
    }
    
    // Check if destination exists and handle force flag
    auto file_exists_result = vfs->file_exists(dest_path);
    if (file_exists_result.success() && file_exists_result.value() && !force) {
        std::cerr << "Error: Destination file already exists. Use --force to overwrite." << std::endl;
        return 1;
    }
    
    // Create parent directories if needed
    std::string parent_dir = vfs->get_parent_path(dest_path);
    if (!parent_dir.empty() && parent_dir != "/") {
        auto dir_exists_result = vfs->directory_exists(parent_dir);
        if (!dir_exists_result.success() || !dir_exists_result.value()) {
            // Create each directory in the path
            std::string path_to_create = "/";
            std::stringstream ss(parent_dir.substr(1)); // Skip leading slash
            std::string dir;
            
            while (std::getline(ss, dir, '/')) {
                if (!dir.empty()) {
                    path_to_create += dir;
                    
                    auto check_result = vfs->directory_exists(path_to_create);
                    if (!check_result.success() || !check_result.value()) {
                        auto create_result = vfs->create_directory(path_to_create);
                        if (!create_result.success()) {
                            std::cerr << "Error: Failed to create directory: " << path_to_create 
                                      << " - " << static_cast<int>(create_result.error()) << std::endl;
                            return 1;
                        }
                    }
                    
                    path_to_create += "/";
                }
            }
        }
    }
    
    // Read source file
    std::ifstream source(source_file, std::ios::binary);
    if (!source) {
        std::cerr << "Error: Failed to open source file: " << source_file << std::endl;
        return 1;
    }
    
    // Get file size
    source.seekg(0, std::ios::end);
    std::streamsize file_size = source.tellg();
    source.seekg(0, std::ios::beg);
    
    // Allocate buffer
    std::vector<uint8_t> buffer(file_size);
    
    // Read file content
    if (!source.read(reinterpret_cast<char*>(buffer.data()), file_size)) {
        std::cerr << "Error: Failed to read source file: " << source_file << std::endl;
        return 1;
    }
    
    // Write to VFS - first open/create the file
    auto create_result = vfs->create_file(dest_path);
    if (!create_result.success()) {
        std::cerr << "Error: Failed to create file in container: " << static_cast<int>(create_result.error()) << std::endl;
        return 1;
    }
    
    auto open_result = vfs->open_file(dest_path, hydra::vfs::FileMode::WRITE);
    if (!open_result.success()) {
        std::cerr << "Error: Failed to open file for writing: " << static_cast<int>(open_result.error()) << std::endl;
        return 1;
    }
    
    auto file = open_result.value();
    auto write_result = file->write(buffer.data(), buffer.size());
    if (!write_result.success()) {
        std::cerr << "Error: Failed to write to destination: " << static_cast<int>(write_result.error()) << std::endl;
        return 1;
    }
    
    // Close the file
    auto close_result = file->close();
    if (!close_result.success()) {
        std::cerr << "Error: Failed to close file: " << static_cast<int>(close_result.error()) << std::endl;
        return 1;
    }
    
    std::cout << "File added successfully: " << source_file << " -> " << dest_path << std::endl;
    std::cout << "Size: " << buffer.size() << " bytes" << std::endl;
    
    return 0;
}

void PutCommand::print_help() const {
    std::cout << "Hydra CLI - VFS Put Command" << std::endl;
    std::cout << "Usage: hydra-cli vfs put <container_path> <source_file> [--dest <dest_path>] [--password <password>|--key-file <key_file>] [--force]" << std::endl;
    std::cout << std::endl;
    std::cout << "Arguments:" << std::endl;
    std::cout << "  container_path       Path to the VFS container" << std::endl;
    std::cout << "  source_file          Path to the file to add to the container" << std::endl;
    std::cout << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  --dest <dest_path>   Destination path within container (default: filename in root)" << std::endl;
    std::cout << "  --password <pwd>     Use the specified password for decryption" << std::endl;
    std::cout << "  --key-file <file>    Use the decryption key from the specified file" << std::endl;
    std::cout << "  --force              Overwrite destination file if it exists" << std::endl;
}

std::shared_ptr<hydra::vfs::IVirtualFileSystem> PutCommand::open_container(
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
