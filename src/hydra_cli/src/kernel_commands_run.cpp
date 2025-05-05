#include "../include/commands/kernel_commands.h"
#include "../include/commands/vfs_utils.h"
#include "../include/config/kernel_config.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <thread>
#include <chrono>
#include <filesystem>
#include <vector>
#include <hydra_vfs/vfs.h>
#include <hydra_vfs/container_vfs.h>
#include <hydra_kernel/kernel.h>
#include <hydra_kernel/process.h>

namespace fs = std::filesystem;

namespace hydra {
namespace cli {
namespace kernel {

int RunCommand::execute(const std::vector<std::string>& args) {
    if (is_help_requested(args)) {
        print_help();
        return 0;
    }

    // Need at least one argument (the config file path)
    if (args.empty()) {
        std::cerr << "Error: Missing config file path" << std::endl;
        print_help();
        return 1;
    }

    std::string config_path = args[0];
    
    // Check if the config file exists
    if (!fs::exists(config_path)) {
        std::cerr << "Error: Config file not found: " << config_path << std::endl;
        return 1;
    }
    
    // Load the kernel configuration
    auto config_opt = config::load_kernel_config(config_path);
    if (!config_opt) {
        std::cerr << "Error: Failed to load kernel configuration from " << config_path << std::endl;
        return 1;
    }
    
    auto& config = config_opt.value();
    
    // Check if container path exists and prompt to create or overwrite
    bool container_exists = fs::exists(config.path);
    if (container_exists) {
        std::cout << "Container already exists: " << config.path << std::endl;
        std::cout << "Do you want to overwrite it? (y/n): ";
        std::string response;
        std::getline(std::cin, response);
        if (response != "y" && response != "Y") {
            std::cout << "Operation cancelled" << std::endl;
            return 0;
        }
    }
    
    // Make sure container parent directory exists
    fs::path container_dir = fs::path(config.path).parent_path();
    if (!container_dir.empty() && !fs::exists(container_dir)) {
        fs::create_directories(container_dir);
    }
    
    // Determine the encryption key
    hydra::vfs::EncryptionKey key;
    if (!config.security.key_file.empty()) {
        // Use key file
        key = vfs::load_key_from_file(config.security.key_file);
    } else if (!config.security.password.empty()) {
        // Use password
        key = vfs::derive_key_from_password(config.security.password);
    } else {
        // Generate a random key
        std::cout << "No password or key file specified. Generating a random key..." << std::endl;
        
        // Initialize random seed
        srand(static_cast<unsigned int>(time(nullptr)));
        
        // Generate random key
        for (size_t i = 0; i < key.size(); i++) {
            key[i] = static_cast<uint8_t>(rand() % 256);
        }
        
        // Save the key to a file next to the container
        fs::path key_path = fs::path(config.path).replace_extension(".key");
        std::ofstream key_file(key_path, std::ios::binary);
        if (!key_file) {
            std::cerr << "Error: Failed to save key file" << std::endl;
            return 1;
        }
        
        // Write the key
        key_file.write(reinterpret_cast<const char*>(key.data()), key.size());
        key_file.close();
        
        std::cout << "Saved key to: " << key_path.string() << std::endl;
        std::cout << "IMPORTANT: Keep this key file secure. You will need it to access the container." << std::endl;
    }
    
    // Set container resource limits
    hydra::vfs::ResourceLimits limits;
    limits.max_storage_size = config::parse_size(config.resources.max_storage);
    limits.max_file_count = config.resources.max_files;
    limits.max_file_size = config::parse_size(config.resources.max_file_size);
    
    // Convert security level
    auto sec_level = config::to_security_level(config.security.security_level);
    
    // Create the container
    std::cout << "Creating container: " << config.path << std::endl;
    auto container_vfs = hydra::vfs::create_container_vfs(
        config.path,
        key,
        nullptr,  // No custom crypto provider
        sec_level,
        limits
    );
    
    if (!container_vfs) {
        std::cerr << "Error: Failed to create container" << std::endl;
        return 1;
    }
    
    // Process mounts if specified
    if (!config.mounts.empty()) {
        std::cout << "Processing mounts..." << std::endl;
        
        for (const auto& mount : config.mounts) {
            std::cout << "Mounting " << mount.source << " to " << mount.target << std::endl;
            
            // Create source directory if it doesn't exist
            if (!fs::exists(mount.source)) {
                fs::create_directories(mount.source);
            }
            
            // Create target directory in container
            auto dir_exists = container_vfs->directory_exists(mount.target);
            if (!dir_exists.success() || !dir_exists.value()) {
                container_vfs->create_directory(mount.target);
            }
            
            // Copy files from source to target
            for (const auto& entry : fs::recursive_directory_iterator(mount.source)) {
                // Calculate relative path
                fs::path rel_path = fs::relative(entry.path(), mount.source);
                std::string target_path = container_vfs->join_paths(mount.target, rel_path.string());
                
                if (entry.is_directory()) {
                    // Create directory in container
                    dir_exists = container_vfs->directory_exists(target_path);
                    if (!dir_exists.success() || !dir_exists.value()) {
                        container_vfs->create_directory(target_path);
                    }
                } else if (entry.is_regular_file()) {
                    // Read source file
                    std::ifstream source_file(entry.path(), std::ios::binary);
                    if (!source_file) {
                        std::cerr << "Error: Failed to open source file: " << entry.path() << std::endl;
                        continue;
                    }
                    
                    // Read file content
                    std::vector<uint8_t> content((std::istreambuf_iterator<char>(source_file)), 
                                               std::istreambuf_iterator<char>());
                    source_file.close();
                    
                    // Create file in container
                    auto create_result = container_vfs->create_file(target_path);
                    if (!create_result.success()) {
                        std::cerr << "Error: Failed to create target file: " << target_path << std::endl;
                        continue;
                    }
                    
                    // Write content to container file
                    auto open_result = container_vfs->open_file(target_path, hydra::vfs::FileMode::WRITE);
                    if (!open_result.success()) {
                        std::cerr << "Error: Failed to open target file for writing: " << target_path << std::endl;
                        continue;
                    }
                    
                    auto file = open_result.value();
                    auto write_result = file->write(content.data(), content.size());
                    if (!write_result.success()) {
                        std::cerr << "Error: Failed to write to target file: " << target_path << std::endl;
                        continue;
                    }
                    
                    file->close();
                    
                    std::cout << "Copied: " << entry.path() << " -> " << target_path << std::endl;
                }
            }
        }
    }
    
    // Create kernel
    std::cout << "Creating kernel..." << std::endl;
    auto kernel = config::create_kernel_from_config(config, container_vfs);
    
    // Start the kernel
    std::cout << "Starting kernel..." << std::endl;
    kernel->start();
    
    // Execute commands for each process
    std::cout << "Launching processes..." << std::endl;
    for (const auto& process_config : config.processes) {
        // Create the specified number of process instances
        for (int i = 0; i < process_config.count; i++) {
            // Append instance number to name if creating multiple instances
            std::string name = process_config.name;
            if (process_config.count > 1) {
                name += "_" + std::to_string(i + 1);
            }
            
            // Find the process by name
            std::shared_ptr<hydra::kernel::Process> process = nullptr;
            auto processes = kernel->listProcesses();
            for (const auto& p : processes) {
                if (p->getName() == name) {
                    process = p;
                    break;
                }
            }
            
            if (!process) {
                std::cerr << "Error: Process not found: " << name << std::endl;
                continue;
            }
            
            // Build command with arguments
            std::string command = process_config.command;
            std::vector<std::string> args = process_config.args;
            
            // Execute command
            std::cout << "Executing command for process " << name << ": " << command;
            for (const auto& arg : args) {
                std::cout << " " << arg;
            }
            std::cout << std::endl;
            
            process->execute(command, args);
        }
    }
    
    // Wait for user input to exit
    std::cout << std::endl;
    std::cout << "Container running. Press Enter to stop." << std::endl;
    std::cin.get();
    
    // Stop the kernel
    std::cout << "Stopping kernel..." << std::endl;
    kernel->stop();
    
    // Display kernel statistics
    auto stats = kernel->getStats();
    std::cout << "Kernel statistics:" << std::endl;
    std::cout << "  Total processes: " << stats.total_processes << std::endl;
    std::cout << "  Total memory usage: " << stats.total_memory_usage << " bytes" << std::endl;
    std::cout << "  Total CPU usage: " << stats.total_cpu_usage * 100.0f << "%" << std::endl;
    std::cout << "  Total threads: " << stats.total_threads << std::endl;
    std::cout << "  Total ports: " << stats.total_ports << std::endl;
    std::cout << "  Uptime: " << stats.uptime / 1000 << " seconds" << std::endl;
    
    // Display terminated processes
    std::cout << "Process exit codes:" << std::endl;
    for (const auto& process : kernel->listProcesses()) {
        if (process->getState() == hydra::kernel::ProcessState::TERMINATED) {
            std::cout << "  " << process->getName() << ": " << process->getExitCode() << std::endl;
        }
    }
    
    std::cout << "Container stopped" << std::endl;
    return 0;
}

void RunCommand::print_help() const {
    std::cout << "Hydra CLI - Kernel Run Command" << std::endl;
    std::cout << "Usage: hydra-cli kernel run <config_file>" << std::endl;
    std::cout << std::endl;
    std::cout << "Description:" << std::endl;
    std::cout << "  Runs a containerized application with the Hydra Kernel" << std::endl;
    std::cout << std::endl;
    std::cout << "Arguments:" << std::endl;
    std::cout << "  config_file    Path to the YAML configuration file" << std::endl;
    std::cout << std::endl;
    std::cout << "Example:" << std::endl;
    std::cout << "  hydra-cli kernel run my_app.yml" << std::endl;
    std::cout << std::endl;
    std::cout << "The configuration file should define:" << std::endl;
    std::cout << "  - Container settings (path, security)" << std::endl;
    std::cout << "  - Isolation mode (none, partial, complete)" << std::endl;
    std::cout << "  - Port mappings" << std::endl;
    std::cout << "  - Processes to run" << std::endl;
    std::cout << "  - Resource limits" << std::endl;
    std::cout << "  - Volume mounts" << std::endl;
    std::cout << std::endl;
    std::cout << "See the documentation for more details on the configuration format." << std::endl;
}

} // namespace kernel
} // namespace cli
} // namespace hydra
