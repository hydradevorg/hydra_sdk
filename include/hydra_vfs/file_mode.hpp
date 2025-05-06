#pragma once

namespace hydra {
namespace vfs {

/**
 * @brief Enumeration of file access modes
 */
enum class FileMode {
    READ,       ///< Open for reading
    WRITE,      ///< Open for writing (creates a new file or truncates an existing one)
    APPEND,     ///< Open for appending (creates a new file if it doesn't exist)
    READ_WRITE  ///< Open for both reading and writing
};

} // namespace vfs
} // namespace hydra
