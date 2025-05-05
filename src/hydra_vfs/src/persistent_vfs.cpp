#include "../../include/hydra_vfs/persistent_vfs.h"
#include <fstream>
#include <filesystem>
#include <cstring>
#include <algorithm>

namespace fs = std::filesystem;

namespace hydra {
namespace vfs {

// PersistentFile implementation
PersistentFile::PersistentFile(const std::string& virtual_path, const std::string& physical_path, FileMode mode)
    : m_virtual_path(virtual_path)
    , m_physical_path(physical_path)
    , m_mode(mode)
    , m_is_open(false) {
    
    std::ios_base::openmode open_mode = std::ios_base::binary;
    
    switch (mode) {
        case FileMode::READ:
            open_mode |= std::ios_base::in;
            break;
        case FileMode::WRITE:
            open_mode |= std::ios_base::out | std::ios_base::trunc;
            break;
        case FileMode::APPEND:
            open_mode |= std::ios_base::out | std::ios_base::app;
            break;
        case FileMode::READ_WRITE:
            open_mode |= std::ios_base::in | std::ios_base::out;
            break;
        case FileMode::READ_APPEND:
            open_mode |= std::ios_base::in | std::ios_base::out | std::ios_base::app;
            break;
        case FileMode::CREATE:
            open_mode |= std::ios_base::out;
            break;
        case FileMode::CREATE_NEW:
            open_mode |= std::ios_base::out | std::ios_base::trunc;
            break;
    }
    
    m_file.open(physical_path, open_mode);
    m_is_open = m_file.is_open();
    
    time_t current_time = std::time(nullptr);
    m_created_time = current_time;
    m_modified_time = current_time;
    m_accessed_time = current_time;
    
    // Try to get actual file times if possible
    if (fs::exists(physical_path)) {
        try {
            auto last_write = fs::last_write_time(physical_path);
            m_modified_time = std::chrono::system_clock::to_time_t(
                std::chrono::system_clock::now() + 
                std::chrono::seconds(1)  // Placeholder conversion
            );
            
            // Access time might not be available on all systems
            m_accessed_time = m_modified_time;
        } catch (...) {
            // Ignore errors
        }
    }
}

PersistentFile::~PersistentFile() {
    close();
}

Result<size_t> PersistentFile::read(uint8_t* buffer, size_t size) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (!m_is_open) {
        return ErrorCode::IO_ERROR;
    }
    
    if (m_mode != FileMode::READ && m_mode != FileMode::READ_WRITE && m_mode != FileMode::READ_APPEND) {
        return ErrorCode::PERMISSION_DENIED;
    }
    
    m_accessed_time = std::time(nullptr);
    
    m_file.read(reinterpret_cast<char*>(buffer), size);
    size_t bytes_read = m_file.gcount();
    
    if (m_file.fail() && !m_file.eof()) {
        return ErrorCode::IO_ERROR;
    }
    
    return bytes_read;
}

Result<size_t> PersistentFile::write(const uint8_t* buffer, size_t size) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (!m_is_open) {
        return ErrorCode::IO_ERROR;
    }
    
    if (m_mode != FileMode::WRITE && m_mode != FileMode::READ_WRITE && 
        m_mode != FileMode::APPEND && m_mode != FileMode::READ_APPEND) {
        return ErrorCode::PERMISSION_DENIED;
    }
    
    m_modified_time = std::time(nullptr);
    
    m_file.write(reinterpret_cast<const char*>(buffer), size);
    
    if (m_file.fail()) {
        return ErrorCode::IO_ERROR;
    }
    
    return size;
}

Result<void> PersistentFile::seek(int64_t offset, int whence) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (!m_is_open) {
        return ErrorCode::IO_ERROR;
    }
    
    std::ios_base::seekdir dir;
    
    switch (whence) {
        case 0: // SEEK_SET
            dir = std::ios_base::beg;
            break;
        case 1: // SEEK_CUR
            dir = std::ios_base::cur;
            break;
        case 2: // SEEK_END
            dir = std::ios_base::end;
            break;
        default:
            return ErrorCode::INVALID_ARGUMENT;
    }
    
    m_file.seekg(offset, dir);
    m_file.seekp(offset, dir);
    
    if (m_file.fail()) {
        return ErrorCode::IO_ERROR;
    }
    
    m_accessed_time = std::time(nullptr);
    
    return {};
}

