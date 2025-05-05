#include "../../include/hydra_vfs/encrypted_vfs.h"
#include <algorithm>
#include <random>
#include <cstring>
#include <iostream>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/err.h>
#include <hydra_crypto/kyber_aes.hpp>

// Define constants
namespace {
    constexpr size_t AES_IV_SIZE = 16; // AES block size
}

namespace hydra {
namespace vfs {

// XOREncryptionProvider implementation
Result<std::vector<uint8_t>> XOREncryptionProvider::encrypt(const std::vector<uint8_t>& data, const EncryptionKey& key) {
    std::vector<uint8_t> encrypted(data.size());
    
    for (size_t i = 0; i < data.size(); ++i) {
        encrypted[i] = data[i] ^ key[i % key.size()];
    }
    
    return encrypted;
}

Result<std::vector<uint8_t>> XOREncryptionProvider::decrypt(const std::vector<uint8_t>& encrypted_data, const EncryptionKey& key) {
    // XOR encryption is symmetric, so decryption is the same as encryption
    return encrypt(encrypted_data, key);
}

// AESEncryptionProvider implementation
Result<std::vector<uint8_t>> AESEncryptionProvider::encrypt(const std::vector<uint8_t>& data, const EncryptionKey& key) {
    // Create a buffer for the encrypted data (includes IV at the beginning)
    std::vector<uint8_t> encrypted(data.size() + AES_IV_SIZE + EVP_MAX_BLOCK_LENGTH);
    
    // Generate random IV
    if (RAND_bytes(encrypted.data(), AES_IV_SIZE) != 1) {
        return ErrorCode::IO_ERROR;
    }
    
    // Create and initialize the context
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        return ErrorCode::IO_ERROR;
    }
    
    // Initialize the encryption operation with AES-256-CBC
    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, key.data(), encrypted.data()) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return ErrorCode::IO_ERROR;
    }
    
    int out_len = 0;
    int final_len = 0;
    
    // Encrypt the data
    if (EVP_EncryptUpdate(ctx, encrypted.data() + AES_IV_SIZE, &out_len, 
                         data.data(), static_cast<int>(data.size())) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return ErrorCode::IO_ERROR;
    }
    
    // Finalize the encryption
    if (EVP_EncryptFinal_ex(ctx, encrypted.data() + AES_IV_SIZE + out_len, &final_len) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return ErrorCode::IO_ERROR;
    }
    
    // Clean up
    EVP_CIPHER_CTX_free(ctx);
    
    // Resize the encrypted data to the actual size
    encrypted.resize(AES_IV_SIZE + out_len + final_len);
    
    return encrypted;
}

Result<std::vector<uint8_t>> AESEncryptionProvider::decrypt(const std::vector<uint8_t>& encrypted_data, const EncryptionKey& key) {
    // Check if the encrypted data is large enough to contain the IV
    if (encrypted_data.size() <= AES_IV_SIZE) {
        return ErrorCode::INVALID_ARGUMENT;
    }
    
    // Create a buffer for the decrypted data
    std::vector<uint8_t> decrypted(encrypted_data.size() - AES_IV_SIZE);
    
    // Create and initialize the context
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        return ErrorCode::IO_ERROR;
    }
    
    // Initialize the decryption operation with AES-256-CBC
    if (EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, key.data(), encrypted_data.data()) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return ErrorCode::IO_ERROR;
    }
    
    int out_len = 0;
    int final_len = 0;
    
    // Decrypt the data
    if (EVP_DecryptUpdate(ctx, decrypted.data(), &out_len, 
                         encrypted_data.data() + AES_IV_SIZE, 
                         static_cast<int>(encrypted_data.size() - AES_IV_SIZE)) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return ErrorCode::IO_ERROR;
    }
    
    // Finalize the decryption
    if (EVP_DecryptFinal_ex(ctx, decrypted.data() + out_len, &final_len) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return ErrorCode::IO_ERROR;
    }
    
    // Clean up
    EVP_CIPHER_CTX_free(ctx);
    
    // Resize the decrypted data to the actual size
    decrypted.resize(out_len + final_len);
    
    return decrypted;
}

// EncryptedFile implementation
EncryptedFile::EncryptedFile(const std::string& path, FileMode mode, std::shared_ptr<IFile> base_file, 
                             std::shared_ptr<IEncryptionProvider> encryption_provider, const EncryptionKey& key)
    : m_path(path), m_mode(mode), m_base_file(base_file), 
      m_encryption_provider(encryption_provider), m_key(key), 
      m_position(0), m_dirty(false), m_is_open(true), m_decryption_failed(false) {
    
    // Load the content if we're opening for read or append
    if (mode == FileMode::READ || mode == FileMode::APPEND) {
        load_content();
    }
}

