#include "../../include/hydra_vfs/memory_vfs.h"
#include <algorithm>
#include <sstream>
#include <cstring>
#include <iostream>

namespace hydra {
namespace vfs {

// MemoryFile implementation
MemoryFile::MemoryFile(const std::string& path, FileMode mode)
    : m_path(path)
    , m_mode(mode)
    , m_data(std::make_shared<std::vector<uint8_t>>())
    , m_position(0)
    , m_is_open(true) {
    
    time_t current_time = std::time(nullptr);
    m_created_time = current_time;
    m_modified_time = current_time;
    m_accessed_time = current_time;
}

MemoryFile::~MemoryFile() {
    close();
}

Result<size_t> MemoryFile::read(uint8_t* buffer, size_t size) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (!m_is_open) {
        return ErrorCode::IO_ERROR;
    }
    
    if (m_mode != FileMode::READ && m_mode != FileMode::READ_WRITE && m_mode != FileMode::READ_APPEND) {
        return ErrorCode::PERMISSION_DENIED;
    }
    
    m_accessed_time = std::time(nullptr);
    
    if (m_position >= m_data->size()) {
        return 0; // EOF
    }
    
    size_t bytes_to_read = std::min(size, m_data->size() - m_position);
    std::memcpy(buffer, m_data->data() + m_position, bytes_to_read);
    m_position += bytes_to_read;
    
    return bytes_to_read;
}

Result<size_t> MemoryFile::write(const uint8_t* buffer, size_t size) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (!m_is_open) {
        return ErrorCode::IO_ERROR;
    }
    
    if (m_mode != FileMode::WRITE && m_mode != FileMode::READ_WRITE && 
        m_mode != FileMode::APPEND && m_mode != FileMode::READ_APPEND) {
        return ErrorCode::PERMISSION_DENIED;
    }
    
    m_modified_time = std::time(nullptr);
    
    if (m_mode == FileMode::APPEND || m_mode == FileMode::READ_APPEND) {
        m_position = m_data->size();
    }
    
    if (m_position + size > m_data->size()) {
        m_data->resize(m_position + size);
    }
    
    std::memcpy(m_data->data() + m_position, buffer, size);
    m_position += size;
    
    return size;
}

Result<void> MemoryFile::seek(int64_t offset, int whence) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (!m_is_open) {
        return ErrorCode::IO_ERROR;
    }
    
    size_t new_position = 0;
    
    // SEEK_SET, SEEK_CUR, SEEK_END (0, 1, 2)
    if (whence == 0) { // SEEK_SET
        new_position = offset;
    } else if (whence == 1) { // SEEK_CUR
        new_position = m_position + offset;
    } else if (whence == 2) { // SEEK_END
        new_position = m_data->size() + offset;
    } else {
        return ErrorCode::INVALID_ARGUMENT;
    }
    
    if (new_position > m_data->size()) {
        m_data->resize(new_position);
    }
    
    m_position = new_position;
    m_accessed_time = std::time(nullptr);
    
    return {};
}

Result<uint64_t> MemoryFile::tell() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (!m_is_open) {
        return ErrorCode::IO_ERROR;
    }
    
    return m_position;
}

Result<void> MemoryFile::flush() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (!m_is_open) {
        return ErrorCode::IO_ERROR;
    }
    
    // No-op for memory files
    return {};
}

Result<void> MemoryFile::close() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (!m_is_open) {
        return {};
    }
    
    m_is_open = false;
    return {};
}

Result<FileInfo> MemoryFile::get_info() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (!m_is_open) {
        return ErrorCode::IO_ERROR;
    }
    
    FileInfo info;
    info.name = m_path.substr(m_path.find_last_of('/') + 1);
    info.path = m_path;
    info.size = m_data->size();
    info.is_directory = false;
    info.created_time = m_created_time;
    info.modified_time = m_modified_time;
    info.accessed_time = m_accessed_time;
    
    return info;
}

void MemoryFile::set_data(std::shared_ptr<std::vector<uint8_t>> data) {
    m_data = data;
}

std::shared_ptr<std::vector<uint8_t>> MemoryFile::get_data() {
    return m_data;
}

// VFSNode implementation
VFSNode::VFSNode(const std::string& name, bool is_directory)
    : name(name)
    , is_directory(is_directory)
    , created_time(std::time(nullptr))
    , modified_time(created_time)
    , accessed_time(created_time) {
    
    if (!is_directory) {
        data = std::make_shared<std::vector<uint8_t>>();
    }
}

// MemoryVFS implementation
MemoryVFS::MemoryVFS() {
    m_root = std::make_shared<VFSNode>("", true);
}

MemoryVFS::~MemoryVFS() = default;