Result<uint64_t> PersistentFile::tell() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (!m_is_open) {
        return ErrorCode::IO_ERROR;
    }
    
    auto pos = m_file.tellg();
    
    if (pos == -1) {
        return ErrorCode::IO_ERROR;
    }
    
    return static_cast<uint64_t>(pos);
}

Result<void> PersistentFile::flush() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (!m_is_open) {
        return ErrorCode::IO_ERROR;
    }
    
    m_file.flush();
    
    if (m_file.fail()) {
        return ErrorCode::IO_ERROR;
    }
    
    return {};
}

Result<void> PersistentFile::close() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (!m_is_open) {
        return {};
    }
    
    m_file.close();
    m_is_open = false;
    
    return {};
}

Result<FileInfo> PersistentFile::get_info() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (!m_is_open) {
        return ErrorCode::IO_ERROR;
    }
    
    FileInfo info;
    info.name = m_virtual_path.substr(m_virtual_path.find_last_of('/') + 1);
    info.path = m_virtual_path;
    info.is_directory = false;
    info.created_time = m_created_time;
    info.modified_time = m_modified_time;
    info.accessed_time = m_accessed_time;
    
    try {
        info.size = fs::file_size(m_physical_path);
    } catch (...) {
        // If we can't get the file size, try to determine it manually
        // We can't use tellg on a const file stream, so just return 0
        info.size = 0;
    }
    
    return info;
}

// PersistentVFS implementation
PersistentVFS::PersistentVFS(const std::string& root_path)
    : m_root_path(root_path) {
    
    // Ensure the root path exists
    if (!root_path.empty()) {
        fs::create_directories(root_path);
    }
}

PersistentVFS::~PersistentVFS() = default;

std::string PersistentVFS::virtual_to_physical_path(const std::string& virtual_path) const {
    std::string normalized = normalize_path(virtual_path);
    
    if (normalized.empty() || normalized == "/") {
        return m_root_path;
    }
    
    // Remove leading slash
    if (normalized[0] == '/') {
        normalized = normalized.substr(1);
    }
    
    return fs::path(m_root_path) / normalized;
}

std::string PersistentVFS::physical_to_virtual_path(const std::string& physical_path) const {
    fs::path path(physical_path);
    fs::path root(m_root_path);
    
    // Check if the physical path is within the root path
    if (path.string().find(root.string()) != 0) {
        return "";
    }
    
    // Get the relative path
    auto rel_path = path.lexically_relative(root);
    
    // Convert to virtual path
    std::string virtual_path = "/" + rel_path.generic_string();
    return normalize_path(virtual_path);
}

FileInfo PersistentVFS::physical_to_file_info(const std::string& virtual_path, const fs::directory_entry& entry) const {
    FileInfo info;
    info.name = entry.path().filename().string();
    info.path = virtual_path;
    info.is_directory = entry.is_directory();
    
    try {
        time_t current_time = std::time(nullptr);
        
        info.created_time = current_time;  // Creation time not always available
        info.modified_time = current_time;
        info.accessed_time = current_time; // Access time not always available
        
        if (entry.is_regular_file()) {
            info.size = entry.file_size();
        } else {
            info.size = 0;
        }
    } catch (...) {
        // Default values if we can't get the file info
        info.created_time = 0;
        info.modified_time = 0;
        info.accessed_time = 0;
        info.size = 0;
    }
    
    return info;
}

std::string PersistentVFS::normalize_path(const std::string& path) const {
    if (path.empty()) {
        return "/";
    }
    
    fs::path fs_path(path);
    fs_path = fs_path.lexically_normal();
    
    std::string normalized = fs_path.generic_string();
    
    // Ensure the path starts with a slash
    if (normalized.empty() || normalized[0] != '/') {
        normalized = "/" + normalized;
    }
    
    // Remove trailing slash unless it's the root
    if (normalized.size() > 1 && normalized.back() == '/') {
        normalized.pop_back();
    }
    
    return normalized;
}

std::string PersistentVFS::join_paths(const std::string& base, const std::string& relative) const {
    if (relative.empty()) {
        return base;
    }
    
    if (relative[0] == '/') {
        return normalize_path(relative);
    }
    
    fs::path base_path(base);
    fs::path rel_path(relative);
    
    fs::path joined = base_path / rel_path;
    return normalize_path(joined.generic_string());
}

