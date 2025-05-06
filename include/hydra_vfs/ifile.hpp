#pragma once

#include "hydra_vfs/result.hpp"
#include <cstdint>
#include <cstddef>

namespace hydra {
namespace vfs {

/**
 * @brief Interface for file operations
 * 
 * This interface defines the operations that can be performed on a file.
 */
class IFile {
public:
    /**
     * @brief Virtual destructor
     */
    virtual ~IFile() = default;
    
    /**
     * @brief Read data from the file
     * 
     * @param buffer Buffer to read into
     * @param size Number of bytes to read
     * @return Result<size_t> Number of bytes read or an error
     */
    virtual Result<size_t> read(uint8_t* buffer, size_t size) = 0;
    
    /**
     * @brief Write data to the file
     * 
     * @param buffer Buffer to write from
     * @param size Number of bytes to write
     * @return Result<size_t> Number of bytes written or an error
     */
    virtual Result<size_t> write(const uint8_t* buffer, size_t size) = 0;
    
    /**
     * @brief Seek to a position in the file
     * 
     * @param position Position to seek to
     * @return Result<size_t> New position or an error
     */
    virtual Result<size_t> seek(size_t position) = 0;
    
    /**
     * @brief Get the current position in the file
     * 
     * @return Result<size_t> Current position or an error
     */
    virtual Result<size_t> tell() = 0;
    
    /**
     * @brief Flush any buffered data to storage
     * 
     * @return Result<bool> True if successful, false otherwise
     */
    virtual Result<bool> flush() = 0;
    
    /**
     * @brief Close the file
     * 
     * @return Result<bool> True if successful, false otherwise
     */
    virtual Result<bool> close() = 0;
};

} // namespace vfs
} // namespace hydra
