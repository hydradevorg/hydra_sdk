#pragma once

#include <string>
#include <filesystem>
#include <vector>

namespace hydra {
namespace vfs {

/**
 * Path utilities for handling filesystem paths consistently
 */
class PathUtils {
public:
    /**
     * Normalize a path by removing redundant separators and resolving relative parts
     * @param path The path to normalize
     * @return The normalized path
     */
    static std::string normalize_path(const std::string& path);
    
    /**
     * Check if a path is absolute
     * @param path The path to check
     * @return True if the path is absolute, false otherwise
     */
    static bool is_absolute_path(const std::string& path);
    
    /**
     * Convert a path to absolute if it's not already
     * @param path The path to convert
     * @param base_dir The base directory to use for relative paths
     * @return The absolute path
     */
    static std::string to_absolute_path(const std::string& path, const std::string& base_dir = "");
    
    /**
     * Get the parent directory of a path
     * @param path The path to get the parent of
     * @return The parent directory
     */
    static std::string get_parent_directory(const std::string& path);
    
    /**
     * Split a path into components
     * @param path The path to split
     * @return Vector of path components
     */
    static std::vector<std::string> split_path(const std::string& path);
    
    /**
     * Join path components into a single path
     * @param components The path components to join
     * @return The joined path
     */
    static std::string join_path(const std::vector<std::string>& components);
    
    /**
     * Get the current working directory
     * @return The current working directory
     */
    static std::string get_current_directory();
};

} // namespace vfs
} // namespace hydra