std::vector<std::string> MemoryVFS::split_path(const std::string& path) const {
    std::vector<std::string> parts;
    std::string normalized = normalize_path(path);
    
    if (normalized.empty() || normalized == "/") {
        return parts;
    }
    
    size_t start = normalized[0] == '/' ? 1 : 0;
    size_t end = normalized.size();
    
    while (start < end) {
        size_t pos = normalized.find('/', start);
        if (pos == std::string::npos) {
            pos = end;
        }
        
        if (pos > start) {
            parts.push_back(normalized.substr(start, pos - start));
        }
        
        start = pos + 1;
    }
    
    return parts;
}

Result<std::shared_ptr<VFSNode>> MemoryVFS::get_node(const std::string& path, bool create_dirs) {
    std::cout << "DEBUG: MemoryVFS::get_node called for path: " << path << ", create_dirs: " << create_dirs << std::endl;
    
    if (path.empty() || path == "/") {
        std::cout << "DEBUG: Returning root node" << std::endl;
        return m_root;
    }
    
    std::vector<std::string> parts = split_path(path);
    std::shared_ptr<VFSNode> current = m_root;
    
    for (size_t i = 0; i < parts.size(); ++i) {
        const std::string& part = parts[i];
        bool is_last = (i == parts.size() - 1);
        
        std::cout << "DEBUG: Processing path part: " << part << ", is_last: " << is_last << std::endl;
        
        auto it = current->children.find(part);
        if (it == current->children.end()) {
            if (!create_dirs) {
                std::cout << "DEBUG: Node not found and create_dirs is false" << std::endl;
                return ErrorCode::FILE_NOT_FOUND;
            }
            
            // Create directory node
            std::cout << "DEBUG: Creating new directory node: " << part << std::endl;
            auto new_node = std::make_shared<VFSNode>(part, true);
            current->children[part] = new_node;
            current = new_node;
        } else {
            current = it->second;
            
            if (!is_last && !current->is_directory) {
                std::cout << "DEBUG: Path component is not a directory" << std::endl;
                return ErrorCode::NOT_A_DIRECTORY;
            }
        }
    }
    
    std::cout << "DEBUG: Returning node for path: " << path << std::endl;
    return current;
}

Result<std::shared_ptr<VFSNode>> MemoryVFS::get_parent_node(const std::string& path, bool create_dirs) {
    std::string parent_path = get_parent_path(path);
    std::cout << "DEBUG: MemoryVFS::get_parent_node called for path: " << path << ", parent_path: " << parent_path << ", create_dirs: " << create_dirs << std::endl;
    return get_node(parent_path, create_dirs);
}

FileInfo MemoryVFS::node_to_file_info(const std::string& path, const VFSNode& node) const {
    FileInfo info;
    info.name = node.name;
    info.path = path;
    info.is_directory = node.is_directory;
    info.created_time = node.created_time;
    info.modified_time = node.modified_time;
    info.accessed_time = node.accessed_time;
    
    if (!node.is_directory && node.data) {
        info.size = node.data->size();
    } else {
        info.size = 0;
    }
    
    return info;
}

std::string MemoryVFS::normalize_path(const std::string& path) const {
    if (path.empty()) {
        return "/";
    }
    
    std::vector<std::string> parts;
    std::istringstream iss(path);
    std::string part;
    
    // Split by '/'
    while (std::getline(iss, part, '/')) {
        if (part.empty() || part == ".") {
            continue;
        } else if (part == "..") {
            if (!parts.empty()) {
                parts.pop_back();
            }
        } else {
            parts.push_back(part);
        }
    }
    
    // Reconstruct path
    std::string result;
    for (const auto& p : parts) {
        result += "/" + p;
    }
    
    return result.empty() ? "/" : result;
}

std::string MemoryVFS::join_paths(const std::string& base, const std::string& relative) const {
    if (relative.empty()) {
        return base;
    }
    
    if (relative[0] == '/') {
        return normalize_path(relative);
    }
    
    std::string joined = base;
    if (joined.back() != '/') {
        joined += '/';
    }
    joined += relative;
    
    return normalize_path(joined);
}

std::string MemoryVFS::get_parent_path(const std::string& path) const {
    std::string normalized = normalize_path(path);
    
    if (normalized == "/") {
        return "/";
    }
    
    size_t pos = normalized.find_last_of('/');
    if (pos == 0) {
        return "/";
    } else if (pos != std::string::npos) {
        return normalized.substr(0, pos);
    }
    
    return "";
}

std::string MemoryVFS::get_filename(const std::string& path) const {
    std::string normalized = normalize_path(path);
    
    if (normalized == "/") {
        return "";
    }
    
    size_t pos = normalized.find_last_of('/');
    if (pos != std::string::npos && pos < normalized.size() - 1) {
        return normalized.substr(pos + 1);
    }
    
    return normalized;
}