EncryptedFile::~EncryptedFile() {
    close();
}

Result<void> EncryptedFile::load_content() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (!m_is_open) {
        return ErrorCode::IO_ERROR;
    }
    
    if (!m_buffer.empty()) {
        // Already loaded
        return Result<void>();
    }
    
    // Get the file size
    auto info_result = m_base_file->get_info();
    if (!info_result.success()) {
        return info_result.error();
    }
    
    auto file_info = info_result.value();
    size_t file_size = file_info.size;
    
    if (file_size == 0) {
        // Empty file, nothing to load
        m_buffer.clear();
        return Result<void>();
    }
    
    // Read the encrypted content
    std::vector<uint8_t> encrypted_content(file_size);
    auto read_result = m_base_file->read(encrypted_content.data(), file_size);
    if (!read_result.success()) {
        return read_result.error();
    }
    
    size_t bytes_read = read_result.value();
    if (bytes_read != file_size) {
        return ErrorCode::IO_ERROR;
    }
    
    // Resize the buffer to the actual size
    encrypted_content.resize(bytes_read);
    
    // Decrypt the content
    auto decrypt_result = m_encryption_provider->decrypt(encrypted_content, m_key);
    if (!decrypt_result.success()) {
        m_decryption_failed = true;
        return decrypt_result.error();
    }
    
    m_buffer = decrypt_result.value();
    
    // If we're in append mode, set the position to the end
    if (m_mode == FileMode::APPEND) {
        m_position = m_buffer.size();
    } else {
        m_position = 0;
    }
    
    return Result<void>();
}

Result<void> EncryptedFile::save_content() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (!m_is_open) {
        return ErrorCode::IO_ERROR;
    }
    
    if (!m_dirty) {
        return Result<void>();
    }
    
    // Encrypt the content
    auto encrypt_result = m_encryption_provider->encrypt(m_buffer, m_key);
    if (!encrypt_result.success()) {
        return encrypt_result.error();
    }
    
    auto encrypted_content = encrypt_result.value();
    
    // Seek to the beginning of the file
    auto seek_result = m_base_file->seek(0, SEEK_SET);
    if (!seek_result.success()) {
        return seek_result.error();
    }
    
    // Write the encrypted content
    auto write_result = m_base_file->write(encrypted_content.data(), encrypted_content.size());
    if (!write_result.success()) {
        return write_result.error();
    }
    
    size_t bytes_written = write_result.value();
    if (bytes_written != encrypted_content.size()) {
        return ErrorCode::IO_ERROR;
    }
    
    // Truncate the file if needed
    auto file_info_result = m_base_file->get_info();
    if (!file_info_result.success()) {
        return file_info_result.error();
    }
    
    auto file_info = file_info_result.value();
    if (file_info.size > encrypted_content.size()) {
        // TODO: Implement truncate functionality
        // For now, we'll just leave the extra data at the end of the file
    }
    
    m_dirty = false;
    
    return Result<void>();
}

Result<size_t> EncryptedFile::read(uint8_t* buffer, size_t size) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (!m_is_open) {
        return ErrorCode::IO_ERROR;
    }
    
    if (m_mode != FileMode::READ && m_mode != FileMode::READ_WRITE) {
        return ErrorCode::PERMISSION_DENIED;
    }
    
    if (m_buffer.empty()) {
        auto load_result = load_content();
        if (!load_result.success()) {
            return load_result.error();
        }
    }
    
    if (m_decryption_failed) {
        return ErrorCode::IO_ERROR;
    }
    
    // Check if we're at the end of the file
    if (m_position >= m_buffer.size()) {
        return 0;
    }
    
    // Calculate how many bytes we can read
    size_t bytes_to_read = std::min(size, m_buffer.size() - m_position);
    
    // Copy the data to the buffer
    std::copy(m_buffer.begin() + m_position, m_buffer.begin() + m_position + bytes_to_read, buffer);
    
    // Update the position
    m_position += bytes_to_read;
    
    return bytes_to_read;
}

