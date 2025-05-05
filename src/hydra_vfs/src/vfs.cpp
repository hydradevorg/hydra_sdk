#include "../../include/hydra_vfs/vfs.h"
#include "../../include/hydra_vfs/memory_vfs.h"
#include "../../include/hydra_vfs/persistent_vfs.h"

namespace hydra {
namespace vfs {

/**
 * @brief Factory function to create a new isolated virtual file system
 * 
 * @param root_path Optional physical root path to store data
 * @return std::shared_ptr<IVirtualFileSystem> A new VFS instance
 */
std::shared_ptr<IVirtualFileSystem> create_vfs(const std::string& root_path) {
    if (root_path.empty()) {
        // Create an in-memory VFS if no root path is provided
        return std::make_shared<MemoryVFS>();
    } else {
        // Create a persistent VFS with the given root path
        return std::make_shared<PersistentVFS>(root_path);
    }
}

} // namespace vfs
} // namespace hydra
