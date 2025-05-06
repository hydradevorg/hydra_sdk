#include "lmvs/p2p_vfs/p2p_vfs.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <chrono>

// Helper function to print file info
void printFileInfo(const hydra::vfs::FileInfo& info) {
    std::cout << "Path: " << info.path << std::endl;
    std::cout << "Size: " << info.size << " bytes" << std::endl;
    
    // Convert last_modified to string
    char time_str[100];
    std::strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", std::localtime(&info.last_modified));
    std::cout << "Last Modified: " << time_str << std::endl;
}

// Helper function to print a list of strings
void printList(const std::vector<std::string>& list) {
    for (const auto& item : list) {
        std::cout << "  - " << item << std::endl;
    }
}

int main() {
    std::cout << "P2P VFS Example using Layered Matrix and Vector System" << std::endl;
    std::cout << "====================================================" << std::endl;
    
    // Create two P2P VFS instances (simulating two nodes)
    std::string node1_id = "node1";
    std::string node2_id = "node2";
    std::string node1_storage = "./p2p_vfs_storage/node1";
    std::string node2_storage = "./p2p_vfs_storage/node2";
    
    lmvs::p2p_vfs::P2PVFS node1_vfs(node1_id, node1_storage);
    lmvs::p2p_vfs::P2PVFS node2_vfs(node2_id, node2_storage);
    
    // Add peers
    node1_vfs.add_peer(node2_id, "localhost:8002");
    node2_vfs.add_peer(node1_id, "localhost:8001");
    
    std::cout << "\n1. Created P2P VFS nodes:" << std::endl;
    std::cout << "   Node 1 ID: " << node1_id << std::endl;
    std::cout << "   Node 1 Storage: " << node1_storage << std::endl;
    std::cout << "   Node 1 Peers: ";
    printList(node1_vfs.get_peers());
    
    std::cout << "   Node 2 ID: " << node2_id << std::endl;
    std::cout << "   Node 2 Storage: " << node2_storage << std::endl;
    std::cout << "   Node 2 Peers: ";
    printList(node2_vfs.get_peers());
    
    // Create a directory on node 1
    std::string test_dir = "/test_dir";
    auto result = node1_vfs.create_directory(test_dir);
    
    std::cout << "\n2. Created directory on Node 1: " << test_dir << std::endl;
    std::cout << "   Success: " << (result.success() ? "Yes" : "No") << std::endl;
    
    // Create a file on node 1
    std::string test_file = "/test_dir/test_file.txt";
    std::string test_content = "This is a test file created by the P2P VFS using LMVS.";
    
    auto file_result = node1_vfs.open_file(test_file, hydra::vfs::FileMode::WRITE);
    if (file_result.success()) {
        auto file = file_result.value();
        file->write(reinterpret_cast<const uint8_t*>(test_content.c_str()), test_content.size());
        file->close();
        
        std::cout << "\n3. Created file on Node 1: " << test_file << std::endl;
        std::cout << "   Content: " << test_content << std::endl;
        
        // Get file info
        auto info_result = node1_vfs.get_file_info(test_file);
        if (info_result.success()) {
            std::cout << "   File Info:" << std::endl;
            printFileInfo(info_result.value());
        }
    }
    
    // Synchronize nodes
    std::cout << "\n4. Synchronizing nodes..." << std::endl;
    node1_vfs.synchronize();
    node2_vfs.synchronize();
    
    // Check if the file exists on node 2
    std::cout << "\n5. Checking if file exists on Node 2: " << test_file << std::endl;
    auto exists_result = node2_vfs.file_exists(test_file);
    if (exists_result.success()) {
        std::cout << "   File exists: " << (exists_result.value() ? "Yes" : "No") << std::endl;
        
        if (exists_result.value()) {
            // Read the file from node 2
            file_result = node2_vfs.open_file(test_file, hydra::vfs::FileMode::READ);
            if (file_result.success()) {
                auto file = file_result.value();
                std::vector<uint8_t> buffer(1024);
                auto read_result = file->read(buffer.data(), buffer.size());
                
                if (read_result.success()) {
                    size_t bytes_read = read_result.value();
                    std::string content(reinterpret_cast<char*>(buffer.data()), bytes_read);
                    
                    std::cout << "   Content read from Node 2: " << content << std::endl;
                    std::cout << "   Content matches: " << (content == test_content ? "Yes" : "No") << std::endl;
                }
                
                file->close();
            }
        }
    }
    
    // List files in the directory on both nodes
    std::cout << "\n6. Listing files in directory on both nodes: " << test_dir << std::endl;
    
    std::cout << "   Node 1 files:" << std::endl;
    auto list_result = node1_vfs.list_files(test_dir);
    if (list_result.success()) {
        printList(list_result.value());
    }
    
    std::cout << "   Node 2 files:" << std::endl;
    list_result = node2_vfs.list_files(test_dir);
    if (list_result.success()) {
        printList(list_result.value());
    }
    
    // Modify the file on node 2
    std::string modified_content = "This file was modified by Node 2.";
    file_result = node2_vfs.open_file(test_file, hydra::vfs::FileMode::WRITE);
    if (file_result.success()) {
        auto file = file_result.value();
        file->write(reinterpret_cast<const uint8_t*>(modified_content.c_str()), modified_content.size());
        file->close();
        
        std::cout << "\n7. Modified file on Node 2: " << test_file << std::endl;
        std::cout << "   New content: " << modified_content << std::endl;
    }
    
    // Synchronize nodes
    std::cout << "\n8. Synchronizing nodes again..." << std::endl;
    node1_vfs.synchronize();
    node2_vfs.synchronize();
    
    // Read the modified file from node 1
    std::cout << "\n9. Reading modified file from Node 1: " << test_file << std::endl;
    file_result = node1_vfs.open_file(test_file, hydra::vfs::FileMode::READ);
    if (file_result.success()) {
        auto file = file_result.value();
        std::vector<uint8_t> buffer(1024);
        auto read_result = file->read(buffer.data(), buffer.size());
        
        if (read_result.success()) {
            size_t bytes_read = read_result.value();
            std::string content(reinterpret_cast<char*>(buffer.data()), bytes_read);
            
            std::cout << "   Content read from Node 1: " << content << std::endl;
            std::cout << "   Content matches modified content: " << (content == modified_content ? "Yes" : "No") << std::endl;
        }
        
        file->close();
    }
    
    // Delete the file from node 1
    std::cout << "\n10. Deleting file from Node 1: " << test_file << std::endl;
    auto delete_result = node1_vfs.delete_file(test_file);
    if (delete_result.success()) {
        std::cout << "    File deleted: " << (delete_result.value() ? "Yes" : "No") << std::endl;
    }
    
    // Synchronize nodes
    std::cout << "\n11. Synchronizing nodes after deletion..." << std::endl;
    node1_vfs.synchronize();
    node2_vfs.synchronize();
    
    // Check if the file exists on both nodes
    std::cout << "\n12. Checking if file exists after deletion:" << std::endl;
    
    std::cout << "    Node 1: " << test_file << std::endl;
    exists_result = node1_vfs.file_exists(test_file);
    if (exists_result.success()) {
        std::cout << "    File exists: " << (exists_result.value() ? "Yes" : "No") << std::endl;
    }
    
    std::cout << "    Node 2: " << test_file << std::endl;
    exists_result = node2_vfs.file_exists(test_file);
    if (exists_result.success()) {
        std::cout << "    File exists: " << (exists_result.value() ? "Yes" : "No") << std::endl;
    }
    
    // Delete the directory from node 2
    std::cout << "\n13. Deleting directory from Node 2: " << test_dir << std::endl;
    auto delete_dir_result = node2_vfs.delete_directory(test_dir);
    if (delete_dir_result.success()) {
        std::cout << "    Directory deleted: " << (delete_dir_result.value() ? "Yes" : "No") << std::endl;
    }
    
    // Synchronize nodes
    std::cout << "\n14. Final synchronization..." << std::endl;
    node1_vfs.synchronize();
    node2_vfs.synchronize();
    
    // Check if the directory exists on both nodes
    std::cout << "\n15. Checking if directory exists after deletion:" << std::endl;
    
    std::cout << "    Node 1: " << test_dir << std::endl;
    auto dir_exists_result = node1_vfs.directory_exists(test_dir);
    if (dir_exists_result.success()) {
        std::cout << "    Directory exists: " << (dir_exists_result.value() ? "Yes" : "No") << std::endl;
    }
    
    std::cout << "    Node 2: " << test_dir << std::endl;
    dir_exists_result = node2_vfs.directory_exists(test_dir);
    if (dir_exists_result.success()) {
        std::cout << "    Directory exists: " << (dir_exists_result.value() ? "Yes" : "No") << std::endl;
    }
    
    std::cout << "\nP2P VFS Example Completed Successfully!" << std::endl;
    
    return 0;
}
