#ifndef HYDRA_VFS_H
#define HYDRA_VFS_H

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <functional>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <optional>

namespace hydra {
namespace vfs {

/**
 * @brief Error codes for VFS operations
 */
enum class ErrorCode {
    SUCCESS,
    FILE_NOT_FOUND,
    PERMISSION_DENIED,
    ALREADY_EXISTS,
    NOT_A_DIRECTORY,
    NOT_A_FILE,
    IO_ERROR,
    INVALID_ARGUMENT,
    NOT_IMPLEMENTED,
    NOT_SUPPORTED,
    HARDWARE_SECURITY_MODULE_NOT_AVAILABLE,
    INITIALIZATION_FAILED,
    INVALID_FORMAT,
    NOT_FOUND,
    UNKNOWN_ERROR
};

/**
 * @brief Result type for VFS operations
 */
template<typename T>
class Result {
public:
    Result() : m_success(false), m_error(ErrorCode::UNKNOWN_ERROR) {}
    Result(T value) : m_success(true), m_value(std::move(value)) {}
    Result(ErrorCode error) : m_success(false), m_error(error) {}

    bool success() const { return m_success; }
    ErrorCode error() const { return m_error; }
    const T& value() const { return m_value.value(); }
    T& value() { return m_value.value(); }
    
    operator bool() const { return m_success; }

private:
    bool m_success;
    ErrorCode m_error;
    std::optional<T> m_value;
};

// Specialization for void
template<>
class Result<void> {
public:
    Result() : m_success(true) {}
    Result(ErrorCode error) : m_success(false), m_error(error) {}

    bool success() const { return m_success; }
    ErrorCode error() const { return m_error; }
    
    operator bool() const { return m_success; }

private:
    bool m_success;
    ErrorCode m_error = ErrorCode::SUCCESS;
};

/**
 * @brief File metadata
 */
struct FileInfo {
    std::string name;
    std::string path;
    uint64_t size;
    bool is_directory;
    time_t created_time;
    time_t modified_time;
    time_t accessed_time;
    time_t last_modified;  // Added for compatibility
};

/**
 * @brief File access modes
 */
enum class FileMode {
    READ,
    WRITE,
    APPEND,
    READ_WRITE,
    READ_APPEND,
    CREATE,
    CREATE_NEW
};

/**
 * @brief Abstract file interface
 */
class IFile {
public:
    virtual ~IFile() = default;
    
    virtual Result<size_t> read(uint8_t* buffer, size_t size) = 0;
    virtual Result<size_t> write(const uint8_t* buffer, size_t size) = 0;
    virtual Result<void> seek(int64_t offset, int whence) = 0;
    virtual Result<uint64_t> tell() = 0;
    virtual Result<void> flush() = 0;
    virtual Result<void> close() = 0;
    virtual Result<FileInfo> get_info() const = 0;
};

/**
 * @brief Virtual File System interface
 */
class IVirtualFileSystem {
public:
    virtual ~IVirtualFileSystem() = default;
    
    // File operations
    virtual Result<std::shared_ptr<IFile>> open_file(const std::string& path, FileMode mode) = 0;
    virtual Result<void> create_file(const std::string& path) = 0;
    virtual Result<void> delete_file(const std::string& path) = 0;
    virtual Result<void> rename_file(const std::string& old_path, const std::string& new_path) = 0;
    virtual Result<bool> file_exists(const std::string& path) = 0;
    virtual Result<FileInfo> get_file_info(const std::string& path) = 0;
    
    // Directory operations
    virtual Result<void> create_directory(const std::string& path) = 0;
    virtual Result<void> delete_directory(const std::string& path, bool recursive = false) = 0;
    virtual Result<std::vector<FileInfo>> list_directory(const std::string& path) = 0;
    virtual Result<bool> directory_exists(const std::string& path) = 0;
    
    // Path operations
    virtual std::string normalize_path(const std::string& path) const = 0;
    virtual std::string join_paths(const std::string& base, const std::string& relative) const = 0;
    virtual std::string get_parent_path(const std::string& path) const = 0;
    virtual std::string get_filename(const std::string& path) const = 0;
    
    // Mount operations
    virtual Result<void> mount(const std::string& mount_point, std::shared_ptr<IVirtualFileSystem> fs) = 0;
    virtual Result<void> unmount(const std::string& mount_point) = 0;
};

/**
 * @brief Factory function to create a new isolated virtual file system
 * 
 * @param root_path Optional physical root path to store data
 * @return std::shared_ptr<IVirtualFileSystem> A new VFS instance
 */
std::shared_ptr<IVirtualFileSystem> create_vfs(const std::string& root_path = "");

} // namespace vfs
} // namespace hydra

#endif // HYDRA_VFS_H
