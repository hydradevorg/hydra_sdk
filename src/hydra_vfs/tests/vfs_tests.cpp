#include "hydra_vfs/vfs.h"
#include "hydra_vfs/memory_vfs.h"
#include "hydra_vfs/persistent_vfs.h"
#include "hydra_vfs/encrypted_vfs.h"
#include "hydra_vfs/container_vfs.h"
#include "hydra_vfs/path_utils.hpp"
#include "vfs_test_helper.hpp"
#include <iostream>
#include <cassert>
#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
#include <random>
#include <unistd.h>

namespace fs = std::filesystem;
using namespace hydra::vfs::test;

// Test helper functions
void assert_true(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "FAILED: " << message << std::endl;
        assert(condition);
    }
}

void assert_equal(size_t expected, size_t actual, const std::string& message) {
    if (expected != actual) {
        std::cerr << "FAILED: " << message << " (Expected: " << expected << ", Actual: " << actual << ")" << std::endl;
        assert(expected == actual);
    }
}

void assert_equal(const std::string& expected, const std::string& actual, const std::string& message) {
    if (expected != actual) {
        std::cerr << "FAILED: " << message << " (Expected: " << expected << ", Actual: " << actual << ")" << std::endl;
        assert(expected == actual);
    }
}

// Test writing and reading a file
void test_file_write_read(hydra::vfs::IVirtualFileSystem* vfs, const std::string& test_name) {
    std::cout << "Running test: " << test_name << " - file_write_read" << std::endl;

    const std::string test_path = "/test_file.txt";
    const std::string test_content = "Hello, Virtual File System!";

    std::cout << "DEBUG: Creating file: " << test_path << std::endl;
    // Create file first
    auto create_result = vfs->create_file(test_path);
    assert_true(create_result.success(), "Failed to create file");
    std::cout << "DEBUG: File created successfully" << std::endl;

    std::cout << "DEBUG: Opening file for writing" << std::endl;
    // Create and write to file
    auto file_result = vfs->open_file(test_path, hydra::vfs::FileMode::WRITE);
    assert_true(file_result.success(), "Failed to open file for writing");
    std::cout << "DEBUG: File opened successfully" << std::endl;

    auto file = file_result.value();
    std::cout << "DEBUG: Writing to file" << std::endl;
    auto write_result = file->write(reinterpret_cast<const uint8_t*>(test_content.c_str()), test_content.size());
    assert_true(write_result.success(), "Failed to write to file");
    assert_equal(test_content.size(), write_result.value(), "Incorrect number of bytes written");
    std::cout << "DEBUG: Write successful, wrote " << write_result.value() << " bytes" << std::endl;

    std::cout << "DEBUG: Closing file after write" << std::endl;
    file->close();
    std::cout << "DEBUG: File closed successfully" << std::endl;

    std::cout << "DEBUG: Opening file for reading" << std::endl;
    // Read from file
    file_result = vfs->open_file(test_path, hydra::vfs::FileMode::READ);
    assert_true(file_result.success(), "Failed to open file for reading");
    std::cout << "DEBUG: File opened for reading successfully" << std::endl;

    file = file_result.value();
    std::vector<uint8_t> buffer(test_content.size());
    std::cout << "DEBUG: Reading from file" << std::endl;
    auto read_result = file->read(buffer.data(), buffer.size());
    assert_true(read_result.success(), "Failed to read from file");
    assert_equal(test_content.size(), read_result.value(), "Incorrect number of bytes read");
    std::cout << "DEBUG: Read successful, read " << read_result.value() << " bytes" << std::endl;

    std::string read_content(reinterpret_cast<char*>(buffer.data()), read_result.value());
    assert_equal(test_content, read_content, "File content mismatch");
    std::cout << "DEBUG: Content matches" << std::endl;

    std::cout << "DEBUG: Closing file after read" << std::endl;
    file->close();
    std::cout << "DEBUG: File closed successfully" << std::endl;

    std::cout << "PASSED: " << test_name << " - file_write_read" << std::endl;
}