Result<std::shared_ptr<IFile>> MemoryVFS::open_file(const std::string& path, FileMode mode) {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::cout << "DEBUG: MemoryVFS::open_file called for path: " << path << ", mode: " << static_cast<int>(mode) << std::endl;
    
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
    
    std::string filename = get_filename(path);
    std::cout << "DEBUG: Filename: " << filename << std::endl;
    auto parent_result = get_parent_node(path, mode != FileMode::READ);
    
    if (!parent_result) {
        std::cout << "DEBUG: Failed to get parent node: " << static_cast<int>(parent_result.error()) << std::endl;
        return parent_result.error();
    }
    
    auto parent = parent_result.value();
    auto it = parent->children.find(filename);
    
    if (it == parent->children.end()) {
        if (mode == FileMode::READ) {
            std::cout << "DEBUG: File not found for reading" << std::endl;
            return ErrorCode::FILE_NOT_FOUND;
        }
        
        std::cout << "DEBUG: Creating new file node" << std::endl;
        // Create new file
        auto file_node = std::make_shared<VFSNode>(filename, false);
        parent->children[filename] = file_node;
        parent->modified_time = std::time(nullptr);
        
        auto file = std::make_shared<MemoryFile>(path, mode);
        file_node->data = std::make_shared<std::vector<uint8_t>>();
        // Link the file's data to the node's data
        file->set_data(file_node->data);
        std::cout << "DEBUG: Returning new file" << std::endl;
        return Result<std::shared_ptr<IFile>>(std::static_pointer_cast<IFile>(file));
    }
    
    auto node = it->second;
    
    if (node->is_directory) {
        std::cout << "DEBUG: Path is a directory, not a file" << std::endl;
        return ErrorCode::NOT_A_FILE;
    }
    
    std::cout << "DEBUG: Opening existing file" << std::endl;
    auto file = std::make_shared<MemoryFile>(path, mode);
    // Link the file's data to the node's data instead of making a copy
    file->set_data(node->data);
    
    node->accessed_time = std::time(nullptr);
    if (mode != FileMode::READ) {
        node->modified_time = node->accessed_time;
    }
    
    std::cout << "DEBUG: Returning existing file" << std::endl;
    return Result<std::shared_ptr<IFile>>(std::static_pointer_cast<IFile>(file));
}

Result<void> MemoryVFS::create_file(const std::string& path) {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::cout << "DEBUG: MemoryVFS::create_file called for path: " << path << std::endl;
    
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
    
    std::string filename = get_filename(path);
    auto parent_result = get_parent_node(path, true);
    
    if (!parent_result) {
        std::cout << "DEBUG: Failed to get parent node: " << static_cast<int>(parent_result.error()) << std::endl;
        return parent_result.error();
    }
    
    auto parent = parent_result.value();
    auto it = parent->children.find(filename);
    
    if (it != parent->children.end()) {
        if (it->second->is_directory) {
            std::cout << "DEBUG: Cannot create file, path is a directory" << std::endl;
            return ErrorCode::NOT_A_FILE;
        }
        
        // File already exists
        std::cout << "DEBUG: File already exists" << std::endl;
        return {};
    }
    
    // Create new file
    auto file_node = std::make_shared<VFSNode>(filename, false);
    parent->children[filename] = file_node;
    parent->modified_time = std::time(nullptr);
    file_node->data = std::make_shared<std::vector<uint8_t>>();
    
    std::cout << "DEBUG: File created successfully" << std::endl;
    return {};
}

Result<void> MemoryVFS::delete_file(const std::string& path) {
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
    
    std::string filename = get_filename(path);
    auto parent_result = get_parent_node(path, false);
    
    if (!parent_result) {
        return parent_result.error();
    }
    
    auto parent = parent_result.value();
    auto it = parent->children.find(filename);
    
    if (it == parent->children.end()) {
        return ErrorCode::FILE_NOT_FOUND;
    }
    
    if (it->second->is_directory) {
        return ErrorCode::NOT_A_FILE;
    }
    
    parent->children.erase(it);
    parent->modified_time = std::time(nullptr);
    
    return {};
}

Result<void> MemoryVFS::rename_file(const std::string& old_path, const std::string& new_path) {
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
    
    // Get the node to be moved
    auto node_result = get_node(old_path, false);
    if (!node_result) {
        return node_result.error();
    }
    
    auto node = node_result.value();
    
    // Get the destination parent
    std::string new_filename = get_filename(new_path);
    auto new_parent_result = get_parent_node(new_path, true);
    
    if (!new_parent_result) {
        return new_parent_result.error();
    }
    
    auto new_parent = new_parent_result.value();
    
    // Check if destination already exists
    if (new_parent->children.find(new_filename) != new_parent->children.end()) {
        return ErrorCode::ALREADY_EXISTS;
    }
    
    // Remove from old parent
    std::string old_filename = get_filename(old_path);
    auto old_parent_result = get_parent_node(old_path, false);
    
    if (!old_parent_result) {
        return old_parent_result.error();
    }
    
    auto old_parent = old_parent_result.value();
    old_parent->children.erase(old_filename);
    old_parent->modified_time = std::time(nullptr);
    
    // Add to new parent
    node->name = new_filename;
    new_parent->children[new_filename] = node;
    new_parent->modified_time = std::time(nullptr);
    
    return {};
}

