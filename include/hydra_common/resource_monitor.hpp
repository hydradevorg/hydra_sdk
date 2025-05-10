#pragma once

#include <unordered_map>
#include <string>
#include <mutex>
#include <atomic>
#include <chrono>
#include <functional>
#include <memory>
#include <thread>
#include <vector>
#include <iostream>
#include "error_handling.hpp"

namespace hydra {
namespace common {

/**
 * @brief Resource type enumeration
 */
enum class ResourceType {
    MEMORY,         ///< Memory usage
    CPU,            ///< CPU usage
    DISK_SPACE,     ///< Disk space
    DISK_IO,        ///< Disk I/O operations
    NETWORK,        ///< Network usage
    FILE_HANDLES,   ///< Open file handles
    THREADS,        ///< Thread count
    CONNECTIONS,    ///< Network connections
    CUSTOM          ///< Custom resource type
};

/**
 * @brief Resource limit configuration
 */
struct ResourceLimit {
    ResourceType type;              ///< Resource type
    std::string name;               ///< Resource name
    size_t soft_limit;              ///< Soft limit (warning)
    size_t hard_limit;              ///< Hard limit (error)
    std::function<void()> callback; ///< Callback when limit is reached
    
    /**
     * @brief Constructor
     * @param type Resource type
     * @param name Resource name
     * @param soft_limit Soft limit
     * @param hard_limit Hard limit
     * @param callback Callback when limit is reached
     */
    ResourceLimit(
        ResourceType type,
        const std::string& name,
        size_t soft_limit,
        size_t hard_limit,
        std::function<void()> callback = nullptr
    ) : type(type),
        name(name),
        soft_limit(soft_limit),
        hard_limit(hard_limit),
        callback(callback) {}
};

/**
 * @brief Resource usage statistics
 */
struct ResourceUsage {
    ResourceType type;              ///< Resource type
    std::string name;               ///< Resource name
    size_t current;                 ///< Current usage
    size_t peak;                    ///< Peak usage
    std::chrono::system_clock::time_point timestamp; ///< Last update timestamp
    
    /**
     * @brief Constructor
     * @param type Resource type
     * @param name Resource name
     * @param current Current usage
     */
    ResourceUsage(
        ResourceType type,
        const std::string& name,
        size_t current = 0
    ) : type(type),
        name(name),
        current(current),
        peak(current),
        timestamp(std::chrono::system_clock::now()) {}
    
    /**
     * @brief Update the usage
     * @param new_value New usage value
     */
    void update(size_t new_value) {
        current = new_value;
        if (new_value > peak) {
            peak = new_value;
        }
        timestamp = std::chrono::system_clock::now();
    }
};

/**
 * @brief Resource monitor for tracking and limiting resource usage
 */
class ResourceMonitor {
public:
    /**
     * @brief Get the singleton instance
     * @return ResourceMonitor instance
     */
    static ResourceMonitor& getInstance() {
        static ResourceMonitor instance;
        return instance;
    }
    
    /**
     * @brief Register a resource limit
     * @param limit Resource limit
     */
    void registerLimit(const ResourceLimit& limit) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_limits[limit.name] = limit;
        
        // Initialize usage if not exists
        if (m_usage.find(limit.name) == m_usage.end()) {
            m_usage[limit.name] = ResourceUsage(limit.type, limit.name);
        }
    }
    
    /**
     * @brief Update resource usage
     * @param name Resource name
     * @param value Current usage value
     * @return True if the usage is within limits
     */
    bool updateUsage(const std::string& name, size_t value) {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        // Update usage
        if (m_usage.find(name) == m_usage.end()) {
            m_usage[name] = ResourceUsage(ResourceType::CUSTOM, name, value);
        } else {
            m_usage[name].update(value);
        }
        
        // Check limits
        if (m_limits.find(name) != m_limits.end()) {
            const auto& limit = m_limits[name];
            
            // Check hard limit
            if (value > limit.hard_limit) {
                // Report error
                ErrorHandling::reportError(
                    "Resource limit exceeded: " + name + " (" + 
                    std::to_string(value) + " > " + std::to_string(limit.hard_limit) + ")",
                    ErrorSeverity::ERROR,
                    ErrorCategory::RESOURCE
                );
                
                // Call callback if exists
                if (limit.callback) {
                    limit.callback();
                }
                
                return false;
            }
            
            // Check soft limit
            if (value > limit.soft_limit) {
                // Report warning
                ErrorHandling::reportError(
                    "Resource limit warning: " + name + " (" + 
                    std::to_string(value) + " > " + std::to_string(limit.soft_limit) + ")",
                    ErrorSeverity::WARNING,
                    ErrorCategory::RESOURCE
                );
            }
        }
        
        return true;
    }
    
    /**
     * @brief Get current resource usage
     * @param name Resource name
     * @return Current usage or 0 if not found
     */
    size_t getCurrentUsage(const std::string& name) const {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        auto it = m_usage.find(name);
        if (it != m_usage.end()) {
            return it->second.current;
        }
        
        return 0;
    }
    
    /**
     * @brief Get peak resource usage
     * @param name Resource name
     * @return Peak usage or 0 if not found
     */
    size_t getPeakUsage(const std::string& name) const {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        auto it = m_usage.find(name);
        if (it != m_usage.end()) {
            return it->second.peak;
        }
        
        return 0;
    }
    
