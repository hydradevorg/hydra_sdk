#pragma once

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <mutex>
#include <memory>
#include <vector>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <functional>
#include <unordered_map>
#include <atomic>
#include <thread>

namespace hydra {
namespace common {

/**
 * @brief Log level enumeration
 */
enum class LogLevel {
    TRACE,      ///< Trace level (most verbose)
    DEBUG,      ///< Debug level
    INFO,       ///< Info level
    WARNING,    ///< Warning level
    ERROR,      ///< Error level
    CRITICAL,   ///< Critical level
    NONE        ///< No logging
};

/**
 * @brief Convert log level to string
 * @param level Log level
 * @return String representation
 */
inline std::string logLevelToString(LogLevel level) {
    switch (level) {
        case LogLevel::TRACE: return "TRACE";
        case LogLevel::DEBUG: return "DEBUG";
        case LogLevel::INFO: return "INFO";
        case LogLevel::WARNING: return "WARNING";
        case LogLevel::ERROR: return "ERROR";
        case LogLevel::CRITICAL: return "CRITICAL";
        case LogLevel::NONE: return "NONE";
        default: return "UNKNOWN";
    }
}

/**
 * @brief Convert string to log level
 * @param level String representation
 * @return Log level
 */
inline LogLevel stringToLogLevel(const std::string& level) {
    if (level == "TRACE") return LogLevel::TRACE;
    if (level == "DEBUG") return LogLevel::DEBUG;
    if (level == "INFO") return LogLevel::INFO;
    if (level == "WARNING") return LogLevel::WARNING;
    if (level == "ERROR") return LogLevel::ERROR;
    if (level == "CRITICAL") return LogLevel::CRITICAL;
    if (level == "NONE") return LogLevel::NONE;
    return LogLevel::INFO; // Default
}

/**
 * @brief Log message structure
 */
struct LogMessage {
    std::string module;                         ///< Module name
    LogLevel level;                             ///< Log level
    std::string message;                        ///< Log message
    std::chrono::system_clock::time_point time; ///< Timestamp
    std::thread::id thread_id;                  ///< Thread ID
    
    /**
     * @brief Constructor
     * @param module Module name
     * @param level Log level
     * @param message Log message
     */
    LogMessage(
        const std::string& module,
        LogLevel level,
        const std::string& message
    ) : module(module),
        level(level),
        message(message),
        time(std::chrono::system_clock::now()),
        thread_id(std::this_thread::get_id()) {}
};

/**
 * @brief Log sink interface
 */
class ILogSink {
public:
    /**
     * @brief Destructor
     */
    virtual ~ILogSink() = default;
    
    /**
     * @brief Write log message
     * @param message Log message
     */
    virtual void write(const LogMessage& message) = 0;
    
    /**
     * @brief Flush the sink
     */
    virtual void flush() = 0;
};

/**
 * @brief Console log sink
 */
class ConsoleLogSink : public ILogSink {
public:
    /**
     * @brief Constructor
     * @param use_colors Whether to use colors
     */
    explicit ConsoleLogSink(bool use_colors = true) : m_use_colors(use_colors) {}
    
    /**
     * @brief Write log message
     * @param message Log message
     */
    void write(const LogMessage& message) override {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        // Format timestamp
        auto time_t = std::chrono::system_clock::to_time_t(message.time);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            message.time.time_since_epoch()
        ).count() % 1000;
        
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
        ss << '.' << std::setfill('0') << std::setw(3) << ms;
        
        // Format thread ID
        std::stringstream thread_ss;
        thread_ss << message.thread_id;
        
        // Format log message
        std::stringstream log_ss;
        log_ss << "[" << ss.str() << "] "
               << "[" << thread_ss.str() << "] "
               << "[" << message.module << "] "
               << "[" << logLevelToString(message.level) << "] "
               << message.message;
        
        // Output with color if enabled
        if (m_use_colors) {
            std::string color_code;
            std::string reset_code = "\033[0m";
            
            switch (message.level) {
                case LogLevel::TRACE:
                    color_code = "\033[90m"; // Dark gray
                    break;
                case LogLevel::DEBUG:
                    color_code = "\033[37m"; // White
                    break;
                case LogLevel::INFO:
                    color_code = "\033[32m"; // Green
                    break;
                case LogLevel::WARNING:
                    color_code = "\033[33m"; // Yellow
                    break;
                case LogLevel::ERROR:
                    color_code = "\033[31m"; // Red
                    break;
                case LogLevel::CRITICAL:
                    color_code = "\033[1;31m"; // Bright red
                    break;
                default:
                    color_code = "\033[0m"; // Reset
                    break;
            }
            
            std::cout << color_code << log_ss.str() << reset_code << std::endl;
        } else {
            std::cout << log_ss.str() << std::endl;
        }
    }
    
