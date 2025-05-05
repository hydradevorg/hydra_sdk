#include <hydra_kernel/kernel.h>
#include <iostream>
#include <chrono>
#include <thread>
#include <algorithm>

namespace hydra {
namespace kernel {

HydraKernel::HydraKernel(std::shared_ptr<hydra::vfs::IVirtualFileSystem> root_fs)
    : m_root_fs(root_fs),
      m_running(false),
      m_start_time(0) {
    
    std::cout << "Hydra Kernel initialized" << std::endl;
}

HydraKernel::~HydraKernel() {
    // Stop the kernel if it's running
    if (m_running.load()) {
        stop();
    }
    
    // Wait for watchdog thread to complete
    if (m_watchdog_thread.joinable()) {
        m_watchdog_thread.join();
    }
    
    std::cout << "Hydra Kernel destroyed" << std::endl;
}

std::shared_ptr<Process> HydraKernel::createProcess(const std::string& name, bool share_fs, int parent_pid) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    // Check if parent process exists if a parent PID was provided
    if (parent_pid != 0 && m_processes.find(parent_pid) == m_processes.end()) {
        std::cerr << "Parent process not found: " << parent_pid << std::endl;
        return nullptr;
    }
    
    // Create a new file system if not sharing
    std::shared_ptr<hydra::vfs::IVirtualFileSystem> process_fs;
    if (share_fs && parent_pid != 0) {
        // Share parent's file system
        process_fs = m_process_fs[parent_pid];
    } else {
        // Create a new file system
        process_fs = m_root_fs;
    }
    
    // Create the process
    auto process = std::make_shared<Process>(name, process_fs, parent_pid);
    if (!process) {
        std::cerr << "Failed to create process: " << name << std::endl;
        return nullptr;
    }
    
    // Store the process
    int pid = process->getPID();
    m_processes[pid] = process;
    m_process_fs[pid] = process_fs;
    
    std::cout << "Process created: " << name << " (PID: " << pid << ")" << std::endl;
    return process;
}

std::shared_ptr<Process> HydraKernel::getProcess(int pid) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    auto it = m_processes.find(pid);
    if (it == m_processes.end()) {
        return nullptr;
    }
    
    return it->second;
}

bool HydraKernel::terminateProcess(int pid, int exit_code) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    auto it = m_processes.find(pid);
    if (it == m_processes.end()) {
        return false;
    }
    
    // Terminate the process
    it->second->terminate(exit_code);
    
    return true;
}

std::vector<std::shared_ptr<Process>> HydraKernel::listProcesses() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    std::vector<std::shared_ptr<Process>> processes;
    for (const auto& pair : m_processes) {
        processes.push_back(pair.second);
    }
    
    return processes;
}

KernelStats HydraKernel::getStats() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    KernelStats stats;
    
    // Count processes
    stats.total_processes = m_processes.size();
    
    // Count running processes and sum resource usage
    for (const auto& pair : m_processes) {
        auto process = pair.second;
        
        if (process->isRunning()) {
            stats.running_processes++;
        }
        
        auto process_stats = process->getStats();
        stats.total_memory_usage += process_stats.memory_usage;
        stats.total_cpu_usage += process_stats.cpu_usage;
        stats.total_threads += process_stats.threads_count;
    }
    
    // Count ports
    stats.total_ports = m_ports.size();
    
    // Calculate uptime
    if (m_start_time > 0) {
        uint64_t now = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        stats.uptime = now - m_start_time;
    }
    
    return stats;
}

std::shared_ptr<hydra::vfs::IVirtualFileSystem> HydraKernel::getProcessFS(int pid) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    auto it = m_process_fs.find(pid);
    if (it == m_process_fs.end()) {
        return nullptr;
    }
    
    return it->second;
}

bool HydraKernel::createPort(int port_number, int external_port, const std::string& protocol) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    // Check if port already exists
    if (m_ports.find(port_number) != m_ports.end()) {
        return false;
    }
    
    // Create port mapping
    m_ports[port_number] = 0; // Not bound to any process yet
    
    // Create port forwarder if isolation mode is not complete
    if (m_isolation_mode != 2) {
        if (external_port == 0) {
            external_port = port_number;
        }
        
        auto port_forwarder = std::make_shared<PortForwarder>();
        port_forwarder->addPortMapping(port_number, external_port, protocol);
        m_port_forwarders[port_number] = port_forwarder;
        
        // Start the port forwarder if kernel is running
        if (m_running.load()) {
            port_forwarder->start();
        }
    }
    
    std::cout << "Port created: " << port_number << " -> " << external_port << " (" << protocol << ")" << std::endl;
    return true;
}

bool HydraKernel::bindProcessToPort(int pid, int port_number) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    // Check if process exists
    if (m_processes.find(pid) == m_processes.end()) {
        return false;
    }
    
    // Check if port exists
    if (m_ports.find(port_number) == m_ports.end()) {
        return false;
    }
    
    // Bind process to port
    m_ports[port_number] = pid;
    
    std::cout << "Port " << port_number << " bound to process " << pid << std::endl;
    return true;
}

bool HydraKernel::connectToPort(int pid, int port_number) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    // Check if process exists
    if (m_processes.find(pid) == m_processes.end()) {
        return false;
    }
    
    // Check if port exists
    if (m_ports.find(port_number) == m_ports.end()) {
        return false;
    }
    
    // Check if port is bound to a process
    int bound_pid = m_ports[port_number];
    if (bound_pid == 0) {
        std::cerr << "Port " << port_number << " is not bound to any process" << std::endl;
        return false;
    }
    
    // TODO: Implement actual connection logic
    
    std::cout << "Process " << pid << " connected to port " << port_number << " (bound to " << bound_pid << ")" << std::endl;
    return true;
}

