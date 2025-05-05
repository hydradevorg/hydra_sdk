#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>
#include <iomanip>

#include <hydra_vfs/vfs.h>
#include <hydra_vfs/container_vfs.h>
#include "../include/commands/vfs_commands.h"
#include "../include/commands/vfs_utils.h"

namespace hydra {
namespace cli {
namespace vfs {

int CatCommand::execute(const std::vector<std::string>& args) {
    if (is_help_requested(args)) {
        print_help();
        return 0;
    }
    
    // Need at least 2 arguments (container path and file path)
    if (args.size() < 2) {
        std::cerr << "Error: Missing required arguments" << std::endl;
        print_help();
        return 1;
    }
    
    // Get parameters from command line
    std::string key_file = get_param_value(args, "--key-file", "");
    std::string password = get_param_value(args, "--password", "");
    
    // The last two arguments should be container_path and file_path
    size_t argCount = args.size();
    std::string container_path = args[argCount - 2];
    std::string file_path = args[argCount - 1];
    
    // Check if we have a key or password
    if (key_file.empty() && password.empty()) {
        std::cerr << "Error: Either --key-file or --password must be provided" << std::endl;
        print_help();
        return 1;
    }
    
    std::cout << "Opening VFS container: " << container_path << std::endl;
    
    // Open the container using the provided credentials
    std::shared_ptr<hydra::vfs::IVirtualFileSystem> vfs;
    
    try {
        // Create encryption key from the key file or password
        hydra::vfs::EncryptionKey key;
        
        if (!key_file.empty()) {
            // Load key from file
            try {
                key = load_key_from_file(key_file);
            } catch (const std::exception& e) {
                std::cerr << "Error loading key file: " << e.what() << std::endl;
                return 1;
            }
        } else {
            // Derive key from password
            key = derive_key_from_password(password);
        }
        
        // Create a container VFS with the key
        vfs = hydra::vfs::create_container_vfs(container_path, key);
    } catch (const std::exception& e) {
        std::cerr << "Error opening container: " << e.what() << std::endl;
        return 1;
    }
    
    if (!vfs) {
        std::cerr << "Failed to open VFS container" << std::endl;
        return 1;
    }
    
    // Read the file from the VFS
    try {
        auto file_result = vfs->open_file(file_path, hydra::vfs::FileMode::READ);
        
        if (!file_result.success()) {
            std::cerr << "Error opening file: " << file_path << std::endl;
            return 1;
        }
        
        auto file = file_result.value();
        
        // Read the file in chunks
        constexpr size_t BUFFER_SIZE = 4096;
        std::vector<uint8_t> buffer(BUFFER_SIZE);
        
        while (true) {
            auto read_result = file->read(buffer.data(), buffer.size());
            if (!read_result.success()) {
                std::cerr << "Error reading file: " << file_path << std::endl;
                return 1;
            }
            
            size_t bytes_read = read_result.value();
            if (bytes_read == 0) {
                break; // End of file
            }
            
            // Output the file contents to stdout
            std::cout.write(reinterpret_cast<const char*>(buffer.data()), bytes_read);
        }
        
        // Add a newline if the output doesn't end with one
        if (!buffer.empty() && buffer.back() != '\n') {
            std::cout << std::endl;
        }
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error reading file: " << e.what() << std::endl;
        return 1;
    }
}

void CatCommand::print_help() const {
    std::cout << "Usage: hydra vfs cat [OPTIONS] CONTAINER_PATH FILE_PATH" << std::endl;
    std::cout << "Display the contents of a file in a container" << std::endl;
    std::cout << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  --key-file PATH       Use key file instead of password" << std::endl;
    std::cout << "  --password PASS       Use password for container" << std::endl;
    std::cout << "  -h, --help            Show this help message" << std::endl;
}

} // namespace vfs
} // namespace cli
} // namespace hydra
