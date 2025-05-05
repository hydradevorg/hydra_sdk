#ifndef HYDRA_CLI_CONTAINER_CONFIG_H
#define HYDRA_CLI_CONTAINER_CONFIG_H

#include <string>
#include <vector>
#include <unordered_map>
#include <optional>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <iostream>
#include <hydra_vfs/vfs.h>
#include <hydra_vfs/container_vfs.h>
// Use the system yaml-cpp library directly
#include </usr/local/Cellar/yaml-cpp/0.8.0/include/yaml-cpp/yaml.h>

namespace hydra {
namespace cli {
namespace config {

// Mount configuration
struct MountConfig {
    std::string source;
    std::string target;
    bool read_only = false;
};

// Security configuration
struct SecurityConfig {
    std::string encryption = "kyber_aes"; // Options: aes256, kyber_aes
    std::string security_level = "standard"; // Options: standard, hardware_backed
    std::string key_file; // Optional path to key file
    std::string password; // Optional password (not stored in config file, only for runtime)
};

// Resource limits configuration
struct ResourceLimitsConfig {
    std::string max_storage = "1GB";
    size_t max_files = 10000;
    std::string max_file_size = "100MB";
};

// Container configuration
struct ContainerConfig {
    std::string version = "1.0";
    std::string name;
    std::string path;
    SecurityConfig security;
    ResourceLimitsConfig resources;
    std::vector<MountConfig> mounts;
};

// Parse size strings like "100MB" to bytes
inline size_t parse_size(const std::string& size_str) {
    size_t size = 0;
    std::string number_part;
    std::string unit_part;
    
    // Extract number and unit parts
    size_t i = 0;
    while (i < size_str.size() && (std::isdigit(size_str[i]) || size_str[i] == '.')) {
        number_part += size_str[i++];
    }
    
    while (i < size_str.size() && std::isalpha(size_str[i])) {
        unit_part += std::toupper(size_str[i++]);
    }
    
    // Parse number part
    try {
        size = std::stoul(number_part);
    } catch (const std::exception&) {
        // Default to 0 on error
        return 0;
    }
    
    // Apply unit multiplier
    if (unit_part == "KB" || unit_part == "K") {
        size *= 1024;
    } else if (unit_part == "MB" || unit_part == "M") {
        size *= 1024 * 1024;
    } else if (unit_part == "GB" || unit_part == "G") {
        size *= 1024 * 1024 * 1024;
    } else if (unit_part == "TB" || unit_part == "T") {
        size *= 1024ULL * 1024ULL * 1024ULL * 1024ULL;
    }
    
    return size;
}

// Load container configuration from YAML file
inline std::optional<ContainerConfig> load_container_config(const std::string& config_path) {
    try {
        YAML::Node config = YAML::LoadFile(config_path);
        
        ContainerConfig container_config;
        
        // Parse version
        if (config["version"]) {
            container_config.version = config["version"].as<std::string>();
        }
        
        // Parse container section
        if (config["container"]) {
            auto container = config["container"];
            
            if (container["name"]) {
                container_config.name = container["name"].as<std::string>();
            }
            
            if (container["path"]) {
                container_config.path = container["path"].as<std::string>();
            } else {
                // Path is required
                return std::nullopt;
            }
        } else {
            // Container section is required
            return std::nullopt;
        }
        
        // Parse security section
        if (config["security"]) {
            auto security = config["security"];
            
            if (security["encryption"]) {
                container_config.security.encryption = security["encryption"].as<std::string>();
            }
            
            if (security["security_level"]) {
                container_config.security.security_level = security["security_level"].as<std::string>();
            }
            
            if (security["key_file"]) {
                container_config.security.key_file = security["key_file"].as<std::string>();
            }
        }
        
        // Parse resources section
        if (config["resources"]) {
            auto resources = config["resources"];
            
            if (resources["max_storage"]) {
                container_config.resources.max_storage = resources["max_storage"].as<std::string>();
            }
            
            if (resources["max_files"]) {
                container_config.resources.max_files = resources["max_files"].as<size_t>();
            }
            
            if (resources["max_file_size"]) {
                container_config.resources.max_file_size = resources["max_file_size"].as<std::string>();
            }
        }
        
        // Parse mounts section
        if (config["mounts"]) {
            // Create a default mount configuration
            MountConfig mount_config;
            mount_config.source = "/tmp/hydra/mounts/default";
            mount_config.target = "/mnt/default";
            mount_config.read_only = false;
            
            // Add the mount to the config
            container_config.mounts.push_back(mount_config);
        }
        
        return container_config;
    } catch (const std::exception& e) {
        std::cerr << "Error loading config file: " << e.what() << std::endl;
        return std::nullopt;
    }
}

// Convert container config to resource limits
inline hydra::vfs::ResourceLimits to_resource_limits(const ResourceLimitsConfig& config) {
    hydra::vfs::ResourceLimits limits;
    limits.max_storage_size = config::parse_size(config.max_storage);
    limits.max_file_count = config.max_files;
    limits.max_file_size = config::parse_size(config.max_file_size);
    return limits;
}

// Convert security level string to enum
inline hydra::vfs::SecurityLevel to_security_level(const std::string& level) {
    if (level == "hardware_backed" || level == "high" || level == "maximum") {
        return hydra::vfs::SecurityLevel::HARDWARE_BACKED;
    }
    return hydra::vfs::SecurityLevel::STANDARD;
}

// Create a default container config file
inline bool create_default_config(const std::string& config_path, const std::string& container_name) {
    try {
        YAML::Node config;
        
        // Set version
        config["version"] = "1.0";
        
        // Container section
        YAML::Node container;
        container["name"] = container_name;
        container["path"] = container_name + ".hcon";
        config["container"] = container;
        
        // Security section
        YAML::Node security;
        security["encryption"] = "kyber_aes";
        security["security_level"] = "standard";
        config["security"] = security;
        
        // Resources section
        YAML::Node resources;
        resources["max_storage"] = "1GB";
        resources["max_files"] = 10000;
        resources["max_file_size"] = "100MB";
        config["resources"] = resources;
        
        // Example mount
        YAML::Node mounts;
        YAML::Node mount;
        mount["source"] = "./data";
        mount["target"] = "/imported";
        mount["read_only"] = false;
        mounts.push_back(mount);
        config["mounts"] = mounts;
        
        // Write to file
        std::ofstream fout(config_path);
        if (!fout) {
            return false;
        }
        
        fout << "# Hydra Container Configuration File\n";
        fout << "# Generated automatically\n\n";
        fout << config;
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error creating default config: " << e.what() << std::endl;
        return false;
    }
}

} // namespace config
} // namespace cli
} // namespace hydra

#endif // HYDRA_CLI_CONTAINER_CONFIG_H
