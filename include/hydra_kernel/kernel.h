#ifndef HYDRA_KERNEL_H
#define HYDRA_KERNEL_H

#include <memory>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <thread>
#include <atomic>
#include <hydra_vfs/vfs.h>
#include <hydra_kernel/process.h>
#include <hydra_kernel/network.h>

namespace hydra {
namespace kernel {

/**
 * @brief Kernel statistics
 */
struct KernelStats {
    size_t total_processes = 0;      // Total number of processes
    size_t running_processes = 0;     // Number of running processes
    size_t total_memory_usage = 0;    // Total memory usage in bytes
    float total_cpu_usage = 0.0f;     // Total CPU usage as percentage (0.0 - 1.0)
    size_t total_threads = 0;         // Total number of threads
    size_t total_ports = 0;           // Total number of ports
    uint64_t uptime = 0;              // Kernel uptime in milliseconds
};

/**
 * @brief Hydra Kernel - Virtual process manager
 */
class HydraKernel {
public:
    /**
     * @brief Create a new kernel
     * 
     * @param root_fs Root file system for the kernel
     */
    explicit HydraKernel(std::shared_ptr<hydra::vfs::IVirtualFileSystem> root_fs);
    
    /**
     * @brief Destructor
     */
    ~HydraKernel();
    
    // Kernel cannot be copied
    HydraKernel(const HydraKernel&) = delete;
    HydraKernel& operator=(const HydraKernel&) = delete;
    
    /**
     * @brief Create a new process
     * 
     * @param name Process name
     * @param share_fs Whether to share the parent's file system
     * @param parent_pid Parent process ID (0 for root process)
     * @return Shared pointer to the created process, or nullptr on failure
     */
    std::shared_ptr<Process> createProcess(const std::string& name, bool share_fs = false, int parent_pid = 0);
    
    /**
     * @brief Get a process by its ID
     * 
     * @param pid Process ID
     * @return Shared pointer to the process, or nullptr if not found
     */
    std::shared_ptr<Process> getProcess(int pid);
    
    /**
     * @brief Terminate a process
     * 
     * @param pid Process ID
     * @param exit_code Exit code to set
     * @return True if process was terminated, false if not found
     */
    bool terminateProcess(int pid, int exit_code = 1);
    
    /**
     * @brief List all processes
     * 
     * @return Vector of process pointers
     */
    std::vector<std::shared_ptr<Process>> listProcesses();
    
    /**
     * @brief Get kernel statistics
     */
    KernelStats getStats();
    
    /**
     * @brief Get the file system for a process
     * 
     * @param pid Process ID
     * @return File system for the process, or nullptr if not found
     */
    std::shared_ptr<hydra::vfs::IVirtualFileSystem> getProcessFS(int pid);
    
    /**
     * @brief Create a new port
     * 
     * @param port_number Internal port number
     * @param external_port External port number (0 to use internal port number)
     * @param protocol Protocol ("tcp" or "udp")
     * @return True if port was created, false if already exists
     */
    bool createPort(int port_number, int external_port = 0, const std::string& protocol = "tcp");
    
    /**
     * @brief Bind a process to a port
     * 
     * @param pid Process ID
     * @param port_number Internal port number
     * @return True if bound successfully, false otherwise
     */
    bool bindProcessToPort(int pid, int port_number);
    
    /**
     * @brief Connect a process to a port
     * 
     * @param pid Process ID
     * @param port_number Internal port number
     * @return True if connected successfully, false otherwise
     */
    bool connectToPort(int pid, int port_number);
    
    /**
     * @brief List all ports
     * 
     * @return Map of port numbers to bound processes
     */
    std::unordered_map<int, int> listPorts();
    
    /**
     * @brief Close a port
     * 
     * @param port_number Internal port number
     * @return True if closed successfully, false otherwise
     */
    bool closePort(int port_number);
    
    /**
     * @brief Set kernel isolation mode
     * 
     * @param isolation_mode Isolation mode (0 = none, 1 = partial, 2 = complete)
     */
    void setIsolationMode(int isolation_mode);
    
    /**
     * @brief Get kernel isolation mode
     * 
     * @return Isolation mode (0 = none, 1 = partial, 2 = complete)
     */
    int getIsolationMode() const;
    
    /**
     * @brief Start the kernel and all services
     * 
     * @return True if started successfully
     */
    bool start();
    
    /**
     * @brief Stop the kernel and all services
     */
    void stop();
    
    /**
     * @brief Check if kernel is running
     */
    bool isRunning() const;

private:
    std::shared_ptr<hydra::vfs::IVirtualFileSystem> m_root_fs;
    std::unordered_map<int, std::shared_ptr<Process>> m_processes;
    std::unordered_map<int, std::shared_ptr<hydra::vfs::IVirtualFileSystem>> m_process_fs;
    std::unordered_map<int, int> m_ports; // Map of port numbers to bound process IDs
    std::unordered_map<int, std::shared_ptr<PortForwarder>> m_port_forwarders;
    
    int m_isolation_mode = 2; // Default: complete isolation
    std::atomic<bool> m_running;
    uint64_t m_start_time;
    
    std::thread m_watchdog_thread;
    std::mutex m_mutex;
    
    // Internal methods
    void watchdogThread();
    void cleanupTerminatedProcesses();
    void updateStats();
};

/**
 * @brief Get the global kernel instance
 * 
 * @return Reference to the global kernel instance
 */
HydraKernel& getKernel();

} // namespace kernel
} // namespace hydra

#endif // HYDRA_KERNEL_H
