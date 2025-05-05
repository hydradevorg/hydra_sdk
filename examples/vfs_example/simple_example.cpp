#include <iostream>
#include <string>
#include <memory>
#include <vector>
#include <hydra_vfs/vfs.h>
#include <hydra_vfs/memory_vfs.h>
#include <hydra_vfs/persistent_vfs.h>
#include <hydra_vfs/container_vfs.h>
#include <hydra_vfs/encrypted_vfs.h>
#include <hydra_crypto/kyber_aes.hpp>

// Helper function to print directory contents
void print_directory_contents(hydra::vfs::IVirtualFileSystem* vfs, const std::string& path) {
    std::cout << "Contents of directory '" << path << "':" << std::endl;
    
    auto result = vfs->list_directory(path);
    if (!result.success()) {
        std::cout << "  Error code: " << static_cast<int>(result.error()) << std::endl;
        return;
    }
    
    auto entries = result.value();
    if (entries.empty()) {
        std::cout << "  (empty directory)" << std::endl;
        return;
    }
    
    for (const auto& entry : entries) {
        std::string type = entry.is_directory ? "DIR" : "FILE";
        std::cout << "  " << type << "\t" << entry.name;
        
        if (!entry.is_directory) {
            std::cout << " (" << entry.size << " bytes)";
        }
        
        std::cout << std::endl;
    }
}

// Example of using the memory VFS
void memory_vfs_example() {
    std::cout << "==== Memory VFS Example ====" << std::endl;
    
    // Create an in-memory VFS
    auto memory_vfs = hydra::vfs::create_vfs();
    
    // Create a file
    std::cout << "Creating file..." << std::endl;
    auto create_result = memory_vfs->create_file("/test.txt");
    if (!create_result.success()) {
        std::cout << "Failed to create file: Error code " << static_cast<int>(create_result.error()) << std::endl;
        return;
    }
    
    // Write to the file
    std::cout << "Writing to file..." << std::endl;
    auto open_result = memory_vfs->open_file("/test.txt", hydra::vfs::FileMode::WRITE);
    if (!open_result.success()) {
        std::cout << "Failed to open file: Error code " << static_cast<int>(open_result.error()) << std::endl;
        return;
    }
    
    auto file = open_result.value();
    std::string data = "Hello, Memory VFS!";
    auto write_result = file->write(reinterpret_cast<const uint8_t*>(data.c_str()), data.size());
    if (!write_result.success()) {
        std::cout << "Failed to write to file: Error code " << static_cast<int>(write_result.error()) << std::endl;
        return;
    }
    
    std::cout << "Wrote " << write_result.value() << " bytes to file" << std::endl;
    file->close();
    
    // Create a directory
    std::cout << "Creating directory..." << std::endl;
    auto mkdir_result = memory_vfs->create_directory("/test_dir");
    if (!mkdir_result.success()) {
        std::cout << "Failed to create directory: Error code " << static_cast<int>(mkdir_result.error()) << std::endl;
        return;
    }
    
    // Create a file in the directory
    std::cout << "Creating file in directory..." << std::endl;
    create_result = memory_vfs->create_file("/test_dir/nested.txt");
    if (!create_result.success()) {
        std::cout << "Failed to create nested file: Error code " << static_cast<int>(create_result.error()) << std::endl;
        return;
    }
    
    // Write to the nested file
    std::cout << "Writing to nested file..." << std::endl;
    open_result = memory_vfs->open_file("/test_dir/nested.txt", hydra::vfs::FileMode::WRITE);
    if (!open_result.success()) {
        std::cout << "Failed to open nested file: Error code " << static_cast<int>(open_result.error()) << std::endl;
        return;
    }
    
    file = open_result.value();
    data = "Hello, Nested File!";
    write_result = file->write(reinterpret_cast<const uint8_t*>(data.c_str()), data.size());
    if (!write_result.success()) {
        std::cout << "Failed to write to nested file: Error code " << static_cast<int>(write_result.error()) << std::endl;
        return;
    }
    
    std::cout << "Wrote " << write_result.value() << " bytes to nested file" << std::endl;
    file->close();
    
    // List root directory
    print_directory_contents(memory_vfs.get(), "/");
    
    // List test_dir directory
    print_directory_contents(memory_vfs.get(), "/test_dir");
    
    std::cout << "Memory VFS example completed successfully!" << std::endl;
}