Result<bool> MemoryVFS::file_exists(const std::string& path) {
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
    
    auto node_result = get_node(path, false);
    if (!node_result) {
        return false;
    }
    
    return !node_result.value()->is_directory;
}

Result<FileInfo> MemoryVFS::get_file_info(const std::string& path) {
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
    
    auto node_result = get_node(path, false);
    if (!node_result) {
        return node_result.error();
    }
    
    auto node = node_result.value();
    if (node->is_directory) {
        return ErrorCode::NOT_A_FILE;
    }
    
    return node_to_file_info(path, *node);
}

Result<void> MemoryVFS::create_directory(const std::string& path) {
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
        return ErrorCode::ALREADY_EXISTS;
    }
    
    std::string dirname = get_filename(path);
    auto parent_result = get_parent_node(path, true);
    
    if (!parent_result) {
        return parent_result.error();
    }
    
    auto parent = parent_result.value();
    auto it = parent->children.find(dirname);
    
    if (it != parent->children.end()) {
        if (it->second->is_directory) {
            return ErrorCode::ALREADY_EXISTS;
        } else {
            return ErrorCode::NOT_A_DIRECTORY;
        }
    }
    
    auto dir_node = std::make_shared<VFSNode>(dirname, true);
    parent->children[dirname] = dir_node;
    parent->modified_time = std::time(nullptr);
    
    return {};
}

Result<void> MemoryVFS::delete_directory(const std::string& path, bool recursive) {
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
        return ErrorCode::PERMISSION_DENIED;
    }
    
    auto node_result = get_node(path, false);
    if (!node_result) {
        return node_result.error();
    }
    
    auto node = node_result.value();
    if (!node->is_directory) {
        return ErrorCode::NOT_A_DIRECTORY;
    }
    
    if (!recursive && !node->children.empty()) {
        return ErrorCode::PERMISSION_DENIED;
    }
    
    std::string dirname = get_filename(path);
    auto parent_result = get_parent_node(path, false);
    
    if (!parent_result) {
        return parent_result.error();
    }
    
    auto parent = parent_result.value();
    parent->children.erase(dirname);
    parent->modified_time = std::time(nullptr);
    
    return {};
}

Result<std::vector<FileInfo>> MemoryVFS::list_directory(const std::string& path) {
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
    
    auto node_result = get_node(path, false);
    if (!node_result) {
        return node_result.error();
    }
    
    auto node = node_result.value();
    if (!node->is_directory) {
        return ErrorCode::NOT_A_DIRECTORY;
    }
    
    std::vector<FileInfo> result;
    std::string normalized_path = normalize_path(path);
    if (normalized_path.back() != '/') {
        normalized_path += '/';
    }
    
    for (const auto& child : node->children) {
        std::string child_path = normalized_path + child.first;
        result.push_back(node_to_file_info(child_path, *child.second));
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

Result<bool> MemoryVFS::directory_exists(const std::string& path) {
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
    
    auto node_result = get_node(path, false);
    if (!node_result) {
        return false;
    }
    
    return node_result.value()->is_directory;
}

Result<void> MemoryVFS::mount(const std::string& mount_point, std::shared_ptr<IVirtualFileSystem> fs) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    std::string normalized = normalize_path(mount_point);
    
    // Check if mount point already exists
    if (m_mounts.find(normalized) != m_mounts.end()) {
        return ErrorCode::ALREADY_EXISTS;
    }
    
    // Create parent directories if they don't exist
    if (normalized != "/") {
        auto parent_result = get_parent_node(normalized, true);
        if (!parent_result) {
            return parent_result.error();
        }
        
        // Create the mount point directory
        std::string dirname = get_filename(normalized);
        auto parent = parent_result.value();
        
        if (parent->children.find(dirname) == parent->children.end()) {
            auto dir_node = std::make_shared<VFSNode>(dirname, true);
            parent->children[dirname] = dir_node;
            parent->modified_time = std::time(nullptr);
        } else if (!parent->children[dirname]->is_directory) {
            return ErrorCode::NOT_A_DIRECTORY;
        }
    }
    
    m_mounts[normalized] = fs;
    return {};
}

Result<void> MemoryVFS::unmount(const std::string& mount_point) {
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
