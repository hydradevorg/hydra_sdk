#ifndef HYDRA_CONTAINER_VFS_H
#define HYDRA_CONTAINER_VFS_H

#include "vfs.h"
#include "encrypted_vfs.h"
#include <memory>
#include <vector>
#include <string>
#include <mutex>
#include <unordered_map>
#include <functional>
#include <chrono>

namespace hydra {
namespace vfs {

/**
 * @brief Security level for the container
 */
enum class SecurityLevel {
    STANDARD,       // Standard encryption
    HARDWARE_BACKED // Hardware-backed encryption (uses TPM/Secure Enclave if available)
};

/**
 * @brief Resource limits for the container
 */
struct ResourceLimits {
    size_t max_storage_size = 0;      // Maximum storage size in bytes (0 = unlimited)
    size_t max_memory_usage = 0;      // Maximum memory usage in bytes (0 = unlimited)
    size_t max_file_count = 0;        // Maximum number of files (0 = unlimited)
    size_t max_file_size = 0;         // Maximum file size in bytes (0 = unlimited)
    size_t max_directory_count = 0;   // Maximum number of directories (0 = unlimited)
    
    ResourceLimits() = default;
};

/**
 * @brief Container metadata for integrity verification
 */
struct ContainerMetadata {
    uint32_t version;                 // Container format version
    std::string container_id;         // Unique container ID
    std::string creator;              // Creator information
    uint64_t creation_time;           // Creation timestamp
    uint64_t last_modified_time;      // Last modified timestamp
    std::vector<uint8_t> integrity_hash; // Hash of the container contents for integrity verification
    
    ContainerMetadata();
    
    /**
     * @brief Deserialize container metadata from a buffer
     * 
     * @param buffer Buffer containing serialized data
     * @param offset Offset into the buffer, updated after deserialization
     * @return Result<ContainerMetadata> Deserialized metadata or error
     */
    static Result<ContainerMetadata> deserialize(const std::vector<uint8_t>& buffer, size_t& offset);
};

/**
 * @brief Container entry representing a file or directory in the container
 */
struct ContainerEntry {
    enum Type {
        FILE,
        DIRECTORY
    };
    
    Type type;
    std::string name;
    uint64_t size;
    uint64_t timestamp;
    uint64_t data_offset; // Only for files, offset in the data section
    std::vector<uint8_t> integrity_hash; // Hash of the file contents for integrity verification
    std::vector<std::shared_ptr<ContainerEntry>> children; // Only for directories
    std::weak_ptr<ContainerEntry> parent;
    
    ContainerEntry() 
        : type(DIRECTORY), size(0), timestamp(0), data_offset(0) {}
    
    ContainerEntry(Type type, const std::string& name)
        : type(type), name(name), size(0), timestamp(0), data_offset(0) {}
    
    /**
     * @brief Deserialize a container entry from a buffer
     * 
     * @param buffer Buffer containing serialized data
     * @param offset Offset into the buffer, updated after deserialization
     * @return Result<std::shared_ptr<ContainerEntry>> Deserialized entry or error
     */
    static Result<std::shared_ptr<ContainerEntry>> deserialize(const std::vector<uint8_t>& buffer, size_t& offset);
};

/**
 * @brief Container header for the encrypted container file
 */
struct ContainerHeader {
    static constexpr uint32_t MAGIC = 0x48595652; // "HYVR"
    static constexpr uint32_t VERSION = 1;
    
    uint32_t magic;
    uint32_t version;
    uint64_t metadata_offset;
    uint64_t metadata_size;
    uint64_t data_offset;
    uint64_t data_size;
    SecurityLevel security_level;
    uint64_t container_metadata_offset;
    uint64_t container_metadata_size;
    
    ContainerHeader()
        : magic(MAGIC), version(VERSION), 
          metadata_offset(0), metadata_size(0),
          data_offset(0), data_size(0),
          security_level(SecurityLevel::STANDARD),
          container_metadata_offset(0), container_metadata_size(0) {}
};

/**
 * @brief Interface for hardware security modules
 */
class IHardwareSecurityModule {
public:
    virtual ~IHardwareSecurityModule() = default;
    