std::unordered_map<int, int> HydraKernel::listPorts() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_ports;
}

bool HydraKernel::closePort(int port_number) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    // Check if port exists
    auto it = m_ports.find(port_number);
    if (it == m_ports.end()) {
        return false;
    }
    
    // Stop and remove port forwarder if exists
    auto forwarder_it = m_port_forwarders.find(port_number);
    if (forwarder_it != m_port_forwarders.end()) {
        forwarder_it->second->stop();
        m_port_forwarders.erase(forwarder_it);
    }
    
    // Remove port
    m_ports.erase(it);
    
    std::cout << "Port closed: " << port_number << std::endl;
    return true;
}

void HydraKernel::setIsolationMode(int isolation_mode) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    m_isolation_mode = isolation_mode;
    
    // If switching to complete isolation, close all port forwarders
    if (isolation_mode == 2) {
        for (auto& pair : m_port_forwarders) {
            pair.second->stop();
        }
        m_port_forwarders.clear();
    } else {
        // If switching to partial or no isolation, start all port forwarders
        for (auto& pair : m_port_forwarders) {
            pair.second->start();
        }
    }
    
    std::cout << "Isolation mode set to " << isolation_mode << std::endl;
}

int HydraKernel::getIsolationMode() const {
    return m_isolation_mode;
}

bool HydraKernel::start() {
    // Check if already running
    if (m_running.load()) {
        return true;
    }
    
    std::cout << "Starting Hydra Kernel..." << std::endl;
    
    // Set the start time
    m_start_time = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    
    // Start all port forwarders if not in complete isolation mode
    if (m_isolation_mode != 2) {
        for (auto& pair : m_port_forwarders) {
            pair.second->start();
        }
    }
    
    // Start the watchdog thread
    m_running.store(true);
    m_watchdog_thread = std::thread(&HydraKernel::watchdogThread, this);
    
    std::cout << "Hydra Kernel started" << std::endl;
    return true;
}

void HydraKernel::stop() {
    // Check if already stopped
    if (!m_running.load()) {
        return;
    }
    
    std::cout << "Stopping Hydra Kernel..." << std::endl;
    
    // Signal watchdog thread to stop
    m_running.store(false);
    
    // Wait for watchdog thread to complete
    if (m_watchdog_thread.joinable()) {
        m_watchdog_thread.join();
    }
    
    // Stop all port forwarders
    for (auto& pair : m_port_forwarders) {
        pair.second->stop();
    }
    
    // Terminate all processes
    for (auto& pair : m_processes) {
        pair.second->terminate();
    }
    
    std::cout << "Hydra Kernel stopped" << std::endl;
}

bool HydraKernel::isRunning() const {
    return m_running.load();
}

void HydraKernel::watchdogThread() {
    std::cout << "Watchdog thread started" << std::endl;
    
    while (m_running.load()) {
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            
            // Check all processes
            for (auto& pair : m_processes) {
                auto process = pair.second;
                
                // Skip processes that are not running
                if (!process->isRunning()) {
                    continue;
                }
                
                // Update process stats
                auto stats = process->getStats();
                
                // Check if process has exceeded its limits
                if (process->getLimits().memory_limit > 0 && stats.memory_usage > process->getLimits().memory_limit) {
                    std::cerr << "Process " << process->getPID() << " exceeded memory limit, terminating" << std::endl;
                    process->terminate(255);
                }
                
                if (process->getLimits().cpu_limit > 0 && stats.cpu_usage > process->getLimits().cpu_limit) {
                    std::cerr << "Process " << process->getPID() << " exceeded CPU limit, terminating" << std::endl;
                    process->terminate(255);
                }
                
                // TODO: Check other limits
            }
            
            // Clean up terminated processes
            cleanupTerminatedProcesses();
        }
        
        // Sleep for a while
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
    
    std::cout << "Watchdog thread stopped" << std::endl;
}

void HydraKernel::cleanupTerminatedProcesses() {
    // Find terminated processes
    std::vector<int> terminated_pids;
    for (auto& pair : m_processes) {
        auto process = pair.second;
        
        if (process->getState() == ProcessState::TERMINATED) {
            terminated_pids.push_back(process->getPID());
        }
    }
    
    // Clean up terminated processes
    for (int pid : terminated_pids) {
        // Remove from ports
        for (auto it = m_ports.begin(); it != m_ports.end(); ) {
            if (it->second == pid) {
                it->second = 0; // Unbind process from port
            }
            ++it;
        }
        
        // Keep the process in the map for now (zombie state)
        // In a real system, we would clean it up after the parent process has acknowledged termination
    }
}

void HydraKernel::updateStats() {
    // Update all process stats
    for (auto& pair : m_processes) {
        auto process = pair.second;
        
        if (process->isRunning()) {
            auto stats = process->getStats();
            // Do something with the stats if needed
        }
    }
}

// Global kernel instance
static std::unique_ptr<HydraKernel> global_kernel = nullptr;

HydraKernel& getKernel() {
    if (!global_kernel) {
        // Create a new kernel with a memory VFS
        auto vfs = hydra::vfs::create_vfs();
        global_kernel = std::make_unique<HydraKernel>(vfs);
    }
    
    return *global_kernel;
}

} // namespace kernel
} // namespace hydra
