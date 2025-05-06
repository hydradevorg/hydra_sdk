#pragma once

#include "hydra_vfs/ifile.hpp"
#include "hydra_vfs/result.hpp"
#include "hydra_vfs/file_mode.hpp"
#include "lmvs/security/secure_vector_transport.hpp"
#include <string>
#include <vector>
#include <memory>
#include <mutex>

namespace lmvs {
namespace p2p_vfs {

class P2PVFS; // Forward declaration

/**
 * @brief Secure file implementation for P2P VFS
 *
 * This class implements a secure file that uses the LMVS security features
 * for encryption, authentication, and integrity verification.
 */
class P2PSecureFile : public hydra::vfs::IFile {
public:
    /**
     * @brief Construct a new P2P Secure File
     *
     * @param vfs Pointer to the P2P VFS
     * @param path Path to the file
     * @param mode File mode (READ, WRITE, APPEND)
     * @param node_id ID of the local node
     * @param private_key_bundle Private key bundle of the local node
     */
    P2PSecureFile(P2PVFS* vfs,
                 const std::string& path,
                 hydra::vfs::FileMode mode,
                 const std::string& node_id,
                 const std::vector<uint8_t>& private_key_bundle);

    /**
     * @brief Destroy the P2P Secure File
     */
    ~P2PSecureFile() override;

    // IFile interface implementation

    /**
     * @brief Read from the file
     *
     * @param buffer Buffer to read into
     * @param size Number of bytes to read
     * @return hydra::vfs::Result<size_t> Result containing the number of bytes read or an error
     */
    hydra::vfs::Result<size_t> read(uint8_t* buffer, size_t size) override;

    /**
     * @brief Write to the file
     *
     * @param buffer Buffer to write from
     * @param size Number of bytes to write
     * @return hydra::vfs::Result<size_t> Result containing the number of bytes written or an error
     */
    hydra::vfs::Result<size_t> write(const uint8_t* buffer, size_t size) override;

    /**
     * @brief Seek to a position in the file
     *
     * @param position Position to seek to
     * @return hydra::vfs::Result<size_t> Result containing the new position or an error
     */
    hydra::vfs::Result<size_t> seek(size_t position) override;

    /**
     * @brief Get the current position in the file
     *
     * @return hydra::vfs::Result<size_t> Result containing the current position or an error
     */
    hydra::vfs::Result<size_t> tell() override;

    /**
     * @brief Flush the file to storage
     *
     * @return hydra::vfs::Result<bool> Result containing true if successful, false otherwise
     */
    hydra::vfs::Result<bool> flush() override;

    /**
     * @brief Close the file
     *
     * @return hydra::vfs::Result<bool> Result containing true if successful, false otherwise
     */
    hydra::vfs::Result<bool> close() override;

private:
    P2PVFS* m_vfs;
    std::string m_path;
    hydra::vfs::FileMode m_mode;
    std::string m_node_id;
    std::vector<uint8_t> m_private_key_bundle;
    std::vector<uint8_t> m_data;
    size_t m_position;
    bool m_is_open;
    bool m_is_modified;
    std::mutex m_mutex;

    std::unique_ptr<security::SecureVectorTransport> m_transport;

    // Helper methods
    hydra::vfs::Result<bool> load_file();
    hydra::vfs::Result<bool> save_file();

    // Convert between file data and layered vector
    LayeredBigIntVector data_to_vector(const std::vector<uint8_t>& data);
    std::vector<uint8_t> vector_to_data(const LayeredBigIntVector& vector);
};

} // namespace p2p_vfs
} // namespace lmvs
