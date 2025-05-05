#ifndef HYDRA_KERNEL_PROCESS_H
#define HYDRA_KERNEL_PROCESS_H

#include <string>
#include <vector>
#include <memory>
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <functional>
#include <unordered_map>
#include <hydra_vfs/vfs.h>

namespace hydra {
namespace kernel {

/**
 * @brief Process states
 */
enum class ProcessState {
    CREATED,    // Process created but not started
    RUNNING,    // Process is running
    BLOCKED,    // Process is blocked (waiting for I/O, etc.)
    TERMINATED, // Process has terminated
    ZOMBIE      // Process has terminated but not cleaned up
};

/**
 * @brief Process resource limits
 */
struct ProcessLimits {
    size_t memory_limit = 0;       // Memory limit in bytes (0 = no limit)
    float cpu_limit = 0.0f;        // CPU usage limit as percentage (0.0 - 1.0, 0 = no limit)
    size_t iops_limit = 0;         // I/O operations per second limit (0 = no limit)
    size_t file_descriptors = 0;   // Maximum number of open file descriptors (0 = no limit)
    size_t threads = 0;            // Maximum number of threads (0 = no limit)
};

/**
 * @brief Process statistics
 */
struct ProcessStats {
    size_t memory_usage = 0;       // Current memory usage in bytes
    float cpu_usage = 0.0f;        // Current CPU usage as percentage (0.0 - 1.0)
    size_t iops = 0;               // Current I/O operations per second
    size_t open_files = 0;         // Number of open files
    size_t threads_count = 0;      // Number of threads
    uint64_t total_cpu_time = 0;   // Total CPU time in microseconds
    uint64_t start_time = 0;       // Process start time (Unix timestamp)
    uint64_t end_time = 0;         // Process end time (Unix timestamp, 0 if still running)
};

/**
 * @brief Virtual process in the Hydra Kernel
 */
class Process {
public:
    /**
     * @brief Create a new process
     * 
     * @param name Process name
     * @param fs Virtual file system for the process
     * @param parent_pid Parent process ID (0 for root process)
     */
    Process(const std::string& name, std::shared_ptr<hydra::vfs::IVirtualFileSystem> fs, int parent_pid = 0);
    
    /**
     * @brief Destructor
     */
    ~Process();
    
    // Process cannot be copied
    Process(const Process&) = delete;
    Process& operator=(const Process&) = delete;
    
    /**
     * @brief Get the process ID
     */
    int getPID() const;
    
    /**
     * @brief Get the parent process ID
     */
    int getParentPID() const;
    
    /**
     * @brief Get the process name
     */
    std::string getName() const;
    
    /**
     * @brief Get the process state
     */
    ProcessState getState() const;
    
    /**
     * @brief Get the process exit code
     * 
     * @return Exit code if process has terminated, -1 otherwise
     */
    int getExitCode() const;
    
    /**
     * @brief Execute a command in this process
     * 
     * @param command Command to execute
     * @param args Command arguments
     * @return Exit code of the command
     */
    int execute(const std::string& command, const std::vector<std::string>& args = {});
    
    /**
     * @brief Execute a function in this process
     * 
     * @param func Function to execute
     * @return Exit code returned by the function
     */
    int executeFunction(std::function<int()> func);
    
    /**
     * @brief Terminate the process
     * 
     * @param exit_code Exit code to set
     */
    void terminate(int exit_code = 1);
    
    /**
     * @brief Check if the process is running
     */
    bool isRunning() const;
    
    /**
     * @brief Wait for the process to terminate
     * 
     * @param timeout_ms Timeout in milliseconds (0 = wait indefinitely)
     * @return True if process terminated, false if timeout
     */
    bool wait(uint64_t timeout_ms = 0);
    
    /**
     * @brief Get the process's virtual file system
     */
    std::shared_ptr<hydra::vfs::IVirtualFileSystem> getFileSystem() const;
    
    /**
     * @brief Set resource limits for this process
     */
    void setLimits(const ProcessLimits& limits);
    
    /**
     * @brief Get the current resource limits
     */
    ProcessLimits getLimits() const;
    
    /**
     * @brief Get the current process statistics
     */
    ProcessStats getStats() const;
    
    /**
     * @brief Redirect standard output to a file
     * 
     * @param file_path Path to the file
     * @return True if successful
     */
    bool redirectStdout(const std::string& file_path);
    
    /**
     * @brief Redirect standard input from a file
     * 
     * @param file_path Path to the file
     * @return True if successful
     */
    bool redirectStdin(const std::string& file_path);
    
    /**
     * @brief Redirect standard error to a file
     * 
     * @param file_path Path to the file
     * @return True if successful
     */
    bool redirectStderr(const std::string& file_path);
    
    /**
     * @brief Get environment variables
     */
    std::unordered_map<std::string, std::string> getEnvironment() const;
    
    /**
     * @brief Set environment variables
     */
    void setEnvironment(const std::unordered_map<std::string, std::string>& env);
    
    /**
     * @brief Set a single environment variable
     */
    void setEnvironmentVariable(const std::string& name, const std::string& value);
    
    /**
     * @brief Get the current working directory
     */
    std::string getWorkingDirectory() const;
    
    /**
     * @brief Set the current working directory
     */
    bool setWorkingDirectory(const std::string& path);

private:
    std::string m_name;
    int m_pid;
    int m_parent_pid;
    std::atomic<ProcessState> m_state;
    std::atomic<int> m_exit_code;
    std::shared_ptr<hydra::vfs::IVirtualFileSystem> m_fs;
    ProcessLimits m_limits;
    ProcessStats m_stats;
    std::unordered_map<std::string, std::string> m_environment;
    std::string m_working_directory;
    
    // I/O redirection
    std::string m_stdout_path;
    std::string m_stdin_path;
    std::string m_stderr_path;
    
    // Thread management
    std::thread m_thread;
    std::mutex m_mutex;
    std::condition_variable m_condition;
    
    // Process management
    static int next_pid;
    static std::mutex pid_mutex;
    
    // Internal methods
    void updateStats();
    bool checkLimits();
    int runCommand(const std::string& command, const std::vector<std::string>& args);
};

} // namespace kernel
} // namespace hydra

#endif // HYDRA_KERNEL_PROCESS_H