Result<size_t> EncryptedFile::write(const uint8_t* buffer, size_t size) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (!m_is_open) {
        return ErrorCode::IO_ERROR;
    }
    
    if (m_mode != FileMode::WRITE && m_mode != FileMode::READ_WRITE && m_mode != FileMode::APPEND) {
        return ErrorCode::PERMISSION_DENIED;
    }
    
    if (m_buffer.empty()) {
        auto load_result = load_content();
        if (!load_result.success() && m_mode != FileMode::WRITE) {
            // For read/write or append mode, we need the existing content
            return load_result.error();
        }
    }
    
    // Resize the buffer if needed
    if (m_position + size > m_buffer.size()) {
        m_buffer.resize(m_position + size);
    }
    
    // Copy the data from the buffer
    std::copy(buffer, buffer + size, m_buffer.begin() + m_position);
    
    // Update the position and mark as dirty
    m_position += size;
    m_dirty = true;
    
    return size;
}

Result<void> EncryptedFile::seek(int64_t offset, int whence) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (!m_is_open) {
        return ErrorCode::IO_ERROR;
    }
    
    if (m_buffer.empty()) {
        auto load_result = load_content();
        if (!load_result.success()) {
            return load_result.error();
        }
    }
    
    int64_t new_position = 0;
    
    switch (whence) {
        case SEEK_SET:
            new_position = offset;
            break;
        case SEEK_CUR:
            new_position = m_position + offset;
            break;
        case SEEK_END:
            new_position = m_buffer.size() + offset;
            break;
        default:
            return ErrorCode::INVALID_ARGUMENT;
    }
    
    if (new_position < 0) {
        return ErrorCode::INVALID_ARGUMENT;
    }
    
    m_position = static_cast<size_t>(new_position);
    
    return Result<void>();
}

Result<uint64_t> EncryptedFile::tell() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (!m_is_open) {
        return ErrorCode::IO_ERROR;
    }
    
    return m_position;
}

Result<void> EncryptedFile::flush() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (!m_is_open) {
        return ErrorCode::IO_ERROR;
    }
    
    return save_content();
}

Result<void> EncryptedFile::close() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (!m_is_open) {
        return Result<void>();
    }
    
    // Save the content if dirty
    if (m_dirty) {
        auto save_result = save_content();
        if (!save_result.success()) {
            return save_result.error();
        }
    }
    
    // Close the base file
    auto close_result = m_base_file->close();
    if (!close_result.success()) {
        return close_result.error();
    }
    
    // Clear sensitive data
    m_buffer.clear();
    m_is_open = false;
    m_dirty = false;
    
    return Result<void>();
}

Result<FileInfo> EncryptedFile::get_info() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (!m_is_open) {
        return ErrorCode::IO_ERROR;
    }
    
    auto info_result = m_base_file->get_info();
    if (!info_result.success()) {
        return info_result.error();
    }
    
    auto file_info = info_result.value();
    
    // If the buffer is not empty, update the size to reflect the decrypted size
    if (!m_buffer.empty()) {
        file_info.size = m_buffer.size();
    } else {
        // Otherwise, we need to estimate the decrypted size
        // This is not accurate, but it's the best we can do without decrypting
        if (file_info.size > AES_IV_SIZE) {
            file_info.size -= AES_IV_SIZE;
        }
    }
    
    return file_info;
}

// EncryptedVFS implementation
EncryptedVFS::EncryptedVFS(std::shared_ptr<IVirtualFileSystem> base_vfs, 
                           std::shared_ptr<IEncryptionProvider> encryption_provider,
                           const EncryptionKey& key)
    : m_base_vfs(base_vfs), m_encryption_provider(encryption_provider), m_key(key) {
}

EncryptedVFS::~EncryptedVFS() {
}

Result<std::shared_ptr<IFile>> EncryptedVFS::open_file(const std::string& path, FileMode mode) {
    auto result = m_base_vfs->open_file(path, mode);
    if (!result.success()) {
        return result.error();
    }
    
    auto base_file = result.value();
    std::shared_ptr<IFile> encrypted_file = std::make_shared<EncryptedFile>(path, mode, base_file, m_encryption_provider, m_key);
    return Result<std::shared_ptr<IFile>>(encrypted_file);
}

Result<void> EncryptedVFS::create_file(const std::string& path) {
    return m_base_vfs->create_file(path);
}

Result<void> EncryptedVFS::delete_file(const std::string& path) {
    return m_base_vfs->delete_file(path);
}

Result<void> EncryptedVFS::rename_file(const std::string& old_path, const std::string& new_path) {
    return m_base_vfs->rename_file(old_path, new_path);
}

Result<bool> EncryptedVFS::file_exists(const std::string& path) {
    return m_base_vfs->file_exists(path);
}

Result<FileInfo> EncryptedVFS::get_file_info(const std::string& path) {
    return m_base_vfs->get_file_info(path);
}

