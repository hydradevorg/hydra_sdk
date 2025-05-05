#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <functional>
#include <memory>
#include <filesystem>
#include <cstring>
#include <iomanip>
#include <fstream>
#include <sstream>

#include "../include/commands/command.h"
#include "../include/commands/vfs_commands.h"
#include "../include/commands/crypto_commands.h"
#include "../include/commands/kernel_commands.h"

namespace fs = std::filesystem;

// Version information
constexpr const char* VERSION = "1.0.0";

// Help text
void print_help() {
    std::cout << "Hydra CLI - Command Line Interface for Hydra SDK" << std::endl;
    std::cout << "Version: " << VERSION << std::endl;
    std::cout << std::endl;
    std::cout << "Usage: hydra-cli [category] [command] [options]" << std::endl;
    std::cout << std::endl;
    std::cout << "Categories:" << std::endl;
    std::cout << "  vfs                 Virtual File System operations" << std::endl;
    std::cout << "  crypto              Cryptography operations" << std::endl;
    std::cout << "  kernel              Container process isolation operations" << std::endl;
    std::cout << std::endl;
    std::cout << "Common Commands:" << std::endl;
    std::cout << "  help                Show this help message" << std::endl;
    std::cout << "  version             Show version information" << std::endl;
    std::cout << std::endl;
    std::cout << "For more information on a specific category, use:" << std::endl;
    std::cout << "  hydra-cli [category] --help" << std::endl;
}

// Version information
void print_version() {
    std::cout << "Hydra CLI v" << VERSION << std::endl;
    std::cout << "Copyright (c) 2025 Hydra SDK Team" << std::endl;
}

