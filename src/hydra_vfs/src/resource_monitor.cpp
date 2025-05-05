#include "hydra_vfs/container_vfs.h"
#include <mutex>

namespace hydra {
namespace vfs {

// ResourceMonitor implementation
ResourceMonitor::ResourceMonitor(const ResourceLimits& limits)
    : m_limits(limits)
    , m_storage_usage(0)
    , m_memory_usage(0)
    , m_file_count(0)
    , m_directory_count(0)
{
}

Result<void> ResourceMonitor::check_limits(int64_t storage_delta, int64_t memory_delta, int64_t file_count_delta, size_t file_size) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    // Check storage limit
    if (m_limits.max_storage_size > 0) {
        if (static_cast<int64_t>(m_storage_usage) + storage_delta > static_cast<int64_t>(m_limits.max_storage_size)) {
            return ErrorCode::INVALID_ARGUMENT;
        }
    }
    
    // Check memory limit
    if (m_limits.max_memory_usage > 0) {
        if (static_cast<int64_t>(m_memory_usage) + memory_delta > static_cast<int64_t>(m_limits.max_memory_usage)) {
            return ErrorCode::INVALID_ARGUMENT;
        }
    }
    
    // Check file count limit
    if (m_limits.max_file_count > 0) {
        if (static_cast<int64_t>(m_file_count) + file_count_delta > static_cast<int64_t>(m_limits.max_file_count)) {
            return ErrorCode::INVALID_ARGUMENT;
        }
    }
    
    // Check file size limit
    if (m_limits.max_file_size > 0 && file_size > 0) {
        if (file_size > m_limits.max_file_size) {
            return ErrorCode::INVALID_ARGUMENT;
        }
    }
    
    return {};
}

void ResourceMonitor::update_usage(int64_t storage_delta, int64_t memory_delta, int64_t file_count_delta) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    // Update storage usage
    if (storage_delta > 0) {
        m_storage_usage += storage_delta;
    } else if (storage_delta < 0) {
        m_storage_usage = (static_cast<int64_t>(m_storage_usage) + storage_delta < 0) ? 0 : m_storage_usage - (-storage_delta);
    }
    
    // Update memory usage
    if (memory_delta > 0) {
        m_memory_usage += memory_delta;
    } else if (memory_delta < 0) {
        m_memory_usage = (static_cast<int64_t>(m_memory_usage) + memory_delta < 0) ? 0 : m_memory_usage - (-memory_delta);
    }
    
    // Update file count
    if (file_count_delta > 0) {
        m_file_count += file_count_delta;
    } else if (file_count_delta < 0) {
        m_file_count = (static_cast<int64_t>(m_file_count) + file_count_delta < 0) ? 0 : m_file_count - (-file_count_delta);
    }
}

ResourceMonitor::ResourceUsage ResourceMonitor::get_usage() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    ResourceUsage usage;
    usage.storage_usage = m_storage_usage;
    usage.memory_usage = m_memory_usage;
    usage.file_count = m_file_count;
    usage.directory_count = m_directory_count;
    
    return usage;
}

} // namespace vfs
} // namespace hydra