    /**
     * @brief Check if hardware security is available
     * 
     * @return bool True if hardware security is available
     */
    virtual bool is_available() const = 0;
    
    /**
     * @brief Encrypt data using hardware security
     * 
     * @param data Data to encrypt
     * @param key Encryption key
     * @return Result<std::vector<uint8_t>> Encrypted data
     */
    virtual Result<std::vector<uint8_t>> encrypt(const std::vector<uint8_t>& data, const EncryptionKey& key) = 0;
    
    /**
     * @brief Decrypt data using hardware security
     * 
     * @param encrypted_data Encrypted data
     * @param key Encryption key
     * @return Result<std::vector<uint8_t>> Decrypted data
     */
    virtual Result<std::vector<uint8_t>> decrypt(const std::vector<uint8_t>& encrypted_data, const EncryptionKey& key) = 0;
    
    /**
     * @brief Generate a secure encryption key using hardware security
     * 
     * @return Result<EncryptionKey> Generated key
     */
    virtual Result<EncryptionKey> generate_key() = 0;
    
    /**
     * @brief Verify the integrity of data
     * 
     * @param data Data to verify
     * @param expected_hash Expected hash
     * @return Result<bool> True if integrity is verified
     */
    virtual Result<bool> verify_integrity(const std::vector<uint8_t>& data, const std::vector<uint8_t>& expected_hash) = 0;
    
    /**
     * @brief Calculate the integrity hash of data
     * 
     * @param data Data to hash
     * @return Result<std::vector<uint8_t>> Calculated hash
     */
    virtual Result<std::vector<uint8_t>> calculate_integrity_hash(const std::vector<uint8_t>& data) = 0;
};

/**
 * @brief Hardware security module implementation for macOS (using Secure Enclave)
 */
class MacOSSecurityModule : public IHardwareSecurityModule {
public:
    MacOSSecurityModule();
    ~MacOSSecurityModule() override;
    
    bool is_available() const override;
    Result<std::vector<uint8_t>> encrypt(const std::vector<uint8_t>& data, const EncryptionKey& key) override;
    Result<std::vector<uint8_t>> decrypt(const std::vector<uint8_t>& encrypted_data, const EncryptionKey& key) override;
    Result<EncryptionKey> generate_key() override;
    Result<bool> verify_integrity(const std::vector<uint8_t>& data, const std::vector<uint8_t>& expected_hash) override;
    Result<std::vector<uint8_t>> calculate_integrity_hash(const std::vector<uint8_t>& data) override;
    
private:
    // Implementation details for Secure Enclave access
    class Impl;
    std::unique_ptr<Impl> m_impl;
};

/**
 * @brief File implementation for files within the container
 */
class ContainerFile : public IFile {
public:
    ContainerFile(const std::string& path, FileMode mode, 
                  std::shared_ptr<ContainerEntry> entry,
                  std::shared_ptr<IFile> container_file,
                  std::shared_ptr<IEncryptionProvider> encryption_provider,
                  const EncryptionKey& key,
                  std::shared_ptr<IHardwareSecurityModule> hsm = nullptr);
    ~ContainerFile() override;
    
    Result<size_t> read(uint8_t* buffer, size_t size) override;
    Result<size_t> write(const uint8_t* buffer, size_t size) override;
    Result<void> seek(int64_t offset, int whence) override;
    Result<uint64_t> tell() override;
    Result<void> flush() override;
    Result<void> close() override;
    Result<FileInfo> get_info() const override;
    
    /**
     * @brief Check if the file is open and ready for I/O operations
     * 
     * @return bool True if the file is open, false otherwise
     */
    bool is_open() const { return m_is_open; }
    
private:
    std::string m_path;
    FileMode m_mode;
    std::shared_ptr<ContainerEntry> m_entry;
    std::shared_ptr<IFile> m_container_file;
    std::shared_ptr<IEncryptionProvider> m_encryption_provider;
    EncryptionKey m_key;
    std::shared_ptr<IHardwareSecurityModule> m_hsm;
    