// Example of using the persistent VFS
void persistent_vfs_example() {
    std::cout << "\n==== Persistent VFS Example ====" << std::endl;
    
    // Create a persistent VFS
    const std::string data_dir = "./persistent_data";
    std::cout << "Creating persistent VFS at " << data_dir << std::endl;
    auto persistent_vfs = hydra::vfs::create_vfs(data_dir);
    
    // Create a file
    std::cout << "Creating file..." << std::endl;
    auto create_result = persistent_vfs->create_file("/persistent.txt");
    if (!create_result.success()) {
        std::cout << "Failed to create file: Error code " << static_cast<int>(create_result.error()) << std::endl;
        return;
    }
    
    // Write to the file
    std::cout << "Writing to file..." << std::endl;
    auto open_result = persistent_vfs->open_file("/persistent.txt", hydra::vfs::FileMode::WRITE);
    if (!open_result.success()) {
        std::cout << "Failed to open file: Error code " << static_cast<int>(open_result.error()) << std::endl;
        return;
    }
    
    auto file = open_result.value();
    std::string data = "This data will be persisted to disk!";
    auto write_result = file->write(reinterpret_cast<const uint8_t*>(data.c_str()), data.size());
    if (!write_result.success()) {
        std::cout << "Failed to write to file: Error code " << static_cast<int>(write_result.error()) << std::endl;
        return;
    }
    
    std::cout << "Wrote " << write_result.value() << " bytes to file" << std::endl;
    file->close();
    
    // List root directory
    print_directory_contents(persistent_vfs.get(), "/");
    
    std::cout << "Persistent VFS example completed successfully!" << std::endl;
    std::cout << "Data has been persisted to " << data_dir << std::endl;
}

// Example of using the container VFS with Kyber AES encryption
void container_vfs_example() {
    std::cout << "\n==== Container VFS with Kyber AES Example ====" << std::endl;
    
    // Create a container VFS
    const std::string container_path = "./container.dat";
    std::cout << "Creating container VFS with post-quantum encryption at " << container_path << std::endl;
    
    // We'll use the KyberAESEncryptionProvider's generate_keypair method
    // The key will be automatically generated by the provider
    hydra::vfs::EncryptionKey key = {};
    
    // Set resource limits
    hydra::vfs::ResourceLimits limits;
    limits.max_file_size = 1024 * 1024;     // 1MB
    limits.max_file_count = 100;
    limits.max_storage_size = 10 * 1024 * 1024;  // 10MB
    
    std::cout << "Using Kyber768 post-quantum encryption" << std::endl;
    
    // Create the container VFS with Kyber AES encryption
    auto container_vfs = hydra::vfs::create_container_vfs(
        container_path,
        key,  // Empty key will trigger automatic key generation
        nullptr,
        hydra::vfs::SecurityLevel::STANDARD,  // Use STANDARD security level
        limits
    );
    
    if (!container_vfs) {
        std::cout << "Failed to create container VFS" << std::endl;
        return;
    }
    
    // Create a file in the container
    std::cout << "Creating encrypted file..." << std::endl;
    auto create_result = container_vfs->create_file("/secret.txt");
    if (!create_result.success()) {
        std::cout << "Failed to create file: Error code " << static_cast<int>(create_result.error()) << std::endl;
        return;
    }
    
    // Write sensitive data to the file
    std::cout << "Writing sensitive data..." << std::endl;
    auto open_result = container_vfs->open_file("/secret.txt", hydra::vfs::FileMode::WRITE);
    if (!open_result.success()) {
        std::cout << "Failed to open file: Error code " << static_cast<int>(open_result.error()) << std::endl;
        return;
    }
    
    auto file = open_result.value();
    std::string sensitive_data = "TOP SECRET: This data will be encrypted!";
    auto write_result = file->write(reinterpret_cast<const uint8_t*>(sensitive_data.c_str()), sensitive_data.size());
    if (!write_result.success()) {
        std::cout << "Failed to write to file: Error code " << static_cast<int>(write_result.error()) << std::endl;
        return;
    }
    
    std::cout << "Wrote " << write_result.value() << " bytes of encrypted data" << std::endl;
    file->close();
    
    // List root directory
    print_directory_contents(container_vfs.get(), "/");
    
    // Read back the encrypted data
    std::cout << "Reading encrypted data..." << std::endl;
    open_result = container_vfs->open_file("/secret.txt", hydra::vfs::FileMode::READ);
    if (!open_result.success()) {
        std::cout << "Failed to open file for reading: Error code " << static_cast<int>(open_result.error()) << std::endl;
        return;
    }
    
    file = open_result.value();
    std::vector<uint8_t> buffer(100);
    auto read_result = file->read(buffer.data(), buffer.size());
    if (!read_result.success()) {
        std::cout << "Failed to read from file: Error code " << static_cast<int>(read_result.error()) << std::endl;
        return;
    }
    
    std::cout << "Read " << read_result.value() << " bytes of encrypted data" << std::endl;
    std::string read_data(reinterpret_cast<char*>(buffer.data()), read_result.value());
    std::cout << "Decrypted content: " << read_data << std::endl;
    file->close();
    
    std::cout << "Container VFS example completed successfully!" << std::endl;
    std::cout << "Encrypted data has been stored in " << container_path << std::endl;
}

