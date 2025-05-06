#pragma once

#include "path_utils.hpp"
#include <string>
#include <filesystem>
#include <memory>
#include <mutex>

namespace hydra {
namespace vfs {

/**
 * @brief Helper class for handling container file paths consistently
 */
class ContainerPathHandler {
public:
    /**
     * @brief Construct a new Container Path Handler object
     *
     * @param container_path Path to the container file
     */
    ContainerPathHandler(const std::string& container_path);

    /**
     * @brief Get the absolute path to the container file
     *
     * @return std::string The absolute container path
     */
    std::string get_absolute_container_path() const;

    /**
     * @brief Ensure that the container directory exists
     *
     * @return bool True if the directory exists or was created successfully
     */
    bool ensure_container_directory() const;

    /**
     * @brief Check if the container file exists
     *
     * @return bool True if the file exists
     */
    bool container_file_exists() const;

    /**
     * @brief Get the parent directory of the container file
     *
     * @return std::string The parent directory path
     */
    std::string get_container_directory() const;

    /**
     * @brief Delete the container file if it exists
     *
     * @return bool True if the file was deleted or didn't exist
     */
    bool delete_container_file() const;

    /**
     * @brief Get a container-relative path for VFS operations
     *
     * @param path The original path
     * @return std::string The container-relative path
     */
    std::string get_vfs_path(const std::string& path) const;

    /**
     * @brief Normalize a path for container operations
     *
     * @param path The path to normalize
     * @return std::string The normalized path
     */
    std::string normalize_container_path(const std::string& path) const;

private:
    std::string m_container_path;
    mutable std::string m_absolute_container_path;
    std::string m_container_directory;
    mutable std::mutex m_mutex;
};

} // namespace vfs
} // namespace hydra
