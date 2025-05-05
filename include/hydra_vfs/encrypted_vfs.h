#ifndef HYDRA_ENCRYPTED_VFS_H
#define HYDRA_ENCRYPTED_VFS_H

#include "vfs.h"
#include <memory>
#include <vector>
#include <string>
#include <mutex>
#include <array>

namespace hydra {
namespace vfs {

/**
 * @brief Encryption key for encrypted files
 */
using EncryptionKey = std::array<uint8_t, 32>; // 256-bit key

/**
 * @brief Encryption provider interface
 */
class IEncryptionProvider {
public:
    virtual ~IEncryptionProvider() = default;
    
    /**
     * @brief Encrypt data using the provided key
     * 
     * @param data Data to encrypt
     * @param key Encryption key
     * @return Result<std::vector<uint8_t>> Encrypted data
     */
    virtual Result<std::vector<uint8_t>> encrypt(const std::vector<uint8_t>& data, const EncryptionKey& key) = 0;
    
    /**
     * @brief Decrypt data using the provided key
     * 
     * @param encrypted_data Encrypted data
     * @param key Encryption key
     * @return Result<std::vector<uint8_t>> Decrypted data
     */
    virtual Result<std::vector<uint8_t>> decrypt(const std::vector<uint8_t>& encrypted_data, const EncryptionKey& key) = 0;
};

/**
 * @brief Simple XOR encryption provider
 * 
 * Note: This is a simple implementation for demonstration purposes.
 * In a real-world scenario, use a proper cryptographic library.
 */
class XOREncryptionProvider : public IEncryptionProvider {
public:
    Result<std::vector<uint8_t>> encrypt(const std::vector<uint8_t>& data, const EncryptionKey& key) override;
    Result<std::vector<uint8_t>> decrypt(const std::vector<uint8_t>& encrypted_data, const EncryptionKey& key) override;
};

/**
 * @brief AES-256 encryption provider using OpenSSL
 * 
 * This implementation uses AES-256 in CBC mode with PKCS7 padding.
 * The first 16 bytes of the encrypted data are the IV (Initialization Vector).
 */
class AESEncryptionProvider : public IEncryptionProvider {
public:
    Result<std::vector<uint8_t>> encrypt(const std::vector<uint8_t>& data, const EncryptionKey& key) override;
    Result<std::vector<uint8_t>> decrypt(const std::vector<uint8_t>& encrypted_data, const EncryptionKey& key) override;
    
private:
    static constexpr size_t IV_SIZE = 16; // AES block size
};

/**
 * @brief Post-quantum secure encryption provider using Kyber KEM and AES-GCM
 * 
 * This implementation uses the Kyber key encapsulation mechanism (KEM) for key exchange
 * and AES-GCM for authenticated encryption of data.
 */
class KyberAESEncryptionProvider : public IEncryptionProvider {
public:
    KyberAESEncryptionProvider(const std::string& kyber_mode = "Kyber768");
    ~KyberAESEncryptionProvider();
    
    Result<std::vector<uint8_t>> encrypt(const std::vector<uint8_t>& data, const EncryptionKey& key) override;
    Result<std::vector<uint8_t>> decrypt(const std::vector<uint8_t>& encrypted_data, const EncryptionKey& key) override;
    
    /**
     * @brief Generate a new Kyber key pair
     * 
     * @return Result<std::pair<std::vector<uint8_t>, std::vector<uint8_t>>> A pair containing {public_key, private_key}
     */
    Result<std::pair<std::vector<uint8_t>, std::vector<uint8_t>>> generate_keypair();
    
private:
    class Impl;
    std::unique_ptr<Impl> m_impl;
};

/**
 * @brief Encrypted file implementation
 */
class EncryptedFile : public IFile {
public:
    EncryptedFile(const std::string& path, FileMode mode, std::shared_ptr<IFile> base_file, 
                  std::shared_ptr<IEncryptionProvider> encryption_provider, const EncryptionKey& key);
    ~EncryptedFile() override;
    
    Result<size_t> read(uint8_t* buffer, size_t size) override;
    Result<size_t> write(const uint8_t* buffer, size_t size) override;
    Result<void> seek(int64_t offset, int whence) override;
    Result<uint64_t> tell() override;
    Result<void> flush() override;
    Result<void> close() override;
    Result<FileInfo> get_info() const override;
    
private:
    std::string m_path;
    FileMode m_mode;
    std::shared_ptr<IFile> m_base_file;
    std::shared_ptr<IEncryptionProvider> m_encryption_provider;
    EncryptionKey m_key;
    std::vector<uint8_t> m_buffer;
    size_t m_position;
    bool m_dirty;
    bool m_is_open;
    bool m_decryption_failed;
    mutable std::mutex m_mutex;
    
    Result<void> load_content();
    Result<void> save_content();
};

/**
 * @brief Encrypted Virtual File System decorator
 * 
 * This class wraps another VFS implementation and provides encryption/decryption
 * for all file operations.
 */
class EncryptedVFS : public IVirtualFileSystem {
public:
    /**
     * @brief Construct a new Encrypted VFS
     * 
     * @param base_vfs Base VFS implementation to wrap
     * @param encryption_provider Encryption provider to use
     * @param key Encryption key
     */
    EncryptedVFS(std::shared_ptr<IVirtualFileSystem> base_vfs, 
                 std::shared_ptr<IEncryptionProvider> encryption_provider,
                 const EncryptionKey& key);
    
    ~EncryptedVFS() override;
    
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
    
private:
    std::shared_ptr<IVirtualFileSystem> m_base_vfs;
    std::shared_ptr<IEncryptionProvider> m_encryption_provider;
    EncryptionKey m_key;
    mutable std::mutex m_mutex;
};

/**
 * @brief Factory function to create a new encrypted virtual file system with AES encryption
 * 
 * @param base_vfs Base VFS implementation to wrap
 * @param key Encryption key (if empty, a random key will be generated)
 * @return std::shared_ptr<IVirtualFileSystem> A new encrypted VFS instance
 */
std::shared_ptr<IVirtualFileSystem> create_encrypted_vfs(
    std::shared_ptr<IVirtualFileSystem> base_vfs,
    const EncryptionKey& key = {});

/**
 * @brief Factory function to create a new encrypted virtual file system with Kyber AES encryption
 * 
 * This function creates an encrypted VFS using post-quantum secure Kyber KEM for key exchange
 * and AES-GCM for authenticated encryption of data.
 * 
 * @param base_vfs Base VFS implementation to wrap
 * @param key Encryption key (if empty, a new keypair will be generated)
 * @param kyber_mode Kyber security mode to use (Kyber512, Kyber768, or Kyber1024)
 * @return std::shared_ptr<IVirtualFileSystem> A new encrypted VFS instance
 */
std::shared_ptr<IVirtualFileSystem> create_kyber_encrypted_vfs(
    std::shared_ptr<IVirtualFileSystem> base_vfs,
    const std::vector<uint8_t>& key = {},
    const std::string& kyber_mode = "Kyber768");

} // namespace vfs
} // namespace hydra

#endif // HYDRA_ENCRYPTED_VFS_H