// Test directory operations
void test_directory_operations(hydra::vfs::IVirtualFileSystem* vfs, const std::string& test_name) {
    std::cout << "Running test: " << test_name << " - directory_operations" << std::endl;

    // Create directories
    auto result = vfs->create_directory("/test_dir");
    assert_true(result.success(), "Failed to create directory");

    result = vfs->create_directory("/test_dir/subdir");
    assert_true(result.success(), "Failed to create subdirectory");

    // Check if directories exist
    auto exists_result = vfs->directory_exists("/test_dir");
    assert_true(exists_result.success(), "Failed to check if directory exists");
    assert_true(exists_result.value(), "Directory should exist");

    exists_result = vfs->directory_exists("/test_dir/subdir");
    assert_true(exists_result.success(), "Failed to check if subdirectory exists");
    assert_true(exists_result.value(), "Subdirectory should exist");

    // Create files in directories
    const std::string test_content1 = "File in root directory";
    const std::string test_content2 = "File in subdirectory";

    auto file_result = vfs->open_file("/test_dir/file1.txt", hydra::vfs::FileMode::WRITE);
    assert_true(file_result.success(), "Failed to open file in directory");
    auto file = file_result.value();
    file->write(reinterpret_cast<const uint8_t*>(test_content1.c_str()), test_content1.size());
    file->close();

    file_result = vfs->open_file("/test_dir/subdir/file2.txt", hydra::vfs::FileMode::WRITE);
    assert_true(file_result.success(), "Failed to open file in subdirectory");
    file = file_result.value();
    file->write(reinterpret_cast<const uint8_t*>(test_content2.c_str()), test_content2.size());
    file->close();

    // List directory contents
    auto list_result = vfs->list_directory("/test_dir");
    assert_true(list_result.success(), "Failed to list directory");
    auto entries = list_result.value();

    bool found_file = false;
    bool found_dir = false;

    for (const auto& entry : entries) {
        if (entry.name == "file1.txt") {
            found_file = true;
            assert_true(!entry.is_directory, "File should not be a directory");
            assert_equal(test_content1.size(), entry.size, "File size mismatch");
        } else if (entry.name == "subdir") {
            found_dir = true;
            assert_true(entry.is_directory, "Subdirectory should be a directory");
        }
    }

    assert_true(found_file, "File not found in directory listing");
    assert_true(found_dir, "Subdirectory not found in directory listing");

    // Delete directory (should fail because it's not empty)
    result = vfs->delete_directory("/test_dir", false);
    assert_true(!result.success(), "Deleting non-empty directory without recursive flag should fail");

    // Delete directory recursively
    result = vfs->delete_directory("/test_dir", true);
    assert_true(result.success(), "Failed to delete directory recursively");

    // Verify directory is gone
    exists_result = vfs->directory_exists("/test_dir");
    assert_true(exists_result.success(), "Failed to check if directory exists after deletion");
    assert_true(!exists_result.value(), "Directory should not exist after deletion");

    std::cout << "PASSED: " << test_name << " - directory_operations" << std::endl;
}

// Test file operations
void test_file_operations(hydra::vfs::IVirtualFileSystem* vfs, const std::string& test_name) {
    std::cout << "Running test: " << test_name << " - file_operations" << std::endl;

    const std::string test_path = "/test_file_ops.txt";
    const std::string test_content = "Original content";
    const std::string new_path = "/renamed_file.txt";

    // Create file
    auto result = vfs->create_file(test_path);
    assert_true(result.success(), "Failed to create file");

    // Check if file exists
    auto exists_result = vfs->file_exists(test_path);
    assert_true(exists_result.success(), "Failed to check if file exists");
    assert_true(exists_result.value(), "File should exist");

    // Write to file
    auto file_result = vfs->open_file(test_path, hydra::vfs::FileMode::WRITE);
    assert_true(file_result.success(), "Failed to open file for writing");

    auto file = file_result.value();
    auto write_result = file->write(reinterpret_cast<const uint8_t*>(test_content.c_str()), test_content.size());
    assert_true(write_result.success(), "Failed to write to file");
    assert_true(write_result.value() == test_content.size(), "Write size mismatch");
    file->close();

    // Get file info
    auto info_result = vfs->get_file_info(test_path);
    assert_true(info_result.success(), "Failed to get file info");
    assert_equal(test_content.size(), info_result.value().size, "File size mismatch in info");

    // Rename file
    auto rename_result = vfs->rename_file(test_path, new_path);
    assert_true(rename_result.success(), "Failed to rename file");

    // Verify old path doesn't exist
    exists_result = vfs->file_exists(test_path);
    assert_true(exists_result.success(), "Failed to check if old path exists");
    assert_true(!exists_result.value(), "Old path should not exist after rename");

    // Verify new path exists
    exists_result = vfs->file_exists(new_path);
    assert_true(exists_result.success(), "Failed to check if new path exists");
    assert_true(exists_result.value(), "New path should exist after rename");

    // Delete file
    auto delete_result = vfs->delete_file(new_path);
    assert_true(delete_result.success(), "Failed to delete file");

    // Verify file is gone
    exists_result = vfs->file_exists(new_path);
    assert_true(exists_result.success(), "Failed to check if file exists after deletion");
    assert_true(!exists_result.value(), "File should not exist after deletion");

    std::cout << "PASSED: " << test_name << " - file_operations" << std::endl;
}

// Main function
int main() {
    std::cout << "Starting VFS tests..." << std::endl;

    // Test with MemoryVFS
    {
        std::cout << "\n=== Testing MemoryVFS ===" << std::endl;
        auto vfs = std::make_unique<hydra::vfs::MemoryVFS>();
        test_file_write_read(vfs.get(), "MemoryVFS");
        test_directory_operations(vfs.get(), "MemoryVFS");
        test_file_operations(vfs.get(), "MemoryVFS");
    }

    // Test with PersistentVFS
    {
        std::cout << "\n=== Testing PersistentVFS ===" << std::endl;
        // Create test directory
        std::string test_dir = "./vfs_test_data";
        if (!std::filesystem::exists(test_dir)) {
            std::filesystem::create_directories(test_dir);
        }

        auto vfs = std::make_unique<hydra::vfs::PersistentVFS>(test_dir);
        test_file_write_read(vfs.get(), "PersistentVFS");
        test_directory_operations(vfs.get(), "PersistentVFS");
        test_file_operations(vfs.get(), "PersistentVFS");
    }

    std::cout << "\nAll tests completed successfully!" << std::endl;
    return 0;
}