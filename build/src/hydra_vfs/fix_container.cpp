#include "hydra_vfs/vfs.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>
#include <filesystem>
#include <unistd.h>

int main() {
    std::cout << "Running ContainerVFS Fix Tool\n";
    
    // Clean up test directory
    std::string test_dir = "./vfs_test_data";
    if (std::filesystem::exists(test_dir)) {
        std::cout << "Removing existing test directory\n";
        std::filesystem::remove_all(test_dir);
    }
    
    // Create fresh test directory
    std::filesystem::create_directories(test_dir);
    std::cout << "Created test directory: " << test_dir << "\n";
    
    // Container path
    std::string container_path = test_dir + "/test_container.dat";
    std::cout << "Container path: " << container_path << "\n";
    
    // Create encryption key
    hydra::vfs::EncryptionKey key = {};
    for (size_t i = 0; i < key.size(); ++i) {
        key[i] = static_cast<uint8_t>(i & 0xFF);
    }
    
    // Create resource limits
    hydra::vfs::ResourceLimits limits;
    limits.max_file_count = 100;
    limits.max_storage_size = 1024 * 1024; // 1MB
    
    // Create container
    std::cout << "Creating container VFS\n";
    auto container_vfs = hydra::vfs::create_container_vfs(
        container_path,
        key,
        nullptr,
        hydra::vfs::SecurityLevel::STANDARD,
        limits
    );
    
    if (!container_vfs) {
        std::cerr << "Failed to create container VFS\n";
        return 1;
    }
    
    // Create a test file
    std::cout << "Creating test file\n";
    auto create_result = container_vfs->create_file("/secret.txt");
    if (!create_result.success()) {
        std::cerr << "Failed to create file: " << static_cast<int>(create_result.error()) << "\n";
        return 1;
    }
    
    // Write to the file
    auto file_result = container_vfs->open_file("/secret.txt", hydra::vfs::FileMode::WRITE);
    if (!file_result.success()) {
        std::cerr << "Failed to open file for writing: " << static_cast<int>(file_result.error()) << "\n";
        return 1;
    }
    
    std::string secret_content = "TOP SECRET: This data should be encrypted";
    auto file = file_result.value();
    auto write_result = file->write(reinterpret_cast<const uint8_t*>(secret_content.c_str()), secret_content.size());
    
    if (!write_result.success()) {
        std::cerr << "Failed to write to file: " << static_cast<int>(write_result.error()) << "\n";
        return 1;
    }
    
    file->close();
    std::cout << "File created and written\n";
    
    // Check if file exists and verify container was created
    if (!std::filesystem::exists(container_path)) {
        std::cerr << "Container file doesn't exist!\n";
        return 1;
    }
    
    std::cout << "Container file exists with size: " << std::filesystem::file_size(container_path) << " bytes\n";
    
    // Read file contents to verify encryption
    std::ifstream container_file(container_path, std::ios::binary);
    if (!container_file.good()) {
        std::cerr << "Failed to open container file for reading\n";
        return 1;
    }
    
    std::string file_contents((std::istreambuf_iterator<char>(container_file)),
                             std::istreambuf_iterator<char>());
    container_file.close();
    
    // Check that the plaintext is not visible
    if (file_contents.find("TOP SECRET") != std::string::npos) {
        std::cerr << "Plaintext found in container file - encryption failed!\n";
        return 1;
    }
    
    std::cout << "SUCCESS: Container file is properly encrypted\n";
    return 0;
}