#include "lmvs/p2p_vfs/p2p_secure_file.hpp"
#include "lmvs/p2p_vfs/p2p_vfs.hpp"
#include <algorithm>
#include <cstring>

namespace lmvs {
namespace p2p_vfs {

P2PSecureFile::P2PSecureFile(P2PVFS* vfs,
                           const std::string& path,
                           hydra::vfs::FileMode mode,
                           const std::string& node_id,
                           const std::vector<uint8_t>& private_key_bundle)
    : m_vfs(vfs),
      m_path(path),
      m_mode(mode),
      m_node_id(node_id),
      m_private_key_bundle(private_key_bundle),
      m_position(0),
      m_is_open(true),
      m_is_modified(false),
      m_transport(std::make_unique<security::SecureVectorTransport>()) {

    // If the file exists and we're in READ or APPEND mode, load it
    if (mode == hydra::vfs::FileMode::READ ||
        mode == hydra::vfs::FileMode::APPEND ||
        mode == hydra::vfs::FileMode::READ_WRITE) {
        load_file();

        // If in APPEND mode, seek to the end
        if (mode == hydra::vfs::FileMode::APPEND) {
            m_position = m_data.size();
        }
    }

    // If in WRITE mode, clear the data
    if (mode == hydra::vfs::FileMode::WRITE) {
        m_data.clear();
        m_is_modified = true;
    }
}

P2PSecureFile::~P2PSecureFile() {
    close();
}

hydra::vfs::Result<size_t> P2PSecureFile::read(uint8_t* buffer, size_t size) {
    std::lock_guard<std::mutex> lock(m_mutex);

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

hydra::vfs::Result<size_t> P2PSecureFile::write(const uint8_t* buffer, size_t size) {
    std::lock_guard<std::mutex> lock(m_mutex);

    if (!m_is_open) {
        return hydra::vfs::Result<size_t>::error("File is not open");
    }

    if (m_mode != hydra::vfs::FileMode::WRITE &&
        m_mode != hydra::vfs::FileMode::APPEND &&
        m_mode != hydra::vfs::FileMode::READ_WRITE) {
        return hydra::vfs::Result<size_t>::error("File is not open for writing");
    }

    if (m_position + size > m_data.size()) {
        m_data.resize(m_position + size);
    }

    std::memcpy(m_data.data() + m_position, buffer, size);
    m_position += size;
    m_is_modified = true;

    return hydra::vfs::Result<size_t>::success(size);
}

hydra::vfs::Result<size_t> P2PSecureFile::seek(size_t position) {
    std::lock_guard<std::mutex> lock(m_mutex);

    if (!m_is_open) {
        return hydra::vfs::Result<size_t>::error("File is not open");
    }

    m_position = std::min(position, m_data.size());
    return hydra::vfs::Result<size_t>::success(m_position);
}

hydra::vfs::Result<size_t> P2PSecureFile::tell() {
    std::lock_guard<std::mutex> lock(m_mutex);

    if (!m_is_open) {
        return hydra::vfs::Result<size_t>::error("File is not open");
    }

    return hydra::vfs::Result<size_t>::success(m_position);
}

hydra::vfs::Result<bool> P2PSecureFile::flush() {
    std::lock_guard<std::mutex> lock(m_mutex);

    if (!m_is_open) {
        return hydra::vfs::Result<bool>::error("File is not open");
    }

    if (m_is_modified) {
        auto result = save_file();
        if (!result.success()) {
            return result;
        }
        m_is_modified = false;
    }

    return hydra::vfs::Result<bool>::success(true);
}

hydra::vfs::Result<bool> P2PSecureFile::close() {
    std::lock_guard<std::mutex> lock(m_mutex);

    if (!m_is_open) {
        return hydra::vfs::Result<bool>::success(true);
    }

    // Flush any pending changes
    if (m_is_modified) {
        auto result = save_file();
        if (!result.success()) {
            return result;
        }
    }

    m_is_open = false;
    return hydra::vfs::Result<bool>::success(true);
}

hydra::vfs::Result<bool> P2PSecureFile::load_file() {
    try {
        std::cout << "P2PSecureFile::load_file - Loading file: " << m_path << std::endl;

        // Retrieve the file data from the VFS
        auto result = m_vfs->retrieve_file(m_path);
        if (!result.success()) {
            std::cerr << "P2PSecureFile::load_file - Failed to retrieve file: " << result.error() << std::endl;
            return hydra::vfs::Result<bool>::error(result.error());
        }

        std::vector<uint8_t> encrypted_data = result.value();
        std::cout << "P2PSecureFile::load_file - Retrieved data size: " << encrypted_data.size() << " bytes" << std::endl;

        // If the file is empty, return success
        if (encrypted_data.empty()) {
            m_data.clear();
            std::cout << "P2PSecureFile::load_file - File is empty" << std::endl;
            return hydra::vfs::Result<bool>::success(true);
        }

        try {
            // Deserialize the secure package
            security::SecureVectorPackage package = security::SecureVectorPackage::deserialize(encrypted_data);

            // Extract the sender ID from metadata
            std::string sender_id(package.metadata.begin(), package.metadata.end());
            std::cout << "P2PSecureFile::load_file - Sender ID: " << sender_id << std::endl;

            // For local storage, we'll directly deserialize the vector
            LayeredBigIntVector vector = LayeredBigIntVector::deserialize(package.encrypted_data);

            std::cout << "P2PSecureFile::load_file - Deserialized vector with " << vector.getNumLayers() << " layers" << std::endl;

            // Convert the vector to file data
            m_data = vector_to_data(vector);

            std::cout << "P2PSecureFile::load_file - Converted to data size: " << m_data.size() << " bytes" << std::endl;

            return hydra::vfs::Result<bool>::success(true);
        } catch (const std::exception& e) {
            std::cerr << "P2PSecureFile::load_file - Exception during deserialization: " << e.what() << std::endl;

            // If deserialization fails, try to use the data directly
            m_data = encrypted_data;
            std::cout << "P2PSecureFile::load_file - Using raw data: " << m_data.size() << " bytes" << std::endl;

            return hydra::vfs::Result<bool>::success(true);
        }
    } catch (const std::exception& e) {
        std::cerr << "P2PSecureFile::load_file - Exception: " << e.what() << std::endl;
        return hydra::vfs::Result<bool>::error(e.what());
    }
}

hydra::vfs::Result<bool> P2PSecureFile::save_file() {
    try {
        std::cout << "P2PSecureFile::save_file - Saving file: " << m_path << std::endl;
        std::cout << "P2PSecureFile::save_file - Data size: " << m_data.size() << " bytes" << std::endl;

        // Convert file data to a layered vector
        LayeredBigIntVector vector = data_to_vector(m_data);

        std::cout << "P2PSecureFile::save_file - Converted to vector with " << vector.getNumLayers() << " layers" << std::endl;

        // Package the vector for secure storage
        // For local storage, we'll package it for ourselves
        // In a real implementation, we would package it for all peers
        std::cout << "P2PSecureFile::save_file - Packaging vector for node: " << m_node_id << std::endl;

        // Create a simple package without encryption
        security::SecureVectorPackage package;
        package.metadata = std::vector<uint8_t>(m_node_id.begin(), m_node_id.end());

        // Serialize the vector
        std::vector<uint8_t> vector_data = vector.serialize();

        // Set the encrypted data (not actually encrypted for local storage)
        package.encrypted_data = vector_data;

        // Create a simple signature
        package.signature = std::vector<uint8_t>(32, 0); // Dummy signature

        // Create a simple authentication tag
        package.auth_tag = std::vector<uint8_t>(16, 0); // Dummy auth tag

        // Serialize the package
        std::vector<uint8_t> serialized_package = package.serialize();

        std::cout << "P2PSecureFile::save_file - Serialized package size: " << serialized_package.size() << " bytes" << std::endl;

        // Distribute the file to peers
        bool success = m_vfs->distribute_file(m_path, serialized_package);

        std::cout << "P2PSecureFile::save_file - Distribute file result: " << (success ? "Success" : "Failure") << std::endl;

        if (!success) {
            return hydra::vfs::Result<bool>::error("Failed to distribute file");
        }

        return hydra::vfs::Result<bool>::success(true);
    } catch (const std::exception& e) {
        std::cerr << "P2PSecureFile::save_file - Exception: " << e.what() << std::endl;
        return hydra::vfs::Result<bool>::error(e.what());
    }
}

LayeredBigIntVector P2PSecureFile::data_to_vector(const std::vector<uint8_t>& data) {
    // For simplicity, we'll create a vector with a single layer containing the file data
    // In a real implementation, we might want to split the data into multiple layers
    // for better compression and security

    const size_t chunk_size = 1024; // Size of each chunk in the vector
    const size_t num_chunks = (data.size() + chunk_size - 1) / chunk_size; // Ceiling division

    std::vector<std::vector<double>> vector_data;
    vector_data.reserve(num_chunks);

    for (size_t i = 0; i < num_chunks; ++i) {
        size_t chunk_start = i * chunk_size;
        size_t chunk_end = std::min(chunk_start + chunk_size, data.size());
        size_t chunk_length = chunk_end - chunk_start;

        std::vector<double> chunk(chunk_size, 0.0);
        for (size_t j = 0; j < chunk_length; ++j) {
            chunk[j] = static_cast<double>(data[chunk_start + j]);
        }

        vector_data.push_back(chunk);
    }

    return LayeredBigIntVector(vector_data);
}

std::vector<uint8_t> P2PSecureFile::vector_to_data(const LayeredBigIntVector& vector) {
    // Convert the vector back to file data
    const size_t chunk_size = 1024;
    const size_t num_chunks = vector.getNumLayers();

    std::vector<uint8_t> data;
    data.reserve(num_chunks * chunk_size);

    for (size_t i = 0; i < num_chunks; ++i) {
        const auto& layer = vector.getLayer(i);
        std::vector<double> double_layer = layer.toDoubleVector();

        for (size_t j = 0; j < double_layer.size(); ++j) {
            // Skip padding zeros at the end of the last chunk
            if (i == num_chunks - 1 && double_layer[j] == 0.0 && j > 0 && double_layer[j-1] == 0.0) {
                continue;
            }

            data.push_back(static_cast<uint8_t>(double_layer[j]));
        }
    }

    return data;
}

} // namespace p2p_vfs
} // namespace lmvs