int main(int argc, char* argv[]) {
    // Handle no arguments
    if (argc < 2) {
        print_help();
        return 0;
    }

    // Parse first argument (category)
    std::string category = argv[1];
    
    // Handle global commands
    if (category == "help") {
        print_help();
        return 0;
    } else if (category == "version") {
        print_version();
        return 0;
    } else if (category == "--help" || category == "-h") {
        print_help();
        return 0;
    } else if (category == "--version" || category == "-v") {
        print_version();
        return 0;
    }

    // Create command registry
    std::map<std::string, std::unique_ptr<hydra::cli::Command>> vfs_commands;
    std::map<std::string, std::unique_ptr<hydra::cli::Command>> crypto_commands;
    std::map<std::string, std::unique_ptr<hydra::cli::Command>> kernel_commands;
    
    // Register VFS commands
    vfs_commands["container"] = std::make_unique<hydra::cli::vfs::ContainerCommand>();
    vfs_commands["ls"] = std::make_unique<hydra::cli::vfs::ListCommand>();
    vfs_commands["cat"] = std::make_unique<hydra::cli::vfs::CatCommand>();
    vfs_commands["put"] = std::make_unique<hydra::cli::vfs::PutCommand>();
    vfs_commands["get"] = std::make_unique<hydra::cli::vfs::GetCommand>();
    vfs_commands["rm"] = std::make_unique<hydra::cli::vfs::RemoveCommand>();
    vfs_commands["mkdir"] = std::make_unique<hydra::cli::vfs::MkdirCommand>();
    vfs_commands["stats"] = std::make_unique<hydra::cli::vfs::StatsCommand>();
    
    // Register Crypto commands
    crypto_commands["keygen"] = std::make_unique<hydra::cli::crypto::KeygenCommand>();
    crypto_commands["encrypt"] = std::make_unique<hydra::cli::crypto::EncryptCommand>();
    crypto_commands["decrypt"] = std::make_unique<hydra::cli::crypto::DecryptCommand>();
    crypto_commands["sign"] = std::make_unique<hydra::cli::crypto::SignCommand>();
    crypto_commands["verify"] = std::make_unique<hydra::cli::crypto::VerifyCommand>();
    crypto_commands["keyinfo"] = std::make_unique<hydra::cli::crypto::KeyInfoCommand>();
    
    // Register Kernel commands
    kernel_commands["run"] = std::make_unique<hydra::cli::kernel::RunCommand>();
    kernel_commands["list"] = std::make_unique<hydra::cli::kernel::ListCommand>();
    kernel_commands["stop"] = std::make_unique<hydra::cli::kernel::StopCommand>();
    kernel_commands["info"] = std::make_unique<hydra::cli::kernel::InfoCommand>();
    kernel_commands["exec"] = std::make_unique<hydra::cli::kernel::ExecCommand>();
    kernel_commands["attach"] = std::make_unique<hydra::cli::kernel::AttachCommand>();
    kernel_commands["port"] = std::make_unique<hydra::cli::kernel::PortCommand>();
    kernel_commands["resource"] = std::make_unique<hydra::cli::kernel::ResourceCommand>();
    kernel_commands["init"] = std::make_unique<hydra::cli::kernel::InitCommand>();
    
    // Handle category commands
    if (category == "vfs") {
        if (argc < 3 || std::string(argv[2]) == "--help" || std::string(argv[2]) == "-h") {
            std::cout << "Hydra CLI - VFS Commands" << std::endl;
            std::cout << "Usage: hydra-cli vfs [command] [options]" << std::endl;
            std::cout << std::endl;
            std::cout << "Available commands:" << std::endl;
            std::cout << "  container           Create, open, or manage VFS containers" << std::endl;
            std::cout << "  ls                  List files and directories in a container" << std::endl;
            std::cout << "  cat                 Display content of a file from a container" << std::endl;
            std::cout << "  put                 Add a file to a container" << std::endl;
            std::cout << "  get                 Extract a file from a container" << std::endl;
            std::cout << "  rm                  Remove a file or directory from a container" << std::endl;
            std::cout << "  mkdir               Create a directory in a container" << std::endl;
            std::cout << "  stats               Show statistics about a container" << std::endl;
            std::cout << std::endl;
            std::cout << "For more information on a specific command, use:" << std::endl;
            std::cout << "  hydra-cli vfs [command] --help" << std::endl;
            return 0;
        }
        
        std::string command = argv[2];
        if (vfs_commands.find(command) != vfs_commands.end()) {
            // Execute the command with the remaining arguments
            std::vector<std::string> args;
            for (int i = 3; i < argc; i++) {
                args.push_back(argv[i]);
            }
            return vfs_commands[command]->execute(args);
        } else {
            std::cerr << "Error: Unknown VFS command '" << command << "'" << std::endl;
            std::cerr << "Use 'hydra-cli vfs --help' for a list of available commands" << std::endl;
            return 1;
        }
    } else if (category == "crypto") {
        if (argc < 3 || std::string(argv[2]) == "--help" || std::string(argv[2]) == "-h") {
            std::cout << "Hydra CLI - Crypto Commands" << std::endl;
            std::cout << "Usage: hydra-cli crypto [command] [options]" << std::endl;
            std::cout << std::endl;
            std::cout << "Available commands:" << std::endl;
            std::cout << "  keygen              Generate cryptographic keys" << std::endl;
            std::cout << "  encrypt             Encrypt a file" << std::endl;
            std::cout << "  decrypt             Decrypt a file" << std::endl;
            std::cout << "  sign                Create a digital signature" << std::endl;
            std::cout << "  verify              Verify a digital signature" << std::endl;
            std::cout << "  keyinfo             Display information about a key" << std::endl;
            std::cout << std::endl;
            std::cout << "For more information on a specific command, use:" << std::endl;
            std::cout << "  hydra-cli crypto [command] --help" << std::endl;
            return 0;
        }
        
        std::string command = argv[2];
        if (crypto_commands.find(command) != crypto_commands.end()) {
            // Execute the command with the remaining arguments
            std::vector<std::string> args;
            for (int i = 3; i < argc; i++) {
                args.push_back(argv[i]);
            }
            return crypto_commands[command]->execute(args);
        } else {
            std::cerr << "Error: Unknown crypto command '" << command << "'" << std::endl;
            std::cerr << "Use 'hydra-cli crypto --help' for a list of available commands" << std::endl;
            return 1;
        }
    } else if (category == "kernel") {
        if (argc < 3 || std::string(argv[2]) == "--help" || std::string(argv[2]) == "-h") {
            std::cout << "Hydra CLI - Kernel Commands" << std::endl;
            std::cout << "Usage: hydra-cli kernel [command] [options]" << std::endl;
            std::cout << std::endl;
            std::cout << "Available commands:" << std::endl;
            std::cout << "  run                 Run a containerized application with the kernel" << std::endl;
            std::cout << "  list                List all running kernels" << std::endl;
            std::cout << "  stop                Stop a running kernel" << std::endl;
            std::cout << "  info                Display information about a running kernel" << std::endl;
            std::cout << "  exec                Execute a command in a running kernel process" << std::endl;
            std::cout << "  attach              Attach to a running kernel process" << std::endl;
            std::cout << "  port                Manage port forwarding" << std::endl;
            std::cout << "  resource            Manage resource limits" << std::endl;
            std::cout << "  init                Generate a sample configuration file" << std::endl;
            std::cout << std::endl;
            std::cout << "For more information on a specific command, use:" << std::endl;
            std::cout << "  hydra-cli kernel [command] --help" << std::endl;
            return 0;
        }
        
        std::string command = argv[2];
        if (kernel_commands.find(command) != kernel_commands.end()) {
            // Execute the command with the remaining arguments
            std::vector<std::string> args;
            for (int i = 3; i < argc; i++) {
                args.push_back(argv[i]);
            }
            return kernel_commands[command]->execute(args);
        } else {
            std::cerr << "Error: Unknown kernel command '" << command << "'" << std::endl;
            std::cerr << "Use 'hydra-cli kernel --help' for a list of available commands" << std::endl;
            return 1;
        }
    } else {
        std::cerr << "Error: Unknown category '" << category << "'" << std::endl;
        std::cerr << "Use 'hydra-cli help' for usage information" << std::endl;
        return 1;
    }
    
    return 0;
}
