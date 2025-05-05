#include "hydra_vfs/path_utils.hpp"
#include <algorithm>
#include <unistd.h>
#include <limits.h>

namespace hydra {
namespace vfs {

std::string PathUtils::normalize_path(const std::string& path) {
    std::filesystem::path fs_path(path);
    
    // Make sure we have a standardized format
    std::string normalized = fs_path.lexically_normal().string();
    
    // Replace any Windows-style backslashes with forward slashes
    std::replace(normalized.begin(), normalized.end(), '\\', '/');
    
    // Make sure path starts with a slash (for internal VFS paths, not filesystem paths)
    if (!normalized.empty() && normalized[0] != '/' && normalized.find(':') == std::string::npos) {
        normalized = "/" + normalized;
    }
    
    // Make sure path doesn't end with a slash (unless it's the root path)
    if (normalized.length() > 1 && normalized.back() == '/') {
        normalized.pop_back();
    }
    
    return normalized;
}

bool PathUtils::is_absolute_path(const std::string& path) {
    return !path.empty() && 
           (path[0] == '/' || 
            (path.length() > 1 && path[1] == ':'));  // Windows drive letter
}

std::string PathUtils::to_absolute_path(const std::string& path, const std::string& base_dir) {
    if (is_absolute_path(path)) {
        return normalize_path(path);
    }
    
    std::string working_dir = base_dir;
    if (working_dir.empty()) {
        working_dir = get_current_directory();
    }
    
    std::filesystem::path abs_path = std::filesystem::path(working_dir) / path;
    return normalize_path(abs_path.string());
}

std::string PathUtils::get_parent_directory(const std::string& path) {
    std::filesystem::path fs_path(path);
    std::string parent = fs_path.parent_path().string();
    
    // If parent is empty, it's the root directory
    if (parent.empty() && path[0] == '/') {
        return "/";
    }
    
    return normalize_path(parent);
}

std::vector<std::string> PathUtils::split_path(const std::string& path) {
    std::string norm_path = normalize_path(path);
    std::vector<std::string> components;
    std::string::size_type start = 0;
    std::string::size_type end = 0;
    
    // Handle absolute path
    if (norm_path[0] == '/') {
        start = 1;  // Skip the leading slash
    }
    
    // Handle Windows-style path with drive letter
    if (norm_path.length() > 1 && norm_path[1] == ':') {
        components.push_back(norm_path.substr(0, 2)); // Add drive letter
        start = 2;
        
        // Skip slash after drive letter if present
        if (norm_path.length() > 2 && norm_path[2] == '/') {
            start = 3;
        }
    }
    
    // Split the path by slashes
    while ((end = norm_path.find('/', start)) != std::string::npos) {
        std::string component = norm_path.substr(start, end - start);
        if (!component.empty()) {
            components.push_back(component);
        }
        start = end + 1;
    }
    
    // Add the last component
    if (start < norm_path.length()) {
        components.push_back(norm_path.substr(start));
    }
    
    return components;
}

std::string PathUtils::join_path(const std::vector<std::string>& components) {
    if (components.empty()) {
        return "/";
    }
    
    std::string result;
    
    // Check if first component is a Windows drive letter
    if (!components.empty() && components[0].length() == 2 && components[0][1] == ':') {
        result = components[0];
    } else {
        result = "/";
    }
    
    for (const auto& component : components) {
        if (component.empty() || component == ".") {
            continue;
        }
        
        // Skip if this is a drive letter and we already added it
        if (component.length() == 2 && component[1] == ':' && result == component) {
            continue;
        }
        
        if (result.back() != '/') {
            result += "/";
        }
        result += component;
    }
    
    return result;
}

std::string PathUtils::get_current_directory() {
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        return normalize_path(std::string(cwd));
    }
    
    // Fall back to filesystem if getcwd fails
    try {
        return normalize_path(std::filesystem::current_path().string());
    } catch (const std::exception&) {
        // Last resort fallback
        return ".";
    }
}

} // namespace vfs
} // namespace hydra
