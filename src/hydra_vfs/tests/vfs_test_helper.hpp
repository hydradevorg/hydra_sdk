#pragma once

#include "hydra_vfs/vfs.h"
#include "hydra_vfs/memory_vfs.h"
#include "hydra_vfs/persistent_vfs.h"
#include "hydra_vfs/encrypted_vfs.h"
#include "hydra_vfs/container_vfs.h"
#include "hydra_vfs/path_utils.hpp"
#include <iostream>
#include <filesystem>
#include <string>
#include <vector>

namespace hydra {
namespace vfs {
namespace test {

/**
 * @brief Helper class for VFS tests to handle paths consistently
 */
class VFSTestHelper {
public:
    /**
     * @brief Initialize test environment
     * 
     * @param test_dir_name Name of the test directory (will be created if it doesn't exist)
     * @return true if initialization succeeded
     */
    static bool initialize_test_environment(const std::string& test_dir_name = "vfs_test_data") {
        // Get current working directory
        std::string current_dir = PathUtils::get_current_directory();
        std::cout << "TEST HELPER: Current working directory: " << current_dir << std::endl;
        
        // Set test directory path
        test_dir_path = current_dir + "/" + test_dir_name;
        std::cout << "TEST HELPER: Using test directory: " << test_dir_path << std::endl;
        
        // Clean up existing test directory
        if (std::filesystem::exists(test_dir_path)) {
            std::cout << "TEST HELPER: Cleaning up existing test directory" << std::endl;
            try {
                std::filesystem::remove_all(test_dir_path);
            } catch (const std::exception& e) {
                std::cerr << "TEST HELPER ERROR: Failed to remove test directory: " << e.what() << std::endl;
                return false;
            }
        }
        
        // Create test directory
        try {
            std::filesystem::create_directories(test_dir_path);
            std::cout << "TEST HELPER: Created test directory: " << test_dir_path << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "TEST HELPER ERROR: Failed to create test directory: " << e.what() << std::endl;
            return false;
        }
        
        return true;
    }
    
    /**
     * @brief Get the absolute path to the test directory
     * 
     * @return std::string The absolute test directory path
     */
    static std::string get_test_dir_path() {
        return test_dir_path;
    }
    
    /**
     * @brief Get an absolute path for a test file
     * 
     * @param filename The filename to use
     * @return std::string The absolute file path
     */
    static std::string get_test_file_path(const std::string& filename) {
        return test_dir_path + "/" + filename;
    }
    
    /**
     * @brief Verify that a file exists in the test directory
     * 
     * @param filename The filename to check
     * @return bool True if the file exists
     */
    static bool file_exists(const std::string& filename) {
        std::string path = get_test_file_path(filename);
        return std::filesystem::exists(path);
    }
    
    /**
     * @brief Clean up the test environment
     * 
     * @return bool True if cleanup succeeded
     */
    static bool cleanup_test_environment() {
        if (std::filesystem::exists(test_dir_path)) {
            try {
                std::filesystem::remove_all(test_dir_path);
                std::cout << "TEST HELPER: Cleaned up test directory" << std::endl;
                return true;
            } catch (const std::exception& e) {
                std::cerr << "TEST HELPER ERROR: Failed to clean up test directory: " << e.what() << std::endl;
                return false;
            }
        }
        
        return true;
    }
    
private:
    static inline std::string test_dir_path;
};

} // namespace test
} // namespace vfs
} // namespace hydra
