#pragma once

#include <string>
#include <ctime>

namespace hydra {
namespace vfs {

/**
 * @brief Structure containing information about a file
 */
struct FileInfo {
    std::string path;      ///< Path to the file
    size_t size;           ///< Size of the file in bytes
    time_t last_modified;  ///< Last modification time
    bool is_directory;     ///< Whether the file is a directory
};

} // namespace vfs
} // namespace hydra
