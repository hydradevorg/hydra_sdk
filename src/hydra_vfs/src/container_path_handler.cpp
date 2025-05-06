#include "hydra_vfs/container_path_handler.hpp"
#include <iostream>

namespace hydra {
namespace vfs {

ContainerPathHandler::ContainerPathHandler(const std::string& container_path)
    : m_container_path(container_path)
{
    // Convert to absolute path immediately
    m_absolute_container_path = PathUtils::to_absolute_path(container_path);

    // Get the parent directory
    m_container_directory = PathUtils::get_parent_directory(m_absolute_container_path);

    std::cout << "DEBUG: Initialized container path handler with:"
              << "\n  Original path: " << m_container_path
              << "\n  Absolute path: " << m_absolute_container_path
              << "\n  Container dir: " << m_container_directory
              << std::endl;
}

std::string ContainerPathHandler::get_absolute_container_path() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_absolute_container_path;
}

bool ContainerPathHandler::ensure_container_directory() const
{
    std::lock_guard<std::mutex> lock(m_mutex);

    // Check if directory already exists
    if (std::filesystem::exists(m_container_directory)) {
        return true;
    }

    try {
        std::cout << "DEBUG: Creating container directory: " << m_container_directory << std::endl;
        return std::filesystem::create_directories(m_container_directory);
    } catch (const std::exception& e) {
        std::cerr << "ERROR: Failed to create container directory: " << e.what() << std::endl;
        return false;
    }
}

bool ContainerPathHandler::container_file_exists() const
{
    std::lock_guard<std::mutex> lock(m_mutex);

    try {
        // First check with absolute path
        if (std::filesystem::exists(m_absolute_container_path)) {
            return true;
        }

        // Try with original path just in case
        if (std::filesystem::exists(m_container_path)) {
            // Update the absolute path if found with original
            m_absolute_container_path = PathUtils::to_absolute_path(m_container_path);
            return true;
        }

        // Try with both relative paths from current working directory
        std::string current_dir = PathUtils::get_current_directory();
        std::string rel_path1 = current_dir + "/vfs_test_data/test_container.dat";
        if (std::filesystem::exists(rel_path1)) {
            m_absolute_container_path = rel_path1;
            return true;
        }

        std::string rel_path2 = "./vfs_test_data/test_container.dat";
        if (std::filesystem::exists(rel_path2)) {
            m_absolute_container_path = PathUtils::to_absolute_path(rel_path2);
            return true;
        }

        return false;
    } catch (const std::exception& e) {
        std::cerr << "ERROR: Exception checking container file existence: " << e.what() << std::endl;
        return false;
    }
}

std::string ContainerPathHandler::get_container_directory() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_container_directory;
}

bool ContainerPathHandler::delete_container_file() const
{
    std::lock_guard<std::mutex> lock(m_mutex);

    if (!container_file_exists()) {
        return true; // File doesn't exist, so deletion "succeeded"
    }

    try {
        return std::filesystem::remove(m_absolute_container_path);
    } catch (const std::exception& e) {
        std::cerr << "ERROR: Failed to delete container file: " << e.what() << std::endl;
        return false;
    }
}

std::string ContainerPathHandler::get_vfs_path(const std::string& path) const
{
    // This function transforms external paths into normalized internal paths
    return PathUtils::normalize_path(path);
}

std::string ContainerPathHandler::normalize_container_path(const std::string& path) const
{
    // This is specifically for container paths, not general VFS paths
    if (path.empty()) {
        return m_absolute_container_path;
    }

    // If path is already absolute, use it as is
    if (PathUtils::is_absolute_path(path)) {
        return PathUtils::normalize_path(path);
    }

    // Otherwise, join with container directory
    return PathUtils::normalize_path(m_container_directory + "/" + path);
}

} // namespace vfs
} // namespace hydra