Result<void> EncryptedVFS::create_directory(const std::string& path) {
    return m_base_vfs->create_directory(path);
}

Result<void> EncryptedVFS::delete_directory(const std::string& path, bool recursive) {
    return m_base_vfs->delete_directory(path, recursive);
}

Result<std::vector<FileInfo>> EncryptedVFS::list_directory(const std::string& path) {
    return m_base_vfs->list_directory(path);
}

Result<bool> EncryptedVFS::directory_exists(const std::string& path) {
    return m_base_vfs->directory_exists(path);
}

std::string EncryptedVFS::normalize_path(const std::string& path) const {
    return m_base_vfs->normalize_path(path);
}

std::string EncryptedVFS::join_paths(const std::string& base, const std::string& relative) const {
    return m_base_vfs->join_paths(base, relative);
}

std::string EncryptedVFS::get_parent_path(const std::string& path) const {
    return m_base_vfs->get_parent_path(path);
}

std::string EncryptedVFS::get_filename(const std::string& path) const {
    return m_base_vfs->get_filename(path);
}

Result<void> EncryptedVFS::mount(const std::string& mount_point, std::shared_ptr<IVirtualFileSystem> fs) {
    return m_base_vfs->mount(mount_point, fs);
}

Result<void> EncryptedVFS::unmount(const std::string& mount_point) {
    return m_base_vfs->unmount(mount_point);
}

// Factory function
std::shared_ptr<IVirtualFileSystem> create_encrypted_vfs(
    std::shared_ptr<IVirtualFileSystem> base_vfs,
    const EncryptionKey& key) {
    
    // Create an AES encryption provider by default
    auto encryption_provider = std::make_shared<AESEncryptionProvider>();
    
    // Create and return a new EncryptedVFS instance
    return std::make_shared<EncryptedVFS>(base_vfs, encryption_provider, key);
}

// KyberAESEncryptionProvider implementation
class KyberAESEncryptionProvider::Impl {
public:
    Impl(const std::string& kyber_mode) : m_kyber_aes(kyber_mode) {
        // Generate a keypair on initialization
        try {
            m_keypair = m_kyber_aes.generate_keypair();
            std::cout << "DEBUG: Generated Kyber keypair successfully" << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "DEBUG: Failed to generate Kyber keypair: " << e.what() << std::endl;
        }
    }
    
