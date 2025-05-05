#ifndef HYDRA_CLI_VFS_MOUNT_UTILS_H
#define HYDRA_CLI_VFS_MOUNT_UTILS_H

#include <string>
#include <filesystem>
#include <vector>
#include <iostream>
#include <hydra_vfs/vfs.h>
#include "../config/container_config.h"

namespace hydra {
namespace cli {
namespace vfs {

/**
 * @brief Process a mount configuration by copying files from the source to the target in the container
 * 
 * @param mount_config The mount configuration to process
 * @param container The container VFS to mount into
 * @return bool True if successful, false otherwise
 */
inline bool process_mount(const hydra::cli::config::MountConfig& mount_config, 
                         std::shared_ptr<hydra::vfs::IVirtualFileSystem> container) {
    namespace fs = std::filesystem;
    
    try {
        // Create the target directory if it doesn't exist
        auto dir_exists = container->directory_exists(mount_config.target);
        if (!dir_exists.success() || !dir_exists.value()) {
            auto mkdir_result = container->create_directory(mount_config.target);
            if (!mkdir_result.success()) {
                std::cerr << "Error: Failed to create target directory: " << mount_config.target << std::endl;
                return false;
            }
        }
        
        // Recursively copy files from source to target
        for (const auto& entry : fs::recursive_directory_iterator(mount_config.source)) {
            // Calculate relative path
            auto rel_path = entry.path().lexically_relative(mount_config.source);
            std::string target_path = container->join_paths(mount_config.target, rel_path.string());
            
            if (entry.is_directory()) {
                // Create directory in container
                auto dir_exists = container->directory_exists(target_path);
                if (!dir_exists.success() || !dir_exists.value()) {
                    auto mkdir_result = container->create_directory(target_path);
                    if (!mkdir_result.success()) {
                        std::cerr << "Error: Failed to create directory: " << target_path << std::endl;
                        continue;
                    }
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
                auto create_result = container->create_file(target_path);
                if (!create_result.success()) {
                    std::cerr << "Error: Failed to create target file: " << target_path << std::endl;
                    continue;
                }
                
                // Write content to container file
                auto open_result = container->open_file(target_path, hydra::vfs::FileMode::WRITE);
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
                
                std::cout << "Imported: " << entry.path() << " -> " << target_path << std::endl;
            }
        }
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error processing mount: " << e.what() << std::endl;
        return false;
    }
}

/**
 * @brief Process all mount configurations from a container configuration
 * 
 * @param config The container configuration containing mount configurations
 * @param container The container VFS to mount into
 * @return int Number of successfully processed mounts
 */
inline int process_mounts(const hydra::cli::config::ContainerConfig& config,
                         std::shared_ptr<hydra::vfs::IVirtualFileSystem> container) {
    int success_count = 0;
    
    for (const auto& mount_config : config.mounts) {
        std::cout << "Processing mount: " << mount_config.source << " -> " << mount_config.target << std::endl;
        
        if (process_mount(mount_config, container)) {
            success_count++;
        }
    }
    
    return success_count;
}

} // namespace vfs
} // namespace cli
} // namespace hydra

#endif // HYDRA_CLI_VFS_MOUNT_UTILS_H