    std::vector<uint8_t> m_buffer;
    size_t m_position;
    bool m_dirty;
    bool m_is_open;
    mutable std::mutex m_mutex;
    
    Result<void> load_content();
    Result<void> save_content();
    Result<void> verify_integrity();
    Result<void> update_integrity_hash();
};

/**
 * @brief Resource monitor for tracking and limiting resource usage
 */
class ResourceMonitor {
public:
    ResourceMonitor(const ResourceLimits& limits);
    
    /**
     * @brief Check if an operation would exceed resource limits
     * 
     * @param storage_delta Change in storage usage
     * @param memory_delta Change in memory usage
     * @param file_count_delta Change in file count
     * @param file_size Size of the file (for max file size check)
     * @return Result<void> Success or error if limits would be exceeded
     */
    Result<void> check_limits(int64_t storage_delta, int64_t memory_delta, int64_t file_count_delta, size_t file_size);
    
    /**
     * @brief Update current resource usage
     * 
     * @param storage_delta Change in storage usage
     * @param memory_delta Change in memory usage
     * @param file_count_delta Change in file count
     */
    void update_usage(int64_t storage_delta, int64_t memory_delta, int64_t file_count_delta);
    
    /**
     * @brief Get current resource usage
     * 
     * @return ResourceUsage Current usage
     */
    struct ResourceUsage {
        size_t storage_usage;
        size_t memory_usage;
        size_t file_count;
        size_t directory_count;
    };
    
    ResourceUsage get_usage() const;
    
private:
    ResourceLimits m_limits;
    size_t m_storage_usage;
    size_t m_memory_usage;
    size_t m_file_count;
    size_t m_directory_count;
    mutable std::mutex m_mutex;
};

// Forward declarations
class ContainerPathHandler;

/**
 * @brief Encrypted container virtual file system with Docker-like isolation
 * 
 * This VFS stores all files in a single encrypted container file with
 * hardware-level security and resource isolation.
 */
class ContainerVFS : public IVirtualFileSystem {
public:
    /**
     * @brief Create a new container VFS
     * 
     * @param container_path Path to the container file
     * @param encryption_provider Encryption provider to use
     * @param key Encryption key
     * @param base_vfs Base VFS to use for accessing the container file
     * @param security_level Security level for the container
     * @param resource_limits Resource limits for the container
     */
    ContainerVFS(const std::string& container_path,
                std::shared_ptr<IEncryptionProvider> encryption_provider,
                const EncryptionKey& key,
                std::shared_ptr<IVirtualFileSystem> base_vfs,
                SecurityLevel security_level = SecurityLevel::STANDARD,
                const ResourceLimits& resource_limits = {});
    
    ~ContainerVFS() override;
    
    // File operations
    Result<std::shared_ptr<IFile>> open_file(const std::string& path, FileMode mode) override;
    Result<void> create_file(const std::string& path) override;
    Result<void> delete_file(const std::string& path) override;
    Result<void> rename_file(const std::string& old_path, const std::string& new_path) override;
    Result<bool> file_exists(const std::string& path) override;
    Result<FileInfo> get_file_info(const std::string& path) override;
    
    // Directory operations
    Result<void> create_directory(const std::string& path) override;
    Result<void> delete_directory(const std::string& path, bool recursive = false) override;
    Result<std::vector<FileInfo>> list_directory(const std::string& path) override;
    Result<bool> directory_exists(const std::string& path) override;
    
    // Path operations
    std::string normalize_path(const std::string& path) const override;
    std::string join_paths(const std::string& base, const std::string& relative) const override;
    std::string get_parent_path(const std::string& path) const override;
    std::string get_filename(const std::string& path) const override;
    
    // Mount operations
    Result<void> mount(const std::string& mount_point, std::shared_ptr<IVirtualFileSystem> fs) override;
    Result<void> unmount(const std::string& mount_point) override;
    
