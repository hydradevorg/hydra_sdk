#pragma once

#include <string>
#include <stdexcept>
#include <system_error>
#include <memory>
#include <functional>
#include <iostream>
#include <sstream>

namespace hydra {
namespace common {

/**
 * @brief Error severity levels
 */
enum class ErrorSeverity {
    INFO,       ///< Informational message, not an error
    WARNING,    ///< Warning, operation can continue
    ERROR,      ///< Error, operation failed but system can recover
    CRITICAL    ///< Critical error, system may be in an unstable state
};

/**
 * @brief Error categories for classification
 */
enum class ErrorCategory {
    MEMORY,         ///< Memory allocation or management errors
    IO,             ///< Input/output errors
    CRYPTO,         ///< Cryptographic errors
    NETWORK,        ///< Network-related errors
    PERMISSION,     ///< Permission or access control errors
    VALIDATION,     ///< Data validation errors
    RESOURCE,       ///< Resource exhaustion errors
    INTERNAL,       ///< Internal logic errors
    EXTERNAL,       ///< Errors from external dependencies
    UNKNOWN         ///< Unknown or unclassified errors
};

/**
 * @brief Exception class for Hydra SDK errors
 */
class HydraException : public std::runtime_error {
public:
    /**
     * @brief Constructor with message
     * @param message Error message
     * @param severity Error severity
     * @param category Error category
     */
    HydraException(
        const std::string& message,
        ErrorSeverity severity = ErrorSeverity::ERROR,
        ErrorCategory category = ErrorCategory::UNKNOWN
    ) : std::runtime_error(message),
        m_severity(severity),
        m_category(category) {}
    
    /**
     * @brief Get the error severity
     * @return Error severity
     */
    ErrorSeverity getSeverity() const { return m_severity; }
    
    /**
     * @brief Get the error category
     * @return Error category
     */
    ErrorCategory getCategory() const { return m_category; }
    
private:
    ErrorSeverity m_severity;
    ErrorCategory m_category;
};

/**
 * @brief Memory allocation exception
 */
class MemoryAllocationException : public HydraException {
public:
    /**
     * @brief Constructor
     * @param message Error message
     * @param size Size of the allocation that failed
     */
    MemoryAllocationException(const std::string& message, size_t size)
        : HydraException(message, ErrorSeverity::CRITICAL, ErrorCategory::MEMORY),
          m_size(size) {}
    
    /**
     * @brief Get the size of the allocation that failed
     * @return Allocation size
     */
    size_t getAllocationSize() const { return m_size; }
    
private:
    size_t m_size;
};

/**
 * @brief Resource exhaustion exception
 */
class ResourceExhaustionException : public HydraException {
public:
    /**
     * @brief Constructor
     * @param message Error message
     * @param resource_name Name of the exhausted resource
     */
    ResourceExhaustionException(const std::string& message, const std::string& resource_name)
        : HydraException(message, ErrorSeverity::ERROR, ErrorCategory::RESOURCE),
          m_resource_name(resource_name) {}
    
    /**
     * @brief Get the name of the exhausted resource
     * @return Resource name
     */
    std::string getResourceName() const { return m_resource_name; }
    
private:
    std::string m_resource_name;
};

/**
 * @brief Error handler interface
 */
class IErrorHandler {
public:
    virtual ~IErrorHandler() = default;
    
    /**
     * @brief Handle an error
     * @param message Error message
     * @param severity Error severity
     * @param category Error category
     */
    virtual void handleError(
        const std::string& message,
        ErrorSeverity severity,
        ErrorCategory category
    ) = 0;
    
    /**
     * @brief Handle an exception
     * @param e Exception to handle
     */
    virtual void handleException(const std::exception& e) = 0;
};

/**
 * @brief Default error handler implementation
 */
class DefaultErrorHandler : public IErrorHandler {
public:
    /**
     * @brief Handle an error
     * @param message Error message
     * @param severity Error severity
     * @param category Error category
     */
    void handleError(
        const std::string& message,
        ErrorSeverity severity,
        ErrorCategory category
    ) override {
        std::stringstream ss;
        ss << "[" << severityToString(severity) << "] "
           << "[" << categoryToString(category) << "] "
           << message;
        
        // Log the error
        std::cerr << ss.str() << std::endl;
        
        // For critical errors, throw an exception
        if (severity == ErrorSeverity::CRITICAL) {
            throw HydraException(message, severity, category);
        }
    }
    
