#include "lmvs/p2p_vfs/p2p_vfs.hpp"
#include "lmvs/security/secure_vector_transport.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include <fstream>
#include <sstream>

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

// Helper function to print a byte vector as hex
void printHex(const std::vector<uint8_t>& data, size_t max_bytes = 32) {
    std::cout << std::hex << std::setfill('0');
    for (size_t i = 0; i < std::min(data.size(), max_bytes); ++i) {
        std::cout << std::setw(2) << static_cast<int>(data[i]);
        if ((i + 1) % 16 == 0 && i + 1 < std::min(data.size(), max_bytes)) {
            std::cout << std::endl << "   ";
        } else if (i + 1 < std::min(data.size(), max_bytes)) {
            std::cout << " ";
        }
    }
    if (data.size() > max_bytes) {
        std::cout << "... (" << std::dec << data.size() << " bytes total)";
    }
    std::cout << std::dec;
}

int main() {
    std::cout << "P2P Secure VFS Example using Kyber, Falcon, and LMVS" << std::endl;
    std::cout << "====================================================" << std::endl;
    
    // Create two P2P VFS instances (simulating two nodes)
    std::string node1_id = "alice";
    std::string node2_id = "bob";
    std::string node1_storage = "./p2p_secure_vfs_storage/alice";
    std::string node2_storage = "./p2p_secure_vfs_storage/bob";
    
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
    std::string test_dir = "/secure_test_dir";
    auto result = node1_vfs.create_directory(test_dir);
    
    std::cout << "\n2. Created directory on Node 1: " << test_dir << std::endl;
    std::cout << "   Success: " << (result.success() ? "Yes" : "No") << std::endl;
    
    // Create a file on node 1 with secure content
    std::string test_file = "/secure_test_dir/secure_test_file.txt";
    std::string test_content = "This is a secure test file created by the P2P VFS using LMVS with Kyber and Falcon.";
    
    auto file_result = node1_vfs.open_file(test_file, hydra::vfs::FileMode::WRITE);
    if (file_result.success()) {
        auto file = file_result.value();
        file->write(reinterpret_cast<const uint8_t*>(test_content.c_str()), test_content.size());
        file->close();
        
        std::cout << "\n3. Created secure file on Node 1: " << test_file << std::endl;
        std::cout << "   Content: " << test_content << std::endl;
        
        // Get file info
        auto info_result = node1_vfs.get_file_info(test_file);
        if (info_result.success()) {
            std::cout << "   File Info:" << std::endl;
            printFileInfo(info_result.value());
        }
    }
    
    // Examine the raw file on disk to verify it's encrypted
    std::string local_path = node1_storage + test_file;
    std::ifstream raw_file(local_path, std::ios::binary);
    if (raw_file) {
        std::vector<uint8_t> raw_data((std::istreambuf_iterator<char>(raw_file)), std::istreambuf_iterator<char>());
        raw_file.close();
        
        std::cout << "\n4. Raw file data on disk (should be encrypted):" << std::endl;
        std::cout << "   ";
        printHex(raw_data);
        std::cout << std::endl;
        
        // Check if the raw data contains the plaintext (it shouldn't)
        std::string raw_str(raw_data.begin(), raw_data.end());
        bool contains_plaintext = raw_str.find(test_content) != std::string::npos;
        std::cout << "   Contains plaintext: " << (contains_plaintext ? "Yes (BAD!)" : "No (GOOD!)") << std::endl;
    }
    
    // Synchronize nodes
    std::cout << "\n5. Synchronizing nodes..." << std::endl;
    node1_vfs.synchronize();
    node2_vfs.synchronize();
    
    // Check if the file exists on node 2
    std::cout << "\n6. Checking if file exists on Node 2: " << test_file << std::endl;
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
    
    // Create a larger file with binary data
    std::string binary_file = "/secure_test_dir/binary_file.dat";
    std::vector<uint8_t> binary_data(4096);
    
    // Fill with pseudo-random data
    for (size_t i = 0; i < binary_data.size(); ++i) {
        binary_data[i] = static_cast<uint8_t>(i % 256);
    }
    
    file_result = node1_vfs.open_file(binary_file, hydra::vfs::FileMode::WRITE);
    if (file_result.success()) {
        auto file = file_result.value();
        file->write(binary_data.data(), binary_data.size());
        file->close();
        
        std::cout << "\n7. Created binary file on Node 1: " << binary_file << std::endl;
        std::cout << "   Size: " << binary_data.size() << " bytes" << std::endl;
        
        // Get file info
        auto info_result = node1_vfs.get_file_info(binary_file);
        if (info_result.success()) {
            std::cout << "   File Info:" << std::endl;
            printFileInfo(info_result.value());
        }
    }
    
    // Synchronize nodes
    std::cout << "\n8. Synchronizing nodes again..." << std::endl;
    node1_vfs.synchronize();
    node2_vfs.synchronize();
    
    // Read the binary file from node 2
    std::cout << "\n9. Reading binary file from Node 2: " << binary_file << std::endl;
    file_result = node2_vfs.open_file(binary_file, hydra::vfs::FileMode::READ);
    if (file_result.success()) {
        auto file = file_result.value();
        std::vector<uint8_t> read_data(4096);
        auto read_result = file->read(read_data.data(), read_data.size());
        
        if (read_result.success()) {
            size_t bytes_read = read_result.value();
            std::cout << "   Read " << bytes_read << " bytes" << std::endl;
            
            // Compare with original data
            bool data_matches = true;
            for (size_t i = 0; i < bytes_read; ++i) {
                if (read_data[i] != binary_data[i]) {
                    data_matches = false;
                    break;
                }
            }
            
            std::cout << "   Data integrity: " << (data_matches ? "Preserved" : "Compromised") << std::endl;
        }
        
        file->close();
    }
    
    // Delete the files and directory
    std::cout << "\n10. Cleaning up..." << std::endl;
    
    auto delete_result = node1_vfs.delete_file(test_file);
    std::cout << "   Deleted text file: " << (delete_result.success() && delete_result.value() ? "Yes" : "No") << std::endl;
    
    delete_result = node1_vfs.delete_file(binary_file);
    std::cout << "   Deleted binary file: " << (delete_result.success() && delete_result.value() ? "Yes" : "No") << std::endl;
    
    auto delete_dir_result = node1_vfs.delete_directory(test_dir);
    std::cout << "   Deleted directory: " << (delete_dir_result.success() && delete_dir_result.value() ? "Yes" : "No") << std::endl;
    
    std::cout << "\nP2P Secure VFS Example Completed Successfully!" << std::endl;
    
    return 0;
}
