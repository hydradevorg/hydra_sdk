#include <hydra_kernel/process.h>
#include <hydra_common/utils/string_utils.hpp>
#include <chrono>
#include <iostream>
#include <sstream>
#include <cstring>
#include <sys/types.h>

namespace hydra {
namespace kernel {

// Static member initialization
int Process::next_pid = 1;
std::mutex Process::pid_mutex;

Process::Process(const std::string& name, std::shared_ptr<hydra::vfs::IVirtualFileSystem> fs, int parent_pid)
    : m_name(name),
      m_parent_pid(parent_pid),
      m_state(ProcessState::CREATED),
      m_exit_code(-1),
      m_fs(fs),
      m_working_directory("/") {
    // Generate a unique process ID
    std::lock_guard<std::mutex> lock(pid_mutex);
    m_pid = next_pid++;
    
    // Initialize stats
    m_stats.start_time = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    
    // Set default environment variables
    m_environment["PATH"] = "/bin:/usr/bin";
    m_environment["HOME"] = "/home";
    m_environment["USER"] = "hydra";
    m_environment["PWD"] = m_working_directory;
    m_environment["HOSTNAME"] = "hydra-container";
    
    std::cout << "Process created: " << m_name << " (PID: " << m_pid << ")" << std::endl;
}

Process::~Process() {
    // Ensure process is terminated
    if (isRunning()) {
        terminate();
    }
    
    // Wait for thread to join
    if (m_thread.joinable()) {
        m_thread.join();
    }
    
    std::cout << "Process destroyed: " << m_name << " (PID: " << m_pid << ")" << std::endl;
}

int Process::getPID() const {
    return m_pid;
}

int Process::getParentPID() const {
    return m_parent_pid;
}

std::string Process::getName() const {
    return m_name;
}

ProcessState Process::getState() const {
    return m_state.load();
}

int Process::getExitCode() const {
    if (m_state.load() == ProcessState::TERMINATED || m_state.load() == ProcessState::ZOMBIE) {
        return m_exit_code.load();
    }
    return -1;
}

int Process::execute(const std::string& command, const std::vector<std::string>& args) {
    // Check if process is already running
    if (isRunning()) {
        std::cerr << "Process is already running: " << m_name << " (PID: " << m_pid << ")" << std::endl;
        return -1;
    }
    
    // Reset state
    m_state.store(ProcessState::CREATED);
    m_exit_code.store(-1);
    
    // Create thread to run the command
    m_thread = std::thread([this, command, args]() {
        m_state.store(ProcessState::RUNNING);
        
        // Execute the command
        int result = runCommand(command, args);
        
        // Update the exit code
        m_exit_code.store(result);
        
        // Update the state
        m_state.store(ProcessState::TERMINATED);
        
        // Notify any waiting threads
        m_condition.notify_all();
    });
    
    return 0;
}

int Process::executeFunction(std::function<int()> func) {
    // Check if process is already running
    if (isRunning()) {
        std::cerr << "Process is already running: " << m_name << " (PID: " << m_pid << ")" << std::endl;
        return -1;
    }
    
    // Reset state
    m_state.store(ProcessState::CREATED);
    m_exit_code.store(-1);
    
    // Create thread to run the function
    m_thread = std::thread([this, func]() {
        m_state.store(ProcessState::RUNNING);
        
        // Execute the function
        int result = func();
        
        // Update the exit code
        m_exit_code.store(result);
        
        // Update the state
        m_state.store(ProcessState::TERMINATED);
        
        // Notify any waiting threads
        m_condition.notify_all();
    });
    
    return 0;
}

void Process::terminate(int exit_code) {
    // Check if process is running
    if (!isRunning()) {
        return;
    }
    
    // Update the exit code
    m_exit_code.store(exit_code);
    
    // Update the state
    m_state.store(ProcessState::TERMINATED);
    
    // Notify any waiting threads
    m_condition.notify_all();
    
    // TODO: Implement proper thread termination
    
    std::cout << "Process terminated: " << m_name << " (PID: " << m_pid << ")" << std::endl;
}

bool Process::isRunning() const {
    ProcessState state = m_state.load();
    return state == ProcessState::RUNNING || state == ProcessState::BLOCKED;
}

bool Process::wait(uint64_t timeout_ms) {
    std::unique_lock<std::mutex> lock(m_mutex);
    
    if (!isRunning()) {
        return true; // Process already terminated
    }
    
    if (timeout_ms == 0) {
        // Wait indefinitely
        m_condition.wait(lock, [this] {
            return !isRunning();
        });
        return true;
    } else {
        // Wait with timeout
        auto result = m_condition.wait_for(lock, std::chrono::milliseconds(timeout_ms), [this] {
            return !isRunning();
        });
        return result;
    }
}

std::shared_ptr<hydra::vfs::IVirtualFileSystem> Process::getFileSystem() const {
    return m_fs;
}

void Process::setLimits(const ProcessLimits& limits) {
    m_limits = limits;
}

ProcessLimits Process::getLimits() const {
    return m_limits;
}

ProcessStats Process::getStats() const {
    // Update stats if the process is running
    if (isRunning()) {
        const_cast<Process*>(this)->updateStats();
    }
    
    return m_stats;
}

bool Process::redirectStdout(const std::string& file_path) {
    m_stdout_path = file_path;
    return true;
}

bool Process::redirectStdin(const std::string& file_path) {
    m_stdin_path = file_path;
    return true;
}

bool Process::redirectStderr(const std::string& file_path) {
    m_stderr_path = file_path;
    return true;
}

std::unordered_map<std::string, std::string> Process::getEnvironment() const {
    return m_environment;
}

void Process::setEnvironment(const std::unordered_map<std::string, std::string>& env) {
    m_environment = env;
}

void Process::setEnvironmentVariable(const std::string& name, const std::string& value) {
    m_environment[name] = value;
}

std::string Process::getWorkingDirectory() const {
    return m_working_directory;
}

bool Process::setWorkingDirectory(const std::string& path) {
    // Check if the directory exists
    auto result = m_fs->directory_exists(path);
    if (!result.success() || !result.value()) {
        return false;
    }
    
    m_working_directory = path;
    m_environment["PWD"] = path; // Update environment variable
    return true;
}

void Process::updateStats() {
    // This is a simple implementation for demonstration purposes
    // In a real system, we would track actual resource usage
    
    // Update CPU time
    m_stats.total_cpu_time += 100; // Incremental update
    
    // Update memory usage (simple simulation)
    m_stats.memory_usage = 1024 * 1024; // Constant 1MB for demo
    
    // Update I/O operations
    m_stats.iops = 10; // Constant 10 IOPS for demo
    
    // Update thread count
    m_stats.threads_count = 1; // Single thread for now
    
    // Update open files
    m_stats.open_files = 3; // stdin, stdout, stderr
    
    // Calculate CPU usage (0-1)
    uint64_t now = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    uint64_t uptime = now - m_stats.start_time;
    if (uptime > 0) {
        m_stats.cpu_usage = static_cast<float>(m_stats.total_cpu_time) / (uptime * 1000000.0f);
        m_stats.cpu_usage = std::min(m_stats.cpu_usage, 1.0f); // Cap at 100%
    }
}

bool Process::checkLimits() {
    // Check memory limit
    if (m_limits.memory_limit > 0 && m_stats.memory_usage > m_limits.memory_limit) {
        std::cerr << "Process exceeded memory limit: " << m_name << " (PID: " << m_pid << ")" << std::endl;
        terminate(255); // Out of memory
        return false;
    }
    
    // Check CPU limit
    if (m_limits.cpu_limit > 0 && m_stats.cpu_usage > m_limits.cpu_limit) {
        std::cerr << "Process exceeded CPU limit: " << m_name << " (PID: " << m_pid << ")" << std::endl;
        terminate(255); // CPU limit exceeded
        return false;
    }
    
    // Check IOPS limit
    if (m_limits.iops_limit > 0 && m_stats.iops > m_limits.iops_limit) {
        std::cerr << "Process exceeded IOPS limit: " << m_name << " (PID: " << m_pid << ")" << std::endl;
        terminate(255); // IOPS limit exceeded
        return false;
    }
    
    // Check file descriptors limit
    if (m_limits.file_descriptors > 0 && m_stats.open_files > m_limits.file_descriptors) {
        std::cerr << "Process exceeded file descriptors limit: " << m_name << " (PID: " << m_pid << ")" << std::endl;
        terminate(255); // File descriptors limit exceeded
        return false;
    }
    
    // Check threads limit
    if (m_limits.threads > 0 && m_stats.threads_count > m_limits.threads) {
        std::cerr << "Process exceeded threads limit: " << m_name << " (PID: " << m_pid << ")" << std::endl;
        terminate(255); // Threads limit exceeded
        return false;
    }
    
    return true;
}

int Process::runCommand(const std::string& command, const std::vector<std::string>& args) {
    std::cout << "Executing command: " << command;
    for (const auto& arg : args) {
        std::cout << " " << arg;
    }
    std::cout << std::endl;
    
    // Check if the command exists
    std::string cmd_path = command;
    if (!command.find("/")) {
        // Search in PATH
        bool found = false;
        for (const auto& path : hydra::common::split(m_environment["PATH"], ":")) {
            std::string full_path = m_fs->join_paths(path, command);
            auto result = m_fs->file_exists(full_path);
            if (result.success() && result.value()) {
                cmd_path = full_path;
                found = true;
                break;
            }
        }
        
        if (!found) {
            std::cerr << "Command not found: " << command << std::endl;
            return 127; // Command not found
        }
    } else {
        // Check if absolute path exists
        auto result = m_fs->file_exists(cmd_path);
        if (!result.success() || !result.value()) {
            std::cerr << "Command not found: " << cmd_path << std::endl;
            return 127; // Command not found
        }
    }
    
    // For demonstration purposes, we'll just print the command
    // In a real implementation, we would execute the command
    
    // Simulate command execution
    std::cout << "Executing " << cmd_path << " in " << m_working_directory << std::endl;
    
    // Sleep to simulate work
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    // Return success
    return 0;
}

} // namespace kernel
} // namespace hydra
