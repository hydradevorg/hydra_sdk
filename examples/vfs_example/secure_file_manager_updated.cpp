#include <iostream>
#include <string>
#include <memory>
#include <vector>
#include <map>
#include <filesystem>
#include <fstream>
#include <algorithm>
#include <hydra_vfs/vfs.h>
#include <hydra_vfs/memory_vfs.h>
#include <hydra_vfs/persistent_vfs.h>
#include <hydra_vfs/container_vfs.h>
#include <hydra_vfs/encrypted_vfs.h>
#include <hydra_crypto/kyber_aes.hpp>

namespace fs = std::filesystem;

// Simple menu-driven secure file manager application
class SecureFileManager {
private:
    std::shared_ptr<hydra::vfs::IVirtualFileSystem> vfs;
    std::string current_path = "/";
    bool running = true;
    std::string container_path;
    hydra::vfs::EncryptionKey encryption_key;

public:
    // Initialize the secure file manager with a container VFS
    bool initialize(const std::string& container_path, const std::string& password) {
        this->container_path = container_path;

        // Create container directory if it doesn't exist
        fs::path dir_path = fs::path(container_path).parent_path();
        if (!dir_path.empty() && !fs::exists(dir_path)) {
            fs::create_directories(dir_path);
        }

        // Derive encryption key from password
        derive_key_from_password(password);

        // Set resource limits
        hydra::vfs::ResourceLimits limits;
        limits.max_file_size = 50 * 1024 * 1024;     // 50MB
        limits.max_file_count = 1000;
        limits.max_storage_size = 500 * 1024 * 1024; // 500MB

        // Create or open the container VFS with Kyber AES encryption
        vfs = hydra::vfs::create_container_vfs(
            container_path,
            encryption_key,
            nullptr,
            hydra::vfs::SecurityLevel::STANDARD,
            limits
        );

        if (!vfs) {
            std::cout << "Failed to create or open container VFS" << std::endl;
            return false;
        }

        // Create root directory if it doesn't exist
        auto dir_exists_result = vfs->directory_exists("/");
        if (dir_exists_result.success() && !dir_exists_result.value()) {
            auto result = vfs->create_directory("/");
            if (!result.success()) {
                std::cout << "Failed to create root directory: Error code " << static_cast<int>(result.error()) << std::endl;
                return false;
            }
        }

        return true;
    }

    // Derive encryption key from password
    void derive_key_from_password(const std::string& password) {
        // Simple key derivation for example purposes
        // In a real application, use a proper key derivation function like PBKDF2
        for (size_t i = 0; i < password.size() && i < encryption_key.size(); ++i) {
            encryption_key[i % encryption_key.size()] ^= password[i];
        }
        
        // Ensure the key has some entropy
        for (size_t i = 0; i < encryption_key.size(); ++i) {
            if (encryption_key[i] == 0) {
                encryption_key[i] = 0xAA; // Arbitrary non-zero value
            }
        }
    }

    // Run the file manager
    void run() {
        std::cout << "\nSecure File Manager" << std::endl;
        std::cout << "Container: " << container_path << std::endl;

        while (running) {
            display_prompt();
            std::string command;
            std::getline(std::cin, command);
            process_command(command);
        }
    }

private:
    // Display command prompt
    void display_prompt() {
        std::cout << "\n" << current_path << "> ";
    }

    // Process user command
    void process_command(const std::string& command_line) {
        if (command_line.empty()) {
            return;
        }

        std::vector<std::string> tokens = tokenize(command_line);
        std::string command = tokens[0];
        
        std::map<std::string, std::function<void(const std::vector<std::string>&)>> commands = {
            {"help", [this](const auto& args) { cmd_help(); }},
            {"ls", [this](const auto& args) { cmd_list(args); }},
            {"cd", [this](const auto& args) { cmd_change_dir(args); }},
            {"mkdir", [this](const auto& args) { cmd_make_dir(args); }},
            {"rm", [this](const auto& args) { cmd_remove(args); }},
            {"cat", [this](const auto& args) { cmd_cat(args); }},
            {"put", [this](const auto& args) { cmd_put(args); }},
            {"get", [this](const auto& args) { cmd_get(args); }},
            {"info", [this](const auto& args) { cmd_info(args); }},
            {"exit", [this](const auto& args) { cmd_exit(); }},
            {"quit", [this](const auto& args) { cmd_exit(); }}
        };

        if (commands.find(command) != commands.end()) {
            commands[command](tokens);
        } else {
            std::cout << "Unknown command: " << command << std::endl;
            cmd_help();
        }
    }

