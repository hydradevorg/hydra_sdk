#include "lmvs/p2p_vfs/p2p_vfs.hpp"
#include "lmvs/p2p_vfs/p2p_secure_file.hpp"
#include <filesystem>
#include <fstream>
#include <algorithm>
#include <iostream>

namespace lmvs {
namespace p2p_vfs {

// Legacy helper class for file implementation - will be replaced by P2PSecureFile
class P2PFile : public hydra::vfs::IFile {
public:
    P2PFile(P2PVFS* vfs, const std::string& path, hydra::vfs::FileMode mode)
        : m_vfs(vfs), m_path(path), m_mode(mode), m_position(0), m_is_open(true) {

        // If the file exists and we're in WRITE mode, clear it
        if (mode == hydra::vfs::FileMode::WRITE) {
            m_data.clear();
        } else {
            // Load the file data
            auto result = m_vfs->retrieve_file(path);
            if (result.success()) {
                m_data = result.value();
            }
        }
    }

    ~P2PFile() override {
        close();
    }

    hydra::vfs::Result<size_t> read(uint8_t* buffer, size_t size) override {
        if (!m_is_open) {
            return hydra::vfs::Result<size_t>::error("File is not open");
        }

        if (m_mode != hydra::vfs::FileMode::READ && m_mode != hydra::vfs::FileMode::READ_WRITE) {
            return hydra::vfs::Result<size_t>::error("File is not open for reading");
        }

        if (m_position >= m_data.size()) {
            return hydra::vfs::Result<size_t>::success(0); // EOF
        }

        size_t bytes_to_read = std::min(size, m_data.size() - m_position);
        std::memcpy(buffer, m_data.data() + m_position, bytes_to_read);
        m_position += bytes_to_read;

        return hydra::vfs::Result<size_t>::success(bytes_to_read);
    }

    hydra::vfs::Result<size_t> write(const uint8_t* buffer, size_t size) override {
        if (!m_is_open) {
            return hydra::vfs::Result<size_t>::error("File is not open");
        }

        if (m_mode != hydra::vfs::FileMode::WRITE &&
            m_mode != hydra::vfs::FileMode::APPEND &&
            m_mode != hydra::vfs::FileMode::READ_WRITE) {
            return hydra::vfs::Result<size_t>::error("File is not open for writing");
        }

        if (m_mode == hydra::vfs::FileMode::APPEND) {
            m_position = m_data.size();
        }

        if (m_position + size > m_data.size()) {
            m_data.resize(m_position + size);
        }

        std::memcpy(m_data.data() + m_position, buffer, size);
        m_position += size;

        return hydra::vfs::Result<size_t>::success(size);
    }

    hydra::vfs::Result<size_t> seek(size_t position) override {
        if (!m_is_open) {
            return hydra::vfs::Result<size_t>::error("File is not open");
        }

        m_position = std::min(position, m_data.size());
        return hydra::vfs::Result<size_t>::success(m_position);
    }

    hydra::vfs::Result<size_t> tell() override {
        if (!m_is_open) {
            return hydra::vfs::Result<size_t>::error("File is not open");
        }

        return hydra::vfs::Result<size_t>::success(m_position);
    }

    hydra::vfs::Result<bool> flush() override {
        if (!m_is_open) {
            return hydra::vfs::Result<bool>::error("File is not open");
        }

        if (m_mode != hydra::vfs::FileMode::WRITE &&
            m_mode != hydra::vfs::FileMode::APPEND &&
            m_mode != hydra::vfs::FileMode::READ_WRITE) {
            return hydra::vfs::Result<bool>::error("File is not open for writing");
        }

        // Distribute the file to peers
        bool success = m_vfs->distribute_file(m_path, m_data);
        return hydra::vfs::Result<bool>::success(success);
    }

