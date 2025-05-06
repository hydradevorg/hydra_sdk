#include "hydra_vfs/container_vfs.h"
#include "hydra_vfs/container_path_handler.hpp"
#include "hydra_vfs/container_vfs_fixes.hpp"
#include "hydra_vfs/crypto_utils.hpp"
#include <cstring>
#include <algorithm>
#include <iostream>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <chrono>
#include <random>
#include <sstream>
#include <iomanip>
#include <filesystem>

namespace hydra {
namespace vfs {

// ContainerVFS implementation
ContainerVFS::ContainerVFS(const std::string& container_path,
                         std::shared_ptr<IEncryptionProvider> encryption_provider,
                         const EncryptionKey& key,
                         std::shared_ptr<IVirtualFileSystem> base_vfs,
                         SecurityLevel security_level,
                         const ResourceLimits& resource_limits)
    : m_container_path_handler(std::make_shared<ContainerPathHandler>(container_path))
    , m_encryption_provider(encryption_provider)
    , m_key(key)
    , m_base_vfs(base_vfs)
    , m_resource_monitor(resource_limits)
    , m_initialized(false)
{
    std::cout << "DEBUG: ContainerVFS constructor called" << std::endl;

    // Create hardware security module based on security level
    auto hsm_result = create_hardware_security_module();
    if (hsm_result) {
        std::cout << "DEBUG: Hardware security module created successfully" << std::endl;
        m_hsm = hsm_result.value();
    } else {
        std::cout << "DEBUG: Failed to create hardware security module: " << static_cast<int>(hsm_result.error()) << std::endl;
    }

    // Create directories if needed
    if (!m_container_path_handler->get_container_directory().empty()) {
        std::cout << "DEBUG: Creating parent directories: " << m_container_path_handler->get_container_directory() << std::endl;
        std::filesystem::create_directories(m_container_path_handler->get_container_directory());
    }

    // Initialize the container (open or create)
    auto result = ContainerVFSFixes::initialize_container(*this, m_container_path_handler);
    if (result.error() != ErrorCode::SUCCESS) {
        std::cerr << "DEBUG: Failed to initialize container: " << static_cast<int>(result.error()) << std::endl;
        // Handle initialization error, but don't return from constructor
        m_initialized = false;
    } else {
        m_initialized = true;
    }
}

ContainerVFS::~ContainerVFS()
{
    std::lock_guard<std::mutex> lock(m_mutex);

    if (m_initialized) {
        std::cout << "DEBUG: ContainerVFS destructor called, saving metadata" << std::endl;

        // Save metadata before closing
        save_metadata();

        // Close container file
        if (m_container_file) {
            m_container_file->close();
            std::cout << "DEBUG: Container file closed successfully" << std::endl;
        }
    } else {
        std::cout << "DEBUG: ContainerVFS destructor called, but not initialized" << std::endl;
    }
}

Result<void> ContainerVFS::initialize_container()
{
    std::cout << "DEBUG: Initializing container..." << std::endl;

    try {
        // Check if container file exists
        std::string container_path = m_container_path_handler->get_absolute_container_path();
        std::cout << "DEBUG: Checking if container file exists at " << container_path << std::endl;

        bool file_exists = m_container_path_handler->container_file_exists();
        std::cout << "DEBUG: Container file exists: " << (file_exists ? "yes" : "no") << std::endl;

        // Use the ContainerVFSFixes implementation
        return ContainerVFSFixes::initialize_container(*this, m_container_path_handler);
    } catch (const std::exception& e) {
        std::cout << "DEBUG: Exception during container initialization: " << e.what() << std::endl;
        return Result<void>(ErrorCode::UNKNOWN_ERROR);
    } catch (...) {
        std::cout << "DEBUG: Unknown exception during container initialization" << std::endl;
        return Result<void>(ErrorCode::UNKNOWN_ERROR);
    }
}