    // Tokenize command line
    std::vector<std::string> tokenize(const std::string& line) {
        std::vector<std::string> tokens;
        std::string token;
        bool in_quotes = false;
        
        for (char c : line) {
            if (c == '"') {
                in_quotes = !in_quotes;
            } else if (c == ' ' && !in_quotes) {
                if (!token.empty()) {
                    tokens.push_back(token);
                    token.clear();
                }
            } else {
                token += c;
            }
        }
        
        if (!token.empty()) {
            tokens.push_back(token);
        }
        
        return tokens;
    }

    // Get full path from relative path
    std::string get_full_path(const std::string& path) {
        if (path.empty()) {
            return current_path;
        }
        
        if (path[0] == '/') {
            return vfs->normalize_path(path);
        } else {
            return vfs->normalize_path(vfs->join_paths(current_path, path));
        }
    }

    // Command handlers
    void cmd_help() {
        std::cout << "Available commands:" << std::endl;
        std::cout << "  help              - Show this help message" << std::endl;
        std::cout << "  ls [path]         - List directory contents" << std::endl;
        std::cout << "  cd <path>         - Change current directory" << std::endl;
        std::cout << "  mkdir <path>      - Create a new directory" << std::endl;
        std::cout << "  rm <path>         - Remove a file or directory" << std::endl;
        std::cout << "  cat <file>        - Display file contents" << std::endl;
        std::cout << "  put <src> <dest>  - Copy a file from the host to the container" << std::endl;
        std::cout << "  get <src> <dest>  - Copy a file from the container to the host" << std::endl;
        std::cout << "  info <path>       - Show file or directory information" << std::endl;
        std::cout << "  exit/quit         - Exit the file manager" << std::endl;
    }

