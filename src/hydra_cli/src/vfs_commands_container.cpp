#include "../include/commands/vfs_commands.h"
#include "../include/commands/vfs_utils.h"
#include "../include/commands/vfs_mount_utils.h"
#include "../include/config/container_config.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <filesystem>
#include <random>
#include <hydra_vfs/vfs.h>
#include <hydra_vfs/container_vfs.h>
#include <hydra_crypto/kyber_kem.hpp>

namespace fs = std::filesystem;

namespace hydra {
namespace cli {
namespace vfs {

int ContainerCommand::execute(const std::vector<std::string>& args) {
    if (is_help_requested(args)) {
        print_help();
        return 0;
    }

    // Need at least one argument (the subcommand)
    if (args.empty()) {
        std::cerr << "Error: Missing subcommand" << std::endl;
        print_help();
        return 1;
    }

    std::string subcommand = args[0];
    
    if (subcommand == "init") {
        // hydra-cli vfs container init <name> [--output <config_path>]
        if (args.size() < 2) {
            std::cerr << "Error: Missing container name" << std::endl;
            std::cout << "Usage: hydra-cli vfs container init <name> [--output <config_path>]" << std::endl;
            return 1;
        }
        
        std::string container_name = args[1];
        std::string config_path = get_param_value(args, "--output", container_name + ".yml");
        
        // Create a default configuration file
        if (!config::create_default_config(config_path, container_name)) {
            std::cerr << "Error: Failed to create configuration file" << std::endl;
            return 1;
        }
        
        std::cout << "Container configuration created successfully: " << config_path << std::endl;
        std::cout << "You can now edit this file and then run 'hydra-cli vfs container create " << config_path << "'" << std::endl;
        return 0;
    }
    else if (subcommand == "create") {
        // For Docker-like config file approach
        // hydra-cli vfs container create <config_file> [--password <password>]
        if (args.size() < 2) {
            std::cerr << "Error: Missing config file path" << std::endl;
            std::cout << "Usage: hydra-cli vfs container create <config_file> [--password <password>]" << std::endl;
            return 1;
        }
        
        std::string config_path = args[1];
        std::string password = get_param_value(args, "--password");
        
        // Check if using legacy format (container path directly)
        bool is_config_file = config_path.ends_with(".yml") || config_path.ends_with(".yaml");
        
        if (is_config_file) {
            // Load the configuration
            auto config_opt = hydra::cli::config::load_container_config(config_path);
            if (!config_opt) {
                std::cerr << "Error: Failed to load configuration file" << std::endl;
                return 1;
            }
            
            auto& config = config_opt.value();
            
            // Set the password from command line if specified
            if (!password.empty()) {
                config.security.password = password;
            }
            
            // Create container directory if it doesn't exist
            fs::path container_dir = fs::path(config.path).parent_path();
            if (!container_dir.empty() && !fs::exists(container_dir)) {
                fs::create_directories(container_dir);
            }
            
            // Determine encryption key
            hydra::vfs::EncryptionKey key;
            if (!config.security.key_file.empty()) {
                // Use key file from config
                key = load_encryption_key(config.security.key_file, true);
            } else if (!config.security.password.empty()) {
                // Use password from config or command line
                key = load_encryption_key(config.security.password, false);
            } else {
                // Generate a random Kyber key pair
                std::cout << "No password or key file specified. Generating a Kyber key pair..." << std::endl;
                
                try {
                    // Create a Kyber key encapsulation mechanism
                    hydra::crypto::KyberKEM kyber("Kyber768");
                    
                    // Generate a key pair
                    auto [public_key, private_key] = kyber.generate_keypair();
                    
                    std::cout << "Generated Kyber key pair" << std::endl;
                    
                    // Save the key to a file next to the container
                    fs::path key_path = fs::path(config.path).replace_extension(".key");
                    std::ofstream key_file(key_path, std::ios::binary);
                    if (!key_file) {
                        std::cerr << "Error: Failed to save key file" << std::endl;
                        return 1;
                    }
                    
                    // Write the private key
                    key_file.write(reinterpret_cast<const char*>(private_key.data()), 
                                   std::min(private_key.size(), key.size()));
                    key_file.close();
                    
                    std::cout << "Saved private key to: " << key_path.string() << std::endl;
                    std::cout << "IMPORTANT: Keep this key file secure. You will need it to access the container." << std::endl;
                    
                    // Use the private key as the encryption key
                    std::copy_n(private_key.begin(), std::min(private_key.size(), key.size()), key.begin());
                } catch (const std::exception& e) {
                    std::cerr << "Error generating key pair: " << e.what() << std::endl;
                    return 1;
                }
            }
            
            // Convert resource limits from config
            auto limits = config::to_resource_limits(config.resources);
            
            // Convert security level from config
            auto sec_level = config::to_security_level(config.security.security_level);
            
            // Create the container
            auto container = hydra::vfs::create_container_vfs(
                config.path,
                key,
                nullptr,  // No custom crypto provider
                sec_level,
                limits
            );
            
            if (!container) {
                std::cerr << "Error: Failed to create container" << std::endl;
                return 1;
            }
            
            std::cout << "Container created successfully: " << config.path << std::endl;
            
            // Process mounts if any
            if (!config.mounts.empty()) {
                std::cout << "Setting up mounts..." << std::endl;
                
                // Process all mounts using our utility function
                int success_count = process_mounts(config, container);
                
                std::cout << "Mounted " << success_count << " of " << config.mounts.size() 
                          << " directories successfully" << std::endl;
            }
            
            return 0;
        } else {
            // Legacy approach - direct container path
            std::string container_path = config_path;
            std::string key_file = get_param_value(args, "--key-file");
            std::string security_level = get_param_value(args, "--security", "standard");
            
            // Validate parameters
            if (password.empty() && key_file.empty()) {
                // Generate a random password if none is provided
                std::random_device rd;
                std::mt19937 gen(rd());
                std::uniform_int_distribution<> dis(33, 126); // Printable ASCII characters
                
                std::stringstream ss;
                for (int i = 0; i < 16; ++i) {
                    ss << static_cast<char>(dis(gen));
                }
                
                password = ss.str();
                std::cout << "Generated random password: " << password << std::endl;
                std::cout << "IMPORTANT: Store this password securely. You will need it to access the container." << std::endl;
            }
            
            // Map security level string to enum
            hydra::vfs::SecurityLevel sec_level = hydra::vfs::SecurityLevel::STANDARD;
            if (security_level == "standard") {
                sec_level = hydra::vfs::SecurityLevel::STANDARD;
            } else if (security_level == "high") {
                sec_level = hydra::vfs::SecurityLevel::HARDWARE_BACKED;
            } else if (security_level == "maximum") {
                sec_level = hydra::vfs::SecurityLevel::HARDWARE_BACKED;
            } else {
                std::cerr << "Error: Unknown security level '" << security_level << "'" << std::endl;
                return 1;
            }
            
            // Create container directory if it doesn't exist
            fs::path container_dir = fs::path(container_path).parent_path();
            if (!container_dir.empty() && !fs::exists(container_dir)) {
                fs::create_directories(container_dir);
            }
            
            // Load or generate encryption key
            hydra::vfs::EncryptionKey key;
            if (!key_file.empty()) {
                key = load_encryption_key(key_file, true);
            } else {
                key = load_encryption_key(password, false);
            }
            
            // Set resource limits
            hydra::vfs::ResourceLimits limits;
            limits.max_file_size = 100 * 1024 * 1024;     // 100MB
            limits.max_file_count = 1000;
            limits.max_storage_size = 1024 * 1024 * 1024; // 1GB
            
            // Create the container
            auto container = hydra::vfs::create_container_vfs(
                container_path,
                key,
                nullptr,  // No custom crypto provider
                sec_level,
                limits
            );
            
            if (!container) {
                std::cerr << "Error: Failed to create container" << std::endl;
                return 1;
            }
            
            std::cout << "Container created successfully: " << container_path << std::endl;
            std::cout << "NOTE: In the future, you can use the more powerful Docker-like configuration with:" << std::endl;
            std::cout << "      hydra-cli vfs container init <name>" << std::endl;
            std::cout << "      hydra-cli vfs container create <config_file>" << std::endl;
            
            return 0;
        }
    }
    else if (subcommand == "info") {
        // hydra-cli vfs container info <container_path> [--password <password>|--key-file <key_file>]
        if (args.size() < 2) {
            std::cerr << "Error: Missing container path" << std::endl;
            std::cout << "Usage: hydra-cli vfs container info <container_path> [--password <password>|--key-file <key_file>]" << std::endl;
            return 1;
        }
        
        std::string container_path = args[1];
        std::string password = get_param_value(args, "--password");
        std::string key_file = get_param_value(args, "--key-file");
        
        // Validate parameters
        if (password.empty() && key_file.empty()) {
            std::cerr << "Error: Either --password or --key-file must be specified" << std::endl;
            return 1;
        }
        
        // Load encryption key
        hydra::vfs::EncryptionKey key;
        if (!key_file.empty()) {
            key = load_encryption_key(key_file, true);
        } else {
            key = load_encryption_key(password, false);
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
            return 1;
        }
        
        // Display container information
        std::cout << "Container Information" << std::endl;
        std::cout << "--------------------" << std::endl;
        std::cout << "Path: " << container_path << std::endl;
        
        // List root directory
        auto result = container->list_directory("/");
        if (!result.success()) {
            std::cerr << "Error: Failed to list root directory: " << static_cast<int>(result.error()) << std::endl;
            return 1;
        }
        
        auto entries = result.value();
        std::cout << "Files: " << std::count_if(entries.begin(), entries.end(), 
            [](const hydra::vfs::FileInfo& e) { return !e.is_directory; }) << std::endl;
        std::cout << "Directories: " << std::count_if(entries.begin(), entries.end(), 
            [](const hydra::vfs::FileInfo& e) { return e.is_directory; }) << std::endl;
        
        // TODO: Add more container statistics once those APIs are available
        
        return 0;
    } 
    else {
        std::cerr << "Error: Unknown subcommand '" << subcommand << "'" << std::endl;
        print_help();
        return 1;
    }
}

void ContainerCommand::print_help() const {
    std::cout << "Hydra CLI - VFS Container Commands" << std::endl;
    std::cout << "Usage: hydra-cli vfs container <subcommand> [options]" << std::endl;
    std::cout << std::endl;
    std::cout << "Subcommands:" << std::endl;
    std::cout << "  init <name>          Initialize a new Docker-like container configuration file" << std::endl;
    std::cout << "  create <config>      Create a new VFS container from a configuration file" << std::endl;
    std::cout << "  create <path>        Create a new VFS container (legacy mode)" << std::endl;
    std::cout << "  info <path>          Display information about a container" << std::endl;
    std::cout << std::endl;
    std::cout << "Options for 'init':" << std::endl;
    std::cout << "  --output <file>      Output configuration file path (default: <name>.yml)" << std::endl;
    std::cout << std::endl;
    std::cout << "Options for 'create' with configuration:" << std::endl;
    std::cout << "  --password <pwd>     Override the password in the configuration file" << std::endl;
    std::cout << std::endl;
    std::cout << "Options for 'create' (legacy mode):" << std::endl;
    std::cout << "  --password <pwd>     Use the specified password for encryption" << std::endl;
    std::cout << "  --key-file <file>    Use the encryption key from the specified file" << std::endl;
    std::cout << "  --security <level>   Security level: standard, high, maximum" << std::endl;
    std::cout << std::endl;
    std::cout << "Options for 'info':" << std::endl;
    std::cout << "  --password <pwd>     Use the specified password for decryption" << std::endl;
    std::cout << "  --key-file <file>    Use the decryption key from the specified file" << std::endl;
    std::cout << std::endl;
    std::cout << "Docker-like configuration example (in YAML):" << std::endl;
    std::cout << "  version: '1.0'" << std::endl;
    std::cout << "  container:" << std::endl;
    std::cout << "    name: my_secure_container" << std::endl;
    std::cout << "    path: ./secure_data.hcon" << std::endl;
    std::cout << "  security:" << std::endl;
    std::cout << "    encryption: kyber_aes" << std::endl;
    std::cout << "    security_level: standard" << std::endl;
    std::cout << "  resources:" << std::endl;
    std::cout << "    max_storage: 1GB" << std::endl;
    std::cout << "    max_files: 10000" << std::endl;
    std::cout << "    max_file_size: 100MB" << std::endl;
    std::cout << "  mounts:" << std::endl;
    std::cout << "    - source: ./data" << std::endl;
    std::cout << "      target: /imported" << std::endl;
    std::cout << "      read_only: false" << std::endl;
}

hydra::vfs::EncryptionKey ContainerCommand::load_encryption_key(const std::string& key_source, bool is_file) const {
    try {
        if (is_file) {
            // Load key from file using the utility function
            return hydra::cli::vfs::load_key_from_file(key_source);
        } else {
            // Derive key from password using the utility function
            return hydra::cli::vfs::derive_key_from_password(key_source);
        }
    } catch (const std::exception& e) {
        std::cerr << "Error loading encryption key: " << e.what() << std::endl;
        return hydra::vfs::EncryptionKey{}; // Return empty key on error
    }
}

} // namespace vfs
} // namespace cli
} // namespace hydra
