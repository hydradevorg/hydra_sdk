#pragma once

#include "hydra_vfs/ivirtual_file_system.hpp"
#include "hydra_vfs/result.hpp"
#include "hydra_vfs/file_info.hpp"
#include "hydra_vfs/file_mode.hpp"
#include "hydra_vfs/ifile.hpp"
#include "lmvs/layered_bigint_vector.hpp"
#include "lmvs/layered_bigint_matrix.hpp"
#include "lmvs/security/secure_vector_transport.hpp"
#include <string>
#include <memory>
#include <vector>
#include <unordered_map>
#include <mutex>

namespace lmvs {
namespace p2p_vfs {

/**
 * @brief Peer-to-Peer Virtual File System using the Layered Matrix and Vector System
 *
 * This class implements a P2P VFS that uses the LMVS for secure, fault-tolerant
 * distributed file storage.
 */
class P2PVFS : public hydra::vfs::IVirtualFileSystem {
public:
    /**
     * @brief Construct a new P2PVFS
     *
     * @param node_id Unique identifier for this node
     * @param local_storage_path Path to local storage directory
     * @param num_layers Number of layers for LMVS vectors
     * @param threshold Minimum number of nodes required for consensus
     */
    P2PVFS(const std::string& node_id,
           const std::string& local_storage_path,
           size_t num_layers = 3,
           size_t threshold = 2);

    /**
     * @brief Destroy the P2PVFS
     */
    virtual ~P2PVFS();

    // IVirtualFileSystem interface implementation

    /**
     * @brief Open a file
     *
     * @param path Path to the file
     * @param mode File mode (READ, WRITE, APPEND)
     * @return hydra::vfs::Result<std::shared_ptr<hydra::vfs::IFile>> Result containing the file or an error
     */
    hydra::vfs::Result<std::shared_ptr<hydra::vfs::IFile>> open_file(
        const std::string& path,
        hydra::vfs::FileMode mode) override;

    /**
     * @brief Check if a file exists
     *
     * @param path Path to the file
     * @return hydra::vfs::Result<bool> Result containing true if the file exists, false otherwise
     */
    hydra::vfs::Result<bool> file_exists(const std::string& path) override;

    /**
     * @brief Get information about a file
     *
     * @param path Path to the file
     * @return hydra::vfs::Result<hydra::vfs::FileInfo> Result containing file information or an error
     */
    hydra::vfs::Result<hydra::vfs::FileInfo> get_file_info(const std::string& path) override;

    /**
     * @brief Create a directory
     *
     * @param path Path to the directory
     * @return hydra::vfs::Result<bool> Result containing true if successful, false otherwise
     */
    hydra::vfs::Result<bool> create_directory(const std::string& path) override;

    /**
     * @brief Check if a directory exists
     *
     * @param path Path to the directory
     * @return hydra::vfs::Result<bool> Result containing true if the directory exists, false otherwise
     */
    hydra::vfs::Result<bool> directory_exists(const std::string& path) override;

    /**
     * @brief Delete a file
     *
     * @param path Path to the file
     * @return hydra::vfs::Result<bool> Result containing true if successful, false otherwise
     */
    hydra::vfs::Result<bool> delete_file(const std::string& path) override;

    /**
     * @brief Delete a directory
     *
     * @param path Path to the directory
     * @return hydra::vfs::Result<bool> Result containing true if successful, false otherwise
     */
    hydra::vfs::Result<bool> delete_directory(const std::string& path) override;

    /**
     * @brief Delete a directory recursively
     *
     * @param path Path to the directory
     * @param recursive Whether to delete recursively
     * @return hydra::vfs::Result<bool> Result containing true if successful, false otherwise
     */
    hydra::vfs::Result<bool> delete_directory_recursive(
        const std::string& path,
        bool recursive = true);

    /**
     * @brief List files in a directory
     *
     * @param path Path to the directory
     * @return hydra::vfs::Result<std::vector<std::string>> Result containing list of files or an error
     */
    hydra::vfs::Result<std::vector<std::string>> list_files(const std::string& path) override;

    /**
     * @brief List directories in a directory
     *
     * @param path Path to the directory
     * @return hydra::vfs::Result<std::vector<std::string>> Result containing list of directories or an error
     */
    hydra::vfs::Result<std::vector<std::string>> list_directories(const std::string& path) override;

    /**
     * @brief Rename a file
     *
     * @param old_path Old path
     * @param new_path New path
     * @return hydra::vfs::Result<bool> Result containing true if successful, false otherwise
     */
    hydra::vfs::Result<bool> rename_file(
        const std::string& old_path,
        const std::string& new_path);

    /**
     * @brief Rename a directory
     *
     * @param old_path Old path
     * @param new_path New path
     * @return hydra::vfs::Result<bool> Result containing true if successful, false otherwise
     */
    hydra::vfs::Result<bool> rename_directory(
        const std::string& old_path,
        const std::string& new_path);

    /**
     * @brief Copy a file
     *
     * @param source_path Source path
     * @param dest_path Destination path
     * @return hydra::vfs::Result<bool> Result containing true if successful, false otherwise
     */
    hydra::vfs::Result<bool> copy_file(
        const std::string& source_path,
        const std::string& dest_path) override;

    /**
     * @brief Move a file
     *
     * @param source_path Source path
     * @param destination_path Destination path
     * @return hydra::vfs::Result<bool> Result containing true if successful, false otherwise
     */
    hydra::vfs::Result<bool> move_file(
        const std::string& source_path,
        const std::string& destination_path) override;

    // P2P-specific methods

    /**
     * @brief Add a peer to the network
     *
     * @param peer_id Unique identifier for the peer
     * @param peer_address Network address of the peer
     * @return bool True if the peer was added successfully
     */
    bool add_peer(const std::string& peer_id, const std::string& peer_address);

    /**
     * @brief Remove a peer from the network
     *
     * @param peer_id Unique identifier for the peer
     * @return bool True if the peer was removed successfully
     */
    bool remove_peer(const std::string& peer_id);

    /**
     * @brief Get the list of peers
     *
     * @return std::vector<std::string> List of peer IDs
     */
    std::vector<std::string> get_peers() const;

    /**
     * @brief Get the local storage path
     *
     * @return std::string Local storage path
     */
    std::string get_local_storage_path() const {
        return m_local_storage_path;
    }

    /**
     * @brief Display the contents of a file (similar to the 'cat' command)
     *
     * @param path Path to the file
     * @return hydra::vfs::Result<std::string> Result containing the file contents or an error
     */
    hydra::vfs::Result<std::string> cat_file(const std::string& path);

    /**
     * @brief Synchronize with peers
     *
     * @return bool True if synchronization was successful
     */
    bool synchronize();

    // File distribution and retrieval methods
    bool distribute_file(const std::string& path, const std::vector<uint8_t>& data);
    hydra::vfs::Result<std::vector<uint8_t>> retrieve_file(const std::string& path);

private:
    std::string m_node_id;
    std::string m_local_storage_path;
    size_t m_num_layers;
    size_t m_threshold;

    std::unordered_map<std::string, std::string> m_peers; // peer_id -> peer_address
    mutable std::mutex m_peers_mutex;

    std::unique_ptr<security::SecureVectorTransport> m_transport;
    std::vector<uint8_t> m_private_key_bundle;

    // Helper methods
    std::string get_local_path(const std::string& path) const;
};

} // namespace p2p_vfs
} // namespace lmvs
