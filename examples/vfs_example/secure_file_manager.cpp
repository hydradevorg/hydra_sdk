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
            std::cerr << "Failed to create or open container VFS" << std::endl;
            return false;
        }

        std::cout << "Secure container opened successfully!" << std::endl;
        return true;
    }

    // Derive encryption key from password (simple implementation)
    void derive_key_from_password(const std::string& password) {
        // In a real application, use a proper key derivation function like PBKDF2
        // This is a simplified example
        
        // Fill key with zeros first
        for (size_t i = 0; i < encryption_key.size(); ++i) {
            encryption_key[i] = 0;
        }
        
        // Simple key derivation (NOT secure for production use)
        for (size_t i = 0; i < password.length(); ++i) {
            encryption_key[i % encryption_key.size()] ^= password[i];
        }
        
        // Additional mixing
        for (size_t i = 0; i < encryption_key.size() - 1; ++i) {
            encryption_key[i + 1] ^= encryption_key[i];
        }
    }

    // Run the file manager main loop
    void run() {
        std::cout << "\nSecure File Manager" << std::endl;
        std::cout << "===================" << std::endl;

        while (running) {
            display_prompt();
            std::string command;
            std::getline(std::cin, command);
            process_command(command);
        }
    }

private:
    // Display the command prompt with current path
    void display_prompt() {
        std::cout << "\n[" << current_path << "] > ";
    }

    // Process user commands
    void process_command(const std::string& command_line) {
        std::vector<std::string> args;
        size_t pos = 0;
        size_t prev = 0;
        bool in_quotes = false;
        std::string token;

        // Parse command line into arguments, respecting quoted strings
        while (pos < command_line.length()) {
            if (command_line[pos] == '"') {
                in_quotes = !in_quotes;
            } else if (command_line[pos] == ' ' && !in_quotes) {
                token = command_line.substr(prev, pos - prev);
                if (!token.empty()) {
                    // Remove quotes if present
                    if (token.front() == '"' && token.back() == '"') {
                        token = token.substr(1, token.length() - 2);
                    }
                    args.push_back(token);
                }
                prev = pos + 1;
            }
            pos++;
        }

        // Add the last argument
        if (prev < command_line.length()) {
            token = command_line.substr(prev);
            if (token.front() == '"' && token.back() == '"') {
                token = token.substr(1, token.length() - 2);
            }
            args.push_back(token);
        }

        if (args.empty()) {
            return;
        }

        const std::string& cmd = args[0];

        // Dispatch command
        if (cmd == "help" || cmd == "?") {
            show_help();
        } else if (cmd == "ls" || cmd == "dir") {
            list_directory(args.size() > 1 ? args[1] : current_path);
        } else if (cmd == "cd") {
            if (args.size() < 2) {
                std::cout << "Error: Missing directory path" << std::endl;
            } else {
                change_directory(args[1]);
            }
        } else if (cmd == "mkdir") {
            if (args.size() < 2) {
                std::cout << "Error: Missing directory name" << std::endl;
            } else {
                create_directory(args[1]);
            }
        } else if (cmd == "rm" || cmd == "del") {
            if (args.size() < 2) {
                std::cout << "Error: Missing file/directory path" << std::endl;
            } else {
                remove_item(args[1], cmd == "rm" && args.size() > 2 && args[2] == "-r");
            }
        } else if (cmd == "cat" || cmd == "type") {
            if (args.size() < 2) {
                std::cout << "Error: Missing file path" << std::endl;
            } else {
                view_file(args[1]);
            }
        } else if (cmd == "write") {
            if (args.size() < 3) {
                std::cout << "Error: Usage: write <file> <content>" << std::endl;
            } else {
                write_file(args[1], args[2]);
            }
        } else if (cmd == "import") {
            if (args.size() < 3) {
                std::cout << "Error: Usage: import <external_file> <vfs_path>" << std::endl;
            } else {
                import_file(args[1], args[2]);
            }
        } else if (cmd == "export") {
            if (args.size() < 3) {
                std::cout << "Error: Usage: export <vfs_path> <external_file>" << std::endl;
            } else {
                export_file(args[1], args[2]);
            }
        } else if (cmd == "exit" || cmd == "quit") {
            running = false;
            std::cout << "Exiting Secure File Manager" << std::endl;
        } else {
            std::cout << "Unknown command: " << cmd << std::endl;
            std::cout << "Type 'help' for available commands" << std::endl;
        }
    }

    // Show help information
    void show_help() {
        std::cout << "\nAvailable commands:" << std::endl;
        std::cout << "  help, ?             Show this help information" << std::endl;
        std::cout << "  ls, dir [path]      List directory contents" << std::endl;
        std::cout << "  cd <path>           Change current directory" << std::endl;
        std::cout << "  mkdir <name>        Create a new directory" << std::endl;
        std::cout << "  rm, del <path>      Remove a file" << std::endl;
        std::cout << "  rm -r <path>        Remove a directory recursively" << std::endl;
        std::cout << "  cat, type <file>    View file contents" << std::endl;
        std::cout << "  write <file> <text> Write text to a file" << std::endl;
        std::cout << "  import <ext> <vfs>  Import external file to VFS" << std::endl;
        std::cout << "  export <vfs> <ext>  Export VFS file to external file" << std::endl;
        std::cout << "  exit, quit          Exit the application" << std::endl;
    }

    // Resolve a path (relative or absolute) to an absolute path
    std::string resolve_path(const std::string& path) {
        if (path.empty()) {
            return current_path;
        }

        std::string resolved;
        if (path[0] == '/') {
            // Absolute path
            resolved = path;
        } else {
            // Relative path
            if (current_path == "/") {
                resolved = current_path + path;
            } else {
                resolved = current_path + "/" + path;
            }
        }
        
        // Normalize path
        std::vector<std::string> segments;
        std::string segment;
        std::stringstream ss(resolved);
        
        while (std::getline(ss, segment, '/')) {
            if (segment == ".") {
                // Skip
            } else if (segment == "..") {
                // Go up one level
                if (!segments.empty()) {
                    segments.pop_back();
                }
            } else if (!segment.empty()) {
                segments.push_back(segment);
            }
        }
        
        if (segments.empty()) {
            return "/";
        }
        
        std::string result = "";
        for (const auto& seg : segments) {
            result += "/" + seg;
        }
        
        return result;
    }

    // List directory contents
    void list_directory(const std::string& path) {
        std::string full_path = resolve_path(path);
        auto result = vfs->list_directory(full_path);
        
        if (!result.success()) {
            std::cout << "Error: " << result.error_message() << std::endl;
            return;
        }
        
        auto entries = result.value();
        if (entries.empty()) {
            std::cout << "Directory is empty" << std::endl;
            return;
        }
        
        // Sort entries: directories first, then files
        std::sort(entries.begin(), entries.end(), 
            [](const hydra::vfs::DirectoryEntry& a, const hydra::vfs::DirectoryEntry& b) {
                if (a.is_directory && !b.is_directory) return true;
                if (!a.is_directory && b.is_directory) return false;
                return a.name < b.name;
            }
        );
        
        // Calculate column widths
        size_t name_width = 4; // "Name"
        size_t size_width = 4; // "Size"
        size_t type_width = 4; // "Type"
        
        for (const auto& entry : entries) {
            name_width = std::max(name_width, entry.name.length());
            
            // For size, calculate width based on largest file size
            if (!entry.is_directory) {
                size_t digits = 0;
                size_t size = entry.size;
                do {
                    digits++;
                    size /= 10;
                } while (size > 0);
                
                size_width = std::max(size_width, digits);
            }
        }
        
        // Add some padding
        name_width += 2;
        size_width += 2;
        
        // Print header
        std::cout << std::left << std::setw(name_width) << "Name" 
                  << std::left << std::setw(type_width) << "Type"
                  << std::right << std::setw(size_width) << "Size" << std::endl;
        
        std::cout << std::string(name_width + type_width + size_width, '-') << std::endl;
        
        // Print entries
        for (const auto& entry : entries) {
            std::string type = entry.is_directory ? "DIR" : "FILE";
            
            std::cout << std::left << std::setw(name_width) << entry.name 
                      << std::left << std::setw(type_width) << type;
            
            if (entry.is_directory) {
                std::cout << std::right << std::setw(size_width) << "-";
            } else {
                std::cout << std::right << std::setw(size_width) << entry.size;
            }
            
            std::cout << std::endl;
        }
    }

    // Change the current directory
    void change_directory(const std::string& path) {
        std::string new_path = resolve_path(path);
        
        // Check if the path exists and is a directory
        auto exists_result = vfs->directory_exists(new_path);
        if (!exists_result.success() || !exists_result.value()) {
            std::cout << "Error: Directory does not exist: " << new_path << std::endl;
            return;
        }
        
        current_path = new_path;
    }

    // Create a new directory
    void create_directory(const std::string& name) {
        std::string full_path = resolve_path(name);
        
        auto result = vfs->create_directory(full_path);
        if (!result.success()) {
            std::cout << "Error creating directory: " << result.error_message() << std::endl;
            return;
        }
        
        std::cout << "Directory created: " << full_path << std::endl;
    }

    // Remove a file or directory
    void remove_item(const std::string& path, bool recursive) {
        std::string full_path = resolve_path(path);
        
        // Check if it's a file or directory
        auto file_exists_result = vfs->file_exists(full_path);
        auto dir_exists_result = vfs->directory_exists(full_path);
        
        if (file_exists_result.success() && file_exists_result.value()) {
            // It's a file
            auto result = vfs->delete_file(full_path);
            if (!result.success()) {
                std::cout << "Error deleting file: " << result.error_message() << std::endl;
                return;
            }
            
            std::cout << "File deleted: " << full_path << std::endl;
        } else if (dir_exists_result.success() && dir_exists_result.value()) {
            // It's a directory
            auto result = vfs->delete_directory(full_path, recursive);
            if (!result.success()) {
                std::cout << "Error deleting directory: " << result.error_message() << std::endl;
                if (!recursive) {
                    std::cout << "Hint: Use 'rm -r' to delete directories with content" << std::endl;
                }
                return;
            }
            
            std::cout << "Directory deleted: " << full_path << std::endl;
        } else {
            std::cout << "Error: File or directory does not exist: " << full_path << std::endl;
        }
    }

    // View file contents
    void view_file(const std::string& path) {
        std::string full_path = resolve_path(path);
        
        // Check if the file exists
        auto exists_result = vfs->file_exists(full_path);
        if (!exists_result.success() || !exists_result.value()) {
            std::cout << "Error: File does not exist: " << full_path << std::endl;
            return;
        }
        
        // Open the file for reading
        auto open_result = vfs->open_file(full_path, hydra::vfs::FileMode::READ);
        if (!open_result.success()) {
            std::cout << "Error opening file: " << open_result.error_message() << std::endl;
            return;
        }
        
        auto file = open_result.value();
        
        // Get file size
        auto size_result = file->get_size();
        if (!size_result.success()) {
            std::cout << "Error getting file size: " << size_result.error_message() << std::endl;
            file->close();
            return;
        }
        
        size_t file_size = size_result.value();
        if (file_size == 0) {
            std::cout << "(Empty file)" << std::endl;
            file->close();
            return;
        }
        
        // Read the file content
        std::vector<uint8_t> buffer(file_size);
        auto read_result = file->read(buffer.data(), buffer.size());
        if (!read_result.success()) {
            std::cout << "Error reading file: " << read_result.error_message() << std::endl;
            file->close();
            return;
        }
        
        // Determine if the file is binary or text
        bool is_binary = false;
        for (size_t i = 0; i < read_result.value() && i < 1024; ++i) {
            // Check for common binary bytes
            if (buffer[i] < 9 || (buffer[i] > 13 && buffer[i] < 32)) {
                is_binary = true;
                break;
            }
        }
        
        if (is_binary) {
            std::cout << "Binary file, showing hex dump:" << std::endl;
            
            // Hex dump display (16 bytes per line)
            const size_t bytes_per_line = 16;
            size_t bytes_read = read_result.value();
            
            for (size_t offset = 0; offset < bytes_read; offset += bytes_per_line) {
                // Print offset
                std::cout << std::setfill('0') << std::setw(8) << std::hex << offset << ": ";
                
                // Print hex values
                for (size_t i = 0; i < bytes_per_line; ++i) {
                    if (offset + i < bytes_read) {
                        std::cout << std::setfill('0') << std::setw(2) << std::hex 
                                  << static_cast<int>(buffer[offset + i]) << " ";
                    } else {
                        std::cout << "   ";
                    }
                    
                    // Extra space in the middle
                    if (i == 7) {
                        std::cout << " ";
                    }
                }
                
                // Print ASCII representation
                std::cout << " | ";
                for (size_t i = 0; i < bytes_per_line; ++i) {
                    if (offset + i < bytes_read) {
                        char c = static_cast<char>(buffer[offset + i]);
                        std::cout << (c >= 32 && c <= 126 ? c : '.');
                    } else {
                        std::cout << " ";
                    }
                }
                
                std::cout << std::endl;
                
                // Limit output for large binary files
                if (offset >= 256) {
                    std::cout << "... (truncated, total size: " 
                              << std::dec << bytes_read << " bytes)" << std::endl;
                    break;
                }
            }
            
            std::cout << std::dec; // Reset to decimal
        } else {
            // Text file
            std::string content(reinterpret_cast<char*>(buffer.data()), read_result.value());
            std::cout << content << std::endl;
        }
        
        file->close();
    }

    // Write text to a file
    void write_file(const std::string& path, const std::string& content) {
        std::string full_path = resolve_path(path);
        
        // Check if the file exists, create it if not
        auto exists_result = vfs->file_exists(full_path);
        if (!exists_result.success() || !exists_result.value()) {
            auto create_result = vfs->create_file(full_path);
            if (!create_result.success()) {
                std::cout << "Error creating file: " << create_result.error_message() << std::endl;
                return;
            }
        }
        
        // Open the file for writing
        auto open_result = vfs->open_file(full_path, hydra::vfs::FileMode::WRITE);
        if (!open_result.success()) {
            std::cout << "Error opening file: " << open_result.error_message() << std::endl;
            return;
        }
        
        auto file = open_result.value();
        
        // Write the content
        auto write_result = file->write(
            reinterpret_cast<const uint8_t*>(content.c_str()), 
            content.size()
        );
        
        if (!write_result.success()) {
            std::cout << "Error writing to file: " << write_result.error_message() << std::endl;
            file->close();
            return;
        }
        
        std::cout << "Wrote " << write_result.value() << " bytes to file" << std::endl;
        file->close();
    }

    // Import an external file to the VFS
    void import_file(const std::string& external_path, const std::string& vfs_path) {
        // Check if the external file exists
        if (!fs::exists(external_path) || !fs::is_regular_file(external_path)) {
            std::cout << "Error: External file does not exist: " << external_path << std::endl;
            return;
        }
        
        // Resolve the VFS path
        std::string full_vfs_path = resolve_path(vfs_path);
        
        // Read the external file
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
            std::cout << "Error creating file in VFS: " << create_result.error_message() << std::endl;
            return;
        }
        
        // Open the file for writing
        auto open_result = vfs->open_file(full_vfs_path, hydra::vfs::FileMode::WRITE);
        if (!open_result.success()) {
            std::cout << "Error opening file in VFS: " << open_result.error_message() << std::endl;
            return;
        }
        
        auto file = open_result.value();
        
        // Write the content
        auto write_result = file->write(buffer.data(), buffer.size());
        if (!write_result.success()) {
            std::cout << "Error writing to file in VFS: " << write_result.error_message() << std::endl;
            file->close();
            return;
        }
        
        std::cout << "Imported " << write_result.value() << " bytes to " << full_vfs_path << std::endl;
        file->close();
    }

    // Export a VFS file to an external file
    void export_file(const std::string& vfs_path, const std::string& external_path) {
        // Resolve the VFS path
        std::string full_vfs_path = resolve_path(vfs_path);
        
        // Check if the VFS file exists
        auto exists_result = vfs->file_exists(full_vfs_path);
        if (!exists_result.success() || !exists_result.value()) {
            std::cout << "Error: File does not exist in VFS: " << full_vfs_path << std::endl;
            return;
        }
        
        // Open the VFS file for reading
        auto open_result = vfs->open_file(full_vfs_path, hydra::vfs::FileMode::READ);
        if (!open_result.success()) {
            std::cout << "Error opening file in VFS: " << open_result.error_message() << std::endl;
            return;
        }
        
        auto file = open_result.value();
        
        // Get file size
        auto size_result = file->get_size();
        if (!size_result.success()) {
            std::cout << "Error getting file size: " << size_result.error_message() << std::endl;
            file->close();
            return;
        }
        
        size_t file_size = size_result.value();
        if (file_size == 0) {
            std::cout << "Warning: VFS file is empty" << std::endl;
        }
        
        // Read the VFS file content
        std::vector<uint8_t> buffer(file_size);
        auto read_result = file->read(buffer.data(), buffer.size());
        if (!read_result.success()) {
            std::cout << "Error reading file from VFS: " << read_result.error_message() << std::endl;
            file->close();
            return;
        }
        
        file->close();
        
        // Create parent directories if needed
        fs::path external_file_path(external_path);
        fs::path parent_path = external_file_path.parent_path();
        if (!parent_path.empty() && !fs::exists(parent_path)) {
            fs::create_directories(parent_path);
        }
        
        // Write to external file
        std::ofstream external_file(external_path, std::ios::binary);
        if (!external_file) {
            std::cout << "Error creating external file: " << external_path << std::endl;
            return;
        }
        
        external_file.write(reinterpret_cast<const char*>(buffer.data()), read_result.value());
        if (!external_file) {
            std::cout << "Error writing to external file" << std::endl;
            return;
        }
        
        std::cout << "Exported " << read_result.value() << " bytes to " << external_path << std::endl;
    }
};

int main(int argc, char* argv[]) {
    std::cout << "Hydra VFS Secure File Manager" << std::endl;
    std::cout << "============================" << std::endl;
    
    std::string container_path = "./secure_container.dat";
    if (argc > 1) {
        container_path = argv[1];
    }
    
    // Get the password to unlock the container
    std::string password;
    std::cout << "Enter password to unlock/create container: ";
    std::getline(std::cin, password);
    
    if (password.empty()) {
        std::cerr << "Error: Password cannot be empty" << std::endl;
        return 1;
    }
    
    // Initialize the secure file manager
    SecureFileManager file_manager;
    if (!file_manager.initialize(container_path, password)) {
        std::cerr << "Failed to initialize secure file manager" << std::endl;
        return 1;
    }
    
    // Run the file manager
    file_manager.run();
    
    return 0;
}