// Example of using the KyberAESEncryptionProvider directly
void kyber_encrypted_vfs_example() {
    std::cout << "\n==== Kyber AES Encrypted VFS Example ====" << std::endl;
    
    // Create a memory VFS as the base
    auto memory_vfs = hydra::vfs::create_vfs();
    
    // Create a KyberAES encryption provider
    auto encryption_provider = std::make_shared<hydra::vfs::KyberAESEncryptionProvider>("Kyber768");
    
    // Generate a keypair
    std::cout << "Generating Kyber keypair..." << std::endl;
    auto keypair_result = encryption_provider->generate_keypair();
    if (!keypair_result.success()) {
        std::cout << "Failed to generate keypair: Error code " << static_cast<int>(keypair_result.error()) << std::endl;
        return;
    }
    
    auto [public_key, private_key] = keypair_result.value();
    std::cout << "Generated Kyber keypair successfully" << std::endl;
    std::cout << "Public key size: " << public_key.size() << " bytes" << std::endl;
    std::cout << "Private key size: " << private_key.size() << " bytes" << std::endl;
    
    // Convert private key to EncryptionKey format
    hydra::vfs::EncryptionKey encryption_key = {};
    size_t copy_size = std::min(private_key.size(), encryption_key.size());
    std::copy_n(private_key.begin(), copy_size, encryption_key.begin());
    
    // Create an encrypted VFS using the KyberAESEncryptionProvider
    auto encrypted_vfs = std::make_shared<hydra::vfs::EncryptedVFS>(
        memory_vfs,
        encryption_provider,
        encryption_key
    );
    
    // Create a file in the encrypted VFS
    std::cout << "Creating file in encrypted VFS..." << std::endl;
    auto create_result = encrypted_vfs->create_file("/quantum_secure.txt");
    if (!create_result.success()) {
        std::cout << "Failed to create file: Error code " << static_cast<int>(create_result.error()) << std::endl;
        return;
    }
    
    // Write data to the file
    std::cout << "Writing data with post-quantum encryption..." << std::endl;
    auto open_result = encrypted_vfs->open_file("/quantum_secure.txt", hydra::vfs::FileMode::WRITE);
    if (!open_result.success()) {
        std::cout << "Failed to open file: Error code " << static_cast<int>(open_result.error()) << std::endl;
        return;
    }
    
    auto file = open_result.value();
    std::string data = "This data is protected with post-quantum cryptography!";
    auto write_result = file->write(reinterpret_cast<const uint8_t*>(data.c_str()), data.size());
    if (!write_result.success()) {
        std::cout << "Failed to write to file: Error code " << static_cast<int>(write_result.error()) << std::endl;
        return;
    }
    
    std::cout << "Wrote " << write_result.value() << " bytes with Kyber AES encryption" << std::endl;
    file->close();
    
    // Read back the encrypted data
    std::cout << "Reading back the encrypted data..." << std::endl;
    open_result = encrypted_vfs->open_file("/quantum_secure.txt", hydra::vfs::FileMode::READ);
    if (!open_result.success()) {
        std::cout << "Failed to open file for reading: Error code " << static_cast<int>(open_result.error()) << std::endl;
        return;
    }
    
    file = open_result.value();
    std::vector<uint8_t> buffer(100);
    auto read_result = file->read(buffer.data(), buffer.size());
    if (!read_result.success()) {
        std::cout << "Failed to read from file: Error code " << static_cast<int>(read_result.error()) << std::endl;
        return;
    }
    
    std::cout << "Read " << read_result.value() << " bytes of encrypted data" << std::endl;
    std::string read_data(reinterpret_cast<char*>(buffer.data()), read_result.value());
    std::cout << "Decrypted content: " << read_data << std::endl;
    file->close();
    
    std::cout << "Kyber AES Encrypted VFS example completed successfully!" << std::endl;
}

int main() {
    // Seed the random number generator
    srand(static_cast<unsigned int>(time(nullptr)));
    
    std::cout << "Hydra VFS Simple Examples\n" << std::endl;
    
    // Run the memory VFS example
    memory_vfs_example();
    
    // Run the persistent VFS example
    persistent_vfs_example();
    
    // Run the container VFS example
    container_vfs_example();
    
    // Run the Kyber AES encrypted VFS example
    kyber_encrypted_vfs_example();
    
    std::cout << "\nAll examples completed!" << std::endl;
    return 0;
}