    void cmd_list(const std::vector<std::string>& args) {
        std::string path = args.size() > 1 ? args[1] : current_path;
        std::string full_path = get_full_path(path);
        
        auto result = vfs->list_directory(full_path);
        if (!result.success()) {
            std::cout << "Error listing directory: Error code " << static_cast<int>(result.error()) << std::endl;
            return;
        }
        
        auto entries = result.value();
        if (entries.empty()) {
            std::cout << "Directory is empty" << std::endl;
            return;
        }
        
        // Sort entries: directories first, then files
        std::sort(entries.begin(), entries.end(), [](const hydra::vfs::FileInfo& a, const hydra::vfs::FileInfo& b) {
            if (a.is_directory != b.is_directory) {
                return a.is_directory > b.is_directory;
            }
            return a.name < b.name;
        });
        
        // Calculate column widths
        size_t name_width = 0;
        size_t size_width = 0;
        
        for (const auto& entry : entries) {
            name_width = std::max(name_width, entry.name.length());
            
            if (!entry.is_directory) {
                std::string size_str = std::to_string(entry.size);
                size_width = std::max(size_width, size_str.length());
            }
        }
        
        name_width = std::min(name_width + 2, size_t(30));
        size_width = std::max(size_width, size_t(8));
        
        // Print header
        std::cout << std::left << std::setw(name_width) << "Name" 
                  << std::right << std::setw(size_width) << "Size" 
                  << "  Type      Modified" << std::endl;
        
        std::cout << std::string(name_width + size_width + 20, '-') << std::endl;
        
        // Print entries
        for (const auto& entry : entries) {
            std::string type = entry.is_directory ? "DIR" : "FILE";
            
            std::cout << std::left << std::setw(name_width) << entry.name;
            
            if (entry.is_directory) {
                std::cout << std::right << std::setw(size_width) << "-";
            } else {
                std::cout << std::right << std::setw(size_width) << entry.size;
            }
            
            std::cout << "  " << std::left << std::setw(8) << type;
            
            // Format modified time
            char time_buf[20];
            std::strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M", 
                          std::localtime(&entry.modified_time));
            std::cout << "  " << time_buf << std::endl;
        }
    }

    void cmd_change_dir(const std::vector<std::string>& args) {
        if (args.size() < 2) {
            std::cout << "Usage: cd <path>" << std::endl;
            return;
        }
        
        std::string path = args[1];
        std::string full_path = get_full_path(path);
        
        auto result = vfs->directory_exists(full_path);
        if (!result.success() || !result.value()) {
            std::cout << "Directory does not exist: " << path << std::endl;
            return;
        }
        
        current_path = full_path;
    }

    void cmd_make_dir(const std::vector<std::string>& args) {
        if (args.size() < 2) {
            std::cout << "Usage: mkdir <path>" << std::endl;
            return;
        }
        
        std::string path = args[1];
        std::string full_path = get_full_path(path);
        
        auto result = vfs->create_directory(full_path);
        if (!result.success()) {
            std::cout << "Error creating directory: Error code " << static_cast<int>(result.error()) << std::endl;
            return;
        }
        
        std::cout << "Directory created: " << path << std::endl;
    }

    void cmd_remove(const std::vector<std::string>& args) {
        if (args.size() < 2) {
            std::cout << "Usage: rm <path>" << std::endl;
            return;
        }
        
        std::string path = args[1];
        std::string full_path = get_full_path(path);
        
        // Check if it's a file or directory
        auto file_exists = vfs->file_exists(full_path);
        if (file_exists.success() && file_exists.value()) {
            auto result = vfs->delete_file(full_path);
            if (!result.success()) {
                std::cout << "Error deleting file: Error code " << static_cast<int>(result.error()) << std::endl;
                return;
            }
            std::cout << "File deleted: " << path << std::endl;
            return;
        }
        
        auto dir_exists = vfs->directory_exists(full_path);
        if (dir_exists.success() && dir_exists.value()) {
            auto result = vfs->delete_directory(full_path, true);
            if (!result.success()) {
                std::cout << "Error deleting directory: Error code " << static_cast<int>(result.error()) << std::endl;
                return;
            }
            std::cout << "Directory deleted: " << path << std::endl;
            return;
        }
        
        std::cout << "File or directory not found: " << path << std::endl;
    }

    void cmd_cat(const std::vector<std::string>& args) {
        if (args.size() < 2) {
            std::cout << "Usage: cat <file>" << std::endl;
            return;
        }
        
        std::string path = args[1];
        std::string full_path = get_full_path(path);
        
        // Check if it's a file
        auto file_exists = vfs->file_exists(full_path);
        if (!file_exists.success() || !file_exists.value()) {
            std::cout << "File not found: " << path << std::endl;
            return;
        }
        
        // Get file info
        auto info_result = vfs->get_file_info(full_path);
        if (!info_result.success()) {
            std::cout << "Error getting file info: Error code " << static_cast<int>(info_result.error()) << std::endl;
            return;
        }
        
        auto file_info = info_result.value();
        if (file_info.size > 1024 * 1024) {
            std::cout << "File is too large to display (size: " << file_info.size << " bytes)" << std::endl;
            return;
        }
        
        // Open the file
        auto open_result = vfs->open_file(full_path, hydra::vfs::FileMode::READ);
        if (!open_result.success()) {
            std::cout << "Error opening file: Error code " << static_cast<int>(open_result.error()) << std::endl;
            return;
        }
        
        auto file = open_result.value();
        
        // Read the file content
        std::vector<uint8_t> buffer(file_info.size);
        auto read_result = file->read(buffer.data(), buffer.size());
        if (!read_result.success()) {
            std::cout << "Error reading file: Error code " << static_cast<int>(read_result.error()) << std::endl;
            return;
        }
        
        // Display the content
        std::cout << "Content of " << path << " (" << read_result.value() << " bytes):" << std::endl;
        std::cout << "----------------------------------------" << std::endl;
        
        // Check if it's a text file
        bool is_text = true;
        for (size_t i = 0; i < read_result.value() && i < 1024; ++i) {
            if (buffer[i] < 32 && buffer[i] != '\n' && buffer[i] != '\r' && buffer[i] != '\t') {
                is_text = false;
                break;
            }
        }
        
        if (is_text) {
            // Display as text
            std::string content(reinterpret_cast<char*>(buffer.data()), read_result.value());
            std::cout << content << std::endl;
        } else {
            // Display as hex dump
            std::cout << "Binary file, showing hex dump:" << std::endl;
            
            const size_t bytes_per_line = 16;
            for (size_t i = 0; i < read_result.value(); i += bytes_per_line) {
                // Print offset
                std::cout << std::hex << std::setw(8) << std::setfill('0') << i << ": ";
                
                // Print hex values
                for (size_t j = 0; j < bytes_per_line; ++j) {
                    if (i + j < read_result.value()) {
                        std::cout << std::hex << std::setw(2) << std::setfill('0') 
                                  << static_cast<int>(buffer[i + j]) << " ";
                    } else {
                        std::cout << "   ";
                    }
                }
                
                std::cout << " | ";
                
                // Print ASCII representation
                for (size_t j = 0; j < bytes_per_line; ++j) {
                    if (i + j < read_result.value()) {
                        char c = buffer[i + j];
                        if (c >= 32 && c <= 126) {
                            std::cout << c;
                        } else {
                            std::cout << ".";
                        }
                    } else {
                        std::cout << " ";
                    }
                }
                
                std::cout << std::endl;
            }
            
            std::cout << std::dec; // Reset to decimal output
        }
        
        std::cout << "----------------------------------------" << std::endl;
    }

    void cmd_put(const std::vector<std::string>& args) {
        if (args.size() < 3) {
            std::cout << "Usage: put <external_file> <vfs_path>" << std::endl;
            return;
        }
        
        std::string external_path = args[1];
        std::string vfs_path = args[2];
        std::string full_vfs_path = get_full_path(vfs_path);
        
        // Check if the external file exists
        if (!fs::exists(external_path)) {
            std::cout << "External file not found: " << external_path << std::endl;
            return;
        }
        
        // Open the external file
        std::ifstream external_file(external_path, std::ios::binary);
        if (!external_file) {
            std::cout << "Error opening external file: " << external_path << std::endl;
            return;
        }
        
        // Get file size
        external_file.seekg(0, std::ios::end);
        std::streamsize file_size = external_file.tellg();
        external_file.seekg(0, std::ios::beg);
        
        // Read the file content
        std::vector<uint8_t> buffer(file_size);
        if (!external_file.read(reinterpret_cast<char*>(buffer.data()), file_size)) {
            std::cout << "Error reading external file" << std::endl;
            return;
        }
        
        // Create the file in VFS
        auto create_result = vfs->create_file(full_vfs_path);
        if (!create_result.success()) {
            std::cout << "Error creating file in VFS: Error code " << static_cast<int>(create_result.error()) << std::endl;
            return;
        }
        
        // Open the file for writing
        auto open_result = vfs->open_file(full_vfs_path, hydra::vfs::FileMode::WRITE);
        if (!open_result.success()) {
            std::cout << "Error opening file in VFS: Error code " << static_cast<int>(open_result.error()) << std::endl;
            return;
        }
        
        auto file = open_result.value();
        
        // Write the content
        auto write_result = file->write(buffer.data(), buffer.size());
        if (!write_result.success()) {
            std::cout << "Error writing to file in VFS: Error code " << static_cast<int>(write_result.error()) << std::endl;
            return;
        }
        
        std::cout << "File copied: " << external_path << " -> " << vfs_path << std::endl;
        std::cout << "Wrote " << write_result.value() << " bytes" << std::endl;
    }

    void cmd_get(const std::vector<std::string>& args) {
        if (args.size() < 3) {
            std::cout << "Usage: get <vfs_path> <external_file>" << std::endl;
            return;
        }
        
        std::string vfs_path = args[1];
        std::string external_path = args[2];
        std::string full_vfs_path = get_full_path(vfs_path);
        
        // Check if the VFS file exists
        auto file_exists = vfs->file_exists(full_vfs_path);
        if (!file_exists.success() || !file_exists.value()) {
            std::cout << "File not found in VFS: " << vfs_path << std::endl;
            return;
        }
        
        // Open the VFS file
        auto open_result = vfs->open_file(full_vfs_path, hydra::vfs::FileMode::READ);
        if (!open_result.success()) {
            std::cout << "Error opening file in VFS: Error code " << static_cast<int>(open_result.error()) << std::endl;
            return;
        }
        
        auto file = open_result.value();
        
        // Get file info
        auto info_result = file->get_info();
        if (!info_result.success()) {
            std::cout << "Error getting file info: Error code " << static_cast<int>(info_result.error()) << std::endl;
            return;
        }
        
        auto file_info = info_result.value();
        
        // Read the file content
        std::vector<uint8_t> buffer(file_info.size);
        auto read_result = file->read(buffer.data(), buffer.size());
        if (!read_result.success()) {
            std::cout << "Error reading file from VFS: Error code " << static_cast<int>(read_result.error()) << std::endl;
            return;
        }
        
        // Create the external file
        std::ofstream external_file(external_path, std::ios::binary);
        if (!external_file) {
            std::cout << "Error creating external file: " << external_path << std::endl;
            return;
        }
        
        // Write the content
        if (!external_file.write(reinterpret_cast<char*>(buffer.data()), read_result.value())) {
            std::cout << "Error writing to external file" << std::endl;
            return;
        }
        
        std::cout << "File copied: " << vfs_path << " -> " << external_path << std::endl;
        std::cout << "Wrote " << read_result.value() << " bytes" << std::endl;
    }

    void cmd_info(const std::vector<std::string>& args) {
        if (args.size() < 2) {
            std::cout << "Usage: info <path>" << std::endl;
            return;
        }
        
        std::string path = args[1];
        std::string full_path = get_full_path(path);
        
        // Check if it's a file
        auto file_exists = vfs->file_exists(full_path);
        if (file_exists.success() && file_exists.value()) {
            auto info_result = vfs->get_file_info(full_path);
            if (!info_result.success()) {
                std::cout << "Error getting file info: Error code " << static_cast<int>(info_result.error()) << std::endl;
                return;
            }
            
            auto file_info = info_result.value();
            
            std::cout << "File information for: " << path << std::endl;
            std::cout << "  Type: File" << std::endl;
            std::cout << "  Size: " << file_info.size << " bytes" << std::endl;
            
            char time_buf[30];
            std::strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", 
                          std::localtime(&file_info.created_time));
            std::cout << "  Created: " << time_buf << std::endl;
            
            std::strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", 
                          std::localtime(&file_info.modified_time));
            std::cout << "  Modified: " << time_buf << std::endl;
            
            return;
        }
        
        // Check if it's a directory
        auto dir_exists = vfs->directory_exists(full_path);
        if (dir_exists.success() && dir_exists.value()) {
            auto list_result = vfs->list_directory(full_path);
            if (!list_result.success()) {
                std::cout << "Error listing directory: Error code " << static_cast<int>(list_result.error()) << std::endl;
                return;
            }
            
            auto entries = list_result.value();
            
            size_t file_count = 0;
            size_t dir_count = 0;
            size_t total_size = 0;
            
            for (const auto& entry : entries) {
                if (entry.is_directory) {
                    dir_count++;
                } else {
                    file_count++;
                    total_size += entry.size;
                }
            }
            
            std::cout << "Directory information for: " << path << std::endl;
            std::cout << "  Type: Directory" << std::endl;
            std::cout << "  Files: " << file_count << std::endl;
            std::cout << "  Subdirectories: " << dir_count << std::endl;
            std::cout << "  Total size: " << total_size << " bytes" << std::endl;
            
            return;
        }
        
        std::cout << "File or directory not found: " << path << std::endl;
    }

    void cmd_exit() {
        std::cout << "Exiting Secure File Manager..." << std::endl;
        running = false;
    }
};

int main(int argc, char* argv[]) {
    std::cout << "Hydra SDK Secure File Manager" << std::endl;
    std::cout << "-----------------------------" << std::endl;
    
    std::string container_path;
    std::string password;
    
    if (argc >= 2) {
        container_path = argv[1];
    } else {
        std::cout << "Enter container path: ";
        std::getline(std::cin, container_path);
    }
    
    std::cout << "Enter password: ";
    std::getline(std::cin, password);
    
    SecureFileManager manager;
    if (!manager.initialize(container_path, password)) {
        std::cout << "Failed to initialize secure file manager" << std::endl;
        return 1;
    }
    
    manager.run();
    
    return 0;
}