    /**
     * @brief Handle an exception
     * @param e Exception to handle
     */
    void handleException(const std::exception& e) override {
        std::cerr << "Exception: " << e.what() << std::endl;
        
        // Check if it's a HydraException
        const HydraException* hydra_ex = dynamic_cast<const HydraException*>(&e);
        if (hydra_ex) {
            std::cerr << "Severity: " << severityToString(hydra_ex->getSeverity()) << std::endl;
            std::cerr << "Category: " << categoryToString(hydra_ex->getCategory()) << std::endl;
        }
    }
    
private:
    /**
     * @brief Convert severity to string
     * @param severity Error severity
     * @return String representation
     */
    std::string severityToString(ErrorSeverity severity) {
        switch (severity) {
            case ErrorSeverity::INFO: return "INFO";
            case ErrorSeverity::WARNING: return "WARNING";
            case ErrorSeverity::ERROR: return "ERROR";
            case ErrorSeverity::CRITICAL: return "CRITICAL";
            default: return "UNKNOWN";
        }
    }
    
    /**
     * @brief Convert category to string
     * @param category Error category
     * @return String representation
     */
    std::string categoryToString(ErrorCategory category) {
        switch (category) {
            case ErrorCategory::MEMORY: return "MEMORY";
            case ErrorCategory::IO: return "IO";
            case ErrorCategory::CRYPTO: return "CRYPTO";
            case ErrorCategory::NETWORK: return "NETWORK";
            case ErrorCategory::PERMISSION: return "PERMISSION";
            case ErrorCategory::VALIDATION: return "VALIDATION";
            case ErrorCategory::RESOURCE: return "RESOURCE";
            case ErrorCategory::INTERNAL: return "INTERNAL";
            case ErrorCategory::EXTERNAL: return "EXTERNAL";
            case ErrorCategory::UNKNOWN: return "UNKNOWN";
            default: return "UNKNOWN";
        }
    }
};

/**
 * @brief Error handling utility
 */
class ErrorHandling {
public:
    /**
     * @brief Get the global error handler
     * @return Error handler
     */
    static std::shared_ptr<IErrorHandler> getErrorHandler() {
        if (!s_error_handler) {
            s_error_handler = std::make_shared<DefaultErrorHandler>();
        }
        return s_error_handler;
    }
    
    /**
     * @brief Set the global error handler
     * @param handler Error handler
     */
    static void setErrorHandler(std::shared_ptr<IErrorHandler> handler) {
        s_error_handler = handler;
    }
    
    /**
     * @brief Report an error
     * @param message Error message
     * @param severity Error severity
     * @param category Error category
     */
    static void reportError(
        const std::string& message,
        ErrorSeverity severity = ErrorSeverity::ERROR,
        ErrorCategory category = ErrorCategory::UNKNOWN
    ) {
        getErrorHandler()->handleError(message, severity, category);
    }
    
    /**
     * @brief Report an exception
     * @param e Exception to report
     */
    static void reportException(const std::exception& e) {
        getErrorHandler()->handleException(e);
    }
    
    /**
     * @brief Execute a function with error handling
     * @param func Function to execute
     * @param error_message Error message if the function throws
     * @return True if the function executed successfully
     */
    template<typename Func>
    static bool tryExecute(Func func, const std::string& error_message) {
        try {
            func();
            return true;
        } catch (const std::exception& e) {
            std::stringstream ss;
            ss << error_message << ": " << e.what();
            reportError(ss.str());
            return false;
        } catch (...) {
            reportError(error_message + ": Unknown exception");
            return false;
        }
    }
    
private:
    static std::shared_ptr<IErrorHandler> s_error_handler;
};

// Initialize static member
std::shared_ptr<IErrorHandler> ErrorHandling::s_error_handler = nullptr;

/**
 * @brief Safe memory allocation function
 * @param size Size to allocate
 * @return Allocated memory
 * @throws MemoryAllocationException if allocation fails
 */
template<typename T>
T* safeAlloc(size_t count) {
    try {
        T* ptr = new T[count];
        if (!ptr) {
            throw MemoryAllocationException("Memory allocation failed", count * sizeof(T));
        }
        return ptr;
    } catch (const std::bad_alloc& e) {
        throw MemoryAllocationException("Memory allocation failed: " + std::string(e.what()), 
                                       count * sizeof(T));
    }
}

/**
 * @brief Safe memory deallocation function
 * @param ptr Pointer to deallocate
 */
template<typename T>
void safeFree(T* ptr) {
    if (ptr) {
        delete[] ptr;
    }
}

/**
 * @brief Safe memory allocation with custom deleter for smart pointers
 * @param size Size to allocate
 * @return Unique pointer with allocated memory
 */
template<typename T>
std::unique_ptr<T[], std::function<void(T*)>> safeAllocUnique(size_t count) {
    return std::unique_ptr<T[], std::function<void(T*)>>(
        safeAlloc<T>(count),
        [](T* ptr) { safeFree(ptr); }
    );
}

} // namespace common
} // namespace hydra
