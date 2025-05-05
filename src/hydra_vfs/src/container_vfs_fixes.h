#pragma once

#include "hydra_vfs/container_vfs.h"
#include "container_path_handler.h"
#include <memory>

namespace hydra {
namespace vfs {

/**
 * @brief Helper methods for implementing proper path handling in ContainerVFS
 */
class ContainerVFSFixes {
public:
    /**
     * @brief Initialize the container with proper path handling
     * 
     * @param vfs The ContainerVFS instance
     * @param path_handler The path handler
     * @return Result<void> Success or error
     */
    static Result<void> initialize_container(ContainerVFS& vfs, 
                                           const std::shared_ptr<ContainerPathHandler>& path_handler);
    
    /**
     * @brief Safely open the container file using the path handler
     * 
     * @param vfs The ContainerVFS instance
     * @param path_handler The path handler
     * @param base_vfs The base VFS to use for file operations
     * @param mode The file mode (read/write)
     * @return Result<std::shared_ptr<IFile>> The opened file or error
     */
    static Result<std::shared_ptr<IFile>> open_container_file(
        const std::shared_ptr<ContainerPathHandler>& path_handler,
        std::shared_ptr<IVirtualFileSystem> base_vfs,
        FileMode mode);
    
    /**
     * @brief Check if the container file exists and is valid
     * 
     * @param path_handler The path handler
     * @param base_vfs The base VFS to use for file operations
     * @return bool True if the container file exists and is valid
     */
    static bool container_file_exists_and_valid(
        const std::shared_ptr<ContainerPathHandler>& path_handler,
        std::shared_ptr<IVirtualFileSystem> base_vfs);
    
    /**
     * @brief Create a new container file with the proper structure
     * 
     * @param path_handler The path handler
     * @param base_vfs The base VFS to use for file operations
     * @return Result<std::shared_ptr<IFile>> The created file or error
     */
    static Result<std::shared_ptr<IFile>> create_new_container_file(
        const std::shared_ptr<ContainerPathHandler>& path_handler,
        std::shared_ptr<IVirtualFileSystem> base_vfs);
    
    /**
     * @brief Normalize a path for VFS operations
     * 
     * @param path The path to normalize
     * @return std::string The normalized path
     */
    static std::string normalize_vfs_path(const std::string& path);
};

} // namespace vfs
} // namespace hydra
