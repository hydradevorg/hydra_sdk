#include "hydra_vfs/container_vfs.h"
#include <chrono>

namespace hydra {
namespace vfs {

// Define container format version
constexpr uint32_t CONTAINER_FORMAT_VERSION = 1;

// ContainerMetadata implementation
ContainerMetadata::ContainerMetadata() 
    : version(CONTAINER_FORMAT_VERSION)
    , creation_time(std::chrono::duration_cast<std::chrono::seconds>(
                    std::chrono::system_clock::now().time_since_epoch()).count())
    , last_modified_time(creation_time)
{
}

} // namespace vfs
} // namespace hydra
