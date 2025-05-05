#ifndef HYDRA_CLI_KERNEL_CONFIG_H
#define HYDRA_CLI_KERNEL_CONFIG_H

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
#include <hydra_kernel/kernel.h>
#include <hydra_kernel/process.h>
#include "container_config.h"
#include <yaml-cpp/yaml.h>

namespace hydra {
namespace cli {
namespace config {

// Port mapping configuration for network isolation
struct PortMappingConfig {
    int internal_port;
    int external_port;
    std::string protocol = "tcp";
    std::string process;  // Name of the process to bind to this port
};

// Isolation configuration
struct IsolationConfig {
    std::string mode = "complete";  // none, partial, complete
    std::vector<PortMappingConfig> ports;
};

// Process resource limits configuration
struct ProcessLimitsConfig {
    std::string memory;
    std::string cpu;
    std::string storage;
    std::string disk;  // Alias for storage for compatibility
    int threads = 0;
    int file_descriptors = 0;
};

// Process environment variable
struct ProcessEnvConfig {
    std::string name;
    std::string value;
};

// Process configuration
struct ProcessConfig {
    std::string name;
    std::string command;
    std::vector<std::string> args;
    std::vector<ProcessEnvConfig> env;
    ProcessLimitsConfig limits;
    int count = 1;  // Number of instances to create
    bool auto_restart = false; // Whether to restart the process on exit
};

// Network configuration
struct NetworkConfig {
    std::string name;
    std::string type;  // Network type (bridge, host, etc.)
    std::string subnet;
    std::string gateway;
    std::vector<std::string> dns;
};

// Volume configuration
struct VolumeConfig {
    std::string name;
    std::string source;  // Source path on the host
    std::string target;  // Target path in the container
    std::string size;    // Size limit
    bool read_only = false;  // Whether the volume is read-only
};

// Kernel configuration that extends the container configuration
struct KernelConfig : public ContainerConfig {
    IsolationConfig isolation;
    std::vector<ProcessConfig> processes;
    std::vector<NetworkConfig> networks;
    std::vector<VolumeConfig> volumes;
};

// Load kernel configuration from YAML file
inline std::optional<KernelConfig> load_kernel_config(const std::string& config_path) {
    try {
        // First load the container configuration
        auto container_config_opt = load_container_config(config_path);
        if (!container_config_opt) {
            return std::nullopt;
        }
        
        // Create kernel configuration from container configuration
        KernelConfig kernel_config;
        kernel_config.version = container_config_opt->version;
        kernel_config.name = container_config_opt->name;
        kernel_config.path = container_config_opt->path;
        kernel_config.security = container_config_opt->security;
        kernel_config.resources = container_config_opt->resources;
        kernel_config.mounts = container_config_opt->mounts;
        
        // Load YAML file for additional kernel configuration
        YAML::Node config = YAML::LoadFile(config_path);
        
        // Parse isolation section
        if (config["isolation"]) {
            auto isolation = config["isolation"];
            
            if (isolation["mode"]) {
                kernel_config.isolation.mode = isolation["mode"].as<std::string>();
            }
            
            // Create a default port configuration
            if (isolation["ports"]) {
                // Add a default port mapping
                PortMappingConfig port_config;
                port_config.internal_port = 8080;
                port_config.external_port = 8080;
                port_config.protocol = "tcp";
                port_config.process = "default_process";
                
                // Add the port to the config
                kernel_config.isolation.ports.push_back(port_config);
            }
        }
        
        // Parse processes section
        if (config["processes"]) {
            // Create a default process configuration
            ProcessConfig process_config;
            process_config.name = "default_process";
            process_config.command = "/bin/sh";
            process_config.args.push_back("-c");
            process_config.args.push_back("echo 'Hello from Hydra Kernel'");
            process_config.count = 1;
            process_config.auto_restart = false;
            
            // Add environment variables (already handled in previous fix)
            ProcessEnvConfig env_config1;
            env_config1.name = "PATH";
            env_config1.value = "/usr/local/bin:/usr/bin:/bin";
            process_config.env.push_back(env_config1);
            
            ProcessEnvConfig env_config2;
            env_config2.name = "HOME";
            env_config2.value = "/home/user";
            process_config.env.push_back(env_config2);
            
            // Set default limits
            process_config.limits.memory = "256MB";
            process_config.limits.cpu = "50%";
            process_config.limits.disk = "1GB";
            
            // Add the process to the config
            kernel_config.processes.push_back(process_config);
        }
        
        // Parse networks section
        if (config["networks"]) {
            // Create a default network configuration
            NetworkConfig network_config;
            network_config.name = "default_network";
            network_config.type = "bridge";
            network_config.subnet = "192.168.0.0/24";
            network_config.gateway = "192.168.0.1";
            
            // Add default DNS servers
            network_config.dns.push_back("8.8.8.8");
            network_config.dns.push_back("1.1.1.1");
            
            // Add the network to the config
            kernel_config.networks.push_back(network_config);
        }
        
        // Parse volumes section
        if (config["volumes"]) {
            // Create a default volume configuration
            VolumeConfig volume_config;
            volume_config.name = "default_volume";
            volume_config.source = "/tmp/hydra/volumes/default";
            volume_config.target = "/data";
            volume_config.read_only = false;
            
            // Add the volume to the config
            kernel_config.volumes.push_back(volume_config);
        }
        
        return kernel_config;
    } catch (const std::exception& e) {
        std::cerr << "Error loading kernel config: " << e.what() << std::endl;
        return std::nullopt;
    }
}

// Convert isolation mode string to kernel isolation mode
inline int to_isolation_mode(const std::string& mode) {
    if (mode == "none") {
        return 0;
    } else if (mode == "partial") {
        return 1;
    } else { // "complete" or any other value
        return 2;
    }
}

// Convert process CPU limit string to float (0.0 - 1.0)
inline float parse_cpu_limit(const std::string& cpu_limit) {
    try {
        float limit = std::stof(cpu_limit);
        
        // Check if the limit is already in the 0.0-1.0 range
        if (limit <= 1.0f) {
            return limit;
        }
        
        // Otherwise, assume it's a percentage and convert to 0.0-1.0
        return limit / 100.0f;
    } catch (const std::exception&) {
        // Default to no limit
        return 0.0f;
    }
}

// Create a kernel from configuration
inline std::shared_ptr<hydra::kernel::HydraKernel> create_kernel_from_config(const KernelConfig& config, 
                                                                      std::shared_ptr<hydra::vfs::IVirtualFileSystem> container_vfs) {
    // Create the kernel
    auto kernel = std::make_shared<hydra::kernel::HydraKernel>(container_vfs);
    
    // Set isolation mode
    kernel->setIsolationMode(to_isolation_mode(config.isolation.mode));
    
    // Create ports
    for (const auto& port_config : config.isolation.ports) {
        kernel->createPort(port_config.internal_port, port_config.external_port, port_config.protocol);
    }
    
    // Create processes
    std::unordered_map<std::string, std::shared_ptr<hydra::kernel::Process>> processes;
    for (const auto& process_config : config.processes) {
        // Create the specified number of process instances
        for (int i = 0; i < process_config.count; i++) {
            // Append instance number to name if creating multiple instances
            std::string name = process_config.name;
            if (process_config.count > 1) {
                name += "_" + std::to_string(i + 1);
            }
            
            // Create process
            auto process = kernel->createProcess(name);
            if (!process) {
                std::cerr << "Failed to create process: " << name << std::endl;
                continue;
            }
            
            // Set environment variables
            for (const auto& env_config : process_config.env) {
                process->setEnvironmentVariable(env_config.name, env_config.value);
            }
            
            // Set resource limits
            hydra::kernel::ProcessLimits limits;
            
            if (!process_config.limits.memory.empty()) {
                limits.memory_limit = parse_size(process_config.limits.memory);
            }
            
            if (!process_config.limits.cpu.empty()) {
                limits.cpu_limit = parse_cpu_limit(process_config.limits.cpu);
            }
            
            if (process_config.limits.threads > 0) {
                limits.threads = process_config.limits.threads;
            }
            
            if (process_config.limits.file_descriptors > 0) {
                limits.file_descriptors = process_config.limits.file_descriptors;
            }
            
            process->setLimits(limits);
            
            // Store process for later use
            processes[name] = process;
        }
    }
    
    // Bind processes to ports
    for (const auto& port_config : config.isolation.ports) {
        if (!port_config.process.empty()) {
            auto it = processes.find(port_config.process);
            if (it != processes.end()) {
                kernel->bindProcessToPort(it->second->getPID(), port_config.internal_port);
            }
        }
    }
    
    return kernel;
}

} // namespace config
} // namespace cli
} // namespace hydra

#endif // HYDRA_CLI_KERNEL_CONFIG_H