    /**
     * @brief Reset peak usage
     * @param name Resource name
     */
    void resetPeakUsage(const std::string& name) {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        auto it = m_usage.find(name);
        if (it != m_usage.end()) {
            it->second.peak = it->second.current;
        }
    }
    
    /**
     * @brief Get all resource usage
     * @return Map of resource name to usage
     */
    std::unordered_map<std::string, ResourceUsage> getAllUsage() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_usage;
    }
    
    /**
     * @brief Start monitoring thread
     * @param interval_ms Monitoring interval in milliseconds
     */
    void startMonitoring(int interval_ms = 1000) {
        if (m_monitoring.load()) {
            return;
        }
        
        m_monitoring.store(true);
        m_monitor_thread = std::thread([this, interval_ms]() {
            while (m_monitoring.load()) {
                // Update system resources
                updateSystemResources();
                
                // Sleep for interval
                std::this_thread::sleep_for(std::chrono::milliseconds(interval_ms));
            }
        });
    }
    
    /**
     * @brief Stop monitoring thread
     */
    void stopMonitoring() {
        if (!m_monitoring.load()) {
            return;
        }
        
        m_monitoring.store(false);
        if (m_monitor_thread.joinable()) {
            m_monitor_thread.join();
        }
    }
    
    /**
     * @brief Destructor
     */
    ~ResourceMonitor() {
        stopMonitoring();
    }
    
private:
    /**
     * @brief Constructor
     */
    ResourceMonitor() : m_monitoring(false) {
        // Initialize default resource limits
        registerLimit(ResourceLimit(ResourceType::MEMORY, "process_memory", 
                                   1024 * 1024 * 1024, // 1 GB soft limit
                                   2 * 1024 * 1024 * 1024)); // 2 GB hard limit
        
        registerLimit(ResourceLimit(ResourceType::FILE_HANDLES, "file_handles", 
                                   900, // 900 files soft limit
                                   1000)); // 1000 files hard limit
        
        registerLimit(ResourceLimit(ResourceType::THREADS, "thread_count", 
                                   50, // 50 threads soft limit
                                   100)); // 100 threads hard limit
    }
    
    /**
     * @brief Update system resource usage
     */
    void updateSystemResources() {
        // Update process memory usage
        updateProcessMemoryUsage();
        
        // Update file handle count
        updateFileHandleCount();
        
        // Update thread count
        updateThreadCount();
    }
    
    /**
     * @brief Update process memory usage
     */
    void updateProcessMemoryUsage() {
        // This is platform-dependent, simplified for example
        size_t memory_usage = 0;
        
        // On Linux, we would read /proc/self/statm
        // On macOS, we would use task_info
        // For now, just use a placeholder
        memory_usage = 100 * 1024 * 1024; // 100 MB placeholder
        
        updateUsage("process_memory", memory_usage);
    }
    
    /**
     * @brief Update file handle count
     */
    void updateFileHandleCount() {
        // This is platform-dependent, simplified for example
        size_t file_handles = 0;
        
        // On Linux, we would count entries in /proc/self/fd
        // On macOS, we would use proc_pidinfo
        // For now, just use a placeholder
        file_handles = 10; // 10 files placeholder
        
        updateUsage("file_handles", file_handles);
    }
    
    /**
     * @brief Update thread count
     */
    void updateThreadCount() {
        // This is platform-dependent, simplified for example
        size_t thread_count = 0;
        
        // On Linux, we would count entries in /proc/self/task
        // On macOS, we would use task_threads
        // For now, just use a placeholder
        thread_count = 5; // 5 threads placeholder
        
        updateUsage("thread_count", thread_count);
    }
    
    mutable std::mutex m_mutex;
    std::unordered_map<std::string, ResourceLimit> m_limits;
    std::unordered_map<std::string, ResourceUsage> m_usage;
    std::atomic<bool> m_monitoring;
    std::thread m_monitor_thread;
};

/**
 * @brief Resource guard for automatic resource tracking
 */
class ResourceGuard {
public:
    /**
     * @brief Constructor
     * @param resource_name Resource name
     * @param initial_usage Initial usage
     */
    ResourceGuard(const std::string& resource_name, size_t initial_usage = 1)
        : m_resource_name(resource_name), m_usage(initial_usage) {
        // Update resource usage
        ResourceMonitor::getInstance().updateUsage(m_resource_name, m_usage);
    }
    
    /**
     * @brief Destructor
     */
    ~ResourceGuard() {
        // Release resource
        ResourceMonitor::getInstance().updateUsage(m_resource_name, 0);
    }
    
    /**
     * @brief Update usage
     * @param new_usage New usage value
     * @return True if the usage is within limits
     */
    bool updateUsage(size_t new_usage) {
        m_usage = new_usage;
        return ResourceMonitor::getInstance().updateUsage(m_resource_name, m_usage);
    }
    
    /**
     * @brief Increment usage
     * @param increment Increment value
     * @return True if the usage is within limits
     */
    bool increment(size_t increment = 1) {
        m_usage += increment;
        return ResourceMonitor::getInstance().updateUsage(m_resource_name, m_usage);
    }
    
    /**
     * @brief Decrement usage
     * @param decrement Decrement value
     * @return True if the usage is within limits
     */
    bool decrement(size_t decrement = 1) {
        if (m_usage >= decrement) {
            m_usage -= decrement;
        } else {
            m_usage = 0;
        }
        return ResourceMonitor::getInstance().updateUsage(m_resource_name, m_usage);
    }
    
private:
    std::string m_resource_name;
    size_t m_usage;
};

} // namespace common
} // namespace hydra
