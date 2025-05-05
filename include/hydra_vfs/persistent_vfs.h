#ifndef HYDRA_PERSISTENT_VFS_H
#define HYDRA_PERSISTENT_VFS_H

#include "vfs.h"
#include "memory_vfs.h"
#include <string>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <filesystem>

namespace hydra {
namespace vfs {

/**
 * @brief Persistent file implementation that stores data on disk
 */
class PersistentFile : public IFile {
public:
    PersistentFile(const std::string& virtual_path, const std::string& physical_path, FileMode mode);
    ~PersistentFile() override;
    
    Result<size_t> read(uint8_t* buffer, size_t size) override;
    Result<size_t> write(const uint8_t* buffer, size_t size) override;
    Result<void> seek(int64_t offset, int whence) override;
    Result<uint64_t> tell() override;
    Result<void> flush() override;
    Result<void> close() override;
    Result<FileInfo> get_info() const override;
    
private:
    std::string m_virtual_path;
    std::string m_physical_path;
    FileMode m_mode;
    std::fstream m_file;
    bool m_is_open;
    mutable std::mutex m_mutex;
    time_t m_created_time;
    time_t m_modified_time;
    time_t m_accessed_time;
};

/**
 * @brief Persistent virtual file system implementation that stores data on disk
 * 
 * This VFS provides persistence by storing files on the physical file system
 * but maintains isolation by restricting access to a specific directory.
 */
class PersistentVFS : public IVirtualFileSystem {
public:
    /**
     * @brief Construct a new Persistent VFS
     * 
     * @param root_path Physical root path where files will be stored
     */
    explicit PersistentVFS(const std::string& root_path);
    ~PersistentVFS() override;
    
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
    std::string m_root_path;
    std::unordered_map<std::string, std::shared_ptr<IVirtualFileSystem>> m_mounts;
    mutable std::mutex m_mutex;
    
    std::string virtual_to_physical_path(const std::string& virtual_path) const;
    std::string physical_to_virtual_path(const std::string& physical_path) const;
    FileInfo physical_to_file_info(const std::string& virtual_path, const std::filesystem::directory_entry& entry) const;
};

} // namespace vfs
} // namespace hydra

#endif // HYDRA_PERSISTENT_VFS_H
