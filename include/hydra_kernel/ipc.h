#ifndef HYDRA_KERNEL_IPC_H
#define HYDRA_KERNEL_IPC_H

#include <string>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <optional>
#include <unordered_map>
#include <memory>
#include <chrono>

namespace hydra {
namespace kernel {

/**
 * @brief Inter-process message queue
 */
class MessageQueue {
public:
    /**
     * @brief Constructor
     * 
     * @param name Queue name
     */
    explicit MessageQueue(const std::string& name);
    
    /**
     * @brief Destructor
     */
    ~MessageQueue();
    
    /**
     * @brief Send a message to the queue
     * 
     * @param message Message data
     * @return True if message was sent successfully
     */
    bool send(const std::vector<uint8_t>& message);
    
    /**
     * @brief Receive a message from the queue
     * 
     * @param blocking Whether to block until a message is available
     * @param timeout_ms Timeout in milliseconds (for blocking mode, 0 = wait indefinitely)
     * @return Optional containing the message if available, empty optional otherwise
     */
    std::optional<std::vector<uint8_t>> receive(bool blocking = false, uint64_t timeout_ms = 0);
    
    /**
     * @brief Get the number of messages in the queue
     */
    size_t getMessageCount() const;
    
    /**
     * @brief Get the queue name
     */
    std::string getName() const;
    
    /**
     * @brief Clear the queue
     */
    void clear();
    
    /**
     * @brief Check if the queue has messages
     */
    bool hasMessages() const;

private:
    std::string m_name;
    std::queue<std::vector<uint8_t>> m_queue;
    mutable std::mutex m_mutex;
    std::condition_variable m_condition;
};

/**
 * @brief Shared memory segment
 */
class SharedMemory {
public:
    /**
     * @brief Constructor
     * 
     * @param name Segment name
     * @param size Segment size in bytes
     */
    SharedMemory(const std::string& name, size_t size);
    
    /**
     * @brief Destructor
     */
    ~SharedMemory();
    
    /**
     * @brief Get a pointer to the shared memory
     */
    uint8_t* getData();
    
    /**
     * @brief Get a const pointer to the shared memory
     */
    const uint8_t* getData() const;
    
    /**
     * @brief Get the size of the shared memory
     */
    size_t getSize() const;
    
    /**
     * @brief Get the segment name
     */
    std::string getName() const;
    
    /**
     * @brief Lock the shared memory for exclusive access
     * 
     * @param timeout_ms Timeout in milliseconds (0 = wait indefinitely)
     * @return True if locked successfully, false on timeout
     */
    bool lock(uint64_t timeout_ms = 0);
    
    /**
     * @brief Try to lock the shared memory without blocking
     * 
     * @return True if locked successfully, false if already locked
     */
    bool tryLock();
    
    /**
     * @brief Unlock the shared memory
     */
    void unlock();

private:
    std::string m_name;
    std::vector<uint8_t> m_data;
    size_t m_size;
    mutable std::mutex m_mutex;
    std::condition_variable m_condition;
    bool m_locked;
};

/**
 * @brief Shared semaphore for synchronization
 */
class Semaphore {
public:
    /**
     * @brief Constructor
     * 
     * @param name Semaphore name
     * @param initial_value Initial value
     */
    Semaphore(const std::string& name, int initial_value = 1);
    
    /**
     * @brief Destructor
     */
    ~Semaphore();
    
    /**
     * @brief Wait on the semaphore
     * 
     * @param timeout_ms Timeout in milliseconds (0 = wait indefinitely)
     * @return True if wait was successful, false on timeout
     */
    bool wait(uint64_t timeout_ms = 0);
    
    /**
     * @brief Try to wait on the semaphore without blocking
     * 
     * @return True if wait was successful, false if semaphore is at 0
     */
    bool tryWait();
    
    /**
     * @brief Signal the semaphore
     */
    void signal();
    
    /**
     * @brief Get the semaphore name
     */
    std::string getName() const;
    
    /**
     * @brief Get the current value
     */
    int getValue() const;

private:
    std::string m_name;
    int m_value;
    mutable std::mutex m_mutex;
    std::condition_variable m_condition;
};

/**
 * @brief IPC manager for creating and managing IPC objects
 */
class IPCManager {
public:
    /**
     * @brief Get or create a message queue
     * 
     * @param name Queue name
     * @return Shared pointer to the message queue
     */
    static std::shared_ptr<MessageQueue> getMessageQueue(const std::string& name);
    
    /**
     * @brief Get or create a shared memory segment
     * 
     * @param name Segment name
     * @param size Segment size in bytes
     * @return Shared pointer to the shared memory
     */
    static std::shared_ptr<SharedMemory> getSharedMemory(const std::string& name, size_t size);
    
    /**
     * @brief Get or create a semaphore
     * 
     * @param name Semaphore name
     * @param initial_value Initial value
     * @return Shared pointer to the semaphore
     */
    static std::shared_ptr<Semaphore> getSemaphore(const std::string& name, int initial_value = 1);
    
    /**
     * @brief Delete a message queue
     * 
     * @param name Queue name
     * @return True if deleted successfully
     */
    static bool deleteMessageQueue(const std::string& name);
    
    /**
     * @brief Delete a shared memory segment
     * 
     * @param name Segment name
     * @return True if deleted successfully
     */
    static bool deleteSharedMemory(const std::string& name);
    
    /**
     * @brief Delete a semaphore
     * 
     * @param name Semaphore name
     * @return True if deleted successfully
     */
    static bool deleteSemaphore(const std::string& name);
    
    /**
     * @brief List all message queues
     * 
     * @return Vector of queue names
     */
    static std::vector<std::string> listMessageQueues();
    
    /**
     * @brief List all shared memory segments
     * 
     * @return Vector of segment names
     */
    static std::vector<std::string> listSharedMemory();
    
    /**
     * @brief List all semaphores
     * 
     * @return Vector of semaphore names
     */
    static std::vector<std::string> listSemaphores();
    
    /**
     * @brief Clear all IPC objects
     */
    static void clearAll();

private:
    static std::unordered_map<std::string, std::shared_ptr<MessageQueue>> s_message_queues;
    static std::unordered_map<std::string, std::shared_ptr<SharedMemory>> s_shared_memory;
    static std::unordered_map<std::string, std::shared_ptr<Semaphore>> s_semaphores;
    static std::mutex s_mutex;
};

} // namespace kernel
} // namespace hydra

#endif // HYDRA_KERNEL_IPC_H