std::string PersistentVFS::get_parent_path(const std::string& path) const {
    std::string normalized = normalize_path(path);
    
    if (normalized == "/") {
        return "/";
    }
    
    fs::path fs_path(normalized);
    fs::path parent = fs_path.parent_path();
    
    if (parent.empty()) {
        return "/";
    }
    
    return normalize_path(parent.generic_string());
}

std::string PersistentVFS::get_filename(const std::string& path) const {
    std::string normalized = normalize_path(path);
    
    if (normalized == "/") {
        return "";
    }
    
    fs::path fs_path(normalized);
    return fs_path.filename().string();
}

Result<std::shared_ptr<IFile>> PersistentVFS::open_file(const std::string& path, FileMode mode) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    // Check if this path is handled by a mounted filesystem
    for (const auto& mount : m_mounts) {
        if (path.find(mount.first) == 0) {
            std::string relative_path = path.substr(mount.first.size());
            if (relative_path.empty() || relative_path[0] != '/') {
                relative_path = "/" + relative_path;
            }
            return mount.second->open_file(relative_path, mode);
        }
    }
    
    std::string physical_path = virtual_to_physical_path(path);
    
    // Check if the file exists
    if (mode == FileMode::READ && !fs::exists(physical_path)) {
        return ErrorCode::FILE_NOT_FOUND;
    }
    
    // Check if it's a directory
    if (fs::exists(physical_path) && fs::is_directory(physical_path)) {
        return ErrorCode::NOT_A_FILE;
    }
    
    // Create parent directories if needed
    if (mode != FileMode::READ) {
        fs::path parent_dir = fs::path(physical_path).parent_path();
        fs::create_directories(parent_dir);
    }
    
    auto file = std::make_shared<PersistentFile>(path, physical_path, mode);
    return Result<std::shared_ptr<IFile>>(std::static_pointer_cast<IFile>(file));
}

Result<void> PersistentVFS::create_file(const std::string& path) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    // Check if this path is handled by a mounted filesystem
    for (const auto& mount : m_mounts) {
        if (path.find(mount.first) == 0) {
            std::string relative_path = path.substr(mount.first.size());
            if (relative_path.empty() || relative_path[0] != '/') {
                relative_path = "/" + relative_path;
            }
            return mount.second->create_file(relative_path);
        }
    }
    
    std::string physical_path = virtual_to_physical_path(path);
    
    // Check if the file already exists
    if (fs::exists(physical_path)) {
        if (fs::is_directory(physical_path)) {
            return ErrorCode::NOT_A_FILE;
        }
        return ErrorCode::ALREADY_EXISTS;
    }
    
    // Create parent directories
    fs::path parent_dir = fs::path(physical_path).parent_path();
    fs::create_directories(parent_dir);
    
    // Create the file
    std::ofstream file(physical_path);
    if (!file) {
        return ErrorCode::IO_ERROR;
    }
    
    return {};
}

Result<void> PersistentVFS::delete_file(const std::string& path) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    // Check if this path is handled by a mounted filesystem
    for (const auto& mount : m_mounts) {
        if (path.find(mount.first) == 0) {
            std::string relative_path = path.substr(mount.first.size());
            if (relative_path.empty() || relative_path[0] != '/') {
                relative_path = "/" + relative_path;
            }
            return mount.second->delete_file(relative_path);
        }
    }
    
    std::string physical_path = virtual_to_physical_path(path);
    
    // Check if the file exists
    if (!fs::exists(physical_path)) {
        return ErrorCode::FILE_NOT_FOUND;
    }
    
    // Check if it's a directory
    if (fs::is_directory(physical_path)) {
        return ErrorCode::NOT_A_FILE;
    }
    
    // Delete the file
    std::error_code ec;
    if (!fs::remove(physical_path, ec)) {
        return ErrorCode::IO_ERROR;
    }
    
    return {};
}

