#pragma once

#include "hydra_vfs/result.hpp"
#include "hydra_vfs/file_info.hpp"
#include "hydra_vfs/file_mode.hpp"
#include "hydra_vfs/ifile.hpp"
#include <string>
#include <vector>
#include <memory>

namespace hydra {
namespace vfs {

/**
 * @brief Interface for virtual file system operations
 * 
 * This interface defines the operations that can be performed on a virtual file system.
 */
class IVirtualFileSystem {
public:
    /**
     * @brief Virtual destructor
     */
    virtual ~IVirtualFileSystem() = default;
    
    /**
     * @brief Open a file
     * 
     * @param path Path to the file
     * @param mode Mode to open the file in
     * @return Result<std::shared_ptr<IFile>> File handle or an error
     */
    virtual Result<std::shared_ptr<IFile>> open_file(const std::string& path, FileMode mode) = 0;
    
    /**
     * @brief Check if a file exists
     * 
     * @param path Path to the file
     * @return Result<bool> True if the file exists, false otherwise
     */
    virtual Result<bool> file_exists(const std::string& path) = 0;
    
    /**
     * @brief Get information about a file
     * 
     * @param path Path to the file
     * @return Result<FileInfo> File information or an error
     */
    virtual Result<FileInfo> get_file_info(const std::string& path) = 0;
    
    /**
     * @brief Delete a file
     * 
     * @param path Path to the file
     * @return Result<bool> True if successful, false otherwise
     */
    virtual Result<bool> delete_file(const std::string& path) = 0;
    
    /**
     * @brief Create a directory
     * 
     * @param path Path to the directory
     * @return Result<bool> True if successful, false otherwise
     */
    virtual Result<bool> create_directory(const std::string& path) = 0;
    
    /**
     * @brief Check if a directory exists
     * 
     * @param path Path to the directory
     * @return Result<bool> True if the directory exists, false otherwise
     */
    virtual Result<bool> directory_exists(const std::string& path) = 0;
    
    /**
     * @brief Delete a directory
     * 
     * @param path Path to the directory
     * @return Result<bool> True if successful, false otherwise
     */
    virtual Result<bool> delete_directory(const std::string& path) = 0;
    
    /**
     * @brief List files in a directory
     * 
     * @param path Path to the directory
     * @return Result<std::vector<std::string>> List of file names or an error
     */
    virtual Result<std::vector<std::string>> list_files(const std::string& path) = 0;
    
    /**
     * @brief List directories in a directory
     * 
     * @param path Path to the directory
     * @return Result<std::vector<std::string>> List of directory names or an error
     */
    virtual Result<std::vector<std::string>> list_directories(const std::string& path) = 0;
    
    /**
     * @brief Copy a file
     * 
     * @param source_path Path to the source file
     * @param destination_path Path to the destination file
     * @return Result<bool> True if successful, false otherwise
     */
    virtual Result<bool> copy_file(const std::string& source_path, const std::string& destination_path) = 0;
    
    /**
     * @brief Move a file
     * 
     * @param source_path Path to the source file
     * @param destination_path Path to the destination file
     * @return Result<bool> True if successful, false otherwise
     */
    virtual Result<bool> move_file(const std::string& source_path, const std::string& destination_path) = 0;
};

} // namespace vfs
} // namespace hydra