    /**
     * @brief Flush the sink
     */
    void flush() override {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::cout.flush();
    }
    
private:
    bool m_use_colors;
    std::mutex m_mutex;
};

/**
 * @brief File log sink
 */
class FileLogSink : public ILogSink {
public:
    /**
     * @brief Constructor
     * @param filename Log file name
     * @param append Whether to append to existing file
     */
    explicit FileLogSink(const std::string& filename, bool append = true)
        : m_filename(filename) {
        // Open file
        m_file.open(filename, append ? std::ios::app : std::ios::trunc);
        if (!m_file.is_open()) {
            throw std::runtime_error("Failed to open log file: " + filename);
        }
    }
    
    /**
     * @brief Destructor
     */
    ~FileLogSink() override {
        if (m_file.is_open()) {
            m_file.close();
        }
    }
    
    /**
     * @brief Write log message
     * @param message Log message
     */
    void write(const LogMessage& message) override {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        if (!m_file.is_open()) {
            return;
        }
        
        // Format timestamp
        auto time_t = std::chrono::system_clock::to_time_t(message.time);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            message.time.time_since_epoch()
        ).count() % 1000;
        
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
        ss << '.' << std::setfill('0') << std::setw(3) << ms;
        
        // Format thread ID
        std::stringstream thread_ss;
        thread_ss << message.thread_id;
        
        // Format log message
        m_file << "[" << ss.str() << "] "
               << "[" << thread_ss.str() << "] "
               << "[" << message.module << "] "
               << "[" << logLevelToString(message.level) << "] "
               << message.message << std::endl;
    }
    
    /**
     * @brief Flush the sink
     */
    void flush() override {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_file.is_open()) {
            m_file.flush();
        }
    }
    
private:
    std::string m_filename;
    std::ofstream m_file;
    std::mutex m_mutex;
};

/**
 * @brief Logger class
 */
class Logger {
public:
    /**
     * @brief Get the singleton instance
     * @return Logger instance
     */
    static Logger& getInstance() {
        static Logger instance;
        return instance;
    }
    
    /**
     * @brief Add a sink
     * @param sink Log sink
     */
    void addSink(std::shared_ptr<ILogSink> sink) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_sinks.push_back(sink);
    }
    
    /**
     * @brief Remove all sinks
     */
    void clearSinks() {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_sinks.clear();
    }
    
    /**
     * @brief Set global log level
     * @param level Log level
     */
    void setLevel(LogLevel level) {
        m_global_level.store(level);
    }
    
    /**
     * @brief Set module log level
     * @param module Module name
     * @param level Log level
     */
    void setModuleLevel(const std::string& module, LogLevel level) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_module_levels[module] = level;
    }
    
    /**
     * @brief Get module log level
     * @param module Module name
     * @return Log level
     */
    LogLevel getModuleLevel(const std::string& module) const {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        auto it = m_module_levels.find(module);
        if (it != m_module_levels.end()) {
            return it->second;
        }
        
        return m_global_level.load();
    }
    
    /**
     * @brief Log a message
     * @param module Module name
     * @param level Log level
     * @param message Log message
     */
    void log(const std::string& module, LogLevel level, const std::string& message) {
        // Check if this level should be logged
        if (level < getModuleLevel(module)) {
            return;
        }
        
        // Create log message
        LogMessage log_message(module, level, message);
        
        // Write to all sinks
        std::lock_guard<std::mutex> lock(m_mutex);
        for (auto& sink : m_sinks) {
            sink->write(log_message);
        }
    }
    
    /**
     * @brief Flush all sinks
     */
    void flush() {
        std::lock_guard<std::mutex> lock(m_mutex);
        for (auto& sink : m_sinks) {
            sink->flush();
        }
    }
    
private:
    /**
     * @brief Constructor
     */
    Logger() : m_global_level(LogLevel::INFO) {
        // Add default console sink
        addSink(std::make_shared<ConsoleLogSink>());
    }
    
    std::vector<std::shared_ptr<ILogSink>> m_sinks;
    std::atomic<LogLevel> m_global_level;
    std::unordered_map<std::string, LogLevel> m_module_levels;
    mutable std::mutex m_mutex;
};

/**
 * @brief Log helper macros
 */
#define HYDRA_LOG_TRACE(module, message) \
    hydra::common::Logger::getInstance().log(module, hydra::common::LogLevel::TRACE, message)

#define HYDRA_LOG_DEBUG(module, message) \
    hydra::common::Logger::getInstance().log(module, hydra::common::LogLevel::DEBUG, message)

#define HYDRA_LOG_INFO(module, message) \
    hydra::common::Logger::getInstance().log(module, hydra::common::LogLevel::INFO, message)

#define HYDRA_LOG_WARNING(module, message) \
    hydra::common::Logger::getInstance().log(module, hydra::common::LogLevel::WARNING, message)

#define HYDRA_LOG_ERROR(module, message) \
    hydra::common::Logger::getInstance().log(module, hydra::common::LogLevel::ERROR, message)

#define HYDRA_LOG_CRITICAL(module, message) \
    hydra::common::Logger::getInstance().log(module, hydra::common::LogLevel::CRITICAL, message)

} // namespace common
} // namespace hydra