Result<void> PersistentVFS::rename_file(const std::string& old_path, const std::string& new_path) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    // Check if paths are handled by mounted filesystems
    for (const auto& mount : m_mounts) {
        if (old_path.find(mount.first) == 0 && new_path.find(mount.first) == 0) {
            std::string old_relative = old_path.substr(mount.first.size());
            std::string new_relative = new_path.substr(mount.first.size());
            
            if (old_relative.empty() || old_relative[0] != '/') {
                old_relative = "/" + old_relative;
            }
            if (new_relative.empty() || new_relative[0] != '/') {
                new_relative = "/" + new_relative;
            }
            
            return mount.second->rename_file(old_relative, new_relative);
        }
    }
    
    std::string old_physical = virtual_to_physical_path(old_path);
    std::string new_physical = virtual_to_physical_path(new_path);
    
    // Check if the source file exists
    if (!fs::exists(old_physical)) {
        return ErrorCode::FILE_NOT_FOUND;
    }
    
    // Check if it's a directory
    if (fs::is_directory(old_physical)) {
        return ErrorCode::NOT_A_FILE;
    }
    
    // Check if the destination already exists
    if (fs::exists(new_physical)) {
        return ErrorCode::ALREADY_EXISTS;
    }
    
    // Create parent directories for the destination
    fs::path parent_dir = fs::path(new_physical).parent_path();
    fs::create_directories(parent_dir);
    
    // Rename the file
    std::error_code ec;
    fs::rename(old_physical, new_physical, ec);
    
    if (ec) {
        return ErrorCode::IO_ERROR;
    }
    
    return {};
}

Result<bool> PersistentVFS::file_exists(const std::string& path) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    // Check if this path is handled by a mounted filesystem
    for (const auto& mount : m_mounts) {
        if (path.find(mount.first) == 0) {
            std::string relative_path = path.substr(mount.first.size());
            if (relative_path.empty() || relative_path[0] != '/') {
                relative_path = "/" + relative_path;
            }
            return mount.second->file_exists(relative_path);
        }
    }
    
    std::string physical_path = virtual_to_physical_path(path);
    
    return fs::exists(physical_path) && !fs::is_directory(physical_path);
}

Result<FileInfo> PersistentVFS::get_file_info(const std::string& path) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    // Check if this path is handled by a mounted filesystem
    for (const auto& mount : m_mounts) {
        if (path.find(mount.first) == 0) {
            std::string relative_path = path.substr(mount.first.size());
            if (relative_path.empty() || relative_path[0] != '/') {
                relative_path = "/" + relative_path;
            }
            return mount.second->get_file_info(relative_path);
        }
    }
    
    std::string physical_path = virtual_to_physical_path(path);
    
    // Check if the file exists
    if (!fs::exists(physical_path)) {
        return ErrorCode::FILE_NOT_FOUND;
    }
    
    // Check if it's a directory
    if (fs::is_directory(physical_path)) {
        return ErrorCode::NOT_A_FILE;
    }
    
    fs::directory_entry entry(physical_path);
    return physical_to_file_info(path, entry);
}

Result<void> PersistentVFS::create_directory(const std::string& path) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    // Check if this path is handled by a mounted filesystem
    for (const auto& mount : m_mounts) {
        if (path.find(mount.first) == 0) {
            std::string relative_path = path.substr(mount.first.size());
            if (relative_path.empty() || relative_path[0] != '/') {
                relative_path = "/" + relative_path;
            }
            return mount.second->create_directory(relative_path);
        }
    }
    
    if (path.empty() || path == "/") {
        return {}; // Root directory always exists
    }
    
    std::string physical_path = virtual_to_physical_path(path);
    
    // Check if it already exists
    if (fs::exists(physical_path)) {
        if (!fs::is_directory(physical_path)) {
            return ErrorCode::NOT_A_DIRECTORY;
        }
        return ErrorCode::ALREADY_EXISTS;
    }
    
    // Create the directory
    std::error_code ec;
    if (!fs::create_directories(physical_path, ec)) {
        return ErrorCode::IO_ERROR;
    }
    
    return {};
}

Result<void> PersistentVFS::delete_directory(const std::string& path, bool recursive) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    // Check if this path is handled by a mounted filesystem
    for (const auto& mount : m_mounts) {
        if (path.find(mount.first) == 0) {
            std::string relative_path = path.substr(mount.first.size());
            if (relative_path.empty() || relative_path[0] != '/') {
                relative_path = "/" + relative_path;
            }
            return mount.second->delete_directory(relative_path, recursive);
        }
    }
    
    if (path.empty() || path == "/") {
        return ErrorCode::PERMISSION_DENIED; // Cannot delete root
    }
    
    std::string physical_path = virtual_to_physical_path(path);
    
    // Check if the directory exists
    if (!fs::exists(physical_path)) {
        return ErrorCode::FILE_NOT_FOUND;
    }
    
    // Check if it's a directory
    if (!fs::is_directory(physical_path)) {
        return ErrorCode::NOT_A_DIRECTORY;
    }
    
    // Check if it's empty (if not recursive)
    if (!recursive) {
        if (!fs::is_empty(physical_path)) {
            return ErrorCode::PERMISSION_DENIED;
        }
        
        std::error_code ec;
        if (!fs::remove(physical_path, ec)) {
            return ErrorCode::IO_ERROR;
        }
    } else {
        std::error_code ec;
        if (!fs::remove_all(physical_path, ec)) {
            return ErrorCode::IO_ERROR;
        }
    }
    
    return {};
}