    /**
     * @brief Save metadata to container file
     * 
     * @param during_initialization If true, bypass the initialization check
     * @return Result<void> Success or error code
     */
    Result<void> save_metadata(bool during_initialization = false);
    
    /**
     * @brief Load the container metadata from the container file
     * 
     * @param bypass_init_check If true, bypass the initialization check
     * @return Result<void> Success or error
     */
    Result<void> load_metadata(bool bypass_init_check = false);
    
    /**
     * @brief Verify the integrity of the entire container
     * 
     * @return Result<bool> True if integrity is verified
     */
    Result<bool> verify_integrity();
    
    /**
     * @brief Get the current resource usage of the container
     * 
     * @return ResourceMonitor::ResourceUsage Current usage
     */
    ResourceMonitor::ResourceUsage get_resource_usage() const;
    
    /**
     * @brief Set a callback to be called when resource usage changes
     * 
     * @param callback Callback function
     */
    void set_resource_callback(std::function<void(const ResourceMonitor::ResourceUsage&)> callback);
    
    /**
     * @brief Get the container metadata
     * 
     * @return const ContainerMetadata& Container metadata
     */
    const ContainerMetadata& get_container_metadata() const;
    
private:
    std::shared_ptr<ContainerPathHandler> m_container_path_handler;
    std::shared_ptr<IEncryptionProvider> m_encryption_provider;
    EncryptionKey m_key;
    std::shared_ptr<IVirtualFileSystem> m_base_vfs;
    std::shared_ptr<IFile> m_container_file;
    ContainerHeader m_header;
    ContainerMetadata m_metadata;
    std::shared_ptr<ContainerEntry> m_root;
    std::unordered_map<std::string, std::shared_ptr<ContainerEntry>> m_path_cache;
    std::shared_ptr<IHardwareSecurityModule> m_hsm;
    ResourceMonitor m_resource_monitor;
    std::function<void(const ResourceMonitor::ResourceUsage&)> m_resource_callback;
    mutable std::mutex m_mutex;
    bool m_initialized;
    
    Result<std::shared_ptr<ContainerEntry>> get_entry(const std::string& path, bool create_dirs = false);
    Result<std::shared_ptr<ContainerEntry>> create_entry(const std::string& path, ContainerEntry::Type type);
    Result<void> delete_entry(const std::string& path, bool recursive = false);
    Result<void> rename_entry(const std::string& old_path, const std::string& new_path);
    Result<void> initialize_container();
    Result<void> serialize_entry(std::vector<uint8_t>& buffer, const std::shared_ptr<ContainerEntry>& entry);
    Result<void> deserialize_entry(const uint8_t* buffer, size_t& offset, size_t size, std::shared_ptr<ContainerEntry>& entry, std::shared_ptr<ContainerEntry> parent);
    Result<void> update_container_integrity_hash();
    Result<std::shared_ptr<IHardwareSecurityModule>> create_hardware_security_module();
    void rebuild_path_cache();
    Result<std::vector<uint8_t>> calculate_container_hash();
};

/**
 * @brief Factory function to create a new container virtual file system
 * 
 * @param container_path Path to the container file
 * @param key Encryption key (if empty, a random key will be generated)
 * @param base_vfs Base VFS to use for accessing the container file (if nullptr, a new PersistentVFS will be created)
 * @param security_level Security level for the container
 * @param resource_limits Resource limits for the container
 * @return std::shared_ptr<IVirtualFileSystem> A new container VFS instance
 */
std::shared_ptr<IVirtualFileSystem> create_container_vfs(
    const std::string& container_path,
    const EncryptionKey& key = {},
    std::shared_ptr<IVirtualFileSystem> base_vfs = nullptr,
    SecurityLevel security_level = SecurityLevel::STANDARD,
    const ResourceLimits& resource_limits = {});

} // namespace vfs
} // namespace hydra

#endif // HYDRA_CONTAINER_VFS_H