    Result<std::vector<uint8_t>> encrypt(const std::vector<uint8_t>& data, const EncryptionKey& key) {
        try {
            // Use AES for simplicity in this example
            // In a real implementation, we would use Kyber for key exchange and AES for encryption
            
            // Create a buffer for the encrypted data (includes IV at the beginning)
            std::vector<uint8_t> encrypted(data.size() + AES_IV_SIZE + EVP_MAX_BLOCK_LENGTH);
            
            // Generate random IV
            if (RAND_bytes(encrypted.data(), AES_IV_SIZE) != 1) {
                return ErrorCode::IO_ERROR;
            }
            
            // Create and initialize the context
            EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
            if (!ctx) {
                return ErrorCode::IO_ERROR;
            }
            
            // Initialize the encryption operation with AES-256-CBC
            if (EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, key.data(), encrypted.data()) != 1) {
                EVP_CIPHER_CTX_free(ctx);
                return ErrorCode::IO_ERROR;
            }
            
            int out_len = 0;
            int final_len = 0;
            
            // Encrypt the data
            if (EVP_EncryptUpdate(ctx, encrypted.data() + AES_IV_SIZE, &out_len, 
                                 data.data(), static_cast<int>(data.size())) != 1) {
                EVP_CIPHER_CTX_free(ctx);
                return ErrorCode::IO_ERROR;
            }
            
            // Finalize the encryption
            if (EVP_EncryptFinal_ex(ctx, encrypted.data() + AES_IV_SIZE + out_len, &final_len) != 1) {
                EVP_CIPHER_CTX_free(ctx);
                return ErrorCode::IO_ERROR;
            }
            
            // Clean up
            EVP_CIPHER_CTX_free(ctx);
            
            // Resize the encrypted data to the actual size
            encrypted.resize(AES_IV_SIZE + out_len + final_len);
            
            return encrypted;
        } catch (const std::exception& e) {
            std::cerr << "KyberAES encryption error: " << e.what() << std::endl;
            return Result<std::vector<uint8_t>>(ErrorCode::IO_ERROR);
        }
    }
    
    Result<std::vector<uint8_t>> decrypt(const std::vector<uint8_t>& encrypted_data, const EncryptionKey& key) {
        try {
            // Use AES for simplicity in this example
            // In a real implementation, we would use Kyber for key exchange and AES for decryption
            
            // Check if the encrypted data is large enough to contain the IV
            if (encrypted_data.size() <= AES_IV_SIZE) {
                return ErrorCode::INVALID_ARGUMENT;
            }
            
            // Create a buffer for the decrypted data
            std::vector<uint8_t> decrypted(encrypted_data.size() - AES_IV_SIZE);
            
            // Create and initialize the context
            EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
            if (!ctx) {
                return ErrorCode::IO_ERROR;
            }
            
            // Initialize the decryption operation with AES-256-CBC
            if (EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, key.data(), encrypted_data.data()) != 1) {
                EVP_CIPHER_CTX_free(ctx);
                return ErrorCode::IO_ERROR;
            }
            
            int out_len = 0;
            int final_len = 0;
            
            // Decrypt the data
            if (EVP_DecryptUpdate(ctx, decrypted.data(), &out_len, 
                                 encrypted_data.data() + AES_IV_SIZE, 
                                 static_cast<int>(encrypted_data.size() - AES_IV_SIZE)) != 1) {
                EVP_CIPHER_CTX_free(ctx);
                return ErrorCode::IO_ERROR;
            }
            
            // Finalize the decryption
            if (EVP_DecryptFinal_ex(ctx, decrypted.data() + out_len, &final_len) != 1) {
                EVP_CIPHER_CTX_free(ctx);
                return ErrorCode::IO_ERROR;
            }
            
            // Clean up
            EVP_CIPHER_CTX_free(ctx);
            
            // Resize the decrypted data to the actual size
            decrypted.resize(out_len + final_len);
            
            return decrypted;
        } catch (const std::exception& e) {
            std::cerr << "KyberAES decryption error: " << e.what() << std::endl;
            return Result<std::vector<uint8_t>>(ErrorCode::IO_ERROR);
        }
    }
    
    Result<std::pair<std::vector<uint8_t>, std::vector<uint8_t>>> generate_keypair() {
        try {
            // Generate a new keypair using hydra_crypto
            m_keypair = m_kyber_aes.generate_keypair();
            return Result<std::pair<std::vector<uint8_t>, std::vector<uint8_t>>>(m_keypair);
        } catch (const std::exception& e) {
            std::cerr << "KyberAES key generation error: " << e.what() << std::endl;
            return Result<std::pair<std::vector<uint8_t>, std::vector<uint8_t>>>(ErrorCode::IO_ERROR);
        }
    }
    
private:
    hydra::crypto::KyberAES m_kyber_aes;
    std::pair<std::vector<uint8_t>, std::vector<uint8_t>> m_keypair;
};

KyberAESEncryptionProvider::KyberAESEncryptionProvider(const std::string& kyber_mode)
    : m_impl(std::make_unique<Impl>(kyber_mode)) {
}

KyberAESEncryptionProvider::~KyberAESEncryptionProvider() = default;

Result<std::vector<uint8_t>> KyberAESEncryptionProvider::encrypt(const std::vector<uint8_t>& data, const EncryptionKey& key) {
    return m_impl->encrypt(data, key);
}

Result<std::vector<uint8_t>> KyberAESEncryptionProvider::decrypt(const std::vector<uint8_t>& encrypted_data, const EncryptionKey& key) {
    return m_impl->decrypt(encrypted_data, key);
}

Result<std::pair<std::vector<uint8_t>, std::vector<uint8_t>>> KyberAESEncryptionProvider::generate_keypair() {
    return m_impl->generate_keypair();
}

// Factory function to create a VFS with Kyber AES encryption
std::shared_ptr<IVirtualFileSystem> create_kyber_encrypted_vfs(
    std::shared_ptr<IVirtualFileSystem> base_vfs,
    const std::vector<uint8_t>& key,
    const std::string& kyber_mode) {
    
    // Create the encryption provider
    auto encryption_provider = std::make_shared<KyberAESEncryptionProvider>(kyber_mode);
    
    // Convert the key to the EncryptionKey format
    EncryptionKey encryption_key = {};
    
    // Copy the key data, truncating or padding as necessary
    size_t copy_size = std::min(key.size(), encryption_key.size());
    std::copy_n(key.begin(), copy_size, encryption_key.begin());
    
    // Create and return the encrypted VFS
    return std::make_shared<EncryptedVFS>(base_vfs, encryption_provider, encryption_key);
}

} // namespace vfs
} // namespace hydra