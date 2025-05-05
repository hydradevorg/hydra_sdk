#ifndef HYDRA_MEMORY_VFS_H
#define HYDRA_MEMORY_VFS_H

#include "vfs.h"
#include <unordered_map>
#include <vector>
#include <mutex>
#include <memory>
#include <ctime>

namespace hydra {
namespace vfs {

/**
 * @brief In-memory file implementation
 */
class MemoryFile : public IFile {
public:
    MemoryFile(const std::string& path, FileMode mode);
    ~MemoryFile() override;
    
    Result<size_t> read(uint8_t* buffer, size_t size) override;
    Result<size_t> write(const uint8_t* buffer, size_t size) override;
    Result<void> seek(int64_t offset, int whence) override;
    Result<uint64_t> tell() override;
    Result<void> flush() override;
    Result<void> close() override;
    Result<FileInfo> get_info() const override;
    
    void set_data(std::shared_ptr<std::vector<uint8_t>> data);
    std::shared_ptr<std::vector<uint8_t>> get_data();
    
private:
    std::string m_path;
    FileMode m_mode;
    std::shared_ptr<std::vector<uint8_t>> m_data;
    size_t m_position;
    bool m_is_open;
    time_t m_created_time;
    time_t m_modified_time;
    time_t m_accessed_time;
    mutable std::mutex m_mutex;
};

/**
 * @brief Node in the virtual file system tree
 */
struct VFSNode {
    std::string name;
    bool is_directory;
    time_t created_time;
    time_t modified_time;
    time_t accessed_time;
    
    // For files
    std::shared_ptr<std::vector<uint8_t>> data;
    
    // For directories
    std::unordered_map<std::string, std::shared_ptr<VFSNode>> children;
    
    VFSNode(const std::string& name, bool is_directory);
    VFSNode(const VFSNode&) = delete;
    VFSNode& operator=(const VFSNode&) = delete;
};

/**
 * @brief Memory-based virtual file system implementation
 */
class MemoryVFS : public IVirtualFileSystem {
public:
    MemoryVFS();
    ~MemoryVFS() override;
    
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
    std::shared_ptr<VFSNode> m_root;
    std::unordered_map<std::string, std::shared_ptr<IVirtualFileSystem>> m_mounts;
    mutable std::mutex m_mutex;
    
    std::vector<std::string> split_path(const std::string& path) const;
    Result<std::shared_ptr<VFSNode>> get_node(const std::string& path, bool create_dirs = false);
    Result<std::shared_ptr<VFSNode>> get_parent_node(const std::string& path, bool create_dirs = false);
    FileInfo node_to_file_info(const std::string& path, const VFSNode& node) const;
};

} // namespace vfs
} // namespace hydra

#endif // HYDRA_MEMORY_VFS_H