Result<std::vector<FileInfo>> PersistentVFS::list_directory(const std::string& path) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    // Check if this path is handled by a mounted filesystem
    for (const auto& mount : m_mounts) {
        if (path.find(mount.first) == 0) {
            std::string relative_path = path.substr(mount.first.size());
            if (relative_path.empty() || relative_path[0] != '/') {
                relative_path = "/" + relative_path;
            }
            return mount.second->list_directory(relative_path);
        }
    }
    
    std::string physical_path = virtual_to_physical_path(path);
    
    // Check if the directory exists
    if (!fs::exists(physical_path)) {
        return ErrorCode::FILE_NOT_FOUND;
    }
    
    // Check if it's a directory
    if (!fs::is_directory(physical_path)) {
        return ErrorCode::NOT_A_DIRECTORY;
    }
    
    std::vector<FileInfo> result;
    std::string normalized_path = normalize_path(path);
    if (normalized_path.back() != '/') {
        normalized_path += '/';
    }
    
    // List directory contents
    std::error_code ec;
    for (const auto& entry : fs::directory_iterator(physical_path, ec)) {
        std::string entry_name = entry.path().filename().string();
        std::string virtual_entry_path = normalized_path + entry_name;
        
        result.push_back(physical_to_file_info(virtual_entry_path, entry));
    }
    
    if (ec) {
        return ErrorCode::IO_ERROR;
    }
    
    // Add mount points that are direct children of this directory
    for (const auto& mount : m_mounts) {
        std::string mount_path = normalize_path(mount.first);
        std::string parent = get_parent_path(mount_path);
        
        if (parent == normalized_path.substr(0, normalized_path.size() - 1)) {
            FileInfo mount_info;
            mount_info.name = get_filename(mount_path);
            mount_info.path = mount_path;
            mount_info.is_directory = true;
            mount_info.size = 0;
            mount_info.created_time = 0;
            mount_info.modified_time = 0;
            mount_info.accessed_time = 0;
            
            result.push_back(mount_info);
        }
    }
    
    return result;
}

Result<bool> PersistentVFS::directory_exists(const std::string& path) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    // Check if this path is handled by a mounted filesystem
    for (const auto& mount : m_mounts) {
        if (path.find(mount.first) == 0) {
            std::string relative_path = path.substr(mount.first.size());
            if (relative_path.empty() || relative_path[0] != '/') {
                relative_path = "/" + relative_path;
            }
            return mount.second->directory_exists(relative_path);
        }
    }
    
    if (path.empty() || path == "/") {
        return true; // Root always exists
    }
    
    std::string physical_path = virtual_to_physical_path(path);
    
    return fs::exists(physical_path) && fs::is_directory(physical_path);
}

Result<void> PersistentVFS::mount(const std::string& mount_point, std::shared_ptr<IVirtualFileSystem> fs) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    std::string normalized = normalize_path(mount_point);
    
    // Check if mount point already exists
    if (m_mounts.find(normalized) != m_mounts.end()) {
        return ErrorCode::ALREADY_EXISTS;
    }
    
    // Create the mount point directory if it doesn't exist
    if (normalized != "/") {
        std::string physical_mount = virtual_to_physical_path(normalized);
        
        if (!fs::exists(physical_mount)) {
            std::error_code ec;
            if (!fs::create_directories(physical_mount, ec)) {
                return ErrorCode::IO_ERROR;
            }
        } else if (!fs::is_directory(physical_mount)) {
            return ErrorCode::NOT_A_DIRECTORY;
        }
    }
    
    m_mounts[normalized] = fs;
    return {};
}

Result<void> PersistentVFS::unmount(const std::string& mount_point) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    std::string normalized = normalize_path(mount_point);
    
    auto it = m_mounts.find(normalized);
    if (it == m_mounts.end()) {
        return ErrorCode::FILE_NOT_FOUND;
    }
    
    m_mounts.erase(it);
    return {};
}

} // namespace vfs
} // namespace hydra
