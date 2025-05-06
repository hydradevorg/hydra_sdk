#include "lmvs/p2p_vfs/p2p_vfs.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <thread>
#include <chrono>

int main(int argc, char* argv[]) {
    std::cout << "P2P VFS Cat Example" << std::endl;
    std::cout << "===================" << std::endl;

    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <file_path>" << std::endl;
        return 1;
    }

    std::string file_path = argv[1];

    // Create a P2P VFS node
    std::string node_id = "local_node";
    std::string storage_path = "./p2p_vfs_storage/local";

    // Create the storage directory if it doesn't exist
    std::filesystem::create_directories(storage_path);

    lmvs::p2p_vfs::P2PVFS vfs(node_id, storage_path);

    std::cout << "Node ID: " << node_id << std::endl;
    std::cout << "Storage Path: " << storage_path << std::endl;

    // Create the parent directory if it doesn't exist
    std::string parent_dir = "/";
    size_t last_slash = file_path.find_last_of('/');
    if (last_slash != std::string::npos) {
        parent_dir = file_path.substr(0, last_slash);
        if (parent_dir.empty()) {
            parent_dir = "/";
        }

        // Create the directory if it doesn't exist
        auto dir_exists_result = vfs.directory_exists(parent_dir);
        if (dir_exists_result.success() && !dir_exists_result.value()) {
            std::cout << "Creating directory: " << parent_dir << std::endl;
            auto create_dir_result = vfs.create_directory(parent_dir);
            if (!create_dir_result.success() || !create_dir_result.value()) {
                std::cerr << "Error creating directory: " << parent_dir << std::endl;
                return 1;
            }
        }
    }

    std::cout << "Storage path: " << vfs.get_local_storage_path() << std::endl;

    // Check if the file exists
    auto exists_result = vfs.file_exists(file_path);
    if (!exists_result.success()) {
        std::cerr << "Error checking if file exists: " << exists_result.error() << std::endl;
        return 1;
    }

    if (!exists_result.value()) {
        std::cout << "File does not exist. Creating a sample file..." << std::endl;

        // Create a sample file
        auto file_result = vfs.open_file(file_path, hydra::vfs::FileMode::WRITE);
        if (!file_result.success()) {
            std::cerr << "Error creating file: " << file_result.error() << std::endl;
            return 1;
        }

        auto file = file_result.value();

        std::string sample_content = "This is a sample file created by the P2P VFS Cat Example.\n"
                                    "It demonstrates the cat_file functionality of the P2P VFS.\n"
                                    "You can use this functionality to display the contents of any file in the P2P VFS.\n";

        auto write_result = file->write(reinterpret_cast<const uint8_t*>(sample_content.c_str()), sample_content.size());
        if (!write_result.success()) {
            std::cerr << "Error writing to file: " << write_result.error() << std::endl;
            return 1;
        }

        file->close();

        std::cout << "Sample file created successfully." << std::endl;
    }

    // Wait a moment to ensure file system operations complete
    std::cout << "Waiting for file system operations to complete..." << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    // Check if the file exists again
    exists_result = vfs.file_exists(file_path);
    std::cout << "File exists check after creation: " << (exists_result.success() && exists_result.value() ? "Yes" : "No") << std::endl;

    // Get file info
    auto info_result = vfs.get_file_info(file_path);
    if (!info_result.success()) {
        std::cerr << "Error getting file info: " << info_result.error() << std::endl;

        // Try to cat the file anyway
        std::cout << "\nAttempting to cat the file anyway:" << std::endl;
        auto cat_result = vfs.cat_file(file_path);
        if (cat_result.success()) {
            std::cout << cat_result.value() << std::endl;
        } else {
            std::cerr << "Error reading file: " << cat_result.error() << std::endl;
        }

        return 1;
    }

    auto file_info = info_result.value();

    std::cout << "\nFile Information:" << std::endl;
    std::cout << "Path: " << file_info.path << std::endl;
    std::cout << "Size: " << file_info.size << " bytes" << std::endl;

    // Convert last_modified to string
    char time_str[100];
    std::strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", std::localtime(&file_info.last_modified));
    std::cout << "Last Modified: " << time_str << std::endl;

    // Display the file contents
    std::cout << "\nFile Contents:" << std::endl;
    std::cout << "==============" << std::endl;

    auto cat_result = vfs.cat_file(file_path);
    if (!cat_result.success()) {
        std::cerr << "Error reading file: " << cat_result.error() << std::endl;
        return 1;
    }

    std::cout << cat_result.value() << std::endl;

    return 0;
}