    hydra::vfs::Result<bool> close() override {
        if (!m_is_open) {
            return hydra::vfs::Result<bool>::success(true);
        }

        m_is_open = false;

        // If the file was opened for writing, distribute it to peers
        if (m_mode == hydra::vfs::FileMode::WRITE ||
            m_mode == hydra::vfs::FileMode::APPEND ||
            m_mode == hydra::vfs::FileMode::READ_WRITE) {
            bool success = m_vfs->distribute_file(m_path, m_data);
            return hydra::vfs::Result<bool>::success(success);
        }

        return hydra::vfs::Result<bool>::success(true);
    }

private:
    P2PVFS* m_vfs;
    std::string m_path;
    hydra::vfs::FileMode m_mode;
    std::vector<uint8_t> m_data;
    size_t m_position;
    bool m_is_open;
};

P2PVFS::P2PVFS(const std::string& node_id,
               const std::string& local_storage_path,
               size_t num_layers,
               size_t threshold)
    : m_node_id(node_id),
      m_local_storage_path(local_storage_path),
      m_num_layers(num_layers),
      m_threshold(threshold) {

    // Create local storage directory if it doesn't exist
    std::filesystem::create_directories(local_storage_path);

    // Initialize the secure vector transport
    m_transport = std::make_unique<security::SecureVectorTransport>();

    // Generate keys for this node
    auto [public_key, private_key] = m_transport->generate_node_keys(node_id);
    m_private_key_bundle = private_key;
}

P2PVFS::~P2PVFS() {
    // Nothing to do here
}

hydra::vfs::Result<std::shared_ptr<hydra::vfs::IFile>> P2PVFS::open_file(
    const std::string& path,
    hydra::vfs::FileMode mode) {

    // Check if the file exists
    auto exists_result = file_exists(path);
    if (!exists_result.success()) {
        return hydra::vfs::Result<std::shared_ptr<hydra::vfs::IFile>>::error(exists_result.error());
    }

    bool file_exists = exists_result.value();

    // If the file doesn't exist and we're not in WRITE mode, return an error
    if (!file_exists && mode != hydra::vfs::FileMode::WRITE) {
        return hydra::vfs::Result<std::shared_ptr<hydra::vfs::IFile>>::error("File does not exist");
    }

    // Create the secure file object
    auto file = std::make_shared<P2PSecureFile>(this, path, mode, m_node_id, m_private_key_bundle);
    return hydra::vfs::Result<std::shared_ptr<hydra::vfs::IFile>>::success(file);
}

hydra::vfs::Result<bool> P2PVFS::file_exists(const std::string& path) {
    // Check if the file exists locally
    std::string local_path = get_local_path(path);
    bool exists = std::filesystem::exists(local_path);

    // If the file doesn't exist locally, check with peers
    if (!exists) {
        // TODO: Implement peer checking
        // For now, just return false
    }

    return hydra::vfs::Result<bool>::success(exists);
}

hydra::vfs::Result<hydra::vfs::FileInfo> P2PVFS::get_file_info(const std::string& path) {
    // Check if the file exists
    auto exists_result = file_exists(path);
    if (!exists_result.success()) {
        return hydra::vfs::Result<hydra::vfs::FileInfo>::error(exists_result.error());
    }

    if (!exists_result.value()) {
        return hydra::vfs::Result<hydra::vfs::FileInfo>::error("File does not exist");
    }

    // Get the file info
    std::string local_path = get_local_path(path);

    try {
        auto file_size = std::filesystem::file_size(local_path);
        auto last_write_time = std::filesystem::last_write_time(local_path);

        // Convert last_write_time to time_t
        auto last_write_time_sys = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
            last_write_time - std::filesystem::file_time_type::clock::now() + std::chrono::system_clock::now());
        auto last_write_time_t = std::chrono::system_clock::to_time_t(last_write_time_sys);

        hydra::vfs::FileInfo info;
        info.path = path;
        info.size = file_size;
        info.last_modified = last_write_time_t;

        return hydra::vfs::Result<hydra::vfs::FileInfo>::success(info);
    } catch (const std::exception& e) {
        return hydra::vfs::Result<hydra::vfs::FileInfo>::error(e.what());
    }
}

hydra::vfs::Result<bool> P2PVFS::create_directory(const std::string& path) {
    // Create the directory locally
    std::string local_path = get_local_path(path);

    try {
        bool success = std::filesystem::create_directories(local_path);
        return hydra::vfs::Result<bool>::success(success);
    } catch (const std::exception& e) {
        return hydra::vfs::Result<bool>::error(e.what());
    }
}

hydra::vfs::Result<bool> P2PVFS::directory_exists(const std::string& path) {
    // Check if the directory exists locally
    std::string local_path = get_local_path(path);

    try {
        bool exists = std::filesystem::exists(local_path) && std::filesystem::is_directory(local_path);
        return hydra::vfs::Result<bool>::success(exists);
    } catch (const std::exception& e) {
        return hydra::vfs::Result<bool>::error(e.what());
    }
}

hydra::vfs::Result<bool> P2PVFS::delete_file(const std::string& path) {
    // Check if the file exists
    auto exists_result = file_exists(path);
    if (!exists_result.success()) {
        return hydra::vfs::Result<bool>::error(exists_result.error());
    }

    if (!exists_result.value()) {
        return hydra::vfs::Result<bool>::error("File does not exist");
    }

    // Delete the file locally
    std::string local_path = get_local_path(path);

    try {
        bool success = std::filesystem::remove(local_path);

        // TODO: Notify peers about the deletion

        return hydra::vfs::Result<bool>::success(success);
    } catch (const std::exception& e) {
        return hydra::vfs::Result<bool>::error(e.what());
    }
}

hydra::vfs::Result<bool> P2PVFS::delete_directory(const std::string& path) {
    return delete_directory_recursive(path, false);
}

hydra::vfs::Result<bool> P2PVFS::delete_directory_recursive(
    const std::string& path,
    bool recursive) {

    // Check if the directory exists
    auto exists_result = directory_exists(path);
    if (!exists_result.success()) {
        return hydra::vfs::Result<bool>::error(exists_result.error());
    }

    if (!exists_result.value()) {
        return hydra::vfs::Result<bool>::error("Directory does not exist");
    }

    // Delete the directory locally
    std::string local_path = get_local_path(path);

    try {
        bool success;
        if (recursive) {
            success = std::filesystem::remove_all(local_path) > 0;
        } else {
            success = std::filesystem::remove(local_path);
        }

        // TODO: Notify peers about the deletion

        return hydra::vfs::Result<bool>::success(success);
    } catch (const std::exception& e) {
        return hydra::vfs::Result<bool>::error(e.what());
    }
}

hydra::vfs::Result<std::vector<std::string>> P2PVFS::list_files(const std::string& path) {
    // Check if the directory exists
    auto exists_result = directory_exists(path);
    if (!exists_result.success()) {
        return hydra::vfs::Result<std::vector<std::string>>::error(exists_result.error());
    }

    if (!exists_result.value()) {
        return hydra::vfs::Result<std::vector<std::string>>::error("Directory does not exist");
    }

    // List files in the directory
    std::string local_path = get_local_path(path);
    std::vector<std::string> files;

    try {
        for (const auto& entry : std::filesystem::directory_iterator(local_path)) {
            if (entry.is_regular_file()) {
                files.push_back(entry.path().filename().string());
            }
        }

        return hydra::vfs::Result<std::vector<std::string>>::success(files);
    } catch (const std::exception& e) {
        return hydra::vfs::Result<std::vector<std::string>>::error(e.what());
    }
}

hydra::vfs::Result<std::vector<std::string>> P2PVFS::list_directories(const std::string& path) {
    // Check if the directory exists
    auto exists_result = directory_exists(path);
    if (!exists_result.success()) {
        return hydra::vfs::Result<std::vector<std::string>>::error(exists_result.error());
    }

    if (!exists_result.value()) {
        return hydra::vfs::Result<std::vector<std::string>>::error("Directory does not exist");
    }

    // List directories in the directory
    std::string local_path = get_local_path(path);
    std::vector<std::string> directories;

    try {
        for (const auto& entry : std::filesystem::directory_iterator(local_path)) {
            if (entry.is_directory()) {
                directories.push_back(entry.path().filename().string());
            }
        }

        return hydra::vfs::Result<std::vector<std::string>>::success(directories);
    } catch (const std::exception& e) {
        return hydra::vfs::Result<std::vector<std::string>>::error(e.what());
    }
}

hydra::vfs::Result<bool> P2PVFS::rename_file(
    const std::string& old_path,
    const std::string& new_path) {

    // Check if the source file exists
    auto exists_result = file_exists(old_path);
    if (!exists_result.success()) {
        return hydra::vfs::Result<bool>::error(exists_result.error());
    }

    if (!exists_result.value()) {
        return hydra::vfs::Result<bool>::error("Source file does not exist");
    }

    // Check if the destination file exists
    exists_result = file_exists(new_path);
    if (!exists_result.success()) {
        return hydra::vfs::Result<bool>::error(exists_result.error());
    }

    if (exists_result.value()) {
        return hydra::vfs::Result<bool>::error("Destination file already exists");
    }

    // Rename the file locally
    std::string local_old_path = get_local_path(old_path);
    std::string local_new_path = get_local_path(new_path);

    try {
        std::filesystem::rename(local_old_path, local_new_path);

        // TODO: Notify peers about the rename

        return hydra::vfs::Result<bool>::success(true);
    } catch (const std::exception& e) {
        return hydra::vfs::Result<bool>::error(e.what());
    }
}

hydra::vfs::Result<bool> P2PVFS::rename_directory(
    const std::string& old_path,
    const std::string& new_path) {

    // Check if the source directory exists
    auto exists_result = directory_exists(old_path);
    if (!exists_result.success()) {
        return hydra::vfs::Result<bool>::error(exists_result.error());
    }

    if (!exists_result.value()) {
        return hydra::vfs::Result<bool>::error("Source directory does not exist");
    }

    // Check if the destination directory exists
    exists_result = directory_exists(new_path);
    if (!exists_result.success()) {
        return hydra::vfs::Result<bool>::error(exists_result.error());
    }

    if (exists_result.value()) {
        return hydra::vfs::Result<bool>::error("Destination directory already exists");
    }

    // Rename the directory locally
    std::string local_old_path = get_local_path(old_path);
    std::string local_new_path = get_local_path(new_path);

    try {
        std::filesystem::rename(local_old_path, local_new_path);

        // TODO: Notify peers about the rename

        return hydra::vfs::Result<bool>::success(true);
    } catch (const std::exception& e) {
        return hydra::vfs::Result<bool>::error(e.what());
    }
}

hydra::vfs::Result<bool> P2PVFS::copy_file(
    const std::string& source_path,
    const std::string& dest_path) {

    // Check if the source file exists
    auto exists_result = file_exists(source_path);
    if (!exists_result.success()) {
        return hydra::vfs::Result<bool>::error(exists_result.error());
    }

    if (!exists_result.value()) {
        return hydra::vfs::Result<bool>::error("Source file does not exist");
    }

    // Check if the destination file exists
    exists_result = file_exists(dest_path);
    if (!exists_result.success()) {
        return hydra::vfs::Result<bool>::error(exists_result.error());
    }

    if (exists_result.value()) {
        return hydra::vfs::Result<bool>::error("Destination file already exists");
    }

    // Copy the file locally
    std::string local_source_path = get_local_path(source_path);
    std::string local_dest_path = get_local_path(dest_path);

    try {
        std::filesystem::copy_file(local_source_path, local_dest_path);

        // TODO: Notify peers about the copy

        return hydra::vfs::Result<bool>::success(true);
    } catch (const std::exception& e) {
        return hydra::vfs::Result<bool>::error(e.what());
    }
}

hydra::vfs::Result<bool> P2PVFS::move_file(
    const std::string& source_path,
    const std::string& destination_path) {

    // Check if the source file exists
    auto exists_result = file_exists(source_path);
    if (!exists_result.success()) {
        return hydra::vfs::Result<bool>::error(exists_result.error());
    }

    if (!exists_result.value()) {
        return hydra::vfs::Result<bool>::error("Source file does not exist");
    }

    // Check if the destination file exists
    exists_result = file_exists(destination_path);
    if (!exists_result.success()) {
        return hydra::vfs::Result<bool>::error(exists_result.error());
    }

    if (exists_result.value()) {
        return hydra::vfs::Result<bool>::error("Destination file already exists");
    }

    // Move the file locally
    std::string local_source_path = get_local_path(source_path);
    std::string local_dest_path = get_local_path(destination_path);

    try {
        std::filesystem::rename(local_source_path, local_dest_path);

        // TODO: Notify peers about the move

        return hydra::vfs::Result<bool>::success(true);
    } catch (const std::exception& e) {
        return hydra::vfs::Result<bool>::error(e.what());
    }
}

bool P2PVFS::add_peer(const std::string& peer_id, const std::string& peer_address) {
    std::lock_guard<std::mutex> lock(m_peers_mutex);

    // Check if the peer already exists
    if (m_peers.find(peer_id) != m_peers.end()) {
        return false;
    }

    // Add the peer
    m_peers[peer_id] = peer_address;
    return true;
}

bool P2PVFS::remove_peer(const std::string& peer_id) {
    std::lock_guard<std::mutex> lock(m_peers_mutex);

    // Check if the peer exists
    if (m_peers.find(peer_id) == m_peers.end()) {
        return false;
    }

    // Remove the peer
    m_peers.erase(peer_id);
    return true;
}

std::vector<std::string> P2PVFS::get_peers() const {
    std::lock_guard<std::mutex> lock(m_peers_mutex);

    std::vector<std::string> peers;
    for (const auto& [peer_id, _] : m_peers) {
        peers.push_back(peer_id);
    }

    return peers;
}

hydra::vfs::Result<std::string> P2PVFS::cat_file(const std::string& path) {
    // Check if the file exists
    auto exists_result = file_exists(path);
    if (!exists_result.success()) {
        return hydra::vfs::Result<std::string>::error(exists_result.error());
    }

    if (!exists_result.value()) {
        return hydra::vfs::Result<std::string>::error("File does not exist");
    }

    // Open the file
    auto file_result = open_file(path, hydra::vfs::FileMode::READ);
    if (!file_result.success()) {
        return hydra::vfs::Result<std::string>::error(file_result.error());
    }

    auto file = file_result.value();

    // Get the file size
    auto info_result = get_file_info(path);
    if (!info_result.success()) {
        return hydra::vfs::Result<std::string>::error(info_result.error());
    }

    size_t file_size = info_result.value().size;

    // Read the file contents
    std::vector<uint8_t> buffer(file_size);
    auto read_result = file->read(buffer.data(), file_size);

    if (!read_result.success()) {
        return hydra::vfs::Result<std::string>::error(read_result.error());
    }

    // Close the file
    file->close();

    // Convert the buffer to a string
    std::string content(buffer.begin(), buffer.end());

    return hydra::vfs::Result<std::string>::success(content);
}

bool P2PVFS::synchronize() {
    // TODO: Implement synchronization with peers
    return true;
}

std::string P2PVFS::get_local_path(const std::string& path) const {
    // Normalize the path
    std::string normalized_path = path;
    std::replace(normalized_path.begin(), normalized_path.end(), '\\', '/');

    // Remove leading slash
    if (!normalized_path.empty() && normalized_path[0] == '/') {
        normalized_path = normalized_path.substr(1);
    }

    // Combine with local storage path
    return m_local_storage_path + "/" + normalized_path;
}

bool P2PVFS::distribute_file(const std::string& path, const std::vector<uint8_t>& data) {
    std::cout << "P2PVFS::distribute_file - Distributing file: " << path << std::endl;
    std::cout << "P2PVFS::distribute_file - Data size: " << data.size() << " bytes" << std::endl;

    // Save the file locally
    std::string local_path = get_local_path(path);

    std::cout << "P2PVFS::distribute_file - Local path: " << local_path << std::endl;

    // Create parent directories if they don't exist
    auto parent_path = std::filesystem::path(local_path).parent_path();
    std::cout << "P2PVFS::distribute_file - Creating parent directories: " << parent_path.string() << std::endl;

    try {
        std::filesystem::create_directories(parent_path);

        // Write the file
        std::ofstream file(local_path, std::ios::binary);
        if (!file) {
            std::cerr << "P2PVFS::distribute_file - Failed to open file for writing: " << local_path << std::endl;
            return false;
        }

        file.write(reinterpret_cast<const char*>(data.data()), data.size());
        file.close();

        std::cout << "P2PVFS::distribute_file - File written successfully: " << local_path << std::endl;

        // TODO: Distribute the file to peers using LMVS

        return true;
    } catch (const std::exception& e) {
        std::cerr << "P2PVFS::distribute_file - Exception: " << e.what() << std::endl;
        return false;
    }
}

hydra::vfs::Result<std::vector<uint8_t>> P2PVFS::retrieve_file(const std::string& path) {
    // Check if the file exists locally
    std::string local_path = get_local_path(path);

    if (!std::filesystem::exists(local_path)) {
        // TODO: Retrieve the file from peers using LMVS
        return hydra::vfs::Result<std::vector<uint8_t>>::error("File does not exist");
    }

    // Read the file
    std::ifstream file(local_path, std::ios::binary);
    if (!file) {
        return hydra::vfs::Result<std::vector<uint8_t>>::error("Failed to open file");
    }

    // Get the file size
    file.seekg(0, std::ios::end);
    size_t size = file.tellg();
    file.seekg(0, std::ios::beg);

    // Read the file data
    std::vector<uint8_t> data(size);
    file.read(reinterpret_cast<char*>(data.data()), size);
    file.close();

    return hydra::vfs::Result<std::vector<uint8_t>>::success(data);
}

} // namespace p2p_vfs
} // namespace lmvs
