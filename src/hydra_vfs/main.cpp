#include <iostream>
#include <string>
#include <vector>
#include "hydra_vfs/vfs.h"

// Helper function to print file info
void print_file_info(const hydra::vfs::FileInfo& info) {
    std::cout << (info.is_directory ? "DIR  " : "FILE ") 
              << info.name << " (Size: " << info.size << " bytes)" << std::endl;
}

// Helper function to print directory contents
void print_directory(hydra::vfs::IVirtualFileSystem* vfs, const std::string& path) {
    std::cout << "\nListing directory: " << path << std::endl;
    std::cout << "------------------------" << std::endl;
    
    auto result = vfs->list_directory(path);
    if (!result) {
        std::cout << "Error listing directory: " << static_cast<int>(result.error()) << std::endl;
        return;
    }
    
    for (const auto& entry : result.value()) {
        print_file_info(entry);
    }
}

// Helper function to write text to a file
void write_text_file(hydra::vfs::IVirtualFileSystem* vfs, const std::string& path, const std::string& content) {
    auto file_result = vfs->open_file(path, hydra::vfs::FileMode::WRITE);
    if (!file_result) {
        std::cout << "Error opening file for writing: " << static_cast<int>(file_result.error()) << std::endl;
        return;
    }
    
    auto file = file_result.value();
    auto write_result = file->write(reinterpret_cast<const uint8_t*>(content.c_str()), content.size());
    
    if (!write_result) {
        std::cout << "Error writing to file: " << static_cast<int>(write_result.error()) << std::endl;
        return;
    }
    
    file->close();
    std::cout << "Successfully wrote " << write_result.value() << " bytes to " << path << std::endl;
}

// Helper function to read text from a file
std::string read_text_file(hydra::vfs::IVirtualFileSystem* vfs, const std::string& path) {
    auto file_result = vfs->open_file(path, hydra::vfs::FileMode::READ);
    if (!file_result) {
        std::cout << "Error opening file for reading: " << static_cast<int>(file_result.error()) << std::endl;
        return "";
    }
    
    auto file = file_result.value();
    auto info_result = file->get_info();
    
    if (!info_result) {
        std::cout << "Error getting file info: " << static_cast<int>(info_result.error()) << std::endl;
        return "";
    }
    
    size_t file_size = info_result.value().size;
    std::vector<uint8_t> buffer(file_size);
    
    auto read_result = file->read(buffer.data(), file_size);
    
    if (!read_result) {
        std::cout << "Error reading from file: " << static_cast<int>(read_result.error()) << std::endl;
        return "";
    }
    
    file->close();
    std::cout << "Successfully read " << read_result.value() << " bytes from " << path << std::endl;
    
    return std::string(buffer.begin(), buffer.end());
}

int main() {
    std::cout << "Hydra Virtual File System Demo" << std::endl;
    std::cout << "=============================" << std::endl;
    
    // Create an in-memory VFS
    std::cout << "\n1. Creating an in-memory VFS" << std::endl;
    auto memory_vfs = hydra::vfs::create_vfs();
    
    // Create a persistent VFS that stores data in a local directory
    std::cout << "\n2. Creating a persistent VFS" << std::endl;
    auto persistent_vfs = hydra::vfs::create_vfs("./vfs_data");
    
    // Demonstrate operations on the in-memory VFS
    std::cout << "\n3. Demonstrating in-memory VFS operations" << std::endl;
    
    // Create directories
    memory_vfs->create_directory("/test");
    memory_vfs->create_directory("/test/subdir");
    
    // Create and write to files
    write_text_file(memory_vfs.get(), "/test/hello.txt", "Hello, World!");
    write_text_file(memory_vfs.get(), "/test/subdir/data.txt", "This is some test data in a subdirectory.");
    
    // List directories
    print_directory(memory_vfs.get(), "/");
    print_directory(memory_vfs.get(), "/test");
    print_directory(memory_vfs.get(), "/test/subdir");
    
    // Read files
    std::string content1 = read_text_file(memory_vfs.get(), "/test/hello.txt");
    std::cout << "Content of /test/hello.txt: " << content1 << std::endl;
    
    std::string content2 = read_text_file(memory_vfs.get(), "/test/subdir/data.txt");
    std::cout << "Content of /test/subdir/data.txt: " << content2 << std::endl;
    
    // Demonstrate operations on the persistent VFS
    std::cout << "\n4. Demonstrating persistent VFS operations" << std::endl;
    
    // Create directories
    persistent_vfs->create_directory("/secure");
    persistent_vfs->create_directory("/secure/keys");
    
    // Create and write to files
    write_text_file(persistent_vfs.get(), "/secure/config.txt", "Secure configuration data");
    write_text_file(persistent_vfs.get(), "/secure/keys/key1.pem", "-----BEGIN PRIVATE KEY-----\nMIIEvQIBADANBgkqhkiG9w0BAQEFAASCBKcwggSj...\n-----END PRIVATE KEY-----");
    
    // List directories
    print_directory(persistent_vfs.get(), "/");
    print_directory(persistent_vfs.get(), "/secure");
    print_directory(persistent_vfs.get(), "/secure/keys");
    
    // Read files
    std::string content3 = read_text_file(persistent_vfs.get(), "/secure/config.txt");
    std::cout << "Content of /secure/config.txt: " << content3 << std::endl;
    
    // Mount the in-memory VFS as a subdirectory of the persistent VFS
    std::cout << "\n5. Mounting in-memory VFS as a subdirectory of persistent VFS" << std::endl;
    persistent_vfs->mount("/secure/memory", memory_vfs);
    
    // List the mount point
    print_directory(persistent_vfs.get(), "/secure");
    
    // Access the mounted VFS through the mount point
    std::string content4 = read_text_file(persistent_vfs.get(), "/secure/memory/test/hello.txt");
    std::cout << "Content of mounted file /secure/memory/test/hello.txt: " << content4 << std::endl;
    
    std::cout << "\nVirtual File System Demo Completed" << std::endl;
    return 0;
}
